#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <string>
#include <vector>
#include <stdexcept>

struct Node {
    std::string data;
    Node* next;
    int startIndex;
    int line;

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

    void insert(int startIndex, const std::string& data);

    void remove(int startIndex, int endIndex);

    void replace(int startIndex, int endIndex, const std::string& data);

    void resetNodeSave();

    int length() const;

    int lines() const;

    int countNodes() const;

    int indexLine(int index) const;
};

#endif