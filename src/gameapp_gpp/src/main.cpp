#include "stdafx.h"

#include "gpp/game.h"

#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/updateFramework.h"

int main(int argc, const char* argv[])
{
    gpp::Game e;
    try {
        g_globalManager.initialize();
        g_globalManager.getUpdateFramework()->registerUpdateCallback(std::bind(&gpp::Game::update, &e, std::placeholders::_1));
        g_globalManager.getUpdateFramework()->registerInitializeCallback(std::bind(&gpp::Game::initialize, &e));
        g_globalManager.getUpdateFramework()->registerDestroyCallback(std::bind(&gpp::Game::destroy, &e));
    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("Fatal error initializing: %s", ex.what());
        g_globalManager.destroy();
        #ifdef _DEBUG
        std::cout << std::endl << "press ENTER to quit...";
        std::cin.get();
        #endif
        return -1;
    }

    try
    {
        g_globalManager.getUpdateFramework()->run();
    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("Fatal error during execution: %s", ex.what());
    }
    g_globalManager.destroy();
    gep::destroy(); // Shut down the engine

    // Uncomment to enable a prompt before exiting the game to read the console or something
    //#ifdef _DEBUG
    //std::cout << std::endl << "press ENTER to quit...";
    //std::cin.get();
    //#endif

    return 0;
}
