#ifndef MON_H
#define MON_H

#include "Cmn_data.h"

#include "Actor.h"
#include "Sound.h"
#include "Spells.h"

struct Ai_att_data
{
    Ai_att_data(Wpn* weapon_to_use, bool is_melee_att) :
        weapon  (weapon_to_use),
        is_melee (is_melee_att) {}

    Wpn* weapon;
    bool is_melee;
};

struct Ai_avail_attacks_data
{
    Ai_avail_attacks_data() :
        is_time_to_reload  (false),
        is_melee         (true) {}

    Ai_avail_attacks_data(const Ai_avail_attacks_data& other) :
        weapons         (other.weapons),
        is_time_to_reload  (other.is_time_to_reload),
        is_melee         (other.is_melee) {}

    Ai_avail_attacks_data& operator=(const Ai_avail_attacks_data& other)
    {
        weapons         = other.weapons;
        is_time_to_reload  = other.is_time_to_reload;
        is_melee         = other.is_melee;
        return *this;
    }

    std::vector<Wpn*> weapons;
    bool is_time_to_reload, is_melee;
};

class Wpn;

class Mon: public Actor
{
public:
    Mon();
    virtual ~Mon();

    virtual void place_() override {}

    void move_dir(Dir dir);

    void        get_avail_attacks(Actor& defender, Ai_avail_attacks_data& avail_attacks_ref);
    Ai_att_data   get_att(const Ai_avail_attacks_data& avail_attacks);
    bool try_attack(Actor& defender);

    virtual void mk_start_items() override = 0;

    void hear_sound(const Snd& snd);

    void become_aware(const bool IS_FROM_SEEING);

    void player_become_aware_of_me(const int DURATION_FACTOR = 1);

    void on_actor_turn() override;

    virtual void on_std_turn() override final;

    virtual std::string get_aggro_phrase_mon_seen() const
    {
        return data_->aggro_text_mon_seen;
    }
    virtual std::string get_aggro_phrase_mon_hidden() const
    {
        return data_->aggro_text_mon_hidden;
    }
    virtual Sfx_id get_aggro_sfx_mon_seen() const
    {
        return data_->aggro_sfx_mon_seen;
    }
    virtual Sfx_id get_aggro_sfx_mon_hidden() const
    {
        return data_->aggro_sfx_mon_hidden;
    }

    void speak_phrase();

    bool is_leader_of(const Actor* const actor)       const override;
    bool is_actor_my_leader(const Actor* const actor)  const override;

    int                 aware_counter_, player_aware_of_me_counter_;
    bool                is_msg_mon_in_view_printed_;
    Dir                 last_dir_travelled_;
    std::vector<Spell*> spells_known_;
    int                 spell_cool_down_cur_;
    bool                is_roaming_allowed_;
    bool                is_stealth_;
    Actor*              leader_;
    Actor*              tgt_;
    bool                waiting_;
    double              shock_caused_cur_;
    bool                has_given_xp_for_spotting_;
    int                 nr_turns_until_unsummoned_;

protected:
    virtual void on_hit(int& dmg) override;

    virtual bool on_actor_turn_() {return false;}
    virtual void on_std_turn_() {}

    int get_group_size();
};

class Rat: public Mon
{
public:
    Rat() : Mon() {}
    ~Rat() {}
    virtual void mk_start_items() override;
};

class Rat_thing: public Rat
{
public:
    Rat_thing() : Rat() {}
    ~Rat_thing() {}
    void mk_start_items() override;
};

class Brown_jenkin: public Rat_thing
{
public:
    Brown_jenkin() : Rat_thing() {}
    ~Brown_jenkin() {}
    void mk_start_items() override;
};

class Spider: public Mon
{
public:
    Spider() : Mon() {}
    virtual ~Spider() {}
    bool on_actor_turn_() override;
};

class Green_spider: public Spider
{
public:
    Green_spider() : Spider() {}
    ~Green_spider() {}
    void mk_start_items() override;
};

