#include <SFML/Graphics.hpp>
#include "cursor.h"

Cursor::Cursor() {
    mTimerMax = sf::milliseconds(500);
    mOffset  = sf::Vector2<float>(1.f, 2.f);
    mVisible = true;
    mShape.setSize(sf::Vector2f(1.f, 24.f));
    mShape.setFillColor(sf::Color::White);
}

void Cursor::restartTimer() {
    mTimer.restart();
    mVisible = true;
}

sf::Vector2<float> Cursor::getPos() const {
    return mPos;
}

void Cursor::setPos(sf::Vector2<float> pos) {
    mPos = pos;
    mShape.setPosition((mPos + mOffset));
}

void Cursor::setPos(float x, float y) {
    mPos = sf::Vector2<float>(x, y);
    mShape.setPosition((mPos + mOffset));
}

void Cursor::Update() {
    sf::Time elapsed = mTimer.getElapsedTime();
    if (elapsed > mTimerMax) {
        mVisible = !mVisible;
        mTimer.restart();
    }
}

void Cursor::Draw(sf::RenderWindow* window) {
    if (!mVisible)
        return;
    window->draw(mShape);
}
