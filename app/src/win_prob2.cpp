#include <cassert>
#include <numeric>
#include <unordered_map>
#include <boost/graph/graph_utility.hpp>
#include "win_prob2.hpp"
using namespace player_impl;
constexpr int L = 64;

namespace {
  int64_t calc_disc2(const std::vector<int>& hand)
  {
    int64_t ret = 0LL;

    for (int i = 0; i < K; ++i) {
      if (hand[i] > 0) {
        ret |= 1LL << i;
      }
    }

    return ret;
  }

  int64_t calc_wait2(const std::vector<int>& hand)
  {
    int64_t ret = 0LL;

    for (int i = 0; i < K; ++i) {
      if (hand[i] < 4) {
        ret |= 1LL << i;
      }
    }

    return ret;
  }

  int distance(const std::vector<int>& hand, const std::vector<int>& origin)
  {
    return std::inner_product(hand.begin(), hand.end(),
                              origin.begin(),
                              0,
                              std::plus<int>(),
                              [](const int x, const int y) { return std::max(x - y, 0); });
  }

  std::vector<int> encode(const std::vector<int>& hand, const std::vector<int>& reds)
  {
    std::vector<int> ret(2 * K);

    for (int i = 0; i < K; ++i) {
      ret[i] = hand[i] - reds[i];
      ret[K + i] = reds[i];
    }

    return ret;
  }

  void draw(std::vector<int>& hand_reds, std::vector<int>& wall_reds, Player& player, const int tile)
  {
    ++hand_reds[tile];
    --wall_reds[tile];
    ++player.hand[tile % K];

    if (tile >= K) {
      ++player.closed_reds[tile % K];
    }
  }

  void discard(std::vector<int>& hand_reds, std::vector<int>& wall_reds, Player& player, const int tile)
  {
    --hand_reds[tile];
    ++wall_reds[tile];
    --player.hand[tile % K];

    if (tile >= K) {
      --player.closed_reds[tile % K];
    }
  }
}

namespace win_prob::win_prob2 {
  int calc_score(player_impl::Player& player, const int mode, const int tile, const Params& params)
  {
    return params.calc_score ? std::get<3>(player.calc_score(mode, tile)) : 1;
  }

  Graph::vertex_descriptor WinProb2::select1(Graph& graph,
                                             Desc& desc1,
                                             Desc& desc2,
                                             std::vector<int>& hand_reds,
                                             std::vector<int>& wall_reds,
                                             Player& player,
                                             const int num,
                                             const std::vector<int>& origin,
                                             const int sht_org,
                                             const Params& params) const
  {
    if (const auto itr = desc1.find(hand_reds); itr != desc1.end()) {
      return itr->second;
    }

    const auto desc = boost::add_vertex(std::valarray<double>(0., params.t_max + 1), graph);

    desc1[hand_reds] = desc;

    const auto [sht, mode, disc, wait] = calsht(player.hand, num / 3, params.mode);
    uint64_t all = distance(hand_reds, origin) + sht < sht_org + params.extra
                       ? calc_wait2(player.hand)
                       : wait;

    all |= all << K;

    for (int i = 0; i < L; ++i) {
      if (wall_reds[i] && (all & (1ULL << i))) {
        const int weight = wall_reds[i];

        draw(hand_reds, wall_reds, player, i);

        const auto target = select2(graph,
                                    desc1,
                                    desc2,
                                    hand_reds,
                                    wall_reds,
                                    player,
                                    num + 1,
                                    origin,
                                    sht_org,
                                    params);

        if (!boost::edge(desc, target, graph).second) {
          const int score = (sht == 1 && (wait & (1LL << i % K)) ? calc_score(player, mode, i % K, params) : 0);

          boost::add_edge(desc, target, {weight, score}, graph);
        }

        discard(hand_reds, wall_reds, player, i);
      }
    }

    return desc;
  }

  Graph::vertex_descriptor WinProb2::select2(Graph& graph,
                                             Desc& desc1,
                                             Desc& desc2,
                                             std::vector<int>& hand_reds,
                                             std::vector<int>& wall_reds,
                                             Player& player,
                                             const int num,
                                             const std::vector<int>& origin,
                                             const int sht_org,
                                             const Params& params) const
  {
    if (const auto itr = desc2.find(hand_reds); itr != desc2.end()) {
      return itr->second;
    }

    const auto desc = boost::add_vertex(std::valarray<double>(0., params.t_max + 1), graph);

    desc2[hand_reds] = desc;

    const auto [sht, mode, disc, wait] = calsht(player.hand, num / 3, params.mode);
    uint64_t all = distance(hand_reds, origin) + sht < sht_org + params.extra
                       ? calc_disc2(player.hand)
                       : disc;

    all |= all << K;

    for (int i = 0; i < L; ++i) {
      if (hand_reds[i] && (all & (1ULL << i))) {
        discard(hand_reds, wall_reds, player, i);

        const int weight = wall_reds[i];
        const auto source = select1(graph,
                                    desc1,
                                    desc2,
                                    hand_reds,
                                    wall_reds,
                                    player,
                                    num - 1,
                                    origin,
                                    sht_org,
                                    params);

        draw(hand_reds, wall_reds, player, i);

        if (!boost::edge(source, desc, graph).second) {
          const int score = (sht == 0 ? calc_score(player, mode, i % K, params) : 0);

          boost::add_edge(source, desc, {weight, score}, graph);
        }
      }
    }

    return desc;
  }

  void WinProb2::update(Graph& graph, const Desc& desc1, const Desc& desc2, const Params& params) const
  {
    for (int t = params.t_max - 1; t >= params.t_min; --t) {
      for (auto& [_, desc] : desc1) {
        auto& value = graph[desc];

        for (auto [first, last] = boost::out_edges(desc, graph); first != last; ++first) {
          const auto target = boost::target(*first, graph);
          const auto [weight, score] = graph[*first];

          value[t] += weight * (std::max(static_cast<double>(score), graph[target][t + 1]) - value[t + 1]);
        }

        value[t] = value[t + 1] + value[t] / params.sum;
      }

      for (auto& [_, desc] : desc2) {
        auto& value = graph[desc];

        for (auto [first, last] = boost::in_edges(desc, graph); first != last; ++first) {
          const auto source = boost::source(*first, graph);

          value[t] = std::max(value[t], graph[source][t]);
        }
      }
    }
  }

  std::tuple<std::vector<Stat>, std::size_t> WinProb2::operator()(Player& player, const Params& params) const
  {
    assert(params.t_min >= 0);
    assert(params.t_min < params.t_max);
    assert(player.num % 3 == 2);

    auto wall = Wall(player, params.use_red);

    std::vector<Stat> stats;

    const auto [sht, mode, disc, wait] = calsht(player.hand, player.num / 3, params.mode);

    Graph graph;
    Desc desc1, desc2;

    auto hand_reds = encode(player.hand, player.closed_reds);
    auto wall_reds = encode(wall.wall, wall.reds);

    select2(graph,
            desc1,
            desc2,
            hand_reds,
            wall_reds,
            player,
            player.num,
            std::vector<int>{hand_reds},
            sht,
            params);
    update(graph, desc1, desc2, params);

    for (int i = 0; i < L; ++i) {
      if (hand_reds[i] > 0) {
        --hand_reds[i];

        if (const auto itr = desc1.find(hand_reds); itr != desc1.end()) {
          stats.emplace_back(Stat{i % K, i > K, graph[itr->second]});
        }
        ++hand_reds[i];
      }
    }

    const auto searched = boost::num_vertices(graph);

    return {stats, searched};
  }
}
