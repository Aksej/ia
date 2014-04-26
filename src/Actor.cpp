#include "Actor.h"

#include "Renderer.h"
#include "ItemArmor.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Fov.h"
#include "Log.h"
#include "Blood.h"
#include "FeatureTrap.h"
#include "ItemDrop.h"
#include "Explosion.h"
#include "DungeonMaster.h"
#include "Inventory.h"
#include "MapParsing.h"

using namespace std;

Actor::Actor() :
  pos(),
  deadState(ActorDeadState::alive),
  clr_(clrBlack),
  glyph_(' '),
  tile_(tile_empty),
  hp_(-1),
  spi_(-1),
  lairCell_(),
  propHandler_(NULL),
  data_(NULL),
  inv_(NULL) {}

Actor::~Actor() {
  delete propHandler_;
  delete inv_;
}

bool Actor::isSpottingHiddenActor(Actor& other) {
  const Pos& otherPos = other.pos;

  int playerBon = 0;
  if(this == Map::player) {
    playerBon = data_->abilityVals.getVal(AbilityId::searching, true, *this) / 3;
  }

  const int SNEAK_BASE = other.getData().abilityVals.getVal(
                           AbilityId::stealth, true, other);

  const int  DIST     = Utils::chebyshevDist(pos, otherPos);
  const int  DIST_BON = getConstrInRange(0, (DIST - 1) * 10, 60);
  const int  LGT_DIV  = Map::cells[otherPos.x][otherPos.y].isLight ? 2 : 1;
  const int  SKILL =
    getConstrInRange(0, (SNEAK_BASE + DIST_BON - playerBon) / LGT_DIV, 90);

  return AbilityRoll::roll(SKILL) <= failSmall;
}

int Actor::getHpMax(const bool WITH_MODIFIERS) const {
  return WITH_MODIFIERS ? propHandler_->getChangedMaxHp(hpMax_) : hpMax_;
}

bool Actor::isSeeingActor(
  const Actor& other, const bool visionBlockingCells[MAP_W][MAP_H]) const {

  if(other.deadState == ActorDeadState::alive) {
    if(this == &other) {
      return true;
    }

    if(this == Map::player) {
      const bool IS_MONSTER_SNEAKING =
        dynamic_cast<const Monster*>(&other)->isStealth;
      return Map::cells[other.pos.x][other.pos.y].isSeenByPlayer &&
             IS_MONSTER_SNEAKING == false;
    }

    if(
      dynamic_cast<const Monster*>(this)->leader ==
      Map::player && &other != Map::player) {
      const bool IS_MONSTER_SNEAKING =
        dynamic_cast<const Monster*>(&other)->isStealth;
      if(IS_MONSTER_SNEAKING) return false;
    }

    if(propHandler_->allowSee() == false) {
      return false;
    }

    if(pos.x - other.pos.x > FOV_STD_RADI_INT) return false;
    if(other.pos.x - pos.x > FOV_STD_RADI_INT) return false;
    if(other.pos.y - pos.y > FOV_STD_RADI_INT) return false;
    if(pos.y - other.pos.y > FOV_STD_RADI_INT) return false;

    if(visionBlockingCells != NULL) {
      const bool IS_BLOCKED_BY_DARKNESS = data_->canSeeInDarkness == false;
      return Fov::checkCell(visionBlockingCells, other.pos, pos,
                            IS_BLOCKED_BY_DARKNESS);
    }
  }
  return false;
}

void Actor::getSpottedEnemies(vector<Actor*>& vectorRef) {
  vectorRef.resize(0);

  const bool IS_SELF_PLAYER = this == Map::player;

  bool visionBlockers[MAP_W][MAP_H];

  if(IS_SELF_PLAYER == false) {
    MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  }

  for(Actor * actor : GameTime::actors_) {
    if(actor != this && actor->deadState == ActorDeadState::alive) {

      if(IS_SELF_PLAYER) {
        if(dynamic_cast<Monster*>(actor)->leader != this) {
          if(isSeeingActor(*actor, NULL)) {
            vectorRef.push_back(actor);
          }
        }
      } else {
        const bool IS_OTHER_PLAYER = actor == Map::player;
        const bool IS_HOSTILE_TO_PLAYER =
          dynamic_cast<Monster*>(this)->leader != Map::player;
        const bool IS_OTHER_HOSTILE_TO_PLAYER =
          IS_OTHER_PLAYER ? false :
          dynamic_cast<Monster*>(actor)->leader != Map::player;

        //Note that IS_OTHER_HOSTILE_TO_PLAYER is false if other IS the player,
        //there is no need to check if IS_HOSTILE_TO_PLAYER && IS_OTHER_PLAYER
        if(
          (IS_HOSTILE_TO_PLAYER && IS_OTHER_HOSTILE_TO_PLAYER == false) ||
          (IS_HOSTILE_TO_PLAYER == false && IS_OTHER_HOSTILE_TO_PLAYER)) {
          if(isSeeingActor(*actor, visionBlockers)) {
            vectorRef.push_back(actor);
          }
        }
      }
    }
  }
}

