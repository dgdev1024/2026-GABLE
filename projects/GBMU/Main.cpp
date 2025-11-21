#include <GBMU/Application.hpp>

auto main (int argc, char** argv) -> int
{
    gbmu::Application app { argc, argv };
    return app.start();
}
