#ifndef CURSOR_H
#define CURSOR_H

#include <SFML/Graphics.hpp>

class Cursor {
private:
    sf::Clock mTimer;
    sf::Time mTimerMax;
    sf::Vector2<float> mPos;
    bool mVisible;
    sf::RectangleShape mShape;
    sf::Vector2<float> mOffset;
public:
    Cursor();

    void restartTimer();

    sf::Vector2<float> getPos() const;

    void setPos(sf::Vector2<float> pos);

    void setPos(float x, float y);

    void Update();

    void Draw(sf::RenderWindow* window);
};

#endif
