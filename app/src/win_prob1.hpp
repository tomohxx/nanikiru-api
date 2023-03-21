#ifndef WIN_PROB1_HPP
#define WIN_PROB1_HPP

#include <tuple>
#include <vector>
#include <boost/container_hash/hash.hpp>
#include "calsht_dw.hpp"
#include "player_impl.hpp"
#include "utils.hpp"

namespace win_prob::win_prob1 {
  struct Hash {
    std::size_t operator()(const std::vector<int>& hand) const
    {
      return boost::hash_range(hand.begin(), hand.end());
    }
  };

  using Cache = std::unordered_map<std::vector<int>, std::valarray<double>, Hash>;

  class WinProb1 {
  private:
    const CalshtDW& calsht;

    std::valarray<double> select1(Cache& cache,
                                  std::vector<int>& hand_reds,
                                  std::vector<int>& wall_reds,
                                  player_impl::Player& player,
                                  int num,
                                  const Params& params) const;
    std::valarray<double> select2(Cache& cache,
                                  std::vector<int>& hand_reds,
                                  std::vector<int>& wall_reds,
                                  player_impl::Player& player,
                                  int num,
                                  const Params& params) const;

  public:
    WinProb1(const CalshtDW& _calsht)
        : calsht(_calsht) {}
    std::tuple<std::vector<Stat>, std::size_t> operator()(player_impl::Player& player, const Params& params) const;
  };
}

#endif
