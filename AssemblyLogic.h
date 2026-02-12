#pragma once
#include <glm/glm.hpp>
#include <vector>

enum IngredientType {
    BOTTOM_BUN,
    PATTY,
    KETCHUP,
    MUSTARD,
    PICKLES,
    ONION,
    LETTUCE,
    CHEESE,
    TOMATO,
    TOP_BUN,
    INGREDIENT_COUNT
};

struct Ingredient {
    IngredientType type;
    glm::vec3 position;
    float height;
};

class AssemblyLogic {
public:
    AssemblyLogic(glm::vec3 plateCenter);

    IngredientType getCurrentType() const;
    glm::vec3& getCurrentPosition();

    // Automatsko zaklju?avanje kada do?e blizuStackTop pozicije
    void updateCurrentPosition();

    glm::vec3 getStackTopPosition() const;
    const std::vector<Ingredient>& getPlaced() const;

    void clampCurrentAboveStack();

    bool isCurrentLocked() const { return locked; }

    glm::vec3& getKetchupPosition();

    glm::vec3 getPlateCenterWorld() const { return platePos; }
    bool isCenteredAboveStack(float tolerance = 0.1f) const;
    void lockCurrent();

    bool isKetchupPlaced() const;  // Provera da li je ke?ap ve? stavljen
    void advanceToMustard();
    void skipToNextIngredient();  // Presko?i trenutni sastojak
    bool shouldHideCurrentBottle() const;
    void hideBottle();
    void showBottle() { hideCurrentBottle = false; }
private:
    IngredientType current;
    glm::vec3 currentPos;
    glm::vec3 platePos;             // CENTAR TANJIRA (3D)
    std::vector<Ingredient> placed;

    bool locked = false;            // Da li je trenutni sastojak zaklju?an
    bool hideCurrentBottle = false;

    float getHeightForType(IngredientType type) const;
};