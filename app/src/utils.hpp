#ifndef UTILS_HPP
#define UTILS_HPP

#include <valarray>

namespace win_prob {
  struct Stat {
    const int tile;                   // 打牌
    const bool is_red;                // 赤ドラか
    const std::valarray<double> prob; // 和了確率
  };

  struct Params {
    int t_min = 0;           // 最小巡目
    int t_max = 0;           // 最大巡目
    int sum = 0;             // 0巡目での牌の枚数
    int extra = 0;           // 手替わり回数
    int mode = 7;            // モード
    bool calc_score = false; // 得点を計算するか
    bool use_red = true;     //赤ドラを考慮するか
  };
}

#endif