class White_spider: public Spider
{
public:
    White_spider() : Spider() {}
    ~White_spider() {}
    void mk_start_items() override;
};

class Red_spider: public Spider
{
public:
    Red_spider() : Spider() {}
    ~Red_spider() {}
    void mk_start_items() override;
};

class Shadow_spider: public Spider
{
public:
    Shadow_spider() : Spider() {}
    ~Shadow_spider() {}
    void mk_start_items() override;
};

class Leng_spider: public Spider
{
public:
    Leng_spider() : Spider() {}
    ~Leng_spider() {}
    void mk_start_items() override;
};

class Zombie: public Mon
{
public:
    Zombie() : Mon()
    {
        dead_turn_counter = 0;
        has_resurrected = false;
    }
    virtual ~Zombie() {}
    virtual bool on_actor_turn_() override;
    void on_death() override;
protected:
    bool try_resurrect();
    int dead_turn_counter;
    bool has_resurrected;
};

class Zombie_claw: public Zombie
{
public:
    Zombie_claw() : Zombie() {}
    ~Zombie_claw() {}
    void mk_start_items() override;
};

class Zombie_axe: public Zombie
{
public:
    Zombie_axe() : Zombie() {}
    ~Zombie_axe() {}
    void mk_start_items() override;
};

class Bloated_zombie: public Zombie
{
public:
    Bloated_zombie() : Zombie() {}
    ~Bloated_zombie() {}

    void mk_start_items() override;
};

class Major_clapham_lee: public Zombie_claw
{
public:
    Major_clapham_lee() :
        Zombie_claw(), has_summoned_tomb_legions(false)
    {
    }
    ~Major_clapham_lee() {}

    bool on_actor_turn_() override;
private:
    bool has_summoned_tomb_legions;
};

class Dean_halsey: public Zombie_claw
{
public:
    Dean_halsey() : Zombie_claw() {}
    ~Dean_halsey() {}
};

class Keziah_mason: public Mon
{
public:
    Keziah_mason() : Mon(), has_summoned_jenkin(false) {}
    ~Keziah_mason() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
private:
    bool has_summoned_jenkin;
};

class Leng_elder: public Mon
{
public:
    Leng_elder() :
        Mon                   (),
        has_given_item_to_player_ (false),
        nr_turns_to_hostile_     (-1) {}
    ~Leng_elder() {}
    void mk_start_items() override;
private:
    void on_std_turn_()    override;
    bool  has_given_item_to_player_;
    int   nr_turns_to_hostile_;
};

class Cultist: public Mon
{
public:
    Cultist() : Mon() {}

    virtual void mk_start_items() override;

    static std::string get_cultist_phrase();

    std::string get_aggro_phrase_mon_seen() const
    {
        return get_name_the() + ": " + get_cultist_phrase();
    }
    std::string get_aggro_phrase_mon_hidden() const
    {
        return "Voice: " + get_cultist_phrase();
    }

    virtual ~Cultist() {}
};

class Cultist_electric: public Cultist
{
public:
    Cultist_electric() : Cultist() {}
    ~Cultist_electric() {}
    void mk_start_items() override;
};

class Cultist_spike_gun: public Cultist
{
public:
    Cultist_spike_gun() : Cultist() {}
    ~Cultist_spike_gun() {}
    void mk_start_items() override;
};

class Cultist_priest: public Cultist
{
public:
    Cultist_priest() : Cultist() {}
    ~Cultist_priest() {}
    void mk_start_items() override;
};

class Lord_of_shadows: public Mon
{
public:
    Lord_of_shadows() : Mon() {}
    ~Lord_of_shadows() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
};

class Lord_of_spiders: public Mon
{
public:
    Lord_of_spiders() : Mon() {}
    ~Lord_of_spiders() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
};

class Lord_of_spirits: public Mon
{
public:
    Lord_of_spirits() : Mon() {}
    ~Lord_of_spirits() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
};

