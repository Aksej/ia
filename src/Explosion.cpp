#include "Explosion.h"

#include "FeatureSmoke.h"
#include "Renderer.h"
#include "Map.h"
#include "Log.h"
#include "MapParsing.h"
#include "SdlWrapper.h"
#include "LineCalc.h"
#include "ActorPlayer.h"
#include "Utils.h"
#include "SdlWrapper.h"
#include "PlayerBon.h"

namespace {

void draw(const vector< vector<Pos> >& posLists, bool blockers[MAP_W][MAP_H],
          const SDL_Color* const clrOverride) {
  Renderer::drawMapAndInterface();

  const SDL_Color& clrInner = clrOverride == NULL ? clrYellow : *clrOverride;
  const SDL_Color& clrOuter = clrOverride == NULL ? clrRedLgt : *clrOverride;

  const bool IS_TILES     = Config::isTilesMode();
  const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

  bool isAnyCellSeenByPlayer = false;

  for(int iAnim = 0; iAnim < NR_ANIM_STEPS; iAnim++) {

    const TileId tile = iAnim == 0 ? tile_blast1 : tile_blast2;

    const int NR_OUTER = posLists.size();
    for(int iOuter = 0; iOuter < NR_OUTER; iOuter++) {
      const SDL_Color& clr = iOuter == NR_OUTER - 1 ? clrOuter : clrInner;
      const vector<Pos>& inner = posLists.at(iOuter);
      for(const Pos & pos : inner) {
        if(
          Map::cells[pos.x][pos.y].isSeenByPlayer &&
          blockers[pos.x][pos.y] == false) {
          isAnyCellSeenByPlayer = true;
          if(IS_TILES) {
            Renderer::drawTile(tile, Panel::map, pos, clr, clrBlack);
          } else {
            Renderer::drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
          }
        }
      }
    }
    if(isAnyCellSeenByPlayer) {
      Renderer::updateScreen();
      SdlWrapper::sleep(Config::getDelayExplosion() / NR_ANIM_STEPS);
    }
  }
}

void getArea(const Pos& c, const int RADI, Rect& rectRef) {
  rectRef = Rect(Pos(max(c.x - RADI, 1),         max(c.y - RADI, 1)),
                 Pos(min(c.x + RADI, MAP_W - 2), min(c.y + RADI, MAP_H - 2)));
}

void getPositionsReached(const Rect& area, const Pos& origin,
                         bool blockers[MAP_W][MAP_H],
                         vector< vector<Pos> >& posListRef) {
  vector<Pos> line;
  for(int y = area.x0y0.y; y <= area.x1y1.y; y++) {
    for(int x = area.x0y0.x; x <= area.x1y1.x; x++) {
      const Pos pos(x, y);
      const int DIST = Utils::chebyshevDist(pos, origin);
      bool isReached = true;
      if(DIST > 1) {
        eng.lineCalc->calcNewLine(origin, pos, true, 999, false, line);
        for(Pos & posCheckBlock : line) {
          if(blockers[posCheckBlock.x][posCheckBlock.y]) {
            isReached = false;
            break;
          }
        }
      }
      if(isReached) {
        if(int(posListRef.size()) <= DIST) {posListRef.resize(DIST + 1);}
        posListRef.at(DIST).push_back(pos);
      }
    }
  }
}

} //namespace


