#include <SFML/Graphics.hpp>
#include "editorUI.h"
#include "textBuffer.h"

/*
   Cursor Class
*/

Cursor::Cursor() {
    mTimerMax = sf::milliseconds(500);     // Timer for cursor blinking
    mOffset  = sf::Vector2<float>(1.f, 2.f); // Offset for position of cursor to look more even
    mVisible = true;                              // Used for draw method to show when to draw cursor (doesnt when blinks)
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

void Cursor::setPos(sf::Vector2<float> pos, bool resetBlinkTimer) {
    if (resetBlinkTimer)
        restartTimer(); // Resets timer 
    mPos = pos;
    mShape.setPosition((mPos + mOffset));
}

void Cursor::setPos(float x, float y, bool resetBlinkTimer) {
    if (resetBlinkTimer)
        restartTimer(); // Resets timer 
    mPos = sf::Vector2<float>(x, y);
    mShape.setPosition((mPos + mOffset));
}

void Cursor::update() {
    sf::Time elapsed = mTimer.getElapsedTime();
    if (elapsed > mTimerMax) {
        mVisible = !mVisible;   // Toggles Visibility when supposed to blink
        mTimer.restart();
    }
}

void Cursor::draw(sf::RenderWindow* window) {
    if (!mVisible)
        return;
    window->draw(mShape); 
}

/* 
    Debug Class
*/

Debug::Debug(sf::Font& font, PieceTable* pieceTable): debugText(font, ""){
    debugText.setCharacterSize(debugTextSize);
    debugText.setFillColor(sf::Color::White);
    this->pieceTable = pieceTable;
    currentNode = pieceTable->mainNode();
}

void Debug::resetDebugNode() { // Resets the node being read to the original parent node.
    currentNode = pieceTable->mainNode();
}

void Debug::update(std::optional<sf::Event> event, int character, int lastRelativeLineIndex, int selectionIndex,
        int scrollY, bool mouseLeftHeld, bool cmdHeld, bool shiftHeld, bool optHeld) {
    // Node navigation: used to look at each individual node in piece table.
    if (auto keyPressed = event->template getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::PageUp && currentNode->next) 
            currentNode = currentNode->next;
        else if (keyPressed->scancode == sf::Keyboard::Scancode::PageDown) 
            currentNode = pieceTable->mainNode();
    }
    std::string debugNodeText = currentNode->data;  // Changes all new lines to '>' for readability
    std::replace(debugNodeText.begin(), debugNodeText.end(), '\n', '>'); 
    debugText.setString(                            // Print variables important for debugging
        "Length: " + std::to_string(pieceTable->length()) +
        ", Nodes: " + std::to_string(pieceTable->countNodes()) +
        ", Character: " + std::to_string(character) + ", Lines: " +
        std::to_string(pieceTable->lines()) + "\nCurrent Index's Node: " +
        std::to_string(pieceTable->getCurrentNode(character)->nodeID) +
        ", Node Line: " +
        std::to_string(pieceTable->getCurrentNode(character)->line) +
        "\nSelected Node: " + std::to_string(currentNode->nodeID) +
        ", Sel. Node's Line: " + std::to_string(currentNode->line) +
        "\nSel. Node's Data: " + debugNodeText +
        "\nSel. Node's StartIndex: " +
        std::to_string(currentNode->startIndex) + ", Index on Line: " +
        std::to_string(pieceTable->relativeLineIndex(character)) +
        ", Index's line: " +
        std::to_string(pieceTable->getCurrentLine(character)) +
        "\nLast Line Index: " + std::to_string(lastRelativeLineIndex) +
        ", Selection Index: " + std::to_string(selectionIndex) + '\n' +
        "Scroll Y: " + std::to_string(scrollY) + '\n' +
        (mouseLeftHeld ? " LMB HELD " : "") + (cmdHeld ? " CMD HELD " : "") + 
        (shiftHeld ? " SHIFT HELD " : "") + (optHeld ? " OPTION HELD " : ""));
}

void Debug::setPosition(sf::Vector2f position) {
    debugText.setPosition(position);
}

void Debug::draw(sf::RenderWindow* window) {
    window->draw(debugText);
}
