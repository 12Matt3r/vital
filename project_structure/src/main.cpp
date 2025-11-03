#include "core/application.h"
#include "core/version.h"

int main(int argc, char* argv[]) {
    vital::Application app;
    return app.run(argc, argv);
}