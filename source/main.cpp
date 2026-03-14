#include "Game.h"
#include "eng.h"

#include <filesystem>
#include <iostream>

bool EnsureDirectoryExists(const std::filesystem::path& dirPath)
{
  std::error_code ec;

  if (std::filesystem::exists(dirPath, ec))
  {
    std::cout << "Folder already exists: " << dirPath << std::endl;
    return std::filesystem::is_directory(dirPath, ec);
  }

  std::cout << "Folder created: " << dirPath << std::endl;
  return std::filesystem::create_directories(dirPath, ec);
}

int main()
{
  try
  {
    EnsureDirectoryExists(".cache");

    Game* game = new Game();
    eng::Engine& engine = eng::Engine::GetInstance();
    engine.SetApplication(game);

    if (engine.Init(1280, 720, "Fain Engine"))
    {
      engine.Run();
    }

    engine.Destroy();
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unhandled exeption: " << e.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unhandled exeption\n";
    return 1;
  }

  return 0;
}