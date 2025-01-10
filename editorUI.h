#ifndef EDITORUI_H
#define EDITORUI_H

#include <SFML/Graphics.hpp>
#include "textBuffer.h"

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

    void setPos(sf::Vector2<float> pos, bool resetBlinkTimer = true);

    void setPos(float x, float y, bool resetBlinkTimer = true);

    void update();

    void draw(sf::RenderWindow* window);
};

class Debug {
private:
    sf::Text debugText;
    PieceTable* pieceTable;
    Node* currentNode;
    int const debugTextSize = 16;
public:
    Debug(sf::Font& font, PieceTable* pieceTable);

    void resetDebugNode();

    void update(std::optional<sf::Event> event, int character, 
        int lastRelativeLineIndex, int selectionIndex, int scrollY,
        bool mouseLeftHeld, bool cmdHeld, bool shiftHeld, bool optHeld);

    void setPosition(sf::Vector2f position);

    void draw(sf::RenderWindow* window);
};

#endif
