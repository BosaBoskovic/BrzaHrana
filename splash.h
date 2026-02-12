#pragma once
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

enum SplashType {
    SPLASH_3D,    // 3D model (na pljeskavici)
    SPLASH_2D,     // 2D tekstura (na stolu)
};

enum SauceType {  // ? NOVO
    SAUCE_KETCHUP,
    SAUCE_MUSTARD
};

struct KetchupSplash {
    glm::vec3 position;  // Gde je mrlja
    SplashType type;// Da li je na stolu ili na burgeru
    SauceType sauce;
};

class SplashManager {
public:
    SplashManager();

    // Dodaje novu mrlju kada se pritisne K
    bool addSplash(glm::vec3 ketchupPos, glm::vec3 stackTop, float tableY, SauceType sauce);

    // Lista svih mrlja
    const std::vector<KetchupSplash>& getSplashes() const { return splashes; }

private:
    std::vector<KetchupSplash> splashes;
};