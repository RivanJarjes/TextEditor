#include <iostream>
#include "TextBuffer.h"


Node::Node(const std::string& data, Node* next, int startIndex, int line)
    : data(data), next(next), startIndex(startIndex), line(line) {}

PieceTable::PieceTable(const std::string& initialData) {
    mLines = 1;
    mLastIndex = -1;
    if (initialData.empty()) {
        mPieces = new Node();
        mLength = 0;
    }
    else {
        mPieces = new Node(initialData);
        mLength = initialData.length();
    }
}

std::string PieceTable::print() const {
    std::string output = "";
    Node* currNode = mPieces;
    while (currNode) {
        output += currNode->data;
        currNode = currNode->next;
    }
    return output;
}

void PieceTable::insert(int startIndex, const std::string& data) {
    if (mLastIndex != -1 && mLastIndex == startIndex - 1 && mLastNode && !data.contains('\n')) {
        mLastNode->data += data;
        mLength++;
        mLastIndex++;
        return;
    }
    mLastIndex = -1;
    mLastNode = nullptr;
    replace(startIndex, startIndex, data);
}

void PieceTable::remove(int startIndex, int endIndex) {
    if (startIndex == endIndex || mLength == 0) return;
    replace(startIndex, endIndex, "");
}

void PieceTable::resetNodeSave() {
    mLastIndex = -1;
    mLastNode = nullptr;
}

int PieceTable::indexLine(int index) const{
    Node* currNode = mPieces;
    while (currNode) {
        if (index <= currNode->startIndex + currNode->data.length()) {
            return currNode->line;
        }
        currNode = currNode->next;
    }
    return 1;
}

int PieceTable::lineIndex(int index) const {
    Node* currNode = mPieces;
    int currentLine = 1;
    int characterDisposition = 0;
    while (currNode) {
        if (currentLine != currNode->line) {
            characterDisposition += currNode->startIndex;
            currentLine = currNode->line;
        }
        if (index <= currNode->startIndex + currNode->data.length()) {
            return index - characterDisposition - (currentLine -1);
        }
        currNode = currNode->next;
    }
}

void PieceTable::replace(int startIndex, int endIndex, const std::string& data) {
    if (startIndex < 0 || endIndex > mLength || startIndex > endIndex) {
       throw std::out_of_range("Invalid start or end index");
    }

    // Alter the length by adding in new text minus displaced existing text
    mLength += data.length() - (endIndex - startIndex);

    int LineBreaks = 0;
    for (int i = 0; i < data.length(); i++) {
        if (data[i] == '\n')
            LineBreaks++;
    }
    mLines += LineBreaks;

    Node* prevNode = nullptr;
    Node* currNode = mPieces;

    while (currNode && startIndex > currNode->startIndex + currNode->data.length()){
        prevNode = currNode;
        currNode = prevNode->next;
    }

    if (!currNode) {
        if (prevNode) {
            prevNode->next = new Node(data, nullptr, prevNode->startIndex + data.length());
        } else {
            mPieces = new Node(data);
        }
        return;
    }

    if (startIndex < currNode->startIndex + currNode->data.length()) {
        int offset = startIndex - currNode->startIndex;
        int length = currNode->data.length();
        std::string prevData;
        if (offset > -length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset > length) usedOffset = length;
            prevData = currNode->data.substr(0, usedOffset);
        }
        else
            prevData = "";
        std::string nextData;
        if (offset < length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset < 0) usedOffset = 0;
            nextData = currNode->data.substr(usedOffset);
        }
        else
            nextData = "";
        Node* newNode = new Node(nextData, currNode->next, currNode->startIndex + offset);
        currNode->next = newNode;
        currNode->data = prevData;
    }

    Node* anchorNode = currNode;
    prevNode = anchorNode;
    currNode = prevNode->next;

    std::vector<Node*> toDelete = {};
    while (currNode && endIndex >= currNode->startIndex + currNode->data.length()) {
        toDelete.push_back(currNode);
        prevNode = currNode;
        currNode = prevNode->next;
    }

    if (currNode && endIndex < currNode->startIndex + currNode->data.length()) {
        int offset = endIndex - currNode->startIndex;
        int length = currNode->data.length();
        std::string prevData;
        if (offset > -length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset > length) usedOffset = length;
            prevData = currNode->data.substr(0, usedOffset);
        }
        else
            prevData = "";
        std::string nextData;
        if (offset < length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset < 0) usedOffset = 0;
            nextData = currNode->data.substr(usedOffset);
        }
        else
            nextData = "";
        Node* newNode = new Node(nextData, currNode->next, currNode->startIndex + offset);
        currNode->next = newNode;
        currNode->data = prevData;
        toDelete.push_back(currNode);
    }

    Node* firstRightNode;
    if (!toDelete.empty())
        firstRightNode = toDelete.back()->next;
    else
        firstRightNode = anchorNode->next;
    anchorNode->next = nullptr;

    while (!toDelete.empty()) {
        Node* nodeToDelete = toDelete.back();
        toDelete.pop_back();
        if (nodeToDelete) {
            delete nodeToDelete;
            nodeToDelete = nullptr;
        }
    }

    Node* newDataNode;
    if (!data.empty()) {
        newDataNode = new Node(data, firstRightNode,
            anchorNode->startIndex + anchorNode->data.length(),
            anchorNode->line + LineBreaks);
        mLastNode = newDataNode;
        mLastIndex = startIndex;
    }
    else
        newDataNode = firstRightNode;

    anchorNode->next = newDataNode;

    if (firstRightNode) {
        prevNode = newDataNode;
        Node* nextNode = firstRightNode;
        while (nextNode) {
            nextNode->startIndex = prevNode->startIndex + prevNode->data.length();
            nextNode->line = prevNode->line + LineBreaks;
            prevNode = nextNode;
            nextNode = prevNode->next;
        }
    }
}

int PieceTable::length() const {
    return mLength;
}

int PieceTable::lines() const {
    return mLines;
}

int PieceTable::countNodes() const {
    int output = 0;
    Node* currNode = mPieces;
    while (currNode) {
        output += 1;
        currNode = currNode->next;
    }
    return output;
}