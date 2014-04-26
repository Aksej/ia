#include "ActorMonster.h"

#include <algorithm>

#include "ItemFactory.h"
#include "Inventory.h"
#include "Explosion.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "ActorFactory.h"
#include "Renderer.h"
#include "CmnData.h"
#include "Map.h"
#include "Blood.h"
#include "FeatureFactory.h"
#include "Knockback.h"
#include "Gods.h"
#include "MapParsing.h"
#include "LineCalc.h"
#include "Utils.h"

using namespace std;

string Cultist::getCultistPhrase() {
  vector<string> phraseCandidates;

  const God* const god = engine.gods->getCurrentGod();
  if(god != NULL && Rnd::coinToss()) {
    const string name = god->getName();
    const string descr = god->getDescr();
    phraseCandidates.push_back(name + " save us!");
    phraseCandidates.push_back(descr + " will save us!");
    phraseCandidates.push_back(name + ", guide us!");
    phraseCandidates.push_back(descr + " guides us!");
    phraseCandidates.push_back("For " + name + "!");
    phraseCandidates.push_back("For " + descr + "!");
    phraseCandidates.push_back("Blood for " + name + "!");
    phraseCandidates.push_back("Blood for " + descr + "!");
    phraseCandidates.push_back("Perish for " + name + "!");
    phraseCandidates.push_back("Perish for " + descr + "!");
    phraseCandidates.push_back("In the name of " + name + "!");
  } else {
    phraseCandidates.push_back("Apigami!");
    phraseCandidates.push_back("Bhuudesco invisuu!");
    phraseCandidates.push_back("Bhuuesco marana!");
    phraseCandidates.push_back("Crudux cruo!");
    phraseCandidates.push_back("Cruento paashaeximus!");
    phraseCandidates.push_back("Cruento pestis shatruex!");
    phraseCandidates.push_back("Cruo crunatus durbe!");
    phraseCandidates.push_back("Cruo lokemundux!");
    phraseCandidates.push_back("Cruo-stragaraNa!");
    phraseCandidates.push_back("Gero shay cruo!");
    phraseCandidates.push_back("In marana domus-bhaava crunatus!");
    phraseCandidates.push_back("Caecux infirmux!");
    phraseCandidates.push_back("Malax sayti!");
    phraseCandidates.push_back("Marana pallex!");
    phraseCandidates.push_back("Marana malax!");
    phraseCandidates.push_back("Pallex ti!");
    phraseCandidates.push_back("Peroshay bibox malax!");
    phraseCandidates.push_back("Pestis Cruento!");
    phraseCandidates.push_back("Pestis cruento vilomaxus pretiacruento!");
    phraseCandidates.push_back("Pretaanluxis cruonit!");
    phraseCandidates.push_back("Pretiacruento!");
    phraseCandidates.push_back("StragarNaya!");
    phraseCandidates.push_back("Vorox esco marana!");
    phraseCandidates.push_back("Vilomaxus!");
    phraseCandidates.push_back("Prostragaranar malachtose!");
    phraseCandidates.push_back("Apigami!");
  }

  return phraseCandidates.at(
           Rnd::range(0, phraseCandidates.size() - 1));
}

