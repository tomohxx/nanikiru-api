#ifndef PLAYER_IMPL_HPP
#define PLAYER_IMPL_HPP

#include <cstdint>
#include <memory>
#include <numeric>
#include <tuple>
#include <vector>
#include "constant.hpp"

namespace player_impl {
  using Tile = std::pair<int, bool>;
  using Tiles = std::vector<Tile>;

  constexpr int MENZEN_TSUMO = 1 << 0;
  constexpr int PINFU = 1 << 1;
  constexpr int IIPEIKOU = 1 << 2;
  constexpr int RYANPEIKOU = 1 << 3;
  constexpr int SANSHOKU_DOUJUN = 1 << 4;
  constexpr int IKKITSUUKAN = 1 << 5;
  constexpr int SANSHOKU_DOUKOU = 1 << 6;
  constexpr int SUUANKOU = 1 << 7;
  constexpr int SANANKOU = 1 << 8;
  constexpr int SUUKANTSU = 1 << 9;
  constexpr int SANKANTSU = 1 << 10;
  constexpr int TOITOI = 1 << 11;
  constexpr int DAISANGEN = 1 << 12;
  constexpr int SHOUSANGEN = 1 << 13;
  constexpr int YAKUHAI = 1 << 14;
  constexpr int TANYAOCHUU = 1 << 15;
  constexpr int CHINROUTOU = 1 << 16;
  constexpr int HONROUTOU = 1 << 17;
  constexpr int CHINIISOU = 1 << 18;
  constexpr int HONIISOU = 1 << 19;
  constexpr int JUNCHANTAIYAO = 1 << 20;
  constexpr int CHANTAIYAO = 1 << 21;
  constexpr int DAISUUSHII = 1 << 22;
  constexpr int SHOUSUUSHII = 1 << 23;
  constexpr int TSUUIISOU = 1 << 24;
  constexpr int RYUUIISOU = 1 << 25;
  constexpr int CHUURENPOUTOU = 1 << 26;
  constexpr int CHIITOITSU = 1 << 27;
  constexpr int KOKUSHIMUSOU = 1 << 28;

  union Meld {
    struct {
      unsigned int : 1;
      unsigned int minkou : 1;
      unsigned int ankou : 1;
      unsigned int minkan : 1;
      unsigned int ankan : 1;
      unsigned int jantou : 1;
      unsigned int minshun : 3;
      unsigned int anshun : 3;
    };

    struct {
      unsigned int : 1;
      unsigned int kotsu : 2;
      unsigned int kantsu : 2;
      unsigned int : 1;
      unsigned int shuntsu : 6;
    };

    struct {
      unsigned int : 1;
      unsigned int kotsu_kantsu : 4;
      unsigned int : 7;
    };

    struct {
      unsigned int : 1;
      unsigned int jantou_kotsu_kantsu : 5;
      unsigned int : 6;
    };
    struct {
      unsigned int fu : 5;
      unsigned int : 7;
    };

    unsigned int all : 12;
  };

  struct Table {
    int wind2;
    int kyoku;
    int honba;
    int kyotaku;
    std::vector<int> dora;
    std::vector<int> field;
    std::vector<int> reds;
    std::vector<int> discards;

    Table()
        : dora(K, 0),
          field(K, 0),
          reds(K, 0) {}
    Table(const Table&) = default;
    void reset();
  };

  class Player;

  struct Wall {
    std::vector<int> wall;
    std::vector<int> reds;

    Wall() = delete;
    explicit Wall(const Table& table, bool use_red = true);
    explicit Wall(const Player& player, bool use_red = true);
  };

  class Player {
  private:
    static constexpr int LIMIT = 1 << 10;
    std::shared_ptr<Table> table;
    std::vector<int>::iterator last_discard;

  public:
    std::vector<int> hand;
    std::vector<int> open;
    std::vector<int> closed_reds;
    std::vector<int> open_reds;

  private:
    std::vector<Meld> melds;
    std::vector<int> tiles;
    std::vector<bool> discards;

  public:
    bool called_riichi = false;
    bool tsumo = false;
    int fulo_count = 0;
    int wind1 = 0;
    int init = 0;
    int num = 0;

  private:
    void menzen_tsumo(int& han, int& yaku) const;
    void iipeikou(int& han, int& yaku) const;
    void sanshoku_doujun(int& han, int& yaku) const;
    void ikkitsuukan(int& han, int& yaku) const;
    void sanshoku_doukou(int& han, int& yaku) const;
    void suuankou(int& han, int& yaku) const;
    void suukantsu(int& han, int& yaku) const;
    void toitoi(int& han, int& yaku) const;
    void daisangen(int& han, int& yaku) const;
    void yakuhai(int& han, int& yaku) const;
    void tanyaochuu(const std::vector<int>& t, int& han, int& yaku) const;
    void chinroutou(const std::vector<int>& t, int& han, int& yaku) const;
    void chiniisou(const std::vector<int>& t, int& han, int& yaku) const;
    void junchantaiyao(int& han, int& yaku) const;
    void daisuushii(int& han, int& yaku) const;
    void ryuuiisou(int& han, int& yaku) const;
    void chuurenpoutou(int& han, int& yaku) const;
    int consume(const Tiles& consumed, bool flag = true);
    bool is_wh0(const int* t, Meld* u) const;
    bool is_wh2(int* t, Meld* u, int r);
    void calc_fu(const std::vector<int>& t, int& fu, int& han, int& yaku, int tile);
    void calc_dora(const std::vector<int>& t, const std::vector<int>& u, int& han) const;
    int less_than_2000(int base) const;
    int greater_than_2000(int fan) const;
    int score(int hu, int fan) const;
    void calc_lh(int& fu, int& han, int& yaku, int tile);
    void calc_sp(int& fu, int& han, int& yaku) const;
    void calc_to(int& fu, int& han, int& yaku) const;

  public:
    explicit Player(const std::shared_ptr<Table> table_)
        : table(table_),
          last_discard(table_->discards.begin()),
          hand(K, 0),
          open(K, 0),
          closed_reds(K, 0),
          open_reds(K, 0),
          melds(K, Meld{}),
          tiles(K, 0),
          discards(K, false) {}
    Player(const Player&) = default;
    Player& operator=(const Player&) = default;
    static constexpr int limit() { return LIMIT; }
    bool fulo() const { return fulo_count > 0; }
    void reset();
    void draw(const Tile& tile);
    void discard(const Tile& tile);
    void call_pong(const Tile& tile, const Tiles& consumed, bool flag = true);
    void call_chii(const Tile& tile, const Tiles& consumed, bool flag = true);
    void call_ankang(const Tiles& consumed, bool flag = true);
    void call_daiminkang(const Tile& tile, const Tiles& consumed, bool flag = true);
    void call_kakang(const Tile& tile, bool flag = true);
    void add_minkou(const Tiles& tiles);
    void add_minshun(const Tiles& tiles);
    void add_ankang(const Tiles& tiles);
    void add_minkang(const Tiles& tiles);
    bool is_lh();
    bool is_sp() const;
    bool is_to() const;
    std::tuple<int, int, int, int> calc_score(int mode, int tile);
    std::vector<bool> safe_tiles() const;
    friend Wall::Wall(const Player& player, bool use_red);
  };

}

#endif
