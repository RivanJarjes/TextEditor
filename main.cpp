#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include "textBuffer.h"
#include "editorUI.h"
#include "clip/clip.h"
#include "fileDialog.h"


void removeText(PieceTable* pieceTable, int* characterIndex, int* selectionIndex) {
    int change;
    if (*selectionIndex == *characterIndex) {
        change = pieceTable->remove(*characterIndex-1, *characterIndex);
        *selectionIndex = *characterIndex += change;
    }else {
        int minValue = std::min(*characterIndex, *selectionIndex);
        int maxValue = std::max(*characterIndex, *selectionIndex);
        change = pieceTable->remove(minValue, maxValue);
        *characterIndex = *selectionIndex = maxValue + change;
    }
    if (*characterIndex < 0 || *selectionIndex < 0)
        characterIndex = selectionIndex = 0;
};

void addCharacter(PieceTable* pieceTable, char character, int* characterIndex, int* selectionIndex) {
    int change;
    if (*selectionIndex == *characterIndex) {
        change = pieceTable->insert(*characterIndex,
        std::string(1, static_cast<char>(character)));
    } else {
        change = pieceTable->replace(std::min(*characterIndex, *selectionIndex), 
        std::max(*characterIndex, *selectionIndex),
        std::string(1, static_cast<char>(character)));
    }
    *selectionIndex = *characterIndex = std::max(*characterIndex, *selectionIndex) + change;
    if (*characterIndex < 0)
        *selectionIndex = *characterIndex = 0;
    else if (*characterIndex > pieceTable->length())
        *selectionIndex = *characterIndex = pieceTable->length();
};