class Lord_of_pestilence: public Mon
{
public:
    Lord_of_pestilence() : Mon() {}
    ~Lord_of_pestilence() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
};

class Fire_hound: public Mon
{
public:
    Fire_hound() : Mon() {}
    ~Fire_hound() {}
    void mk_start_items() override;
};

class Frost_hound: public Mon
{
public:
    Frost_hound() : Mon() {}
    ~Frost_hound() {}
    void mk_start_items() override;
};

class Zuul: public Mon
{
public:
    Zuul() : Mon() {}
    ~Zuul() {}

    void place_() override;

    void mk_start_items() override;
};

class Ghost: public Mon
{
public:
    Ghost() : Mon() {}
    ~Ghost() {}
    bool on_actor_turn_() override;
    virtual void mk_start_items() override;
};

class Phantasm: public Ghost
{
public:
    Phantasm() : Ghost() {}
    ~Phantasm() {}
    void mk_start_items() override;
};

class Wraith: public Ghost
{
public:
    Wraith() : Ghost() {}
    ~Wraith() {}
    void mk_start_items() override;
};

class Giant_bat: public Mon
{
public:
    Giant_bat() : Mon() {}
    ~Giant_bat() {}
    void mk_start_items() override;
};

class Byakhee: public Giant_bat
{
public:
    Byakhee() : Giant_bat() {}
    ~Byakhee() {}
    void mk_start_items() override;
};

class Giant_mantis: public Mon
{
public:
    Giant_mantis() : Mon() {}
    ~Giant_mantis() {}
    void mk_start_items() override;
};

class Chthonian: public Mon
{
public:
    Chthonian() : Mon() {}
    ~Chthonian() {}
    void mk_start_items() override;
};

class Hunting_horror: public Giant_bat
{
public:
    Hunting_horror() : Giant_bat() {}
    ~Hunting_horror() {}
    void mk_start_items() override;
};

class Wolf: public Mon
{
public:
    Wolf() : Mon() {}
    ~Wolf() {}
    void mk_start_items() override;
};

class Mi_go: public Mon
{
public:
    Mi_go() : Mon() {}
    ~Mi_go() {}
    void mk_start_items() override;
};

class Mi_go_commander: public Mi_go
{
public:
    Mi_go_commander() : Mi_go() {}
    ~Mi_go_commander() {}
};

class Sentry_drone: public Mon
{
public:
    Sentry_drone() : Mon() {}
    ~Sentry_drone() {}
    void mk_start_items() override;
};

class Flying_polyp: public Mon
{
public:
    Flying_polyp() : Mon() {}
    ~Flying_polyp() {}
    void mk_start_items() override;
};

class Greater_polyp: public Flying_polyp
{
public:
    Greater_polyp() : Flying_polyp() {}
    ~Greater_polyp() {}
    void mk_start_items() override;
};

class Ghoul: public Mon
{
public:
    Ghoul() : Mon() {}
    ~Ghoul() {}
    virtual void mk_start_items() override;
};

class Deep_one: public Mon
{
public:
    Deep_one() : Mon() {}
    ~Deep_one() {}
    void mk_start_items() override;
};

class Ape: public Mon
{
public:
    Ape() : Mon(), frenzy_cool_down_(0) {}
    ~Ape() {}
    void mk_start_items() override;
    bool on_actor_turn_() override;
private:
    int frenzy_cool_down_;
};

class Mummy: public Mon
{
public:
    Mummy() : Mon() {}
    ~Mummy() {}
    virtual void mk_start_items() override;

    virtual bool on_actor_turn_() override;
};

class Mummy_croc_head: public Mummy
{
public:
    Mummy_croc_head() : Mummy() {}
    ~Mummy_croc_head() {}
    virtual void mk_start_items() override;
};

class Mummy_unique: public Mummy
{
public:
    Mummy_unique() : Mummy() {}
    ~Mummy_unique() {}
    void mk_start_items() override;
};

