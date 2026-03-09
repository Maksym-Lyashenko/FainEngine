#include "Game.h"

#include <iostream>

bool Game::Init()
{
  std::cout << "Hello from Game::Init!" << std::endl;

  return true;
}

void Game::Update(float DeltaTime)
{
  std::printf("%f\n", DeltaTime);
}

void Game::Destroy()
{
  std::cout << "Hello from Game::Destroy!" << std::endl;
}
