#include <iostream>
#include <SFML/Graphics.hpp>
#include "textBuffer.h"
#include "editorUI.h"



int main() {
    // Window
    sf::RenderWindow window(sf::VideoMode({640u, 640u}), "Text Editor", 
        sf::Style::Close | sf::Style::Resize);

    // Main Piece Table and character variables
    PieceTable pTable("Hello World");
    int character = pTable.length();
    int lastRelativeLineIndex = (pTable.relativeLineIndex(character));

    // Sizes
    int characterWidth = 15;
    int charatcerHeight = 30;
    int lineGutterCharacters = 4;
    int lineGutterWidth = characterWidth * lineGutterCharacters;
    int lineGutterOffset = 7;
    int mainTextOffset = lineGutterWidth + 2 * lineGutterOffset;

    // Line Gutter
    sf::RectangleShape lineGutter(sf::Vector2f(lineGutterWidth + lineGutterOffset, window.getSize().y));
    lineGutter.setFillColor(sf::Color(21,21,21));

    // Text
    sf::Font font;
    font.openFromFile("./Resources/FiraCode.ttf");
    sf::Text text{font, ""};
    sf::Text gutterText{font, ""};
    text.setCharacterSize(24);
    gutterText.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    gutterText.setFillColor(sf::Color(80,80,80));
    text.setPosition(sf::Vector2f(mainTextOffset, 0));
    gutterText.setPosition(sf::Vector2f(0, 0));

    // Objects
    Cursor* cursor = new Cursor();
    cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(character)), 
        charatcerHeight * (pTable.getCurrentLine(character)-1));
    Debug debug(font, sf::Vector2f(0.f, 510.f), &pTable);

    // Functions
    auto gutterTextUpdate = [&] () {
        std::string gutterString;
        for (int i = 0; i < pTable.lines(); i++) {
            std::string nextString = std::to_string(i+1);
            gutterString += (std::string(nextString.length() < lineGutterCharacters ? 
                lineGutterCharacters - nextString.length() : 0, ' ')) + nextString + '\n';
        }
        gutterText.setString(gutterString);
    };
    gutterTextUpdate();
    
    // Window Update Event
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (auto* textEntered = event->getIf<sf::Event::TextEntered>()){
                if (textEntered->unicode < 128) {
                    debug.resetDebugNode();
                    cursor->restartTimer();
                    if (static_cast<char>(textEntered->unicode) == '\b') {
                        character -= pTable.remove(character-1, character);
                        if (character < 0)
                            character = 0;
                    }
                    else 
                        character += pTable.insert(character, 
                            std::string(1, static_cast<char>(textEntered->unicode)));
                    cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(character)), 
                        charatcerHeight * (pTable.getCurrentLine(character)-1));
                    lastRelativeLineIndex = (pTable.relativeLineIndex(character));
                    gutterTextUpdate();
                }
            }
            else if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                auto moveCursor = [&](int newIndex) {
                    cursor->restartTimer();
                    character = newIndex;
                    lastRelativeLineIndex = pTable.relativeLineIndex(character);
                    pTable.resetNodeSave();
                };
                switch (keyPressed->scancode) {
                    case sf::Keyboard::Scancode::Right:
                        if (character < pTable.length()) 
                            moveCursor(character + 1);
                        break;
                    case sf::Keyboard::Scancode::Left:
                        if (character > 0)
                            moveCursor(character - 1);
                        break;
                    case sf::Keyboard::Scancode::Up:
                        moveCursor(pTable.indexOnLine(lastRelativeLineIndex,
                            pTable.getCurrentLine(character) - 1));
                        break;
                    case sf::Keyboard::Scancode::Down:
                        moveCursor(pTable.indexOnLine(lastRelativeLineIndex, 
                            pTable.getCurrentLine(character) + 1));
                        break;
                    case sf::Keyboard::Scancode::Escape:
                        pTable = PieceTable("");
                        moveCursor(0);
                        gutterTextUpdate();
                        break;
                    default:
                        break;
                }
                cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(character)), 
                    charatcerHeight * (pTable.getCurrentLine(character)-1));
            }
            else if (auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (sf::Mouse::getPosition(window).x > (lineGutterWidth + lineGutterOffset) && 
                    sf::Mouse::getPosition(window).x <=  window.getSize().x &&
                    sf::Mouse::getPosition(window).y > 0 && 
                    sf::Mouse::getPosition(window).x <=  window.getSize().y) {
                    
                }
            }
            // Update input objects
            debug.update(event, character, lastRelativeLineIndex);
        }

        // Cursor manipulation
        if (sf::Mouse::getPosition(window).x > (lineGutterWidth + lineGutterOffset) && 
            sf::Mouse::getPosition(window).x <=  window.getSize().x &&
            sf::Mouse::getPosition(window).y > 0 && 
            sf::Mouse::getPosition(window).x <=  window.getSize().y)
            window.setMouseCursor(sf::Cursor::createFromSystem(sf::Cursor::Type::Text).value());
        else 
            window.setMouseCursor(sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value());

        // Update Objects
        cursor->update();
        text.setString(pTable.print());

        // Drawing
        window.clear();
        window.draw(lineGutter);
        window.draw(gutterText);
        cursor->draw(&window);
        debug.draw(&window);
        window.draw(text);
        window.display();
    }
    delete cursor;
    return 0;
}
