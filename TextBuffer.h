#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <string>

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
    PieceTable(const std::string& initialData = "");

    std::string print() const;

    int insert(int startIndex, const std::string& data);

    int remove(int startIndex, int endIndex);

    void replace(int startIndex, int endIndex, const std::string& data);

    void resetNodeSave();

    int length() const;

    int lines() const;

    int getCurrentLine(int index) const;

    int relativeLineIndex(int index) const;

    int indexOnLine(int index, int line) const;

    //DEBUG
    int countNodes() const;

    //DEBUG
    Node* getCurrentNode(int index) const;

    //DEBUG
    Node* mainNode() const;
};

#endif