int main() {
    // Window
    sf::RenderWindow window(sf::VideoMode({640u, 640u}), "Text Editor", 
        sf::Style::Close | sf::Style::Resize);

    // Main Piece Table and characterIndex variables
    PieceTable pTable("");
    int characterIndex = pTable.length();
    int lastRelativeLineIndex = (pTable.relativeLineIndex(characterIndex));
    int selectionIndex = pTable.length();

    // Transformation values
    int scrollX = 0;
    int scrollY = 0;

    // Sizes
    int characterWidth = 15;
    int characterHeight = 30;
    int lineGutterCharacters = 4;
    int lineGutterWidth = characterWidth * lineGutterCharacters;
    int lineGutterOffset = 7;
    int mainTextOffset = lineGutterWidth + 2 * lineGutterOffset;
    int scrollBarWidth = 14;

    // Line Gutter
    sf::RectangleShape lineGutter(sf::Vector2f(lineGutterWidth + lineGutterOffset, window.getSize().y));
    lineGutter.setFillColor(sf::Color(21,21,21));

    // Text
    sf::Font font;
    if (!font.openFromFile("./Resources/FiraCode.ttf")) {
        std::cerr << "Error loading font" << '\n';
        return -1;
    }
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
    Debug debug(font, &pTable);
    std::vector<sf::RectangleShape> selectionBoxes;
    sf::RectangleShape scrollBar(sf::Vector2f(14, 20));

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

    auto gutterPositionUpdate = [&] () {
        gutterText.setPosition(sf::Vector2f(0, scrollY));
    };
    gutterPositionUpdate();

    auto cursorPositionUpdate = [&] (bool resetTimer = true) {
        cursor->setPos(mainTextOffset + characterWidth * (pTable.relativeLineIndex(characterIndex)) + scrollX, 
            (characterHeight) * (pTable.getCurrentLine(characterIndex)-1) + scrollY, resetTimer);
    };
    cursorPositionUpdate();
    
    auto debugPositionUpdate = [&] () {
        debug.setPosition(sf::Vector2f(0.f, window.getSize().y-170));
    };
    debugPositionUpdate();

    // Toggles
    bool showDebug = false;

    //Cache variables
    int lastLine = -1;
    int lastIndex = -1;

    // Inputs
    bool cmdHeld = false;
    bool optHeld = false;
    bool shiftHeld = false;
    bool selectionChanged = false;
    bool mouseLeftHeld = false;

    // Window Update Event
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (auto* textEntered = event->getIf<sf::Event::TextEntered>()){
                if (textEntered->unicode < 128) {
                    debug.resetDebugNode();
                    if (static_cast<char>(textEntered->unicode) == '\b') 
                        removeText(&pTable, &characterIndex, &selectionIndex);
                    else 
                        addCharacter(&pTable, static_cast<char>(textEntered->unicode), 
                        &characterIndex, &selectionIndex);
                        
                    cursorPositionUpdate();
                    lastRelativeLineIndex = (pTable.relativeLineIndex(characterIndex));
                    gutterTextUpdate();
                }
            }

            if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                auto moveCursor = [&](int newIndex, bool changeRelativeIndex = true) {
                    characterIndex = selectionIndex = newIndex;
                    if (changeRelativeIndex)
                        lastRelativeLineIndex = pTable.relativeLineIndex(characterIndex);
                    pTable.resetNodeSave();
                };
                int movingIndex;
                switch (keyPressed->scancode) {
                    case sf::Keyboard::Scancode::LSystem:
                        cmdHeld = true;
                        break;
                    case sf::Keyboard::Scancode::LAlt:
                        optHeld = true;
                        break;
                    case sf::Keyboard::Scancode::LShift:
                        shiftHeld = true;
                        break;
                    case sf::Keyboard::Scancode::Right:
                        selectionChanged = true;
                        if (shiftHeld) {
                            if (characterIndex < pTable.length()){
                                characterIndex++;
                                lastRelativeLineIndex = pTable.relativeLineIndex(std::min(characterIndex, 
                                    selectionIndex));
                            }
                            break;
                        }
                        movingIndex = std::max(characterIndex, selectionIndex);
                        if (movingIndex < pTable.length() && characterIndex == selectionIndex) 
                            moveCursor(movingIndex + 1);
                        else
                            moveCursor(movingIndex);
                        break;
                    case sf::Keyboard::Scancode::Left:
                        selectionChanged = true;
                        if (shiftHeld) {
                            if (characterIndex > 0){
                                characterIndex--;
                                lastRelativeLineIndex = pTable.relativeLineIndex(std::min(characterIndex, 
                                    selectionIndex));
                            }
                            break;
                        }
                        movingIndex = std::min(characterIndex, selectionIndex);
                        if (movingIndex > 0 && characterIndex == selectionIndex) 
                            moveCursor(movingIndex - 1);
                        else 
                            moveCursor(movingIndex);
                        break;
                    case sf::Keyboard::Scancode::Up:
                        selectionChanged = true;
                        if (cmdHeld){
                            moveCursor(0);
                            break;
                        }
                        else if (shiftHeld) {
                            characterIndex = pTable.indexOnLine(lastRelativeLineIndex,
                                pTable.getCurrentLine(characterIndex) - 1);
                            break;
                        }
                        moveCursor(pTable.indexOnLine(lastRelativeLineIndex,
                            pTable.getCurrentLine(characterIndex) - 1), false);
                        break;
                    case sf::Keyboard::Scancode::Down:
                        selectionChanged = true;
                        if (cmdHeld){
                            moveCursor(pTable.length());
                            break;
                        }else if (shiftHeld) {
                            characterIndex = pTable.indexOnLine(lastRelativeLineIndex,
                                pTable.getCurrentLine(characterIndex) + 1);
                            break;
                        }
                        moveCursor(pTable.indexOnLine(lastRelativeLineIndex, 
                            pTable.getCurrentLine(characterIndex) + 1), false);
                        break;
                    case sf::Keyboard::Scancode::F3:
                        pTable = PieceTable("");
                        moveCursor(0);
                        selectionIndex = 0;
                        gutterTextUpdate();
                        break;
                    case sf::Keyboard::Scancode::F1:
                        showDebug = !showDebug;
                        break;
                    case sf::Keyboard::Scancode::F2:
                        std::cout << pTable.mHistory.historyLength() << '\n';
                        break;
                    case sf::Keyboard::Scancode::C:
                        if (cmdHeld && selectionIndex != characterIndex) {
                            clip::set_text(pTable.printSelection(std::min(characterIndex, selectionIndex),
                                std::max(characterIndex, selectionIndex)));
                        }
                        break;
                    case sf::Keyboard::Scancode::X:
                        if (cmdHeld && selectionIndex != characterIndex) {
                            clip::set_text(pTable.printSelection(std::min(characterIndex, selectionIndex),
                                std::max(characterIndex, selectionIndex)));
                            removeText(&pTable, &characterIndex, &selectionIndex);
                        }
                        break;
                    case sf::Keyboard::Scancode::V:
                        if (cmdHeld) {
                            std::string paste;
                            clip::get_text(paste);
                            int change = pTable.replace(std::min(characterIndex, selectionIndex), 
                            std::max(characterIndex, selectionIndex), paste);
                            selectionIndex = characterIndex = std::max(selectionIndex, characterIndex) + change;
                            lastRelativeLineIndex = (pTable.relativeLineIndex(characterIndex));
                            gutterTextUpdate();
                        }
                        break;
                    case sf::Keyboard::Scancode::A:
                        if (cmdHeld) {
                            selectionIndex = 0;
                            characterIndex = pTable.length();
                            selectionChanged = true;
                        }
                        break;
                    case sf::Keyboard::Scancode::Z:
                        if (cmdHeld) {
                            if (shiftHeld) {
                                int change = pTable.redo();
                                if (change != -1) 
                                    selectionIndex = characterIndex = change;
                                selectionChanged = true;
                                gutterTextUpdate();
                                break;
                            }

                            sf::Vector2i change = pTable.undo();
                            if (change.y == -1)
                                break;
                            if (change.x == -1)
                                characterIndex = selectionIndex = change.y;
                            else {
                                selectionIndex = change.x;
                                characterIndex = change.y;
                            }
                            selectionChanged = true;
                            gutterTextUpdate();
                            if (characterIndex > pTable.length() || selectionIndex > pTable.length()) 
                                characterIndex = selectionIndex = pTable.length();
                        }
                        break;
                    case sf::Keyboard::Scancode::O:
                        if (cmdHeld) {
                            std::string selectedFile = ShowOpenFileDialog();
                            
                            if (!selectedFile.empty()) {
                                std::ifstream file(selectedFile);
                                if (file.is_open()) {
                                    pTable.open(file);
                                    file.close();
                                }
                                break;
                            }

                            std::cout << "No file selected or user canceled.\n";
                        }
                        break;
                    case sf::Keyboard::Scancode::S:
                        if (cmdHeld) {
                            pTable.save();
                        }
                        break;
                    
                    default:
                        break;
                }
                cursorPositionUpdate();
            }

            if (auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                switch (keyReleased->scancode) {
                    case sf::Keyboard::Scancode::LSystem:
                        cmdHeld = false;
                        break;
                    case sf::Keyboard::Scancode::LAlt:
                        optHeld = false;
                        break;
                    case sf::Keyboard::Scancode::LShift:
                        shiftHeld = false;
                        break;
                    default:
                        break;
                }
            } 

            if (auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left &&
                        sf::Mouse::getPosition(window).x > (lineGutterWidth + lineGutterOffset) && 
                        sf::Mouse::getPosition(window).x <=  window.getSize().x &&
                        sf::Mouse::getPosition(window).y > 0 && 
                        sf::Mouse::getPosition(window).y <=  window.getSize().y) {
                    selectionChanged = true;
                    mouseLeftHeld = true;
                    int line = (sf::Mouse::getPosition(window).y + characterHeight - scrollY) / characterHeight;
                    if (line < 1)
                        line = 1;
                    int index = (sf::Mouse::getPosition(window).x - mainTextOffset + characterWidth / 2 - scrollX) 
                        / characterWidth;
                    characterIndex = selectionIndex = pTable.indexOnLine(index, line);
                    cursorPositionUpdate();
                }
            }

            if (auto* mouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseButtonReleased->button == sf::Mouse::Button::Left)
                    mouseLeftHeld = false;
            }

            if (auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
                switch (mouseWheelScrolled->wheel) {
                    case sf::Mouse::Wheel::Vertical:
                        scrollY += mouseWheelScrolled->delta;
                        if (scrollY > 0) 
                            scrollY = 0;
                        text.setPosition(sf::Vector2f(mainTextOffset + scrollX, scrollY));
                        cursorPositionUpdate(false);
                        gutterPositionUpdate();
                        selectionChanged = true;
                        break;
                    case sf::Mouse::Wheel::Horizontal:
                        scrollX += mouseWheelScrolled->delta;
                        if (scrollX > 0) 
                            scrollX = 0;
                        text.setPosition(sf::Vector2f(mainTextOffset + scrollX, scrollY));
                        cursorPositionUpdate(false);
                        selectionChanged = true;
                        break;
                }
            } 
            
            if (auto resized = event->getIf<sf::Event::Resized>()) {
                sf::View view = window.getView();
                view.setSize(sf::Vector2f(static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)));
                view.setCenter(sf::Vector2f(static_cast<float>(resized->size.x) / 2.f, 
                    static_cast<float>(resized->size.y) / 2.f));
                window.setView(view);
                lineGutter.setSize(sf::Vector2f(lineGutterWidth + lineGutterOffset, window.getSize().y));
                debugPositionUpdate();
            }
            
            // Update input objects
            if (showDebug)
                debug.update(event, characterIndex, lastRelativeLineIndex, selectionIndex, scrollY,
                    mouseLeftHeld, cmdHeld, shiftHeld, optHeld);
        }

        // Cursor manipulation
        if (sf::Mouse::getPosition(window).x > (lineGutterWidth + lineGutterOffset) && 
                sf::Mouse::getPosition(window).x <=  window.getSize().x &&
                sf::Mouse::getPosition(window).y > 0 && 
                sf::Mouse::getPosition(window).y <=  window.getSize().y){
            if (mouseLeftHeld) {
                int line = (sf::Mouse::getPosition(window).y + characterHeight - scrollY) / characterHeight;
                if (line < 1)
                    line = 1;
                int index = (sf::Mouse::getPosition(window).x - mainTextOffset + characterWidth / 2 - scrollX) 
                    / characterWidth;
                characterIndex = pTable.indexOnLine(index, line);
                if (line != lastLine || index != lastIndex) {
                    lastLine = line;
                    lastIndex = index;
                    selectionChanged = true;
                    cursorPositionUpdate();
                }
            }
            window.setMouseCursor(sf::Cursor::createFromSystem(sf::Cursor::Type::Text).value());
        }
        else if (!mouseLeftHeld){
            window.setMouseCursor(sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value());
        }


        if (selectionIndex != characterIndex && selectionChanged) {
            std::vector<std::array<int, 3>> selectionPositions = pTable.getSelectionBoxes(
                std::min(selectionIndex, characterIndex), std::max(selectionIndex, characterIndex));
            selectionBoxes.clear();
            for (std::array<int, 3>& i : selectionPositions) {
                if (i[0] < 0 || i[1] < 0 || i[2] < 0)
                    continue;
                sf::RectangleShape selectionBox;
                selectionBox.setSize(sf::Vector2f((i[1]-i[0]) * characterWidth, characterHeight));
                selectionBox.setPosition(sf::Vector2f(i[0] * characterWidth + mainTextOffset + scrollX, 
                    characterHeight * (i[2]-1) + scrollY));
                selectionBox.setFillColor(sf::Color(40, 67, 107));
                selectionBoxes.push_back(selectionBox);
            }
            selectionChanged = false;
        }else if (selectionIndex == characterIndex && !selectionBoxes.empty()) 
            selectionBoxes.clear();

        // Update Objects
        cursor->update();
        text.setString(pTable.print());

        // Drawing
        window.clear();
        for (sf::RectangleShape& i : selectionBoxes) 
            window.draw(i);
        window.draw(text);
        cursor->draw(&window);
        window.draw(lineGutter);
        window.draw(gutterText);
        if (showDebug)
            debug.draw(&window);
        window.display();
    }
    delete cursor;
    return 0;
}
