#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <string>
#include "history.h"
#include <SFML/Graphics.hpp>
#include <fstream>

struct Node {
    std::string data;
    Node* next;
    int startIndex;
    int line;
    int nodeID;

    Node(const std::string& data = "", Node* next = nullptr, int startIndex = 0,
        int line = 1);
};

class PieceTable {
private:
    Node* mPieces;
    int mLength;
    int mLines;
    Node* mLastNode;
    int mLastIndex;

public:
    History mHistory; // make private later

    PieceTable(const std::string& initialData = "");

    std::string print() const;

    std::string printSelection(int startIndex, int endIndex) const;

    int insert(int startIndex, const std::string& data);

    int remove(int startIndex, int endIndex);

    int replace(int startIndex, int endIndex, const std::string& data, 
        bool isRecalled = false, bool undo = false, bool resetNode = true);

    void resetNodeSave();

    int length() const;

    int lines() const;

    int getCurrentLine(int index) const;

    int relativeLineIndex(int index) const;

    int indexOnLine(int index, int line) const;

    std::vector<std::array<int, 3>> getSelectionBoxes(int startIndex, int endIndex);

    sf::Vector2i undo();

    int redo();

    void open(std::ifstream& file);

    void save();

    //DEBUG
    int countNodes() const;

    //DEBUG
    Node* getCurrentNode(int index) const;

    //DEBUG
    Node* mainNode() const;
};

#endif
