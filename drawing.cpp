#include "drawing.h"

DrawableState Drawable::getState() {
    return state;
}

Drawable::~Drawable() {
    //no particular destructor behavior needed.
    //virtual destructor defined to support polymorphism in destructors
}

void Drawable::resetState() {
    state = NormalS;
}