void Actor::place(const Pos& pos_, ActorData& data) {
  pos             = pos_;
  data_           = &data;
  inv_      = new Inventory(data_->isHumanoid);
  propHandler_    = new PropHandler(this, eng);
  deadState       = ActorDeadState::alive;
  clr_            = data_->color;
  glyph_          = data_->glyph;
  tile_           = data_->tile;
  hp_             = hpMax_  = data_->hp;
  spi_            = spiMax_ = data_->spi;
  lairCell_       = pos;

  if(this != Map::player) {spawnStartItems();}

  place_();

  updateColor();
}

void Actor::teleport(const bool MOVE_TO_POS_AWAY_FROM_MONSTERS) {
  (void)MOVE_TO_POS_AWAY_FROM_MONSTERS;

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksActor(*this, true, eng), blockers);
  vector<Pos> freeCells;
  Utils::makeVectorFromBoolMap(false, blockers, freeCells);
  const Pos newPos = freeCells.at(Rnd::range(0, freeCells.size() - 1));

  if(this == Map::player) {
    Map::player->updateFov();
    Renderer::drawMapAndInterface();
    Map::updateVisualMemory();
  } else {
    dynamic_cast<Monster*>(this)->playerAwareOfMeCounter_ = 0;
  }

  pos = newPos;

  if(this == Map::player) {
    Map::player->updateFov();
    Renderer::drawMapAndInterface();
    Map::updateVisualMemory();
    eng.log->addMsg("I suddenly find myself in a different location!");
    propHandler_->tryApplyProp(new PropConfused(eng, propTurnsSpecific, 8));
  }
}

void Actor::updateColor() {
  if(deadState != ActorDeadState::alive) {
    clr_ = clrRed;
    return;
  }

  if(propHandler_->changeActorClr(clr_)) {
    return;
  }

  clr_ = data_->color;
}

bool Actor::restoreHp(const int HP_RESTORED, const bool ALLOW_MSG,
                      const bool IS_ALLOWED_ABOVE_MAX) {
  bool isHpGained = IS_ALLOWED_ABOVE_MAX;

  const int DIF_FROM_MAX = getHpMax(true) - HP_RESTORED;

  //If hp is below limit, but restored hp will push it over the limit,
  //hp is set to max.
  if(
    IS_ALLOWED_ABOVE_MAX == false &&
    getHp() > DIF_FROM_MAX &&
    getHp() < getHpMax(true)) {

    hp_ = getHpMax(true);
    isHpGained = true;
  }

  //If hp is below limit, and restored hp will NOT push it
  //over the limit - restored hp is added to current.
  if(IS_ALLOWED_ABOVE_MAX || getHp() <= DIF_FROM_MAX) {
    hp_ += HP_RESTORED;
    isHpGained = true;
  }

  updateColor();

  if(ALLOW_MSG) {
    if(isHpGained) {
      if(this == Map::player) {
        eng.log->addMsg("I feel healthier!", clrMsgGood);
      } else {
        if(Map::player->isSeeingActor(*this, NULL)) {
          eng.log->addMsg(data_->name_the + " looks healthier.");
        }
      }
      Renderer::drawMapAndInterface();
    }
  }

  return isHpGained;
}

bool Actor::restoreSpi(const int SPI_RESTORED, const bool ALLOW_MSG,
                       const bool IS_ALLOWED_ABOVE_MAX) {
  bool isSpiGained = IS_ALLOWED_ABOVE_MAX;

  const int DIF_FROM_MAX = getSpiMax() - SPI_RESTORED;

  //If spi is below limit, but restored spi will push it over the limit,
  //spi is set to max.
  if(
    IS_ALLOWED_ABOVE_MAX == false &&
    getSpi() > DIF_FROM_MAX &&
    getSpi() < getSpiMax()) {

    spi_ = getSpiMax();
    isSpiGained = true;
  }

  //If spi is below limit, and restored spi will NOT push it
  //over the limit - restored spi is added to current.
  if(
    IS_ALLOWED_ABOVE_MAX ||
    getSpi() <= DIF_FROM_MAX) {

    spi_ += SPI_RESTORED;
    isSpiGained = true;
  }

  if(ALLOW_MSG) {
    if(isSpiGained) {
      if(this == Map::player) {
        eng.log->addMsg("I feel more spirited!", clrMsgGood);
      } else {
        if(Map::player->isSeeingActor(*this, NULL)) {
          eng.log->addMsg(data_->name_the + " looks more spirited.");
        }
      }
      Renderer::drawMapAndInterface();
    }
  }

  return isSpiGained;
}

