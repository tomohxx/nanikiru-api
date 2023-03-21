#include <algorithm>
#include <array>
#include <climits>
#include <iostream>
#include <stdexcept>
#include "player_impl.hpp"

constexpr unsigned int BIT_MASK = 0x1DA;

inline bool is_five(const int i)
{
  return i == 4 || i == 13 || i == 22;
}

inline void result(const int h, const int y, int& han, int& yaku)
{
  han += h;
  yaku |= y;
}

namespace player_impl {
  void Table::reset()
  {
    std::fill(dora.begin(), dora.end(), 0);
    std::fill(field.begin(), field.end(), 0);
    std::fill(reds.begin(), reds.end(), 0);
    discards.clear();
  }

  Wall::Wall(const Table& table, const bool use_red)
      : wall(K, 0), reds(K, 0)
  {
    for (int i = 0; i < K; ++i) {
      wall[i] = 4 - table.field[i];
      reds[i] = use_red && is_five(i) && !table.reds[i] ? 1 : 0;
    }
  }

  Wall::Wall(const Player& player, const bool use_red)
      : wall(K, 0), reds(K, 0)
  {
    for (int i = 0; i < K; ++i) {
      wall[i] = 4 - player.table->field[i] - player.hand[i];
      reds[i] = use_red &&
                        is_five(i) &&
                        !player.table->reds[i] &&
                        !player.closed_reds[i] &&
                        !player.open_reds[i]
                    ? 1
                    : 0;
    }
  }

  void Player::menzen_tsumo(int& han, int& yaku) const
  {
    if (!fulo() && tsumo) {
      result(1, MENZEN_TSUMO, han, yaku);
    }
  }

  void Player::iipeikou(int& han, int& yaku) const
  {
    if (fulo()) return;

    int cnt = 0;

    for (int i = 0; i < 27; ++i) {
      if (melds[i].anshun >= 2) ++cnt;
    }

    if (cnt == 1) {
      result(1, IIPEIKOU, han, yaku);
    }
    else if (cnt == 2) {
      result(3, RYANPEIKOU, han, yaku);
    }
  }

  void Player::sanshoku_doujun(int& han, int& yaku) const
  {
    for (int i = 0; i < 7; ++i) {
      if (melds[i].minshun + melds[i].anshun >= 1 &&
          melds[i + 9].minshun + melds[i + 9].anshun >= 1 &&
          melds[i + 18].minshun + melds[i + 18].anshun >= 1) {
        return fulo() ? result(1, SANSHOKU_DOUJUN, han, yaku)
                      : result(2, SANSHOKU_DOUJUN, han, yaku);
      }
    }
  }

  void Player::ikkitsuukan(int& han, int& yaku) const
  {
    for (int i = 0; i < 3; ++i) {
      if (melds[9 * i].minshun + melds[9 * i].anshun >= 1 &&
          melds[9 * i + 3].minshun + melds[9 * i + 3].anshun >= 1 &&
          melds[9 * i + 6].minshun + melds[9 * i + 6].anshun >= 1) {
        return fulo() ? result(1, IKKITSUUKAN, han, yaku)
                      : result(2, IKKITSUUKAN, han, yaku);
      }
    }
  }

  void Player::sanshoku_doukou(int& han, int& yaku) const
  {
    for (int i = 0; i < 9; ++i) {
      if (melds[i].kotsu_kantsu &&
          melds[i + 9].kotsu_kantsu &&
          melds[i + 18].kotsu_kantsu) {
        return result(2, SANSHOKU_DOUKOU, han, yaku);
      }
    }
  }

  void Player::suuankou(int& han, int& yaku) const
  {
    int cnt = 0;

    for (int i = 0; i < K; ++i) {
      if (melds[i].ankou || melds[i].ankan) ++cnt;
    }

    if (cnt == 4) {
      result(LIMIT, SUUANKOU, han, yaku);
    }
    else if (cnt == 3) {
      result(2, SANANKOU, han, yaku);
    }
  }

  void Player::suukantsu(int& han, int& yaku) const
  {
    int cnt = 0;

    for (int i = 0; i < K; ++i) {
      if (melds[i].kantsu) ++cnt;
    }

    if (cnt == 4) {
      result(LIMIT, SUUKANTSU, han, yaku);
    }
    else if (cnt == 3) {
      result(2, SANKANTSU, han, yaku);
    }
  }