namespace Explosion {

void runExplosionAt(const Pos& origin, const ExplType explType,
                    const ExplSrc explSrc, const int RADI_CHANGE,
                    const SfxId sfx, Prop* const prop,
                    const SDL_Color* const clrOverride) {
  Rect area;
  const int RADI = EXPLOSION_STD_RADI + RADI_CHANGE;
  getArea(origin, RADI, area);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(), blockers);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, blockers, eng, posLists);

  SndVol vol = explType == ExplType::expl ? SndVol::high : SndVol::low;

  Snd snd("I hear an explosion!", sfx, IgnoreMsgIfOriginSeen::yes, origin,
          NULL, vol, AlertsMonsters::yes);
  SndEmit::emitSnd(snd, eng);

  draw(posLists, blockers, clrOverride, eng);

  //Do damage, apply effect
  const int DMG_ROLLS = 5;
  const int DMG_SIDES = 6;
  const int DMG_PLUS  = 10;

  Actor* livingActors[MAP_W][MAP_H];
  vector<Actor*> corpses[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      livingActors[x][y] = NULL;
      corpses[x][y].resize(0);
    }
  }

  for(Actor * actor : GameTime::actors_) {
    const Pos& pos = actor->pos;
    if(actor->deadState == ActorDeadState::alive) {
      livingActors[pos.x][pos.y] = actor;
    } else if(actor->deadState == ActorDeadState::corpse) {
      corpses[pos.x][pos.y].push_back(actor);
    }
  }

  const bool IS_DEM_EXP = PlayerBon::hasTrait(Trait::demolitionExpert);

  const int NR_OUTER = posLists.size();
  for(int curRadi = 0; curRadi < NR_OUTER; curRadi++) {
    const vector<Pos>& inner = posLists.at(curRadi);

    for(const Pos & pos : inner) {

      Actor* livingActor          = livingActors[pos.x][pos.y];
      vector<Actor*> corpsesHere  = corpses[pos.x][pos.y];

      if(explType == ExplType::expl) {
        //Damage environment
        if(curRadi <= 1) {Map::switchToDestroyedFeatAt(pos);}
        const int DMG = Rnd::dice(DMG_ROLLS - curRadi, DMG_SIDES) + DMG_PLUS;

        //Damage living actor
        if(livingActor != NULL) {
          if(livingActor == Map::player) {
            eng.log->addMsg("I am hit by an explosion!", clrMsgBad);
          }
          livingActor->hit(DMG, DmgType::physical, true);
        }
        //Damage dead actors
        for(Actor * corpse : corpsesHere) {
          corpse->hit(DMG, DmgType::physical, true);
        }

        if(Rnd::fraction(6, 10)) {
          eng.featureFactory->spawnFeatureAt(
            feature_smoke, pos, new SmokeSpawnData(Rnd::range(2, 4)));
        }
      }

      //Apply property
      if(prop != NULL) {
        if(
          livingActor != NULL &&
          (livingActor != Map::player || IS_DEM_EXP == false ||
           explSrc != ExplSrc::playerUseMoltvIntended)) {
          PropHandler& propHlr = livingActor->getPropHandler();
          Prop* propCpy = propHlr.makeProp(prop->getId(), propTurnsSpecific,
                                           prop->turnsLeft_);
          propHlr.tryApplyProp(propCpy);
        }
        //If property is burning, also apply it to corpses
        if(prop->getId() == propBurning) {
          for(Actor * corpse : corpsesHere) {
            PropHandler& propHlr = corpse->getPropHandler();
            Prop* propCpy = propHlr.makeProp(prop->getId(), propTurnsSpecific,
                                             prop->turnsLeft_);
            propHlr.tryApplyProp(propCpy);
          }
        }
      }
    }
  }

  Map::player->updateFov();
  Renderer::drawMapAndInterface();

  if(prop != NULL) {delete prop;}
}

void runSmokeExplosionAt(const Pos& origin) {
  Rect area;
  const int RADI = EXPLOSION_STD_RADI;
  getArea(origin, RADI, area);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(), blockers);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, blockers, eng, posLists);

  //TODO Sound message?
  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, origin, NULL,
          SndVol::low, AlertsMonsters::yes);
  SndEmit::emitSnd(snd, eng);

  for(const vector<Pos>& inner : posLists) {
    for(const Pos & pos : inner) {
      if(blockers[pos.x][pos.y] == false) {
        eng.featureFactory->spawnFeatureAt(
          feature_smoke, pos, new SmokeSpawnData(Rnd::range(17, 22)));
      }
    }
  }

  Map::player->updateFov();
  Renderer::drawMapAndInterface();
}

} //Explosion