void Actor::changeMaxHp(const int CHANGE, const bool ALLOW_MSG) {
  hpMax_  = max(1, hpMax_ + CHANGE);
  hp_     = max(1, hp_ + CHANGE);

  if(ALLOW_MSG) {
    if(this == Map::player) {
      if(CHANGE > 0) {
        eng.log->addMsg("I feel more vigorous!");
      }
      if(CHANGE < 0) {
        eng.log->addMsg("I feel frailer!");
      }
    } else {
      if(Map::player->isSeeingActor(*this, NULL)) {
        if(CHANGE > 0) {
          eng.log->addMsg(getNameThe() + " looks more vigorous.");
        }
        if(CHANGE < 0) {
          eng.log->addMsg(getNameThe() + " looks frailer.");
        }
      }
    }
  }
}

void Actor::changeMaxSpi(const int CHANGE, const bool ALLOW_MESSAGES) {
  spiMax_ = max(1, spiMax_ + CHANGE);
  spi_    = max(1, spi_ + CHANGE);

  if(ALLOW_MESSAGES) {
    if(this == Map::player) {
      if(CHANGE > 0) {
        eng.log->addMsg("My spirit is stronger!");
      }
      if(CHANGE < 0) {
        eng.log->addMsg("My spirit is weaker!");
      }
    } else {
      if(Map::player->isSeeingActor(*this, NULL)) {
        if(CHANGE > 0) {
          eng.log->addMsg(getNameThe() + " appears to grow in spirit.");
        }
        if(CHANGE < 0) {
          eng.log->addMsg(getNameThe() + " appears to shrink in spirit.");
        }
      }
    }
  }
}

bool Actor::hit(int dmg, const DmgType dmgType, const bool ALLOW_WOUNDS) {
  traceVerbose << "Actor::hit()..." << endl;
  traceVerbose << "Actor: Damage from parameter: " << dmg << endl;

  if(this == Map::player) {
    Map::player->interruptActions();
  }

  vector<PropId> props;
  propHandler_->getAllActivePropIds(props);

  if(
    dmgType == DmgType::light &&
    find(props.begin(), props.end(), propLightSensitive) == props.end()) {
    return false;
  }

  //Damage to corpses
  //Note: corpse is automatically destroyed if damage is high enough,
  //otherwise it is destroyed with a random chance
  if(deadState != ActorDeadState::alive) {
    if(Rnd::oneIn(3) || dmg >= ((getHpMax(true) * 2) / 3)) {

      if(this != Map::player) {
        if(Map::cells[pos.x][pos.y].isSeenByPlayer) {
          eng.log->addMsg("The body of " + getNameA() + " is destroyed.");
        }
      }

      deadState = ActorDeadState::destroyed;
      glyph_ = ' ';
      if(isHumanoid()) {eng.gore->makeGore(pos);}
    }
    traceVerbose << "Actor::hit() [DONE]" << endl;
    return false;
  }

  if(dmgType == DmgType::spirit) {
    return hitSpi(dmg, true);
  }

  //Property resists?
  const bool ALLOW_DMG_RES_MSG = deadState == ActorDeadState::alive;
  if(propHandler_->tryResistDmg(dmgType, ALLOW_DMG_RES_MSG)) {
    return false;
  }

  hit_(dmg, ALLOW_WOUNDS);
  traceVerbose << "Actor: Damage after hit_(): " << dmg << endl;

  dmg = max(1, dmg);

  //Filter damage through worn armor
  if(isHumanoid()) {
    Armor* armor =
      dynamic_cast<Armor*>(inv_->getItemInSlot(SlotId::armorBody));
    if(armor != NULL) {
      traceVerbose << "Actor: Has armor, running hit on armor" << endl;

      if(dmgType == DmgType::physical) {
        dmg = armor->takeDurabilityHitAndGetReducedDamage(dmg);

        if(armor->isDestroyed()) {
          trace << "Actor: Armor was destroyed" << endl;
          if(this == Map::player) {
            const string armorName =
              eng.itemDataHandler->getItemRef(*armor, ItemRefType::plain);
            eng.log->addMsg("My " + armorName + " is torn apart!",
                            clrMsgWarning);
          }
          delete armor;
          armor = NULL;
          inv_->getSlot(SlotId::armorBody)->item = NULL;
        }
      }
    }
  }

  propHandler_->onHit();

  if(this != Map::player || Config::isBotPlaying() == false) {
    hp_ -= dmg;
  }

  if(getHp() <= 0) {
    const bool IS_ON_BOTTOMLESS =
      Map::cells[pos.x][pos.y].featureStatic->isBottomless();
    const bool IS_DMG_ENOUGH_TO_DESTROY = dmg > ((getHpMax(true) * 5) / 4);
    const bool IS_DESTROYED =
      data_->canLeaveCorpse == false  ||
      IS_ON_BOTTOMLESS                ||
      IS_DMG_ENOUGH_TO_DESTROY;

    die(IS_DESTROYED, IS_ON_BOTTOMLESS == false, IS_ON_BOTTOMLESS == false);
    traceVerbose << "Actor::hit() [DONE]" << endl;
    return true;
  } else {
    traceVerbose << "Actor::hit() [DONE]" << endl;
    return false;
  }
}

