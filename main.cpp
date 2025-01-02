#include <iostream>
#include <SFML/Graphics.hpp>
#include "textBuffer.h"
#include "cursor.h"


void Tests() {
    PieceTable Table("hey world");
    std::cout << Table.print() << '\n';
    Table.replace(5, 7, "!!!");
    std::cout << Table.print() << '\n';
    Table.remove(5, 8);
    std::cout << Table.print() << '\n';
    Table.insert(5, "or");
    std::cout << Table.print() << '\n';
    Table.remove(2, 6);
    std::cout << Table.print() << '\n';
    Table.insert(0, "A new word: ");
    std::cout << Table.print() << '\n';
    Table.insert(15, "a");
    std::cout << Table.print() << '\n';
    Table.remove(0, 0);
    std::cout << Table.print() << '\n';
    Table.replace(1, 5, "n existing");
    std::cout << Table.print() << '\n';
    std::cout << "The length of this string is " << Table.length() << " and the amount of nodes is " << Table.countNodes() << '\n';

}

int main() {
    PieceTable pTable("Hello World");
    sf::RenderWindow window(sf::VideoMode({640u, 640u}), "Text Editor", sf::Style::Close | sf::Style::Resize);
    sf::Font font;
    font.openFromFile("./Resources/FiraCode.ttf");
    sf::Text text{font, ""};
    sf::Text debugText{font, ""};
    sf::Text gutterText{font, ""};
    Cursor* cursor = new Cursor();
    text.setCharacterSize(24);
    gutterText.setCharacterSize(24);
    gutterText.setFillColor(sf::Color(80,80,80));
    debugText.setCharacterSize(16);
    text.setFillColor(sf::Color::White);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(sf::Vector2f(0.f, 510.f));
    int character = pTable.length();
    Node* RefNode = pTable.mainNode();
    int characterWidth = 15;
    int charatcerHeight = 30;
    int lineGutterCharacters = 4;
    int lineGutterWidth = characterWidth * lineGutterCharacters;
    int lineGutterOffset = 7;
    sf::RectangleShape lineGutter(sf::Vector2f(lineGutterWidth + lineGutterOffset, window.getSize().y));
    lineGutter.setFillColor(sf::Color(21,21,21));
    int lastrelativeLineIndex = (pTable.relativeLineIndex(character));
    int mainTextOffset = lineGutterWidth + 2 * lineGutterOffset;
    cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(character)), charatcerHeight * (pTable.getCurrentLine(character)-1));
    text.setPosition(sf::Vector2f(mainTextOffset, 0));
    bool highlightingText = false;

    auto gutterTextUpdate = [&] () {
        std::string gutterString;
        for (int i = 0; i < pTable.lines(); i++) {
            std::string nextString = std::to_string(i+1);
            gutterString += (std::string(nextString.length() < lineGutterCharacters ? lineGutterCharacters - nextString.length() : 0, ' '))
                + nextString + '\n';
        }
        gutterText.setString(gutterString);
    };
    gutterTextUpdate();
    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (auto textEntered = event->getIf<sf::Event::TextEntered>()){
                if (textEntered->unicode < 128) {
                    RefNode = pTable.mainNode();
                    cursor->restartTimer();
                    if (static_cast<char>(textEntered->unicode) == '\b') {
                        character -= pTable.remove(character-1, character);
                        if (character < 0)
                            character = 0;
                    }
                    else 
                        character += pTable.insert(character, std::string(1, static_cast<char>(textEntered->unicode)));
                    cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(character)), charatcerHeight * (pTable.getCurrentLine(character)-1));
                    lastrelativeLineIndex = (pTable.relativeLineIndex(character));
                    gutterTextUpdate();
                }
            }
            else if (auto keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Right && character < pTable.length()) {
                    cursor->restartTimer();
                    character++;
                    lastrelativeLineIndex = (pTable.relativeLineIndex(character));
                    pTable.resetNodeSave();
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left && character > 0) {
                    cursor->restartTimer();
                    character--;
                    lastrelativeLineIndex = (pTable.relativeLineIndex(character));
                    pTable.resetNodeSave();
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
                    cursor->restartTimer();
                    character = pTable.indexOnLine(lastrelativeLineIndex, pTable.getCurrentLine(character)-1);
                    pTable.resetNodeSave();
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
                    cursor->restartTimer();
                    character = pTable.indexOnLine(lastrelativeLineIndex, pTable.getCurrentLine(character)+1);
                    pTable.resetNodeSave();
                }
                //Debug
                else if (keyPressed->scancode == sf::Keyboard::Scancode::PageUp && RefNode->next) {
                    RefNode = RefNode->next;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::PageDown) {
                    RefNode = pTable.mainNode();
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    pTable = PieceTable("");
                    character = 0;
                    lastrelativeLineIndex = 0;
                    RefNode = pTable.mainNode();
                    gutterTextUpdate();
                }
                cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(character)), charatcerHeight * (pTable.getCurrentLine(character)-1));
            }
        }
        if (sf::Mouse::getPosition(window).x > (lineGutterWidth + lineGutterOffset) && sf::Mouse::getPosition(window).x <=  window.getSize().x &&
            sf::Mouse::getPosition(window).y > 0 && sf::Mouse::getPosition(window).x <=  window.getSize().y)
            window.setMouseCursor(sf::Cursor::createFromSystem(sf::Cursor::Type::Text).value());
        else 
            window.setMouseCursor(sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value());

        cursor->Update();
        std::string DebugNodeText = RefNode->data;
        std::replace(DebugNodeText.begin(), DebugNodeText.end(), '\n', '>');
        debugText.setString("Length: " + std::to_string(pTable.length()) + ", Nodes: " + std::to_string(pTable.countNodes()) + ", Character: " +
            std::to_string(character) + ", Lines: " + std::to_string(pTable.lines()) + "\nCurrent Index's Node: " + std::to_string(pTable.getCurrentNode(character)->nodeID)
            + ", Node Line: " + std::to_string(pTable.getCurrentNode(character)->line) + "\nSelected Node: " + std::to_string(RefNode->nodeID) + ", Sel. Node's Line: " + std::to_string(RefNode->line) + 
            "\nSel. Node's Data: " + DebugNodeText + "\nSel. Node's StartIndex: " + std::to_string(RefNode->startIndex) +
            ", Index on Line: " + std::to_string(pTable.relativeLineIndex(character)) + ", Index's line: " + std::to_string(pTable.getCurrentLine(character)) + "\nLast Line Index: " + std::to_string(lastrelativeLineIndex));
        text.setString(pTable.print());
        window.clear();
        window.draw(lineGutter);
        window.draw(gutterText);
        cursor->Draw(&window);
        window.draw(text);
        window.draw(debugText);
        window.display();
    }
    delete cursor;
    return 0;
}
