#include "Application.h"

int main(int argc, char **argv)
{
    lunarmp::Application::getInstance().run(argc, argv);
    return 0;
}
