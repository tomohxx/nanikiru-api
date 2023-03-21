#include <array>
#include "constant.hpp"
#include "getlabel.hpp"

const std::array<std::string, K> label = {
    "1m", "2m", "3m", "4m", "5m", "6m", "7m", "8m", "9m",
    "1p", "2p", "3p", "4p", "5p", "6p", "7p", "8p", "9p",
    "1s", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s",
    "1z", "2z", "3z", "4z", "5z", "6z", "7z"};
const std::array<std::string, 3> label_red = {
    "0m", "0p", "0s"};

std::string get_label(const int tile, const bool is_red)
{
  return is_red ? label_red[tile / 9] : label[tile];
}
