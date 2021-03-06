#include "init.hpp"

#include "bot.hpp"

#include <algorithm>
#include <vector>

#include "properties.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "feature.hpp"
#include "input.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "actor_factory.hpp"
#include "attack.hpp"
#include "feature_door.hpp"
#include "inventory.hpp"
#include "actor_mon.hpp"
#include "map_parsing.hpp"
#include "game_time.hpp"
#include "map_travel.hpp"
#include "explosion.hpp"
#include "render.hpp"
#include "sdl_wrapper.hpp"

namespace bot
{

namespace
{

std::vector<P> path_;

void show_map_and_freeze(const std::string& msg)
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Cell& cell = map::cells[x][y];

            cell.is_explored = true;
            cell.is_seen_by_player = true;
        }
    }

    for (Actor* const actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            Mon* const mon = static_cast<Mon*>(actor);

            mon->player_aware_of_me_counter_ = 999;
        }
    }

    while (true)
    {
        render::draw_map_state(Update_screen::no);

        render::draw_text("[" + msg + "]",
                          Panel::screen,
                          P(0, 0),
                          clr_red_lgt);

        render::update_screen();

        sdl_wrapper::sleep(1);

        sdl_wrapper::flush_input();
    }
}

void find_stair_path()
{
    path_.clear();

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    P stair_p(-1, -1);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const auto id = map::cells[x][y].rigid->id();

            if (id == Feature_id::stairs)
            {
                blocked[x][y] = false;
                stair_p.set(x, y);
            }
            else if (id == Feature_id::door)
            {
                blocked[x][y] = false;
            }
        }
    }

    if (stair_p.x == -1)
    {
        show_map_and_freeze("Could not find stairs");
    }

    const P& player_p = map::player->pos;

    if (blocked[player_p.x][player_p.y])
    {
        show_map_and_freeze("Player on blocked position");
    }

    path_find::run(player_p,
                   stair_p,
                   blocked,
                   path_);

    if (path_.empty())
    {
        show_map_and_freeze("Could not find path to stairs");
    }

    ASSERT(path_.front() == stair_p);
}

bool walk_to_adj_cell(const P& p)
{
    ASSERT(is_pos_adj(map::player->pos, p, true));

    char key = '0' + (int)dir_utils::dir(p - map::player->pos);

    //Occasionally randomize movement
    if (rnd::one_in(5))
    {
        key = '0' + rnd::range(1, 9);
    }

    input::handle_map_mode_key_press(Key_data(key));

    return map::player->pos == p;
}

} //namespace

void init()
{
    path_.clear();
}

void act()
{
    //=======================================================================
    // TESTS
    //=======================================================================
#ifndef NDEBUG
    for (size_t outer_idx = 0; outer_idx < game_time::actors.size(); ++outer_idx)
    {
        const Actor* const actor = game_time::actors[outer_idx];

        ASSERT(map::is_pos_inside_map(actor->pos));

        for (size_t inner_idx = 0; inner_idx < game_time::actors.size(); ++inner_idx)
        {
            const Actor* const other_actor = game_time::actors[inner_idx];

            if (
                outer_idx != inner_idx  &&
                actor->is_alive()       &&
                other_actor->is_alive())
            {
                if (actor == other_actor)
                {
                    show_map_and_freeze("Same actor encountered twice in list");
                }

                if (actor->pos == other_actor->pos)
                {
                    show_map_and_freeze("Two living actors at same pos (" +
                                        to_str(actor->pos.x) + ", " +
                                        to_str(actor->pos.y) + ")");
                }
            }
        }
    }
#endif
    //=======================================================================

    //Abort?
    //TODO: Reimplement this
//    if(input::is_key_held(SDLK_ESCAPE))
//    {
//        config::toggle_bot_playing();
//    }

    //Check if we are finished with the current run, if so, go back to DLVL 1
    if (map::dlvl >= DLVL_LAST)
    {
        TRACE << "Starting new run on first dungeon level" << std::endl;
        map_travel::init();
        map::dlvl = 1;
        return;
    }

    Prop_handler& prop_handler = map::player->prop_handler();

    //Keep an allied Mi-go around to help getting out of sticky situations
    bool has_allied_mon = false;

    for (const Actor* const actor : game_time::actors)
    {
        if (map::player->is_leader_of(actor))
        {
            has_allied_mon = true;
            break;
        }
    }

    if (!has_allied_mon)
    {
        actor_factory::summon(map::player->pos,
                              std::vector<Actor_id>(1, Actor_id::mi_go),
                              Make_mon_aware::yes,
                              map::player);
    }

    //Occasionally apply rFear (to avoid getting stuck on fear-causing monsters)
    if (rnd::one_in(7))
    {
        prop_handler.try_add(new Prop_rFear(Prop_turns::specific, 4));
    }

    //Occasionally apply Burning to a random actor (helps to avoid getting stuck)
    if (rnd::one_in(10))
    {
        const int ELEMENT = rnd::range(0, game_time::actors.size() - 1);
        Actor* const actor = game_time::actors[ELEMENT];

        if (actor != map::player)
        {
            actor->prop_handler().try_add(new Prop_burning(Prop_turns::std));
        }
    }

    //Occasionally teleport (to avoid getting stuck)
    if (rnd::one_in(200))
    {
        map::player->teleport();
    }

    //Occasionally send a TAB command to attack nearby monsters
    if (rnd::coin_toss())
    {
        input::handle_map_mode_key_press(Key_data(SDLK_TAB));
        return;
    }

    //Occasionally send a 'wait 5 turns' command (just code exercise)
    if (rnd::one_in(50))
    {
        input::handle_map_mode_key_press(Key_data('s'));
        return;
    }

    //Occasionally apply a random property to exercise the prop code
    if (rnd::one_in(20))
    {
        std::vector<Prop_id> prop_bucket;

        for (size_t i = 0; i < (size_t)Prop_id::END; ++i)
        {
            if (prop_data::data[i].allow_test_on_bot)
            {
                prop_bucket.push_back(Prop_id(i));
            }
        }

        Prop_id     prop_id = prop_bucket[rnd::range(0, prop_bucket.size() - 1)];
        Prop* const prop    = prop_handler.mk_prop(prop_id, Prop_turns::specific, 5);

        prop_handler.try_add(prop);
    }

    //Occasionally swap weapon (just some code exercise)
    if (rnd::one_in(50))
    {
        input::handle_map_mode_key_press(Key_data('z'));
        return;
    }

    //Occasionally cause shock spikes
    if (rnd::one_in(100))
    {
        map::player->incr_shock(200, Shock_src::misc);
        return;
    }

    //Occasionally run an explosion around the player
    if (rnd::one_in(50))
    {
        explosion::run(map::player->pos, Expl_type::expl);

        return;
    }

    //Handle blocking door
    for (const P& d : dir_utils::dir_list)
    {
        const P p(map::player->pos + d);

        auto* const f = map::cells[p.x][p.y].rigid;

        if (f->id() == Feature_id::door)
        {
            Door* const door = static_cast<Door*>(f);
            door->reveal(false);

            if (door->is_stuck())
            {
                f->hit(Dmg_type::physical, Dmg_method::kick, map::player);
                return;
            }
        }
    }

    //If we are terrified, wait in place
    if (map::player->has_prop(Prop_id::terrified))
    {
        if (walk_to_adj_cell(map::player->pos))
        {
            return;
        }
    }

    find_stair_path();

    walk_to_adj_cell(path_.back());
}

} //bot
