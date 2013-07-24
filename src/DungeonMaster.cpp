#include "DungeonMaster.h"

#include "Engine.h"
#include "Highscore.h"
#include "TextFormatting.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "CharacterLines.h"
#include "Log.h"

void DungeonMaster::addSaveLines(vector<string>& lines) const {
  lines.push_back(intToString(timeStarted.year_));
  lines.push_back(intToString(timeStarted.month_));
  lines.push_back(intToString(timeStarted.day_));
  lines.push_back(intToString(timeStarted.hour_));
  lines.push_back(intToString(timeStarted.minute_));
  lines.push_back(intToString(timeStarted.second_));
}

void DungeonMaster::setParametersFromSaveLines(vector<string>& lines) {
  timeStarted.year_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.month_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.day_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.hour_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.minute_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.second_ = stringToInt(lines.front());
  lines.erase(lines.begin());
}

void DungeonMaster::winGame() {
  eng->highScore->gameOver(true);

  string winMessage = "As I touch the crystal, there is a jolt of electricity. A surreal glow suddenly illuminates the area. ";
  winMessage += "I feel as if I have stirred something. I notice a dark figure observing me from the edge of the light. ";
  winMessage += "It is the shape of a human. The figure approaches me, but still no light falls on it as it enters. ";
  winMessage += "There is no doubt in my mind concerning the nature of this entity; It is the Faceless God who dwells in the depths of ";
  winMessage += "the earth, it is the Crawling Chaos, the 'Black Man' of the witch-cult, the Devil - NYARLATHOTEP! ";
  winMessage += "I panic. Why is it I find myself here, stumbling around in darkness? ";
  winMessage += "The being beckons me to gaze into the stone. In the divine radiance I see visions beyond eternity, ";
  winMessage += "visions of unreal reality, visions of the brightest light of day and the darkest night of madness. ";
  winMessage += "The torrent of revelations does not seem unwelcome - I feel as if under a spell. There is only onward now. ";
  winMessage += "I demand to attain more - everything! So I make a pact with the Fiend......... ";
  winMessage += "I now harness the shadows that stride from world to world to sow death and madness. ";
  winMessage += "The destinies of all things on earth, living and dead, are mine. ";

  const vector<string> winMessageLines = eng->textFormatting->lineToLines(winMessage, 68);

  eng->renderer->coverPanel(panel_screen);
  eng->renderer->updateScreen();

  const int Y0 = 2;
  const unsigned int NR_OF_WIN_MESSAGE_LINES = winMessageLines.size();
  const int DELAY_BETWEEN_LINES = 40;
  eng->sleep(DELAY_BETWEEN_LINES);
  for(unsigned int i = 0; i < NR_OF_WIN_MESSAGE_LINES; i++) {
    for(unsigned int ii = 0; ii <= i; ii++) {
      eng->renderer->drawTextCentered(winMessageLines.at(ii), panel_screen,
                                      Pos(MAP_X_CELLS_HALF, Y0 + ii),
                                      clrMessageBad, clrBlack, true);
      if(i == ii && ii == NR_OF_WIN_MESSAGE_LINES - 1) {
        const string CMD_LABEL = "Space/Esc to record high-score and return to main menu";
        eng->renderer->drawTextCentered(
          CMD_LABEL, panel_screen,
          Pos(MAP_X_CELLS_HALF, Y0 + NR_OF_WIN_MESSAGE_LINES + 2),
          clrWhite, clrBlack, true);
      }
    }
    eng->renderer->updateScreen();
    eng->sleep(DELAY_BETWEEN_LINES);
  }

  eng->query->waitForEscOrSpace();
}

void DungeonMaster::monsterKilled(Actor* monster) {
  const int MONSTER_LVL = monster->getDef()->monsterLvl;

  monster->getDef()->nrOfKills += 1;

  if(MONSTER_LVL > 1) {
    if(eng->player->insanityObsessions[insanityObsession_sadism] == true) {
      eng->player->shock_ = max(0.0, eng->player->shock_ - 3.0);
    }
  }
}

void DungeonMaster::setTimeStartedToNow() {
  timeStarted = eng->basicUtils->getCurrentTime();
}