bool Actor::hitSpi(const int DMG, const bool ALLOW_MSG) {
  if(ALLOW_MSG) {
    if(this == Map::player) {
      eng.log->addMsg("My spirit is drained!", clrMsgBad);
    }
  }

  propHandler_->onHit();

  if(this != Map::player || Config::isBotPlaying() == false) {
    spi_ = max(0, spi_ - DMG);
  }
  if(getSpi() <= 0) {
    if(this == Map::player) {
      eng.log->addMsg("All my spirit is depleted, I am devoid of life!",
                      clrMsgBad);
    } else {
      if(Map::player->isSeeingActor(*this, NULL)) {
        eng.log->addMsg(getNameThe() + " has no spirit left!");
      }
    }
    die(false, false, true);
    return true;
  }
  return false;
}

void Actor::die(const bool IS_DESTROYED, const bool ALLOW_GORE,
                const bool ALLOW_DROP_ITEMS) {
  //Check all monsters and unset this actor as leader
  for(Actor * actor : GameTime::actors_) {
    if(actor != this && actor != Map::player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      if(monster->leader == this) {monster->leader = NULL;}
    }
  }

  bool isOnVisibleTrap = false;

  //If died on a visible trap, destroy the corpse
  const Feature* const f = Map::cells[pos.x][pos.y].featureStatic;
  if(f->getId() == feature_trap) {
    if(dynamic_cast<const Trap*>(f)->isHidden() == false) {
      isOnVisibleTrap = true;
    }
  }

  bool isPlayerSeeDyingActor = true;

  //Print death messages
  if(this != Map::player) {
    //Only print if visible
    if(Map::player->isSeeingActor(*this, NULL)) {
      isPlayerSeeDyingActor = true;
      const string deathMessageOverride = data_->deathMessageOverride;
      if(deathMessageOverride.empty() == false) {
        eng.log->addMsg(deathMessageOverride);
      } else {
        eng.log->addMsg(getNameThe() + " dies.");
      }
    }
  }

  deadState =
    (IS_DESTROYED || (isOnVisibleTrap && this != Map::player)) ?
    ActorDeadState::destroyed : ActorDeadState::corpse;

  if(this != Map::player) {
    if(isHumanoid() == true) {
      Snd snd(
        "I hear agonised screaming.", SfxId::endOfSfxId,
        IgnoreMsgIfOriginSeen::yes, pos, this, SndVol::low,
        AlertsMonsters::no);
      SndEmit::emitSnd(snd, eng);
    }
    dynamic_cast<Monster*>(this)->leader = NULL;
  }

  if(ALLOW_DROP_ITEMS) {
    eng.itemDrop->dropAllCharactersItems(this, true);
  }

  if(IS_DESTROYED) {
    glyph_ = ' ';
    tile_ = tile_empty;
    if(isHumanoid() == true) {
      if(ALLOW_GORE) {
        eng.gore->makeGore(pos);
      }
    }
  } else {
    if(this != Map::player) {
      Pos newPos;
      Feature* featureHere = Map::cells[pos.x][pos.y].featureStatic;
      //TODO this should be decided with a floodfill instead
      if(featureHere->canHaveCorpse() == false) {
        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            newPos = pos + Pos(dx, dy);
            featureHere =
              Map::cells[pos.x + dx][pos.y + dy].featureStatic;
            if(featureHere->canHaveCorpse()) {
              pos.set(newPos);
              dx = 9999;
              dy = 9999;
            }
          }
        }
      }
    }
    glyph_ = '&';
    tile_ = tile_corpse2;
  }

  clr_ = clrRedLgt;

  die_();

  propHandler_->onDeath(isPlayerSeeDyingActor);

  //Give exp if monster, and count up nr of kills.
  if(this != Map::player) {
    eng.dungeonMaster->onMonsterKilled(*this);
  }

  Renderer::drawMapAndInterface();
}

void Actor::addLight(bool lightMap[MAP_W][MAP_H]) const {
  vector<PropId> props;
  propHandler_->getAllActivePropIds(props);

  if(find(props.begin(), props.end(), propBurning) != props.end()) {
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        lightMap[pos.x + dx][pos.y + dy] = true;
      }
    }
  }

  addLight_(lightMap);
}
