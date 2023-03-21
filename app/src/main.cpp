#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <spdlog/spdlog.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <nlohmann/json-schema.hpp>
#include "calsht_dw.hpp"
#include "player_impl.hpp"
#include "settile.hpp"
#include "getlabel.hpp"
#include "win_prob1.hpp"
#include "win_prob2.hpp"
#define LOG_MESSAGE "{} ({})"
constexpr int HAND_TILE_NUM = 14;
constexpr int MAX_DORA_TILE_NUM = 4;
constexpr int EAST_INDEX = 27;
constexpr int T_MIN = 1;
constexpr int T_MAX = 18;
constexpr int SUM = 123;

class custom_error_handler : public nlohmann::json_schema::basic_error_handler {
  void error(const nlohmann::json::json_pointer&, const nlohmann::json&, const std::string&) override
  {
    throw std::invalid_argument("Validation of schema failed");
  }
};

inline const auto get_request_id(const web::http::http_request request)
{
  return request._get_impl().get();
}

void handle_post(web::http::http_request request);
void handle_error(const pplx::task<void> task, const web::http::http_request request);
void validate(const nlohmann::json& json);
std::pair<std::shared_ptr<player_impl::Player>, win_prob::Params> get_params(const web::json::value& json);
web::json::value get_result(const std::vector<win_prob::Stat>& stats, const std::size_t searched);

nlohmann::json_schema::json_validator validator;
CalshtDW calsht;

int main()
{
  std::ifstream fin(SCHEMA_FILE_PATH);

  if (!fin) {
    std::cerr << "Failed to open the schema file.\n";
    return 1;
  }

  const auto schema = nlohmann::json::parse(fin);

  validator.set_root_schema(schema);

  calsht.initialize(INDEX_FILE_PATH);

  const utility::string_t address = U("http://0.0.0.0:18000");
  const web::uri_builder uri(address);
  web::http::experimental::listener::http_listener listener(address);

  listener.support(web::http::methods::POST, handle_post);

  try {
    listener.open().wait();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  spdlog::info("Listening for requests at: {}", address);

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  listener.close().wait();

  return 0;
}

void handle_post(web::http::http_request request)
{
  const auto path = web::uri::split_path(web::uri::decode(request.relative_uri().path()));

  if (path.size() == 2 && path[0] == U("prob") && (path[1] == U("t1") || path[1] == U("t2"))) {
    request.extract_json()
        .then([request, &path](web::json::value body) {
          const auto serialized = body.serialize();

          spdlog::info(LOG_MESSAGE,
                       fmt::format("accessd at {}, {}", path[1], serialized),
                       fmt::ptr(get_request_id(request)));

          const auto parsed = nlohmann::json::parse(serialized);

          validate(parsed);

          auto [player, params] = get_params(body);
          web::json::value result;

          if (path[1] == U("t1")) {
            const auto [stats, searched] = win_prob::win_prob1::WinProb1{calsht}(*player, params);
            result = get_result(stats, searched);
          }
          else {
            const auto [stats, searched] = win_prob::win_prob2::WinProb2{calsht}(*player, params);
            result = get_result(stats, searched);
          }

          request.reply(web::http::status_codes::OK, result);
        })
        .then([request](pplx::task<void> task) {
          handle_error(task, request);
        });
  }
  else {
    request.reply(web::http::status_codes::NotFound);
  }
}

void handle_error(const pplx::task<void> task, const web::http::http_request request)
{
  try {
    task.get();
  }
  catch (const web::json::json_exception& e) {
    spdlog::error(LOG_MESSAGE, e.what(), fmt::ptr(get_request_id(request)));
    request.reply(web::http::status_codes::BadRequest);
  }
  catch (const std::invalid_argument& e) {
    spdlog::error(LOG_MESSAGE, e.what(), fmt::ptr(get_request_id(request)));
    request.reply(web::http::status_codes::BadRequest);
  }
  catch (const std::exception& e) {
    spdlog::error(LOG_MESSAGE, e.what(), fmt::ptr(get_request_id(request)));
    request.reply(web::http::status_codes::InternalError);
  }
}

void validate(const nlohmann::json& json)
{
  custom_error_handler err;
  validator.validate(json, err);

  if (count_tile_num(json.at("hand")) != HAND_TILE_NUM) {
    throw std::invalid_argument("Invalid number of hand tiles");
  }

  if (count_tile_num(json.at("dora")) > MAX_DORA_TILE_NUM) {
    throw std::invalid_argument("Invalid number of dora tiles");
  }
}

std::pair<std::shared_ptr<player_impl::Player>, win_prob::Params> get_params(const web::json::value& json)
{
  auto table = std::make_shared<player_impl::Table>();
  auto player = std::make_shared<player_impl::Player>(table);

  set_dora(json.at("dora").as_string(), *table);
  set_hand(json.at("hand").as_string(), *player);
  check_tile_num(*table, *player);

  table->wind2 = json.at("bakaze").as_integer() + EAST_INDEX;
  player->wind1 = json.at("jikaze").as_integer() + EAST_INDEX;
  player->tsumo = true;
  player->num = HAND_TILE_NUM;

  win_prob::Params params{
      .t_min = T_MIN,
      .t_max = T_MAX,
      .sum = SUM,
      .extra = json.at("extra").as_integer(),
      .mode = json.at("mode").as_integer(),
      .calc_score = json.at("calcScore").as_bool(),
      .use_red = json.at("useRed").as_bool(),
  };

  return {player, params};
}

web::json::value get_result(const std::vector<win_prob::Stat>& stats, const std::size_t searched)
{
  web::json::value result;

  for (std::size_t i = 0; i < stats.size(); ++i) {
    const auto& stat = stats[i];
    const std::vector<web::json::value> prob(std::begin(stat.prob), std::end(stat.prob));
    const std::string label = get_label(stat.tile, stat.is_red);
    result["stats"][i]["tile"] = web::json::value::string(label);
    result["stats"][i]["prob"] = web::json::value::array(prob);
  }

  result["searched"] = searched;

  return result;
}