  void Player::toitoi(int& han, int& yaku) const
  {
    int cnt = 0;

    for (int i = 0; i < K; ++i) {
      if (melds[i].kotsu_kantsu) ++cnt;
    }

    if (cnt == 4) {
      result(2, TOITOI, han, yaku);
    }
  }

  void Player::daisangen(int& han, int& yaku) const
  {
    int cnt = 0;

    for (int i = 31; i < K; ++i) {
      if (melds[i].kotsu_kantsu) ++cnt;
    }

    if (cnt == 3) {
      result(LIMIT, DAISANGEN, han, yaku);
    }
    else if (cnt == 2 && (melds[31].jantou || melds[32].jantou || melds[33].jantou)) {
      result(4, SHOUSANGEN | YAKUHAI, han, yaku);
    }
    else if (cnt > 0) {
      result(cnt, YAKUHAI, han, yaku);
    }
  }

  void Player::yakuhai(int& han, int& yaku) const
  {
    int cnt = 0;

    if (melds[wind1].kotsu_kantsu) ++cnt;
    if (melds[table->wind2].kotsu_kantsu) ++cnt;

    if (cnt > 0) {
      result(cnt, YAKUHAI, han, yaku);
    }
  }

  void Player::tanyaochuu(const std::vector<int>& t, int& han, int& yaku) const
  {
    for (const int i : {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33}) {
      if (t[i] > 0) return;
    }

    result(1, TANYAOCHUU, han, yaku);
  }

  void Player::chinroutou(const std::vector<int>& t, int& han, int& yaku) const
  {
    int cnt = 0;

    for (const int i : {0, 8, 9, 17, 18, 26}) {
      cnt += t[i] > 3 ? 3 : t[i];
    }

    if (cnt == 14) {
      return result(LIMIT, CHINROUTOU, han, yaku);
    }

    for (const int i : {27, 28, 29, 30, 31, 32, 33}) {
      cnt += t[i] > 3 ? 3 : t[i];
    }

    if (cnt == 14) {
      result(2, HONROUTOU, han, yaku);
    }
  }

  void Player::chiniisou(const std::vector<int>& t, int& han, int& yaku) const
  {
    std::array<int, 4> cnt = {};

    for (int i = 0; i < K; ++i) {
      if (t[i] > 0) ++cnt[i / 9];
    }

    if (cnt[0] == 0 && cnt[1] == 0 && cnt[2] == 0) {
      result(LIMIT, TSUUIISOU, han, yaku);
    }
    else if ((cnt[0] == 0 && cnt[1] == 0) ||
             (cnt[0] == 0 && cnt[2] == 0) ||
             (cnt[1] == 0 && cnt[2] == 0)) {
      cnt[3] == 0 ? (fulo() ? result(5, CHINIISOU, han, yaku)
                            : result(6, CHINIISOU, han, yaku))
                  : (fulo() ? result(2, HONIISOU, han, yaku)
                            : result(3, HONIISOU, han, yaku));
    }
  }

  void Player::junchantaiyao(int& han, int& yaku) const
  {
    int cnt = 0;

    for (const int i : {0, 6, 9, 15, 18, 24}) {
      cnt += melds[i].minshun + melds[i].anshun;
    }

    if (cnt == 0) return;

    for (const int i : {0, 8, 9, 17, 18, 26}) {
      if (melds[i].jantou_kotsu_kantsu) ++cnt;
    }

    if (cnt == 5) {
      return fulo() ? result(2, JUNCHANTAIYAO, han, yaku)
                    : result(3, JUNCHANTAIYAO, han, yaku);
    }

    for (int i = 27; i < K; ++i) {
      if (melds[i].jantou_kotsu_kantsu) ++cnt;
    }

    if (cnt == 5) {
      fulo() ? result(1, CHANTAIYAO, han, yaku)
             : result(2, CHANTAIYAO, han, yaku);
    }
  }

  void Player::daisuushii(int& han, int& yaku) const
  {
    int cnt = 0;

    for (int i = 27; i < 31; ++i) {
      if (melds[i].kotsu_kantsu) ++cnt;
    }

    if (cnt == 4) {
      result(LIMIT, DAISUUSHII, han, yaku);
    }
    else if (cnt == 3 && (melds[27].jantou || melds[28].jantou || melds[29].jantou || melds[30].jantou)) {
      result(LIMIT, SHOUSUUSHII, han, yaku);
    }
  }