void Cultist::spawnStartItems() {
  const int DLVL = Map::getDlvl();

  const int PISTOL = 6;
  const int PUMP_SHOTGUN = PISTOL + 4;
  const int SAWN_SHOTGUN = PUMP_SHOTGUN + 3;
  const int MG = SAWN_SHOTGUN + (DLVL < 3 ? 0 : 2);

  const int TOT = MG;
  const int RND = DLVL == 0 ? PISTOL : Rnd::range(1, TOT);

  if(RND <= PISTOL) {
    inv_->putItemInSlot(
      SlotId::wielded, eng.itemFactory->spawnItem(ItemId::pistol), true);
    if(Rnd::percentile() < 40) {
      inv_->putItemInGeneral(
        eng.itemFactory->spawnItem(ItemId::pistolClip));
    }
  } else if(RND <= PUMP_SHOTGUN) {
    inv_->putItemInSlot(
      SlotId::wielded, eng.itemFactory->spawnItem(ItemId::pumpShotgun), true);
    Item* item = eng.itemFactory->spawnItem(ItemId::shotgunShell);
    item->nrItems = Rnd::range(5, 9);
    inv_->putItemInGeneral(item);
  } else if(RND <= SAWN_SHOTGUN) {
    inv_->putItemInSlot(
      SlotId::wielded, eng.itemFactory->spawnItem(ItemId::sawedOff), true);
    Item* item = eng.itemFactory->spawnItem(ItemId::shotgunShell);
    item->nrItems = Rnd::range(6, 12);
    inv_->putItemInGeneral(item);
  } else {
    inv_->putItemInSlot(
      SlotId::wielded, eng.itemFactory->spawnItem(ItemId::machineGun), true);
  }

  if(Rnd::percentile() < 33) {
    inv_->putItemInGeneral(
      eng.itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(Rnd::percentile() < 8) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void CultistTeslaCannon::spawnStartItems() {
  Item* item = eng.itemFactory->spawnItem(ItemId::teslaCannon);
  inv_->putItemInSlot(SlotId::wielded, item, true);

  inv_->putItemInGeneral(
    eng.itemFactory->spawnItem(ItemId::teslaCanister));

  if(Rnd::percentile() < 33) {
    inv_->putItemInGeneral(
      eng.itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(Rnd::percentile() < 10) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void CultistSpikeGun::spawnStartItems() {
  inv_->putItemInSlot(
    SlotId::wielded, eng.itemFactory->spawnItem(ItemId::spikeGun), true);
  Item* item = eng.itemFactory->spawnItem(ItemId::ironSpike);
  item->nrItems = 8 + Rnd::dice(1, 8);
  inv_->putItemInGeneral(item);
}

void CultistPriest::spawnStartItems() {
  Item* item = eng.itemFactory->spawnItem(ItemId::dagger);
  dynamic_cast<Weapon*>(item)->meleeDmgPlus = 2;
  inv_->putItemInSlot(SlotId::wielded, item, true);

  inv_->putItemInGeneral(
    eng.itemFactory->spawnRandomScrollOrPotion(true, true));
  inv_->putItemInGeneral(
    eng.itemFactory->spawnRandomScrollOrPotion(true, true));

  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());

  if(Rnd::percentile() < 33) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void FireHound::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::fireHoundBreath));
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::fireHoundBite));
}

void FrostHound::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::frostHoundBreath));
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::frostHoundBite));
}

void Zuul::place_() {
  if(ActorData::dataList[actor_zuul].nrLeftAllowedToSpawn > 0) {
    //Note: Do not call die() here, that would have side effects such as
    //player getting XP. Instead, simply set the dead state to destroyed.
    deadState = ActorDeadState::destroyed;
    Actor* actor = eng.actorFactory->spawnActor(actor_cultistPriest, pos);
    PropHandler& propHandler = actor->getPropHandler();
    propHandler.tryApplyProp(
      new PropPossessedByZuul(eng, propTurnsIndefinite), true);
    actor->restoreHp(999, false);
  }
}

void Zuul::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::zuulBite));
}

bool Vortex::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(pullCooldown > 0) {
      pullCooldown--;
    }

    if(pullCooldown <= 0) {
      if(awareOfPlayerCounter_ > 0) {
        trace << "Vortex: pullCooldown: " << pullCooldown << endl;
        trace << "Vortex: Is player aware" << endl;
        const Pos& playerPos = Map::player->pos;
        if(Utils::isPosAdj(pos, playerPos, true) == false) {

          const int CHANCE_TO_KNOCK = 25;
          if(Rnd::percentile() < CHANCE_TO_KNOCK) {
            trace << "Vortex: Passed random chance to pull" << endl;

            const Pos playerDelta = playerPos - pos;
            Pos knockBackFromPos = playerPos;
            if(playerDelta.x > 1)   {knockBackFromPos.x++;}
            if(playerDelta.x < -1)  {knockBackFromPos.x--;}
            if(playerDelta.y > 1)   {knockBackFromPos.y++;}
            if(playerDelta.y < -1)  {knockBackFromPos.y--;}

            if(knockBackFromPos != playerPos) {
              trace << "Vortex: Good pos found to knockback player from (";
              trace << knockBackFromPos.x << ",";
              trace << knockBackFromPos.y << ")" << endl;
              trace << "Vortex: Player position: ";
              trace << playerPos.x << "," << playerPos.y << ")" << endl;
              bool visionBlockers[MAP_W][MAP_H];
              MapParse::parse(CellPred::BlocksVision(), visionBlockers);
              if(isSeeingActor(*(Map::player), visionBlockers)) {
                trace << "Vortex: I am seeing the player" << endl;
                if(Map::player->isSeeingActor(*this, NULL)) {
                  eng.log->addMsg("The Vortex attempts to pull me in!");
                } else {
                  eng.log->addMsg("A powerful wind is pulling me!");
                }
                trace << "Vortex: Attempt pull (knockback)" << endl;
                eng.knockBack->tryKnockBack(
                  *(Map::player), knockBackFromPos, false, false);
                pullCooldown = 5;
                GameTime::actorDidAct();
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

void DustVortex::die_() {
  Explosion::runExplosionAt(
    pos, eng, ExplType::applyProp, ExplSrc::misc, 0, SfxId::endOfSfxId,
    new PropBlind(eng, propTurnsStd), &clrGray);
}

void DustVortex::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::dustVortexEngulf));
}

void FireVortex::die_() {
  Explosion::runExplosionAt(
    pos, eng, ExplType::applyProp, ExplSrc::misc, 0, SfxId::endOfSfxId,
    new PropBurning(eng, propTurnsStd), &clrRedLgt);
}

void FireVortex::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::fireVortexEngulf));
}

