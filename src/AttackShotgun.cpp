#include "Attack.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Postmortem.h"

void Attack::shotgun(Actor& attacker, const Weapon& wpn, const Pos& aimPos) {

  RangedAttackData* data = new RangedAttackData(
    attacker, wpn, aimPos, attacker.pos, eng);

  printRangedInitiateMessages(*data);

  const ActorSizes_t intendedAimLevel = data->intendedAimLevel;

  //Add blocking features to array
  bool featureBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeShootBlockerFeaturesArray(featureBlockers);

  //An array of actors
  Actor* actorArray[MAP_X_CELLS][MAP_Y_CELLS];
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      actorArray[x][y] = NULL;
    }
  }
  const unsigned int ACTOR_LOOP_SIZE = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < ACTOR_LOOP_SIZE; i++) {
    Actor* curActor = eng->gameTime->getActorAt(i);
    if(curActor->deadState == actorDeadState_alive) {
      actorArray[curActor->pos.x][curActor->pos.y] = curActor;
    }
  }

  const Pos origin = attacker.pos;
  vector<Pos> path;
  eng->mapTests->getLine(origin, aimPos, false, 9999, path);

  int nrActorsHit = 0;

  int monsterKilledInElement = -1;

  for(unsigned int i = 1; i < path.size(); i++) {
    //If travelled further than two steps after a killed monster, stop projectile.
    if(monsterKilledInElement != -1) {
      if(i > static_cast<unsigned int>(monsterKilledInElement + 1)) {
        break;
      }
    }

    const Pos curPos(path.at(i));

    if(actorArray[curPos.x][curPos.y] != NULL) {

      //Only attempt hit if aiming at a level that would hit the actor
      const ActorSizes_t sizeOfActor =
        actorArray[curPos.x][curPos.y]->getData()->actorSize;
      if(sizeOfActor >= actorSize_humanoid || curPos == aimPos) {

        //Actor hit?
        delete data;
        data = new RangedAttackData(
          attacker, wpn, aimPos, curPos, eng, intendedAimLevel);
        const bool IS_WITHIN_RANGE_LMT =
          eng->basicUtils->chebyshevDistance(origin, curPos) <=
          wpn.effectiveRangeLimit;
        if(
          IS_WITHIN_RANGE_LMT &&
          data->attackResult >= successSmall &&
          data->isEtherealDefenderMissed == false) {
          if(eng->map->playerVision[curPos.x][curPos.y]) {
            eng->renderer->drawMapAndInterface(false);
            eng->renderer->coverCellInMap(curPos);
            if(eng->config->isTilesMode) {
              eng->renderer->drawTile(
                tile_blastAnimation2, panel_map, curPos, clrRedLgt);
            } else {
              eng->renderer->drawGlyph('*', panel_map, curPos, clrRedLgt);
            }
            eng->renderer->updateScreen();
            eng->sleep(eng->config->delayShotgun);
          }

          //Messages
          printProjectileAtActorMessages(*data, true);

          //Damage
          data->currentDefender->hit(data->dmg, wpn.getData().rangedDmgType);

          nrActorsHit++;

          eng->renderer->drawMapAndInterface();

          //Special shotgun behavior:
          //If current defender was killed, and player aimed at humanoid level, or at floor level
          //but beyond the current position, the shot will continue one cell.
          const bool IS_TARGET_KILLED =
            data->currentDefender->deadState != actorDeadState_alive;
          if(IS_TARGET_KILLED && monsterKilledInElement == -1) {
            monsterKilledInElement = i;
          }
          if(
            (nrActorsHit >= 2) ||
            (IS_TARGET_KILLED == false) ||
            (intendedAimLevel == actorSize_floor && curPos == aimPos)) {
            break;
          }
        }
      }
    }

    //Wall hit?
    if(featureBlockers[curPos.x][curPos.y]) {
      if(eng->map->playerVision[curPos.x][curPos.y]) {
        eng->renderer->drawMapAndInterface(false);
        eng->renderer->coverCellInMap(curPos);
        if(eng->config->isTilesMode) {
          eng->renderer->drawTile(
            tile_blastAnimation2, panel_map, curPos, clrYellow);
        } else {
          eng->renderer->drawGlyph('*', panel_map, curPos, clrYellow);
        }
        eng->renderer->updateScreen();
        eng->sleep(eng->config->delayShotgun);
        eng->renderer->drawMapAndInterface();
      }
      break;
    }

    //Floor hit?
    if(intendedAimLevel == actorSize_floor && curPos == aimPos) {
      if(eng->map->playerVision[curPos.x][curPos.y]) {
        eng->renderer->drawMapAndInterface(false);
        eng->renderer->coverCellInMap(curPos);
        if(eng->config->isTilesMode) {
          eng->renderer->drawTile(
            tile_blastAnimation2, panel_map, curPos, clrYellow);
        } else {
          eng->renderer->drawGlyph('*', panel_map, curPos, clrYellow);
        }
        eng->renderer->updateScreen();
        eng->sleep(eng->config->delayShotgun);
        eng->renderer->drawMapAndInterface();
      }
      break;
    }
  }
}

