#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "TextBuffer.h"

class Cursor {
private:
    sf::Clock mTimer;
    const sf::Time mTimerMax = sf::milliseconds(500);
    sf::Vector2<float> mPos;
    bool mVisible;
    sf::RectangleShape mShape;
    sf::Vector2<float> mOffset = sf::Vector2<float>(1.f, 2.f);
public:
    Cursor() {
        mVisible = true;
        mShape.setSize(sf::Vector2f(1.f, 24.f));
        mShape.setFillColor(sf::Color::White);
    }

    void restartTimer() {
        mTimer.restart();
        mVisible = true;
    }

    sf::Vector2<float> getPos() const {
        return mPos;
    }

    void setPos(sf::Vector2<float> pos) {
        mPos = pos;
        mShape.setPosition(mPos + mOffset);
    }

    void setPos(float x, float y) {
        mPos = sf::Vector2<float>(x, y);
        mShape.setPosition(mPos + mOffset);
    }

    void Update() {
        sf::Time elapsed = mTimer.getElapsedTime();
        if (elapsed > mTimerMax) {
            mVisible = !mVisible;
            mTimer.restart();
        }
    }

    void Draw(sf::RenderWindow* Window) {
        if (!mVisible)
            return;
        Window->draw(mShape);
    }
};

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
    sf::Font debugFont;
    sf::Text text{font, ""};
    sf::Text debugText{font, ""};
    Cursor* cursor = new Cursor();
    font.openFromFile("./Resources/FiraCode.ttf");
    text.setFont(font);
    debugText.setFont(font);
    text.setCharacterSize(24);
    debugText.setCharacterSize(16);
    text.setFillColor(sf::Color::White);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(sf::Vector2f(0.f, 600.f));
    int character = pTable.length();
    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (auto textEntered = event->getIf<sf::Event::TextEntered>()){
                if (textEntered->unicode < 128) {
                    cursor->restartTimer();
                    if (static_cast<char>(textEntered->unicode) == '\b') {
                        pTable.remove(character-1, character);
                        if (character > 0)
                            character--;
                    }
                    else {
                        pTable.insert(character, std::string(1, static_cast<char>(textEntered->unicode)));
                        character++;
                    }
                }
            }
            else if (auto keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Right && character < pTable.length()) {
                    cursor->restartTimer();
                    character++;
                    pTable.resetNodeSave();
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left && character > 0) {
                    cursor->restartTimer();
                    character--;
                    pTable.resetNodeSave();
                }
            }
        }
        cursor->Update();
        cursor->setPos(15 * (pTable.lineIndex(character)), 30 * (pTable.indexLine(character)-1));
        debugText.setString("Length: " + std::to_string(pTable.length()) + ", Nodes: " + std::to_string(pTable.countNodes()) + ", Character: " +
            std::to_string(character) + ", Lines: " + std::to_string(pTable.lines()));
        text.setString(pTable.print());
        window.clear();
        cursor->Draw(&window);
        window.draw(text);
        window.draw(debugText);
        window.display();
    }
    delete cursor;
    return 0;
}