  void Player::ryuuiisou(int& han, int& yaku) const
  {
    int cnt = 0;

    for (const int i : {19, 20, 21, 23, 25, 32}) {
      if (melds[i].jantou_kotsu_kantsu) ++cnt;
    }

    cnt += melds[19].minshun + melds[19].anshun;

    if (cnt == 5) {
      result(LIMIT, RYUUIISOU, han, yaku);
    }
  }

  void Player::chuurenpoutou(int& han, int& yaku) const
  {
    if (!fulo()) {
      for (int i = 0; i < 3; ++i) {
        int cnt = 0;

        if (hand[9 * i] < 3 || hand[9 * i + 8] < 3) {
          continue;
        }

        cnt += hand[9 * i] + hand[9 * i + 8];

        if (hand[9 * i + 1] < 1 ||
            hand[9 * i + 2] < 1 ||
            hand[9 * i + 3] < 1 ||
            hand[9 * i + 4] < 1 ||
            hand[9 * i + 5] < 1 ||
            hand[9 * i + 6] < 1 ||
            hand[9 * i + 7] < 1) {
          continue;
        }

        for (int j = 1; j < 8; ++j) {
          cnt += hand[9 * i + j];
        }

        if (cnt == 14) {
          return result(LIMIT, CHUURENPOUTOU, han, yaku);
        }
      }
    }
  }

  void Player::reset()
  {
    std::fill(hand.begin(), hand.end(), 0);
    std::fill(open.begin(), open.end(), 0);
    std::fill(closed_reds.begin(), closed_reds.end(), 0);
    std::fill(open_reds.begin(), open_reds.end(), 0);
    std::fill(melds.begin(), melds.end(), Meld{});
    std::fill(tiles.begin(), tiles.end(), 0);
    std::fill(discards.begin(), discards.end(), false);
    last_discard = table->discards.begin();
    called_riichi = false;
    tsumo = false;
    fulo_count = 0;
    wind1 = 0;
    init = 0;
    num = 0;
  }

  void Player::draw(const Tile& tile)
  {
    const auto [i, b] = tile;
    ++hand[i];
    ++num;
    if (b) closed_reds[i] = 1;
  }

  void Player::discard(const Tile& tile)
  {
    const auto [i, b] = tile;
    --hand[i];
    --num;
    discards[i] = true;
    ++table->field[i];

    if (b) {
      closed_reds[i] = 0;
      table->reds[i] = 1;
    }

    const auto inserted = table->discards.insert(table->discards.end(), i);

    if (!called_riichi) {
      last_discard = inserted;
    }
  }

  int Player::consume(const Tiles& consumed, const bool flag)
  {
    int m = INT_MAX;

    if (flag) {
      for (const auto& [i, b] : consumed) {
        --hand[i];
        --num;
        ++open[i];
        ++table->field[i];

        if (b) {
          closed_reds[i] = 0;
          open_reds[i] = 1;
        }
        m = std::min(m, i);
      }
    }
    else {
      for (const auto& [i, b] : consumed) {
        ++hand[i];
        ++num;
        --open[i];
        --table->field[i];

        if (b) {
          closed_reds[i] = 1;
          open_reds[i] = 0;
        }
        m = std::min(m, i);
      }
    }

    return m;
  }

  void Player::call_pong(const Tile& tile, const Tiles& consumed, const bool flag)
  {
    const auto [i, b] = tile;
    consume(consumed, flag);

    if (flag) {
      ++open[i];

      if (b) {
        closed_reds[i] = 0;
        open_reds[i] = 1;
      }
      ++fulo_count;
    }
    else {
      --open[i];

      if (b) {
        closed_reds[i] = 1;
        open_reds[i] = 0;
      }
      --fulo_count;
    }
    melds[i].minkou ^= 1;
  }

  void Player::call_chii(const Tile& tile, const Tiles& consumed, const bool flag)
  {
    const auto [i, b] = tile;
    const auto j = consume(consumed, flag);

    if (flag) {
      ++open[i];

      if (b) {
        closed_reds[i] = 0;
        open_reds[i] = 1;
      }
      ++fulo_count;
      melds[std::min(i, j)].minshun += 1;
    }
    else {
      --open[i];

      if (b) {
        closed_reds[i] = 1;
        open_reds[i] = 0;
      }
      --fulo_count;
      melds[std::min(i, j)].minshun -= 1;
    }
  }

