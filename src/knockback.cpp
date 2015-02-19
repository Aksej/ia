#include "Knockback.h"

#include <algorithm>
#include <vector>

#include "Attack.h"
#include "Actor_player.h"
#include "Map.h"
#include "Log.h"
#include "Config.h"
#include "Game_time.h"
#include "Render.h"
#include "Map_parsing.h"
#include "Sdl_wrapper.h"
#include "Feature_rigid.h"
#include "Feature_mob.h"

using namespace std;

namespace Knock_back
{

void try_knock_back(Actor&        defender,
                  const Pos&    attacked_from_pos,
                  const bool    IS_SPIKE_GUN,
                  const bool    IS_MSG_ALLOWED)
{
    const bool  IS_DEF_MON    = !defender.is_player();
    const auto& defender_data  = defender.get_data();

    bool props[size_t(Prop_id::END)];
    defender.get_prop_handler().get_prop_ids(props);

    if (
        defender_data.prevent_knockback               ||
        defender_data.actor_size >= Actor_size::giant  ||
        props[int(Prop_id::ethereal)]                ||
        props[int(Prop_id::ooze)]                    ||
        /*Do not knock back if bot is playing*/
        (!IS_DEF_MON && Config::is_bot_playing()))
    {
        //Defender is not knockable
        return;
    }

    const Pos d = (defender.pos - attacked_from_pos).get_signs();

    const int KNOCK_RANGE = 2;

    for (int i = 0; i < KNOCK_RANGE; ++i)
    {
        const Pos new_pos = defender.pos + d;

        bool blocked[MAP_W][MAP_H];
        Map_parse::run(Cell_check::Blocks_actor(defender, true), blocked);

        const bool IS_CELL_BOTTOMLESS =
            Map::cells[new_pos.x][new_pos.y].rigid->is_bottomless();

        const bool IS_CELL_BLOCKED    =
            blocked[new_pos.x][new_pos.y] && !IS_CELL_BOTTOMLESS;

        if (IS_CELL_BLOCKED)
        {
            //Defender nailed to a wall from a spike gun?
            if (IS_SPIKE_GUN)
            {
                Rigid* const f = Map::cells[new_pos.x][new_pos.y].rigid;
                if (!f->is_los_passable())
                {
                    defender.get_prop_handler().try_apply_prop(
                        new Prop_nailed(Prop_turns::indefinite));
                }
            }
            return;
        }
        else //Target cell is free
        {
            const bool IS_PLAYER_SEE_DEF = IS_DEF_MON ?
                                           Map::player->can_see_actor(defender, nullptr) :
                                           true;

            if (i == 0)
            {
                if (IS_MSG_ALLOWED)
                {
                    if (IS_DEF_MON && IS_PLAYER_SEE_DEF)
                    {
                        Log::add_msg(defender.get_name_the() + " is knocked back!");
                    }
                    else
                    {
                        Log::add_msg("I am knocked back!");
                    }
                }
                defender.get_prop_handler().try_apply_prop(
                    new Prop_paralyzed(Prop_turns::specific, 1), false, false);
            }

            defender.pos = new_pos;

            if (i == KNOCK_RANGE - 1)
            {
                Render::draw_map_and_interface();
                Sdl_wrapper::sleep(Config::get_delay_projectile_draw());
            }

            if (IS_CELL_BOTTOMLESS && !props[int(Prop_id::flying)])
            {
                if (IS_DEF_MON && IS_PLAYER_SEE_DEF)
                {
                    Log::add_msg(defender.get_name_the() + " plummets down the depths.",
                                clr_msg_good);
                }
                else
                {
                    Log::add_msg("I plummet down the depths!", clr_msg_bad);
                }
                defender.die(true, false, false);
                return;
            }

            // Bump features (e.g. so monsters can be knocked back into traps)
            vector<Mob*> mobs;
            Game_time::get_mobs_at_pos(defender.pos, mobs);
            for (Mob* const mob : mobs)
            {
                mob->bump(defender);
            }

            if (!defender.is_alive())
            {
                return;
            }

            Rigid* const f = Map::cells[defender.pos.x][defender.pos.y].rigid;
            f->bump(defender);

            if (!defender.is_alive())
            {
                return;
            }
        }
    }
}

} //Knock_back