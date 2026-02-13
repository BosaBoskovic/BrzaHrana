#include "AssemblyLogic.h"
#include <glm/glm.hpp>
#include <iostream>

static constexpr float LOCK_DISTANCE = 0.15f;

AssemblyLogic::AssemblyLogic(glm::vec3 plateCenter) {
    platePos = plateCenter;
    current = BOTTOM_BUN;
    currentPos = platePos + glm::vec3(0.0f, 1.5f, 0.0f);
    locked = false;
}

IngredientType AssemblyLogic::getCurrentType() const {
    return current;
}

glm::vec3& AssemblyLogic::getCurrentPosition() {
    return currentPos;
}

float AssemblyLogic::getHeightForType(IngredientType type) const {
    if (type == BOTTOM_BUN) return 0.04f;   
    if (type == PATTY) return -0.32f;        
    if (type == KETCHUP || type == MUSTARD) return 0.01f; 
    if (type == PICKLES) return 0.27f;      
    if (type == CHEESE) return 0.08f;     
    if (type == TOMATO) return 0.05f;      
    if (type == LETTUCE) return 0.04f;     
    if (type == ONION) return 0.03f;        
    if (type == TOP_BUN) return 0.56f;      
    return 0.03f; 
}

glm::vec3 AssemblyLogic::getStackTopPosition() const {
    glm::vec3 pos = platePos;
    pos.y += 0.25f; // Vrh tanjira

    std::cout << "?? getStackTopPosition(): platePos.y = " << platePos.y
        << ", broj postavljenih = " << placed.size() << "\n";

    for (const auto& i : placed) {
        float h = i.height;

        if (i.type == PATTY && current == CHEESE) {
            h = 0.0f;
        }

        pos.y += h;
    }


    std::cout << "   = FINALNA pozicija vrha: " << pos.y << "\n";
    return pos;
}

void AssemblyLogic::updateCurrentPosition() {
    if (locked) return;

    if (current == KETCHUP || current == MUSTARD) {
        return;
    }

    glm::vec3 target = getStackTopPosition();
    float h = getHeightForType(current);

    
    target.y += h / 2.0f; // Centar sastojka na vrh steka

    if (current == CHEESE) {
        target.y -= 0.09f;  
    }
    if (current == TOMATO) {
        target.y -= 0.02f;  
    }


    float distXZ = glm::length(glm::vec2(currentPos.x - target.x, currentPos.z - target.z));
    float distY = abs(currentPos.y - target.y);

    if (distXZ <= LOCK_DISTANCE && distY <= 0.3f) {
        currentPos = target;

        placed.push_back({
            current,
            currentPos,
            h  
            });

        std::cout << "? ZAKLJUCAN: " << current << " na ("
            << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ") sa visinom " << h << "\n";

        
        current = (IngredientType)(current + 1);

        if (current < INGREDIENT_COUNT) {
            currentPos = getStackTopPosition() + glm::vec3(0.0f, 1.0f, 0.0f);
            locked = false;
            this->hideCurrentBottle = false;

            std::cout << "?? SLEDE?I SASTOJAK: " << current << "\n";
        }
        else {
            std::cout << "?? GOTOV BURGER!\n";
            locked = true;
        }
    }
    float cheeseOffset = 0.01f; 
    if (current == CHEESE) {
        target.y += cheeseOffset; 
    }
    else {
        target.y += h / 2.0f;
    }

}

const std::vector<Ingredient>& AssemblyLogic::getPlaced() const {
    return placed;
}

void AssemblyLogic::clampCurrentAboveStack() {
    if (locked) return;
}

glm::vec3& AssemblyLogic::getKetchupPosition() {
    return currentPos;
}

bool AssemblyLogic::isCenteredAboveStack(float tolerance) const {
    glm::vec3 top = getStackTopPosition();

    float dx = fabs(currentPos.x - top.x);
    float dz = fabs(currentPos.z - top.z);

    return dx < tolerance && dz < tolerance;
}

void AssemblyLogic::lockCurrent() {
    glm::vec3 top = getStackTopPosition();

    currentPos.y = top.y;

    placed.push_back({ current, currentPos });

    locked = true;
    hideCurrentBottle = true;

    if (current == BOTTOM_BUN) current = PATTY;
    else if (current == PATTY) current = KETCHUP;
    else current = MUSTARD;
}

bool AssemblyLogic::isKetchupPlaced() const {
    for (const auto& ing : placed) {
        if (ing.type == KETCHUP) return true;
    }
    return false;
}

void AssemblyLogic::advanceToMustard() {
    if (current == KETCHUP) {
        std::cout << " advanceToMustard() POZVAN!\n";
        current = MUSTARD;
        currentPos = getStackTopPosition() + glm::vec3(0.0f, 1.2f, 0.0f);
        locked = false;
        this->hideCurrentBottle = false;
        std::cout << "    Senf postavljen na: (" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")\n";
        std::cout << "    hideCurrentBottle = " << hideCurrentBottle << "\n";
    }
}

void AssemblyLogic::hideBottle() {
    hideCurrentBottle = true;
}

bool AssemblyLogic::shouldHideCurrentBottle() const {
    return hideCurrentBottle;
}

void AssemblyLogic::skipToNextIngredient() {
    std::cout << "?? PRESKA?EM SASTOJAK: " << current << "\n";
    hideCurrentBottle = true;
    current = (IngredientType)(current + 1);

    if (current < INGREDIENT_COUNT) {
        glm::vec3 stackTop = getStackTopPosition();

       currentPos = stackTop + glm::vec3(0.0f, 1.0f, 0.0f);

        locked = false;
        if (current != KETCHUP && current != MUSTARD) {
            hideCurrentBottle = false;
        }

        std::cout << "   ? Novi sastojak: " << current << "\n";
        std::cout << "   ?? Stack vrh: (" << stackTop.x << ", " << stackTop.y << ", " << stackTop.z << ")\n";
        std::cout << "   ?? Nova pozicija: (" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")\n";
        std::cout << "   ??? hideCurrentBottle = " << hideCurrentBottle << "\n";
    }
    else {
        std::cout << "?? GOTOV BURGER!\n";
        locked = true;
    }
}