class Khephren: public Mummy_unique
{
public:
    Khephren() :
        Mummy_unique         (),
        has_summoned_locusts  (false) {}
    ~Khephren() {}

    bool on_actor_turn_() override;
private:
    bool has_summoned_locusts;
};

class Shadow: public Mon
{
public:
    Shadow() : Mon() {}
    ~Shadow() {}

    virtual void mk_start_items() override;
};

class Worm_mass: public Mon
{
public:
    Worm_mass() : Mon(), spawn_new_one_in_n(10) {}
    ~Worm_mass() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
private:
    int spawn_new_one_in_n;
};

class Giant_locust: public Mon
{
public:
    Giant_locust() : Mon(), spawn_new_one_in_n(40) {}
    ~Giant_locust() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
private:
    int spawn_new_one_in_n;
};

class Vortex: public Mon
{
public:
    Vortex() : Mon(), pull_cooldown(0) {}
    virtual ~Vortex() {}

    bool on_actor_turn_() override;

    virtual void mk_start_items() = 0;
    virtual void on_death() = 0;
private:
    int pull_cooldown;
};

class Dust_vortex: public Vortex
{
public:
    Dust_vortex() : Vortex() {}
    ~Dust_vortex() {}
    void mk_start_items() override;
    void on_death();
};

class Fire_vortex: public Vortex
{
public:
    Fire_vortex() : Vortex() {}
    ~Fire_vortex() {}
    void mk_start_items() override;
    void on_death();
};

class Frost_vortex: public Vortex
{
public:
    Frost_vortex() : Vortex() {}
    ~Frost_vortex() {}
    void mk_start_items() override;
    void on_death();
};

class Ooze: public Mon
{
public:
    Ooze() : Mon() {}
    ~Ooze() {}
    virtual void mk_start_items() = 0;
private:
    virtual void on_std_turn_() override;
};

class Ooze_black: public Ooze
{
public:
    Ooze_black() : Ooze() {}
    ~Ooze_black() {}
    void mk_start_items() override;
};

class Ooze_clear: public Ooze
{
public:
    Ooze_clear() : Ooze() {}
    ~Ooze_clear() {}
    void mk_start_items() override;
};

class Ooze_putrid: public Ooze
{
public:
    Ooze_putrid() : Ooze() {}
    ~Ooze_putrid() {}
    void mk_start_items() override;
};

class Ooze_poison: public Ooze
{
public:
    Ooze_poison() : Ooze() {}
    ~Ooze_poison() {}
    void mk_start_items() override;
};

class Color_oOSpace: public Ooze
{
public:
    Color_oOSpace() : Ooze(),
        cur_color(clr_magenta_lgt) {}
    ~Color_oOSpace() {}
    void mk_start_items() override;
    const Clr& get_clr();
private:
    void on_std_turn_() override;
    Clr cur_color;
};

class Mold: public Mon
{
public:
    Mold() : Mon(), spawn_new_one_in_n(28) {}
    ~Mold() {}
    bool on_actor_turn_() override;
    void mk_start_items() override;
private:
    int spawn_new_one_in_n;
};

class Gas_spore: public Mon
{
public:
    Gas_spore() : Mon() {}
    ~Gas_spore() {}

    void on_death();

    void mk_start_items() override {}
};

class The_high_priest: public Mon
{
public:
    The_high_priest();
    ~The_high_priest() {}
    void mk_start_items() override;
    void on_death()      override;
    bool on_actor_turn_() override;
private:
    void on_std_turn_()   override;

    bool      has_greeted_player_;
    const int NR_TURNS_BETWEEN_CPY_;
    int       nr_turns_until_next_cpy_;
};

class The_high_priest_cpy: public Mon
{
public:
    The_high_priest_cpy() {}
    ~The_high_priest_cpy() {}
    void mk_start_items() override;
};

#endif