  void Player::call_ankang(const Tiles& consumed, const bool flag)
  {
    const auto m = consume(consumed, flag);
    melds[m].ankan ^= 1;
  }

  void Player::call_daiminkang(const Tile& tile, const Tiles& consumed, const bool flag)
  {
    const auto [i, b] = tile;
    consume(consumed, flag);

    if (flag) {
      ++open[i];

      if (b) {
        closed_reds[i] = 0;
        open_reds[i] = 1;
      }
      ++fulo_count;
    }
    else {
      --open[i];

      if (b) {
        closed_reds[i] = 1;
        open_reds[i] = 0;
      }
      --fulo_count;
    }
    melds[i].minkan ^= 1;
  }

  void Player::call_kakang(const Tile& tile, const bool flag)
  {
    const auto m = consume({tile}, flag);
    melds[m].minkou ^= 1;
    melds[m].minkan ^= 1;
  }

  void Player::add_minkou(const Tiles& tiles)
  {
    int m = INT_MAX;

    for (const auto& [i, b] : tiles) {
      ++open[i];

      if (b) {
        open_reds[i] = 1;
      }
      m = std::min(m, i);
    }

    ++fulo_count;
    melds[m].minkou = 1;
  }

  void Player::add_minshun(const Tiles& tiles)
  {
    int m = INT_MAX;

    for (const auto& [i, b] : tiles) {
      ++open[i];

      if (b) {
        open_reds[i] = 1;
      }
      m = std::min(m, i);
    }

    ++fulo_count;
    melds[m].minshun += 1;
  }

  void Player::add_ankang(const Tiles& tiles)
  {
    int m = INT_MAX;

    for (const auto& [i, b] : tiles) {
      ++open[i];

      if (b) {
        open_reds[i] = 1;
      }
      m = std::min(m, i);
    }

    melds[m].ankan = 1;
  }

  void Player::add_minkang(const Tiles& tiles)
  {
    int m = INT_MAX;

    for (const auto& [i, b] : tiles) {
      ++open[i];

      if (b) {
        open_reds[i] = 1;
      }
      m = std::min(m, i);
    }

    ++fulo_count;
    melds[m].minkan = 1;
  }

  bool Player::is_wh0(const int* t, Meld* u) const
  {
    int a = t[0], b = t[1];

    for (int i = 0; i < 7; ++i) {
      if (a >= 3) {
        a -= 3;
        u[i].ankou = 1;
      }

      if (const int r = a; b >= r && t[i + 2] >= r) {
        a = b - r;
        b = t[i + 2] - r;
        u[i].anshun += r;
      }
      else return false;
    }

    if (a == 3) {
      a = 0;
      u[7].ankou = 1;
    }

    if (b == 3) {
      b = 0;
      u[8].ankou = 1;
    }

    return a == 0 && b == 0;
  }

  bool Player::is_wh2(int* t, Meld* u, const int r)
  {
    int p = 0;

    for (int i = 0; i < 9; ++i) {
      p += i * t[i];
    }

    p = p * 2 % 3 + r * 3;
    t[p] -= 2;

    if (t[p] >= 0) {
      if (is_wh0(t, u)) {
        t[p] += 2;
        u[p].jantou = 1;
        return true;
      }
      else {
        std::for_each(u, u + 9, [](Meld& meld) { meld.all &= BIT_MASK; });
      }
    }

    t[p] += 2;
    return false;
  }

