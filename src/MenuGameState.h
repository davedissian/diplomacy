#pragma once

#include "GameState.h"

class MenuGameState : public GameState
{
public:
  MenuGameState(Game* game);
  ~MenuGameState() override;

  void tick(float dt) override;
};