void FrostVortex::die_() {
  //TODO Add explosion with cold damage
}

void FrostVortex::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::frostVortexEngulf));
}

bool Ghost::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {

      if(Utils::isPosAdj(pos, Map::player->pos, false)) {
        if(Rnd::percentile() < 30) {

          bool blockers[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksVision(), blockers);
          const bool PLAYER_SEES_ME =
            Map::player->isSeeingActor(*this, blockers);
          const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
          eng.log->addMsg(refer + " reaches for me... ");
          const AbilityRollResult rollResult =
            AbilityRoll::roll(Map::player->getData().abilityVals.getVal(
                                AbilityId::dodgeAttack, true, *this));
          const bool PLAYER_DODGES = rollResult >= successSmall;
          if(PLAYER_DODGES) {
            eng.log->addMsg("I dodge!", clrMsgGood);
          } else {
            Map::player->getPropHandler().tryApplyProp(
              new PropSlowed(eng, propTurnsStd));
          }
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void Ghost::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::ghostClaw));
}

void Phantasm::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::phantasmSickle));
}

void Wraith::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::wraithClaw));
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
}

void MiGo::spawnStartItems() {
  Item* item = eng.itemFactory->spawnItem(ItemId::miGoElectricGun);
  inv_->putItemInIntrinsics(item);

  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellMiGoHypnosis);
  spellsKnown.push_back(new SpellHealSelf);

  if(Rnd::coinToss()) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void FlyingPolyp::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::polypTentacle));
}

void Rat::spawnStartItems() {
  Item* item = NULL;
  if(Rnd::percentile() < 15) {
    item = eng.itemFactory->spawnItem(ItemId::ratBiteDiseased);
  } else {
    item = eng.itemFactory->spawnItem(ItemId::ratBite);
  }
  inv_->putItemInIntrinsics(item);
}

void RatThing::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::ratThingBite));
}

void Shadow::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::shadowClaw));
}

void Ghoul::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::ghoulClaw));
}

void Mummy::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::mummyMaul));

  spellsKnown.push_back(eng.spellHandler->getSpellFromId(SpellId::disease));

  for(int i = Rnd::range(1, 2); i > 0; i--) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void MummyUnique::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::mummyMaul));

  spellsKnown.push_back(eng.spellHandler->getSpellFromId(SpellId::disease));

  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
}

bool Khephren::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(hasSummonedLocusts == false) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blockers);

        if(isSeeingActor(*(Map::player), blockers)) {
          MapParse::parse(CellPred::BlocksMoveCmn(true, eng), blockers);

          const int SPAWN_AFTER_X =
            Map::player->pos.x + FOV_STD_RADI_INT + 1;
          for(int y = 0; y  < MAP_H; y++) {
            for(int x = 0; x <= SPAWN_AFTER_X; x++) {
              blockers[x][y] = true;
            }
          }

          vector<Pos> freeCells;
          Utils::makeVectorFromBoolMap(false, blockers, freeCells);

          sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(pos, eng));

          const int NR_OF_SPAWNS = 15;
          if(freeCells.size() >= NR_OF_SPAWNS + 1) {
            eng.log->addMsg("Khephren calls a plague of Locusts!");
            Map::player->incrShock(ShockValue::shockValue_heavy,
                                   ShockSrc::misc);
            for(int i = 0; i < NR_OF_SPAWNS; i++) {
              Actor* const actor =
                eng.actorFactory->spawnActor(actor_giantLocust,
                                             freeCells.at(0));
              Monster* const monster = dynamic_cast<Monster*>(actor);
              monster->awareOfPlayerCounter_ = 999;
              monster->leader = this;
              freeCells.erase(freeCells.begin());
            }
            Renderer::drawMapAndInterface();
            hasSummonedLocusts = true;
            GameTime::actorDidAct();
            return true;
          }
        }
      }
    }
  }

  return false;
}



