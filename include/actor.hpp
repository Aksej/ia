#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>

#include "Cmn_data.h"

#include "Actor_data.h"
#include "Sound.h"
#include "Config.h"
#include "Art.h"

class Prop_handler;
class Inventory;

enum class Actor_died {no, yes};

class Actor
{
public:
    Actor();
    virtual ~Actor();

    Prop_handler&        get_prop_handler()    {return *prop_handler_;}
    Actor_data_t&         get_data()           {return *data_;}
    const Actor_data_t&   get_data() const     {return *data_;}
    Inventory&          get_inv()            {return *inv_;}

    //This function is not concerned with whether the parameter actor is within
    //FOV, or if the actor is actually hidden or not. It merely tests the sneak
    //skill of the actor, and various conditions such as light/dark.
    //It has no side effects - it merely does a randomized check.
    bool is_spotting_hidden_actor(Actor& actor);

    void place(const Pos& pos_, Actor_data_t& data);
    virtual void place_() {}

    Actor_died hit(int dmg, const Dmg_type dmg_type,
                  const Dmg_method method = Dmg_method::END);
    Actor_died hit_spi(const int DMG, const bool ALLOW_MSG);

    bool restore_hp   (const int HP_RESTORED, const bool ALLOW_MSG = true,
                      const bool IS_ALLOWED_ABOVE_MAX = false);
    bool restore_spi  (const int SPI_RESTORED, const bool ALLOW_MSG = true,
                      const bool IS_ALLOWED_ABOVE_MAX = false);
    void change_max_hp (const int CHANGE, const bool ALLOW_MSG);
    void change_max_spi(const int CHANGE, const bool ALLOW_MSG);

    void die(const bool IS_DESTROYED, const bool ALLOW_GORE,
             const bool ALLOW_DROP_ITEMS);

    virtual void on_actor_turn() {}
    virtual void on_std_turn() {}

    virtual void move_dir(Dir dir) = 0;

    virtual void update_clr();

    //Function taking into account FOV, invisibility, status, etc
    //This is the final word on whether an actor can visually perceive
    //another actor.
    bool can_see_actor(const Actor& other, const bool blocked_los[MAP_W][MAP_H]) const;

    void get_seen_foes(std::vector<Actor*>& out);

    Actor_id     get_id()                             const {return data_->id;}
    int         get_hp()                             const {return hp_;}
    int         get_spi()                            const {return spi_;}
    int         get_hp_max(const bool WITH_MODIFIERS) const;
    int         get_spi_max()                         const {return spi_max_;}
    Actor_speed  get_speed()                          const;

    std::string get_name_the()        const   {return data_->name_the;}
    std::string get_name_a()          const   {return data_->name_a;}
    std::string get_corpse_name_a()    const   {return data_->corpse_name_a;}
    std::string get_corpse_name_the()  const   {return data_->corpse_name_the;}
    bool is_humanoid()               const   {return data_->is_humanoid;}
    char get_glyph()                 const   {return glyph_;}
    virtual const Clr& get_clr()             {return clr_;}
    const Tile_id& get_tile()         const   {return tile_;}

    void add_light(bool light_map[MAP_W][MAP_H]) const;

    virtual void add_light_(bool light[MAP_W][MAP_H]) const {(void)light;}

    void teleport();

    bool       is_alive ()  const {return state_ == Actor_state::alive;}
    bool       is_corpse()  const {return state_ == Actor_state::corpse;}
    Actor_state get_state()  const {return state_;}

    virtual bool is_leader_of(const Actor* const actor)       const = 0;
    virtual bool is_actor_my_leader(const Actor* const actor)  const = 0;

    bool is_player() const;

    Pos pos;

protected:
    //TODO: Try to get rid of these friend declarations
    friend class Ability_vals;
    friend class Prop_diseased;
    friend class Prop_poss_by_zuul;
    friend class Trap;

    virtual void on_death() {}
    virtual void on_hit(int& dmg) {(void)dmg;}
    virtual void mk_start_items() = 0;

    Actor_state  state_;
    Clr         clr_;
    char        glyph_;
    Tile_id      tile_;

    int hp_, hp_max_, spi_, spi_max_;

    Pos lair_cell_;

    Prop_handler*  prop_handler_;
    Actor_data_t*   data_;
    Inventory*    inv_;
};

#endif