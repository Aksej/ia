#ifndef ART_H
#define ART_H

const int FONT_SHEET_X_CELLS = 16;
const int FONT_SHEET_Y_CELLS = 7;
const int TILE_SHEET_X_CELLS = 21;
const int TILE_SHEET_Y_CELLS = 13;

enum class Tile_id
{
    empty,
    player_firearm,
    player_melee,
    zombie_unarmed,
    zombie_armed,
    zombie_bloated,
    cultist_firearm,
    cultist_dagger,
    witch_or_warlock,
    ghoul,
    mummy,
    deep_one,
    shadow,
    leng_elder,
    fiend,
    ape,
    croc_head_mummy,
    the_high_priest,
    floating_head,
    crawling_hand,
    crawling_intestines,
    raven,
    armor,
    potion,
    ammo,
    scroll,
    elder_sign,
    chest_closed,
    chest_open,
    amulet,
    ring,
    lantern,
    medical_bag,
    mask,
    rat,
    spider,
    wolf,
    phantasm,
    rat_thing,
    hound,
    bat,
    byakhee,
    mass_of_worms,
    ooze,
    gas_spore,
    polyp,
    fungi,
    vortex,
    ghost,
    wraith,
    mantis,
    locust,
    mi_go_armor,
    pistol,
    tommy_gun,
    shotgun,
    dynamite,
    dynamite_lit,
    molotov,
    incinerator,
    mi_go_gun,
    flare,
    flare_gun,
    flare_lit,
    dagger,
    crowbar,
    axe,
    club,
    hammer,
    machete,
    pitchfork,
    sledge_hammer,
    rock,
    iron_spike,
    pharaoh_staff,
    lockpick,
    hunting_horror,
    spider_leng,
    chthonian,
    mi_go,
    floor,
    aim_marker_head,
    aim_marker_trail,
    wall_top,
    wall_front_alt1,
    wall_front_alt2,
    wall_front,
    square_checkered,
    rubble_high,
    rubble_low,
    stairs_down,
    lever_left,
    lever_right,
    device1,
    device2,
    cabinet_closed,
    cabinet_open,
    pillar_broken,
    pillar,
    pillar_carved,
    barrel,
    sarcophagus,
    cave_wall_front,
    cave_wall_top,
    egypt_wall_front,
    egypt_wall_top,
    monolith,
    brazier,
    altar,
    web,
    door_closed,
    door_open,
    door_broken,
    fountain,
    tree,
    bush,
    church_bench,
    grave_stone,
    tomb_open,
    tomb_closed,
    water1,
    water2,
    trap_general,
    cocoon_open,
    cocoon_closed,
    blast1,
    blast2,
    corpse,
    corpse2,
    projectile_std_front_slash,
    projectile_std_back_slash,
    projectile_std_dash,
    projectile_std_vertical_bar,
    gore1,
    gore2,
    gore3,
    gore4,
    gore5,
    gore6,
    gore7,
    gore8,
    smoke,
    trapezohedron,
    pit,
    popup_top_l,
    popup_top_r,
    popup_btm_l,
    popup_btm_r,
    popup_hor,
    popup_ver,
    hangbridge_ver,
    hangbridge_hor,
    scorched_ground,
    popup_hor_down,
    popup_hor_up,
    popup_ver_r,
    popup_ver_l,
    stalagmite,
    heart,
    brain,
    weight,
    spirit,
    stopwatch,
};


struct Pos;

namespace art
{

Pos glyph_pos(const char glyph);
Pos tile_pos(const Tile_id tile);

} //Art

#endif