void DeepOne::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(
                              ItemId::deepOneJavelinAtt));
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::deepOneSpearAtt));
}

void GiantBat::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::giantBatBite));
}

void Byakhee::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::byakheeClaw));
}

void GiantMantis::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::giantMantisClaw));
}

void Chthonian::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::chthonianBite));
}

void HuntingHorror::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::huntingHorrorBite));
}

bool KeziahMason::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(hasSummonedJenkin == false) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blockers);

        if(isSeeingActor(*(Map::player), blockers)) {

          MapParse::parse(CellPred::BlocksMoveCmn(true, eng), blockers);

          vector<Pos> line;
          eng.lineCalc->calcNewLine(pos, Map::player->pos, true, 9999,
                                    false, line);

          const int LINE_SIZE = line.size();
          for(int i = 0; i < LINE_SIZE; i++) {
            const Pos c = line.at(i);
            if(blockers[c.x][c.y] == false) {
              //TODO Make a generalized summoning functionality
              eng.log->addMsg("Keziah summons Brown Jenkin!");
              Actor* const actor =
                eng.actorFactory->spawnActor(actor_brownJenkin, c);
              Monster* jenkin = dynamic_cast<Monster*>(actor);
              Renderer::drawMapAndInterface();
              hasSummonedJenkin = true;
              jenkin->awareOfPlayerCounter_ = 999;
              jenkin->leader = this;
              GameTime::actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

void KeziahMason::spawnStartItems() {
  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellHealSelf);
  spellsKnown.push_back(new SpellSummonRandom);
  spellsKnown.push_back(new SpellAzathothsWrath);
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
}

void Ooze::onStandardTurn() {
  restoreHp(1, false);
}

void OozeBlack::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::oozeBlackSpewPus));
}

void OozeClear::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::oozeClearSpewPus));
}

void OozePutrid::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::oozePutridSpewPus));
}

void OozePoison::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::oozePoisonSpewPus));
}

void ColourOOSpace::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::colourOOSpaceTouch));
}

const SDL_Color& ColourOOSpace::getClr() {
  return currentColor;
}

void ColourOOSpace::onStandardTurn() {
  currentColor.r = Rnd::range(40, 255);
  currentColor.g = Rnd::range(40, 255);
  currentColor.b = Rnd::range(40, 255);

  restoreHp(1, false);

  if(Map::player->isSeeingActor(*this, NULL)) {
    Map::player->getPropHandler().tryApplyProp(
      new PropConfused(eng, propTurnsStd));
  }
}

bool Spider::onActorTurn_() {
  return false;
}

void GreenSpider::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::greenSpiderBite));
}

void WhiteSpider::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::whiteSpiderBite));
}

void RedSpider::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::redSpiderBite));
}

void ShadowSpider::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::shadowSpiderBite));
}

void LengSpider::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::lengSpiderBite));
}

void Wolf::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::wolfBite));
}

bool WormMass::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(Rnd::percentile() < chanceToSpawnNew) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(*this, true, eng), blockers);

        Pos spawnPos;
        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            spawnPos.set(pos + Pos(dx, dy));
            if(blockers[spawnPos.x][spawnPos.y] == false) {
              Actor* const actor =
                eng.actorFactory->spawnActor(data_->id, spawnPos);
              WormMass* const worm = dynamic_cast<WormMass*>(actor);
              chanceToSpawnNew -= 4;
              worm->chanceToSpawnNew = chanceToSpawnNew;
              worm->awareOfPlayerCounter_ = awareOfPlayerCounter_;
              GameTime::actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void WormMass::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::wormMassBite));
}

bool GiantLocust::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(Rnd::percentile() < chanceToSpawnNew) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(*this, true, eng), blockers);

        Pos spawnPos;
        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            spawnPos.set(pos + Pos(dx, dy));
            if(blockers[spawnPos.x][spawnPos.y] == false) {
              Actor* const actor =
                eng.actorFactory->spawnActor(data_->id, spawnPos);
              GiantLocust* const locust = dynamic_cast<GiantLocust*>(actor);
              chanceToSpawnNew -= 2;
              locust->chanceToSpawnNew = chanceToSpawnNew;
              locust->awareOfPlayerCounter_ = awareOfPlayerCounter_;
              GameTime::actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void GiantLocust::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::giantLocustBite));
}

bool LordOfShadows::onActorTurn_() {
  return false;
}

