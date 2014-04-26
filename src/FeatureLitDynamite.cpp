#include "FeatureLitDynamite.h"

#include "Explosion.h"
#include "Map.h"
#include "Fov.h"
#include "PlayerBon.h"
#include "Utils.h"

void LitDynamite::newTurn() {
  turnsLeftToExplosion_--;
  if(turnsLeftToExplosion_ <= 0) {
    const int D_R =
      PlayerBon::hasTrait(Trait::demolitionExpert) ? 1 : 0;
    Explosion::runExplosionAt(pos_, eng, ExplType::expl, ExplSrc::misc, D_R);
    GameTime::eraseFeatureMob(this, true);
  }
}

void LitFlare::newTurn() {
  life_--;
  if(life_ <= 0) {GameTime::eraseFeatureMob(this, true);}
}

LitFlare::LitFlare(FeatureId id, Pos pos,
                   DynamiteSpawnData* spawnData) :
  FeatureMob(id, pos), life_(spawnData->turnsLeftToExplosion_) {
}

void LitFlare::addLight(bool light[MAP_W][MAP_H]) const {
  bool myLight[MAP_W][MAP_H];
  Utils::resetArray(myLight, false);
  const int RADI = FOV_STD_RADI_INT; //getLightRadius();
  Pos x0y0(max(0, pos_.x - RADI), max(0, pos_.y - RADI));
  Pos x1y1(min(MAP_W - 1, pos_.x + RADI),
           min(MAP_H - 1, pos_.y + RADI));
  bool visionBlockers[MAP_W][MAP_H];
  for(int y = x0y0.y; y <= x1y1.y; y++) {
    for(int x = x0y0.x; x <= x1y1.x; x++) {
      visionBlockers[x][y] =
        Map::cells[x][y].featureStatic->isVisionPassable() == false;
    }
  }

  Fov::runFovOnArray(visionBlockers, pos_, myLight, false);
  for(int y = x0y0.y; y <= x1y1.y; y++) {
    for(int x = x0y0.x; x <= x1y1.x; x++) {
      if(myLight[x][y]) {
        light[x][y] = true;
      }
    }
  }
}



