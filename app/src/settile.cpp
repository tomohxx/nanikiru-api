#include <algorithm>
#include <array>
#include <stdexcept>
#include <vector>
#include "settile.hpp"

const std::array<int, K> dora = {
    1, 2, 3, 4, 5, 6, 7, 8, 0,
    10, 11, 12, 13, 14, 15, 16, 17, 9,
    19, 20, 21, 22, 23, 24, 25, 26, 18,
    28, 29, 30, 27, 32, 33, 31};

inline bool is_red(const char chr)
{
  return chr == '0';
}

inline int index(const char chr, const int off)
{
  return (is_red(chr) ? '5' : chr) - '1' + off;
}

void set_dora(const std::string& str, player_impl::Table& table)
{
  int off = 0;

  for (auto itr = str.rbegin(); itr != str.rend(); ++itr) {
    switch (*itr) {
    case 'm': off = 0; break;
    case 'p': off = 9; break;
    case 's': off = 18; break;
    case 'z': off = 27; break;
    default: {
      const int idx = index(*itr, off);
      ++table.field[idx];
      ++table.dora[dora[idx]];
      if (is_red(*itr)) table.reds[idx] = 1;
    }
    }
  }
}

void set_hand(const std::string& str, player_impl::Player& player)
{
  int off = 0;

  for (auto itr = str.rbegin(); itr != str.rend(); ++itr) {
    switch (*itr) {
    case 'm': off = 0; break;
    case 'p': off = 9; break;
    case 's': off = 18; break;
    case 'z': off = 27; break;
    default: {
      const int idx = index(*itr, off);
      ++player.hand[idx];
      if (is_red(*itr)) player.closed_reds[idx] = 1;
    }
    }
  }
}

void check_tile_num(const player_impl::Table& table, const player_impl::Player& player)
{
  for (int i = 0; i < K; ++i) {
    if (table.field[i] + player.hand[i] > 4) {
      throw std::invalid_argument("Invalid number of tiles at " + i);
    }
  }
}
