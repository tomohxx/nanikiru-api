#include <cassert>
#include <numeric>
#include <unordered_map>
#include "win_prob1.hpp"
using namespace player_impl;
constexpr int L = 64;

namespace {
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

namespace win_prob::win_prob1 {
  int calc_score(Player& player, const int mode, const int tile, const Params& params)
  {
    return params.calc_score ? std::get<3>(player.calc_score(mode, tile)) : 1;
  }

  std::valarray<double> WinProb1::select1(Cache& cache,
                                          std::vector<int>& hand_reds,
                                          std::vector<int>& wall_reds,
                                          Player& player,
                                          const int num,
                                          const Params& params) const
  {
    if (const auto itr = cache.find(hand_reds); itr != cache.end()) {
      return itr->second;
    }

    const auto [sht, mode, disc, wait] = calsht(player.hand, num / 3, params.mode);
    const uint64_t all = wait | wait << K;
    int sum = 0;
    std::valarray<double> tmp(0., params.t_max + 1);

    for (int i = 0; i < L; ++i) {
      if (wall_reds[i] && (all & (1ULL << i))) {
        const int weight = wall_reds[i];

        draw(hand_reds, wall_reds, player, i);

        sum += weight;
        tmp += weight * (sht == 1
                             ? std::valarray<double>(calc_score(player, mode, i % K, params), params.t_max + 1)
                             : select2(cache, hand_reds, wall_reds, player, num + 1, params));

        discard(hand_reds, wall_reds, player, i);
      }
    }

    std::valarray<double> ret(0., params.t_max + 1);

    for (int t = params.t_max - 1; t >= params.t_min; --t) {
      ret[t] = (1. - static_cast<double>(sum) / params.sum) * ret[t + 1] + tmp[t + 1] / params.sum;
    }

    return cache[hand_reds] = ret;
  }

  std::valarray<double> WinProb1::select2(Cache& cache,
                                          std::vector<int>& hand_reds,
                                          std::vector<int>& wall_reds,
                                          Player& player,
                                          const int num,
                                          const Params& params) const
  {
    if (const auto itr = cache.find(hand_reds); itr != cache.end()) {
      return itr->second;
    }

    const auto [sht, mode, disc, wait] = calsht(player.hand, num / 3, params.mode);
    const uint64_t all = disc | disc << K;
    std::valarray<double> ret(0., params.t_max + 1);

    for (int i = 0; i < L; ++i) {
      if (hand_reds[i] && (all & (1ULL << i))) {
        discard(hand_reds, wall_reds, player, i);

        const auto tmp = select1(cache, hand_reds, wall_reds, player, num - 1, params);

        for (int j = params.t_min; j <= params.t_max; ++j) {
          ret[j] = std::max(ret[j], tmp[j]);
        }

        draw(hand_reds, wall_reds, player, i);
      }
    }

    return cache[hand_reds] = ret;
  }

  std::tuple<std::vector<Stat>, std::size_t> WinProb1::operator()(Player& player, const Params& params) const
  {
    assert(params.t_min >= 0);
    assert(params.t_min < params.t_max);
    assert(player.num % 3 == 2);

    auto wall = Wall(player, params.use_red);

    std::vector<Stat> stats;

    Cache cache;

    auto hand_reds = encode(player.hand, player.closed_reds);
    auto wall_reds = encode(wall.wall, wall.reds);

    select2(cache, hand_reds, wall_reds, player, player.num, params);

    for (int i = 0; i < L; ++i) {
      if (hand_reds[i] > 0) {
        --hand_reds[i];

        if (const auto itr = cache.find(hand_reds); itr != cache.end()) {
          stats.emplace_back(Stat{i % K, i > K, itr->second});
        }
        ++hand_reds[i];
      }
    }

    const auto searched = cache.size();

    return {stats, searched};
  }
}