  void Player::calc_fu(const std::vector<int>& t, int& fu, int& han, int& yaku, const int tile)
  {
    bool flag = false;
    int r = tile % 9;

    fu = 0;
    han = 0;
    yaku = 0;

    for (int i = 0; i < K; ++i) {
      fu += (i < 27 && i % 9 % 8 != 0) ? melds[i].fu : melds[i].fu * 2;
    }

    if (melds[31].jantou || melds[32].jantou || melds[33].jantou || melds[wind1].jantou) {
      fu += 2;
    }

    if (melds[table->wind2].jantou) {
      fu += 2;
    }

    if (fu == 0 && ((r < 6 && melds[tile].anshun >= 1) || (r > 2 && melds[tile - 2].anshun >= 1))) {
      fu = fulo() ? 30 : (tsumo ? 20 : 30);
      han = fulo() ? 0 : 1;
      yaku |= fulo() ? 0 : PINFU;
    }
    else {
      if ((melds[tile].jantou) ||
          (r != 0 && melds[tile - 1].anshun >= 1) ||
          (r == 2 && melds[tile - 2].anshun >= 1) ||
          (r == 6 && melds[tile].anshun >= 1)) {
        fu += tsumo ? 4 : (fulo() ? 2 : 12);
      }
      else if ((r < 6 && melds[tile].anshun >= 1) || (r > 2 && melds[tile - 2].anshun >= 1)) {
        fu += tsumo ? 2 : (fulo() ? 0 : 10);
      }
      else {
        if (tsumo) {
          fu += 2;
        }
        else {
          fu += (tile >= 27 || r % 8 == 0) ? (fulo() ? -4 : 6) : (fulo() ? -2 : 8);
          melds[tile].ankou ^= 1;
          melds[tile].minkou ^= 1;
          flag = true;
        }
      }

      fu = (fu + 29) / 10 * 10;
    }

    han += init;
    menzen_tsumo(han, yaku);
    iipeikou(han, yaku);
    sanshoku_doujun(han, yaku);
    ikkitsuukan(han, yaku);
    sanshoku_doukou(han, yaku);
    suuankou(han, yaku);
    suukantsu(han, yaku);
    toitoi(han, yaku);
    daisangen(han, yaku);
    yakuhai(han, yaku);
    tanyaochuu(t, han, yaku);
    chinroutou(t, han, yaku);
    chiniisou(t, han, yaku);
    junchantaiyao(han, yaku);
    daisuushii(han, yaku);
    ryuuiisou(han, yaku);
    chuurenpoutou(han, yaku);

    if (flag) {
      melds[tile].minkou ^= 1;
      melds[tile].ankou ^= 1;
    }
  }

  void Player::calc_dora(const std::vector<int>& t, const std::vector<int>& u, int& han) const
  {
    if (han == 0) return;

    for (int i = 0; i < K; ++i) {
      han += table->dora[i] * (t[i] + u[i]);
      han += ((closed_reds[i] && t[i] > 0) || (open_reds[i] && u[i] > 0)) ? 1 : 0;
    }
  }

  int Player::less_than_2000(const int base) const
  {
    if (tsumo) {
      if (wind1 == 27) {
        return (base * 2 + 99) / 100 * 100 * 3;
      }
      else {
        return (base + 99) / 100 * 100 * 2 + (base * 2 + 99) / 100 * 100;
      }
    }
    else {
      if (wind1 == 27) {
        return (base * 6 + 99) / 100 * 100;
      }
      else {
        return (base * 4 + 99) / 100 * 100;
      }
    }
  }

