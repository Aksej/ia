#include "Init.h"

#include "Map.h"

#include "Feature.h"
#include "Actor_factory.h"
#include "Item_factory.h"
#include "Game_time.h"
#include "Render.h"
#include "Map_gen.h"
#include "Item.h"
#include "Utils.h"
#include "Feature_rigid.h"
#ifdef DEMO_MODE
#include "Sdl_wrapper.h"
#endif // DEMO_MODE

using namespace std;

Cell::Cell() :
    is_explored          (false),
    is_seen_by_player      (false),
    is_lit               (false),
    is_dark              (false),
    item                (nullptr),
    rigid               (nullptr),
    player_visual_memory  (Cell_render_data()),
    pos                 (Pos(-1, -1)) {}

Cell::~Cell()
{
    if (rigid) {delete rigid;}
    if (item)  {delete item;}
}

void Cell::reset()
{
    is_explored = is_seen_by_player = is_lit = is_dark = false;

    player_visual_memory = Cell_render_data();

    pos.set(-1, -1);

    if (rigid)
    {
        delete rigid;
        rigid = nullptr;
    }

    if (item)
    {
        delete item;
        item = nullptr;
    }
}

namespace Map
{

Player*       player  = nullptr;
int           dlvl    = 0;
Cell          cells[MAP_W][MAP_H];
vector<Room*> room_list;
Room*         room_map[MAP_W][MAP_H];

namespace
{

void reset_cells(const bool MAKE_STONE_WALLS)
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            cells[x][y].reset();
            cells[x][y].pos = Pos(x, y);

            room_map[x][y]   = nullptr;

            Render::render_array[x][y]         = Cell_render_data();
            Render::render_array_no_actors[x][y] = Cell_render_data();

            if (MAKE_STONE_WALLS) {put(new Wall(Pos(x, y)));}
        }
    }
}

} //Namespace

void init()
{
    dlvl = 0;

    room_list.clear();

    reset_cells(false);

    if (player) {delete player; player = nullptr;}

    const Pos player_pos(PLAYER_START_X, PLAYER_START_Y);
    player = static_cast<Player*>(Actor_factory::mk(Actor_id::player, player_pos));

    Actor_factory::delete_all_mon();

    Game_time::erase_all_mobs();
    Game_time::reset_turn_type_and_actor_counters();
}

void cleanup()
{
    player = nullptr; //NOTE: Game_time has deleted player at this point

    reset_map();

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            delete cells[x][y].rigid;
            cells[x][y].rigid = nullptr;
        }
    }
}

void store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(dlvl));
}

void setup_from_save_lines(vector<string>& lines)
{
    dlvl = to_int(lines.front());
    lines.erase(begin(lines));
}

void reset_map()
{
    Actor_factory::delete_all_mon();

    for (auto* room : room_list) {delete room;}
    room_list.clear();

    reset_cells(true);
    Game_time::erase_all_mobs();
    Game_time::reset_turn_type_and_actor_counters();
}

Rigid* put(Rigid* const f)
{
    assert(f);

    const Pos p     = f->get_pos();
    Cell&     cell  = cells[p.x][p.y];

    delete cell.rigid;

    cell.rigid = f;

#ifdef DEMO_MODE
    if (f->get_id() == Feature_id::floor)
    {
        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                Map::cells[x][y].is_seen_by_player = Map::cells[x][y].is_explored = true;
            }
        }
        Render::draw_map();
        Render::draw_glyph('X', Panel::map, p, clr_yellow);
        Render::update_screen();
        Sdl_wrapper::sleep(10); //NOTE: Delay must be > 1 for user input to be read
    }
#endif // DEMO_MODE

    return f;
}

void update_visual_memory()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            cells[x][y].player_visual_memory = Render::render_array_no_actors[x][y];
        }
    }
}

void mk_blood(const Pos& origin)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const Pos c = origin + Pos(dx, dy);
            Rigid* const f  = cells[c.x][c.y].rigid;
            if (f->can_have_blood())
            {
                if (Rnd::one_in(3)) {f->mk_bloody();}
            }
        }
    }
}

void mk_gore(const Pos& origin)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const Pos c = origin + Pos(dx, dy);
            if (Rnd::one_in(3)) {cells[c.x][c.y].rigid->try_put_gore();}
        }
    }
}

void delete_and_remove_room_from_list(Room* const room)
{
    for (size_t i = 0; i < room_list.size(); ++i)
    {
        if (room_list[i] == room)
        {
            delete room;
            room_list.erase(room_list.begin() + i);
            return;
        }
    }
    assert(false && "Tried to remove non-existing room");
}

bool is_pos_seen_by_player(const Pos& p)
{
    assert(Utils::is_pos_inside_map(p));
    return cells[p.x][p.y].is_seen_by_player;
}

} //Map