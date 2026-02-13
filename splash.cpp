#include "splash.h"
#include <iostream>

SplashManager::SplashManager() {}

bool SplashManager::addSplash(glm::vec3 ketchupPos, glm::vec3 stackTop, float tableY, SauceType sauce) {
    // ========== TA?NE DIMENZIJE STOLA ==========
    float tableWidth = 1.7f;
    float tableDepth = 1.0f;
    float tableThickness = 0.1f;

    float tableHalfWidth = tableWidth / 2.0f;   
    float tableHalfDepth = tableDepth / 2.0f;   
    float tableTopY = tableY + tableThickness / 2.0f; // -0.55
    float maxReasonableHeight = tableTopY + 1.5f; // Smanji toleranciju

    // ========== PROVERA 1: Da li je VAN STOLA? ==========
    bool outOfBoundsX = (ketchupPos.x < -tableHalfWidth || ketchupPos.x > tableHalfWidth);
    bool outOfBoundsZ = (ketchupPos.z < -tableHalfDepth || ketchupPos.z > tableHalfDepth);
    bool outOfBoundsY = (ketchupPos.y < tableTopY || ketchupPos.y > maxReasonableHeight);

    if (outOfBoundsX || outOfBoundsZ || outOfBoundsY) {
        std::cout << "\nK kliknut VAN STOLA  ignoriše se!\n";
        std::cout << "   Pozicija: (" << ketchupPos.x << ", " << ketchupPos.y << ", " << ketchupPos.z << ")\n";
        std::cout << "   Granice:\n";
        std::cout << "      X: [" << -tableHalfWidth << " do " << tableHalfWidth << "]\n";
        std::cout << "      Y: [" << tableTopY << " do " << maxReasonableHeight << "]\n";
        std::cout << "      Z: [" << -tableHalfDepth << " do " << tableHalfDepth << "]\n\n";
        return false; //  VAN STOLA  NIŠTA SE NE DODAJE
    }

    // ========== PROVERA 2: Da li je IZNAD TANJIRA? ==========
    KetchupSplash splash;
    splash.sauce = sauce;

    float ketchupTipY = ketchupPos.y - 0.115f;
    float burgerTopY = stackTop.y;

    float horizontalDist = glm::length(glm::vec2(
        ketchupPos.x - stackTop.x,
        ketchupPos.z - stackTop.z
    ));
    float verticalDist = abs(ketchupTipY - burgerTopY);

    std::cout << "\nK IZNAD STOLA - proveravam tanjir:\n";
    std::cout << "   Sos: " << (sauce == SAUCE_KETCHUP ? "KE?AP " : "SENF ") << "\n";
    std::cout << "   horizontalDist: " << horizontalDist << " (limit: 0.25)\n";
    std::cout << "   verticalDist: " << verticalDist << " (limit: ";

    float verticalLimit = (sauce == SAUCE_MUSTARD) ? 1.5f : 1.0f;
    std::cout << verticalLimit << ")\n";

    //  POGODAK TANJIRA 3D mrlja
    if (horizontalDist < 0.25f && verticalDist < verticalLimit) {
        splash.position = stackTop;

        if (sauce == SAUCE_MUSTARD) {
            splash.position.y -= 0.03f;
            std::cout << "   POGODAK! SENF 3D (Y = " << splash.position.y << ")\n";
        }
        else {
            splash.position.y -= 0.08f;
            std::cout << "   POGODAK! KE?AP 3D (Y = " << splash.position.y << ")\n";
        }

        splash.type = SPLASH_3D;
        splashes.push_back(splash);

        std::cout << "    Ukupno mrlja: " << splashes.size() << "\n";
        std::cout << "    Vra?am: true\n\n";
        return true;
    }
    //  PROMAŠAJ TANJIRA  2D mrlja na sto
    else {
        splash.position.x = ketchupPos.x;
        splash.position.y = tableY + 0.01f;
        splash.position.z = ketchupPos.z;
        splash.type = SPLASH_2D;
        splashes.push_back(splash);

        std::cout << "  PROMAŠAJ tanjira! "
            << (sauce == SAUCE_KETCHUP ? "KE?AP" : "SENF")
            << " 2D mrlja na sto\n";
        std::cout << "    Ukupno mrlja: " << splashes.size() << "\n";
        std::cout << "    Vra?am: false\n\n";
        return false;
    }
}