void LordOfShadows::spawnStartItems() {

}

bool LordOfSpiders::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {

      if(Rnd::coinToss()) {

        const Pos playerPos = Map::player->pos;

        if(Map::player->isSeeingActor(*this, NULL)) {
          eng.log->addMsg(data_->spellCastMessage);
        }

        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {

            if(Rnd::percentile() < 75) {

              const Pos c(playerPos + Pos(dx, dy));
              const FeatureStatic* const mimicFeature =
                Map::cells[c.x][c.y].featureStatic;

              if(mimicFeature->canHaveStaticFeature()) {

                const FeatureData* const mimicData =
                  eng.featureDataHandler->getData(
                    mimicFeature->getId());

                Feature* const f =
                  eng.featureFactory->spawnFeatureAt(
                    feature_trap, c,
                    new TrapSpawnData(mimicData, trap_spiderWeb));

                dynamic_cast<Trap*>(f)->reveal(false);
              }
            }
          }
        }
      }
    }
  }
  return false;
}

void LordOfSpiders::spawnStartItems() {

}

bool LordOfSpirits::onActorTurn_() {
  return false;
}

void LordOfSpirits::spawnStartItems() {

}

bool LordOfPestilence::onActorTurn_() {
  return false;
}

void LordOfPestilence::spawnStartItems() {

}

bool Zombie::onActorTurn_() {
  return tryResurrect();
}

bool MajorClaphamLee::onActorTurn_() {
  if(tryResurrect()) {
    return true;
  }

  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(hasSummonedTombLegions == false) {

        bool visionBlockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), visionBlockers);

        if(isSeeingActor(*(Map::player), visionBlockers)) {
          eng.log->addMsg("Major Clapham Lee calls forth his Tomb-Legions!");
          vector<ActorId> monsterIds;
          monsterIds.resize(0);

          monsterIds.push_back(actor_deanHalsey);

          const int NR_OF_EXTRA_SPAWNS = 4;

          for(int i = 0; i < NR_OF_EXTRA_SPAWNS; i++) {
            const int ZOMBIE_TYPE = Rnd::range(1, 3);
            ActorId id = actor_zombie;
            switch(ZOMBIE_TYPE) {
              case 1: id = actor_zombie;        break;
              case 2: id = actor_zombieAxe;     break;
              case 3: id = actor_bloatedZombie; break;
            }
            monsterIds.push_back(id);
          }
          eng.actorFactory->summonMonsters(pos, monsterIds, true, this);
          Renderer::drawMapAndInterface();
          hasSummonedTombLegions = true;
          Map::player->incrShock(ShockValue::shockValue_heavy, ShockSrc::misc);
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }

  return false;
}

bool Zombie::tryResurrect() {
  if(deadState == ActorDeadState::corpse) {
    if(hasResurrected == false) {
      deadTurnCounter += 1;
      if(deadTurnCounter > 5) {
        if(pos != Map::player->pos && Rnd::percentile() < 7) {
          deadState = ActorDeadState::alive;
          hp_ = (getHpMax(true) * 3) / 4;
          glyph_ = data_->glyph;
          tile_ = data_->tile;
          clr_ = data_->color;
          hasResurrected = true;
          data_->nrKills--;
          if(Map::cells[pos.x][pos.y].isSeenByPlayer) {
            eng.log->addMsg(
              getNameThe() + " rises again!!", clrWhite, true);
            Map::player->incrShock(ShockValue::shockValue_some, ShockSrc::misc);
          }

          awareOfPlayerCounter_ = data_->nrTurnsAwarePlayer * 2;
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void Zombie::die_() {
  //If resurrected once and has corpse, blow up the corpse
  if(hasResurrected && deadState == ActorDeadState::corpse) {
    deadState = ActorDeadState::destroyed;
    eng.gore->makeBlood(pos);
    eng.gore->makeGore(pos);
  }
}

void ZombieClaw::spawnStartItems() {
  Item* item = NULL;
  if(Rnd::percentile() < 20) {
    item = eng.itemFactory->spawnItem(ItemId::zombieClawDiseased);
  } else {
    item = eng.itemFactory->spawnItem(ItemId::zombieClaw);
  }
  inv_->putItemInIntrinsics(item);
}

void ZombieAxe::spawnStartItems() {
  inv_->putItemInIntrinsics(eng.itemFactory->spawnItem(ItemId::zombieAxe));
}

void BloatedZombie::spawnStartItems() {
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::bloatedZombiePunch));
  inv_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(ItemId::bloatedZombieSpit));
}