  int Player::greater_than_2000(const int han) const
  {
    switch (han) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      return wind1 == 27 ? 12000 : 8000;
    case 6:
    case 7:
      return wind1 == 27 ? 18000 : 12000;
    case 8:
    case 9:
    case 10:
      return wind1 == 27 ? 24000 : 16000;
    case 11:
    case 12:
      return wind1 == 27 ? 36000 : 24000;
    default:
      return wind1 == 27 ? 48000 : 32000;
    }
  }

  int Player::score(const int fu, const int han) const
  {
    if (han == 0) return 0;

    int base = fu * (1 << (han + 2));

    if (base < 2000) {
      return less_than_2000(base);
    }
    else {
      return greater_than_2000(han);
    }
  }

  void Player::calc_lh(int& fu, int& han, int& yaku, const int tile)
  {
    int head = 0;
    int fu_ = 0;
    int han_ = 0;
    int yaku_ = 0;

    fu = 0;
    han = 0;
    yaku = 0;

    std::for_each(melds.begin(), melds.end(), [](Meld& meld) {
      meld.all &= BIT_MASK;
    });

    for (int i = 0; i < 3; ++i) {
      const int sum = std::accumulate(&hand[9 * i], &hand[9 * i + 9], 0);
      if (sum % 3 == 2) head = i;
      else is_wh0(&hand[9 * i], &melds[9 * i]);
    }

    for (int i = 27; i < K; ++i) {
      if (hand[i] == 2) {
        head = 3;
        melds[i].jantou = 1;
      }
      else if (hand[i] == 3) {
        melds[i].ankou = 1;
      }
    }

    for (int i = 0; i < K; ++i) {
      tiles[i] = hand[i] + open[i];
    }

    for (int r = 0; r < 3; ++r) {
      if (head < 3) {
        std::for_each(&melds[9 * head], &melds[9 * head + 9], [](Meld& meld) {
          meld.all &= BIT_MASK;
        });
        if (!is_wh2(&hand[9 * head], &melds[9 * head], r)) continue;
      }

      calc_fu(tiles, fu_, han_, yaku_, tile);

      if (han_ > han || (han_ == han && fu_ > fu)) {
        fu = fu_;
        han = han_;
        yaku = yaku_;
      }

      for (int i = 0; i < 3; ++i) {
        for (int n = 9 * i; n < 9 * i + 7; ++n) {
          if (melds[n].ankou && melds[n + 1].ankou && melds[n + 2].ankou) {
            melds[n].anshun += 3;
            melds[n].ankou = 0;
            melds[n + 1].ankou = 0;
            melds[n + 2].ankou = 0;
            calc_fu(tiles, fu_, han_, yaku_, tile);

            if (han_ > han || (han_ == han && fu_ > fu)) {
              fu = fu_;
              han = han_;
              yaku = yaku_;
            }

            break;
          }
        }
      }

      if (head == 3) break;
    }

    calc_dora(hand, open, han);
  }

  void Player::calc_sp(int& fu, int& han, int& yaku) const
  {
    fu = 25;
    han = init + 2;
    yaku = CHIITOITSU;
    menzen_tsumo(han, yaku);
    tanyaochuu(hand, han, yaku);
    chinroutou(hand, han, yaku);
    chiniisou(hand, han, yaku);
    calc_dora(hand, open, han);
  }

  void Player::calc_to(int& fu, int& han, int& yaku) const
  {
    fu = 0;
    han = LIMIT;
    yaku = KOKUSHIMUSOU;
  }

  bool Player::is_lh()
  {
    int head = INT_MIN;

    for (int i = 0; i < 3; ++i) {
      switch (std::accumulate(&hand[9 * i], &hand[9 * i + 9], 0) % 3) {
      case 1: return false;
      case 2:
        if (head == INT_MIN) head = i;
        else return false;
      }
    }

    for (int i = 27; i < K; ++i) {
      switch (hand[i] % 3) {
      case 1: return false;
      case 2:
        if (head == INT_MIN) head = i;
        else return false;
      }
    }

    for (int i = 0; i < 3; ++i) {
      if (i == head) {
        if (!is_wh2(&hand[9 * i], &melds[9 * i], 0) &&
            !is_wh2(&hand[9 * i], &melds[9 * i], 1) &&
            !is_wh2(&hand[9 * i], &melds[9 * i], 2)) {
          return false;
        }
      }
      else {
        if (!is_wh0(&hand[9 * i], &melds[9 * i])) {
          return false;
        }
      }
    }

    return true;
  }

  bool Player::is_sp() const
  {
    int pair = 0;

    for (int i = 0; i < K; ++i) {
      if (hand[i] == 2) ++pair;
    }
    return pair == 7;
  }

  bool Player::is_to() const
  {
    int pair = 0;
    int kind = 0;

    for (const int i : {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33}) {
      if (hand[i] > 0) {
        ++kind;
        if (hand[i] >= 2) ++pair;
      }
    }
    return kind == 13 && pair == 1;
  }

  std::tuple<int, int, int, int> Player::calc_score(const int mode, const int tile)
  {
    int fu;
    int han;
    int yaku;
    int score_;

    if (mode == 0) {
      if (is_lh()) {
        calc_lh(fu, han, yaku, tile);
      }
      else if (is_sp()) {
        calc_sp(fu, han, yaku);
      }
      else if (is_to()) {
        calc_to(fu, han, yaku);
      }
      else {
        throw std::runtime_error("Not a winning hand");
      }
    }
    else if (mode & 1) {
      calc_lh(fu, han, yaku, tile);
    }
    else if (mode & 2) {
      calc_sp(fu, han, yaku);
    }
    else if (mode & 4) {
      calc_to(fu, han, yaku);
    }
    else {
      throw std::runtime_error("Bad mode");
    }
    score_ = score(fu, han);

    return {fu, han, yaku, score_};
  }

  std::vector<bool> Player::safe_tiles() const
  {
    std::vector<bool> ret = discards;

    for (auto itr = last_discard; itr != table->discards.end(); ++itr) {
      ret[*itr] = true;
    }

    return ret;
  }
}
