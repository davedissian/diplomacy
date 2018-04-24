#include "Common.h"
#include "Game.h"
#include "GameState.h"

GameState::GameState(Game* game): game_{game}
{
  viewport_.setSize(toSFML(game_->screenSize()));
}