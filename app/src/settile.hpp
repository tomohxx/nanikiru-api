#ifndef SETTILE_HPP
#define SETTILE_HPP

#include <string>
#include "player_impl.hpp"

inline int count_tile_num(const std::string& str)
{
  return std::count_if(str.begin(), str.end(), isdigit);
}

void set_dora(const std::string& str, player_impl::Table& table);
void set_hand(const std::string& str, player_impl::Player& player);
void check_tile_num(const player_impl::Table& table, const player_impl::Player& player);

#endif
