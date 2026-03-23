#include "Engine/Engine.hpp"

int main()
{
    try
    {
        Engine::Engine engine;
        engine.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}