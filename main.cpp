#include <iostream>
#include <SFML/Graphics.hpp>
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
    sf::RenderWindow window(sf::VideoMode(640, 640), "Text Editor");
    sf::Font font;
    sf::Font debugFont;
    sf::Text text;
    sf::Text debugText;
    Cursor* cursor = new Cursor();
    font.loadFromFile("./Resources/FiraCode.ttf");
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
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::TextEntered){
                if (event.text.unicode < 128) {
                    cursor->restartTimer();
                    if (static_cast<char>(event.text.unicode) == '\b') {
                        pTable.remove(character-1, character);
                        if (character > 0)
                            character--;
                    }
                    else {
                        pTable.insert(character, std::string(1, static_cast<char>(event.text.unicode)));
                        character++;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right && character < pTable.length()) {
                    cursor->restartTimer();
                    character++;
                    pTable.resetNodeSave();
                }
                else if (event.key.code == sf::Keyboard::Left && character > 0) {
                    cursor->restartTimer();
                    character--;
                    pTable.resetNodeSave();
                }
            }
        }
        cursor->Update();
        cursor->setPos(15 * character, 24 * (pTable.indexLine(character)-1));
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
