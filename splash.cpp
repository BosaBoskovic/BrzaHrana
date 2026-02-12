#include "splash.h"
#include <iostream>

SplashManager::SplashManager() {}

bool SplashManager::addSplash(glm::vec3 ketchupPos, glm::vec3 stackTop, float tableY, SauceType sauce) {
    KetchupSplash splash;
    splash.sauce = sauce;

    float ketchupTipY = ketchupPos.y - 0.115f;
    float burgerTopY = stackTop.y;
    float horizontalDist = glm::length(glm::vec2(
        ketchupPos.x - stackTop.x,
        ketchupPos.z - stackTop.z
    ));
    float verticalDist = abs(ketchupTipY - burgerTopY);

    std::cout << "\n?? addSplash pozvan:\n";
    std::cout << "   Sos: " << (sauce == SAUCE_KETCHUP ? "KE?AP ??" : "SENF ??") << "\n";
    std::cout << "   horizontalDist: " << horizontalDist << " (limit: 0.25)\n";
    std::cout << "   verticalDist: " << verticalDist << " (limit: ";

    // ?? KLJU?NA IZMENA - VE?I LIMIT ZA SENF
    float verticalLimit = (sauce == SAUCE_MUSTARD) ? 1.5f : 1.0f;

    std::cout << verticalLimit << ")\n";

    if (horizontalDist < 0.25f && verticalDist < verticalLimit) {
        // ?? 3D MODEL NA BURGERU
        splash.position = stackTop;

        if (sauce == SAUCE_MUSTARD) {
            splash.position.y -= 0.03f;  // SENF IZNAD KE?APA
            std::cout << "   ? SENF 3D na Y = " << splash.position.y << "\n";
        }
        else {
            splash.position.y -= 0.08f;  // KE?AP
            std::cout << "   ? KE?AP 3D na Y = " << splash.position.y << "\n";
        }

        splash.type = SPLASH_3D;
    }
    else {
        // ? 2D MRLJA NA STOLU
        splash.position.x = ketchupPos.x;
        splash.position.y = tableY + 0.01f;
        splash.position.z = ketchupPos.z;
        splash.type = SPLASH_2D;

        std::cout << "   ? "
            << (sauce == SAUCE_KETCHUP ? "KE?AP" : "SENF")
            << " 2D mrlja na sto\n";
    }

    splashes.push_back(splash);

    std::cout << "   ?? Ukupno mrlja posle dodavanja: " << splashes.size() << "\n";
    std::cout << "   ?? Vra?am: " << (splash.type == SPLASH_3D ? "true (POGODAK)" : "false (PROMAŠAJ)") << "\n\n";

    return (splash.type == SPLASH_3D);
}