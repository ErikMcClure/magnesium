// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Everglade.h

#include "Game.h"

int main(int argc, char** argv)
{
  SetWorkDirToCur();
  Game::Run();
}

struct HINSTANCE__;

// WinMain function, simply a catcher that calls the main function
int __stdcall WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nShowCmd)
{
  main(0, (char**)hInstance);
}