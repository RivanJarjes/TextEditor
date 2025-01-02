#include <iostream>
#include "textBuffer.h"

// For debug purposes: each node has a discrete ID which increments from the previously created node.
std::atomic<int> nodeID{};

int create_id() {
    return nodeID++;  // Increments the ID for the next node
}

// Essentially just a linked list
Node::Node(const std::string& data, Node* next, int startIndex, int line)
    : data(data), next(next), startIndex(startIndex), line(line), nodeID(create_id()) {}

PieceTable::PieceTable(const std::string& initialData) {
    mLines = 1;      // Line Count
    mLastIndex = -1; // Will be used to add continuously typed character to the same node (w/o making a new one)
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
    while (currNode) {  // Goes through all nodes in order until the chain breaks
        output += currNode->data;
        currNode = currNode->next;
    }
    return output;
}

int PieceTable::insert(int startIndex, const std::string& data) {
    //If tab, replace it with three spaces for simplicity sake
    std::string modifiedData = data;
    if (modifiedData == "\t") {
        int whiteSpace = 0; //Counts how much whitespace occurs before index @ line to have an even tab
        int line = getCurrentLine(startIndex);
        Node* currNode = mPieces;
        while (currNode) {
            if (currNode->line == line){
                if (startIndex <= currNode->startIndex + currNode->data.length()) {
                    for (int i = 0; i < startIndex - currNode->startIndex; i++) 
                        if (currNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
                    break;
                }
                else 
                    for (int i = 0; i < currNode->data.length(); i++) 
                        if (currNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
            }
            else if(currNode-> line > line)
                break;
            currNode = currNode->next;
        }
        modifiedData = std::string(4 - whiteSpace % 4, ' '); // Adds an even amount of whitespace.
    }
    // If a character was just typed at the previous index AND the cursor did not move, it will just
    // be added to the previous node.
    if (mLastIndex != -1 && mLastIndex == startIndex - 1 && mLastNode && modifiedData.find('\n') == modifiedData.npos) {
        mLastNode->data += modifiedData;
        mLength += modifiedData.length();
        mLastIndex++;
        Node* currNode = mLastNode->next;
        // Update the starting indexes of each node afterwards.
        while (currNode) {
            currNode->startIndex += modifiedData.length();
            currNode = currNode->next;
        }
        return modifiedData.length();
    }
    mLastIndex = -1;
    mLastNode = nullptr;
    // Calls replace function by not replacing any character (start and end index are equal)
    replace(startIndex, startIndex, modifiedData);
    return modifiedData.length();
}

int PieceTable::remove(int startIndex, int endIndex) {
    // If can't erase, return
    if (startIndex == endIndex || mLength == 0) 
        return 0;
    // Checks if theres a tab
    if (endIndex - startIndex == 1) {
        int whiteSpace = 0; //Counts how much whitespace before character, if exactly 4 or divisible by 4, thats a tab
        int line = getCurrentLine(endIndex);
        Node* currNode = mPieces;
        while (currNode) {
            if (currNode->line == line){
                if (endIndex <= currNode->startIndex + currNode->data.length()) {
                    for (int i = 0; i < endIndex - currNode->startIndex; i++) 
                        if (currNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
                    break;
                }
                else 
                    for (int i = 0; i < currNode->data.length(); i++) 
                        if (currNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
            }
            else if(currNode-> line > line)
                break;
            currNode = currNode->next;
        }
        if (whiteSpace && whiteSpace % 4 == 0) {
            replace(startIndex-3, endIndex, "");
            return 4;
        }
    }
    // Calls replace function without any data
    replace(startIndex, endIndex, "");

    return endIndex - startIndex;
}

// To be called when cursor moves to reset last saved node, thus will make a new node next insertion
// Even makes new node if user returns to original last node position, (will be) used for undo/redo procedure.
void PieceTable::resetNodeSave() {
    mLastIndex = -1;
    mLastNode = nullptr;
}

// Gets the current line the index is located at.
int PieceTable::getCurrentLine(int index) const{
    Node* currNode = mPieces;
    while (currNode) {
        if (index <= currNode->startIndex + currNode->data.length() || !currNode->next) 
            return currNode->line;
        currNode = currNode->next;
    }
    return 1;
}

// Gets the index relative to it's current line. Used for cursor location.
int PieceTable::relativeLineIndex(int index) const {
    Node* currNode = mPieces;
    int currentLine = 1;
    int characterDisposition = 0;
    while (currNode) {
        if (currentLine != currNode->line) {
            characterDisposition = currNode->startIndex + 1;
            currentLine = currNode->line;
        }
        if (index <= currNode->startIndex + currNode->data.length() || !currNode->next) 
            return index - characterDisposition ;
        currNode = currNode->next;
    }
    return 0;
}

// Gets the current node the index is located at. For debugging purposes mainly.
Node* PieceTable::getCurrentNode(int index) const {
    Node* currNode = mPieces;
    while (currNode) {
        if (index <= currNode->startIndex + currNode->data.length() || !currNode->next) 
            return currNode;
        currNode = currNode->next;
    }
    return nullptr;
}

// Gets the global index of a requested relative index on a specific line.
int PieceTable::indexOnLine(int index, int line) const {
    // If tries going above first line, just go to beginning.
    if (line == 0)
        return 0;
    Node* currNode = mPieces;
    int offset = 0;
    bool foundLine = false;
    while (currNode) { 
        if (currNode->line == line) {
            // Once reaches first node of specified line, adds it's offset.
            if (!foundLine) {
                offset += currNode->startIndex;
                foundLine = true;
            }
            // Don't count new lines - ruins up and down arrow traversal
            if (currNode->data.contains('\n'))
                offset++;
            // If found, return
            if (index + offset <= currNode->startIndex + currNode->data.length())
                return index + offset;
            // Otherwise, return last character of the line
            else if (!currNode->next || currNode->next->line != line)
                return currNode->startIndex + currNode->data.length();
        }
        currNode = currNode->next;
    }
    return 0;
}

// Main text manipulation functionality
void PieceTable::replace(int startIndex, int endIndex, const std::string& data) {
    if (startIndex < 0 || endIndex > mLength || startIndex > endIndex) {
       std::cerr << ("Invalid start or end index");
       return;
    }

    // Counts how many linebreaks there are to calculate new lines
    int LineBreaks = 0;
    for (int i = 0; i < data.length(); i++) {
        if (data[i] == '\n')
            LineBreaks++;
    }

    // Modify length, adds new text and removes displaced text
    mLength += data.length() - (endIndex - startIndex);


    Node* prevNode = nullptr;
    Node* currNode = mPieces;

    // Finds startIndex location
    while (currNode && startIndex > currNode->startIndex + currNode->data.length()){
        prevNode = currNode;
        currNode = prevNode->next;
    }

    // If located at the end, will just add a new node
    if (!currNode) {
        if (!prevNode) 
            mPieces = new Node(data);
        else if (prevNode->data.empty()) // If last node is empty, attempt to replace it
            prevNode = new Node(data, nullptr, prevNode->startIndex + data.length(),
                prevNode->line + LineBreaks);
        else
            prevNode->next = new Node(data, nullptr, prevNode->startIndex + data.length(),
                prevNode->line + LineBreaks);
        mLines += LineBreaks;
        return;
    }

    // If start index is located in the middle of the node, split that node into two.
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

        Node* newNode = new Node(nextData, currNode->next, currNode->startIndex + offset, currNode->line);
        currNode->next = newNode;
        currNode->data = prevData;
    }

    // Records last node before manipulation as an anchor to attach new nodes to later
    Node* anchorNode = currNode;
    prevNode = anchorNode;
    currNode = prevNode->next;

    // Adds all nodes to be replaced to deletion pile
    std::vector<Node*> toDelete = {};
    while (currNode && endIndex >= currNode->startIndex + currNode->data.length()) {
        toDelete.push_back(currNode);
        prevNode = currNode;
        currNode = prevNode->next;
    }

    // If end index is in the middle of a node, split the node and add the first half to the deletion pile
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
        Node* newNode = new Node(nextData, currNode->next, currNode->startIndex + offset, currNode->line);
        currNode->next = newNode;
        currNode->data = prevData;
        toDelete.push_back(currNode);
    }

    // Creates first node after end index
    Node* firstRightNode;
    if (!toDelete.empty())
        firstRightNode = toDelete.back()->next;// Either becomes first node after the deleted nodes
    else
        firstRightNode = anchorNode->next;     // Or first node after our original anchor point if no data to be added
    anchorNode->next = nullptr;

    // Deletes all nodes in pile
    while (!toDelete.empty()) {
        Node* nodeToDelete = toDelete.back();
        toDelete.pop_back();
        if (nodeToDelete) {
            for (int i = 0; i < nodeToDelete->data.length(); i++) {
                if (nodeToDelete->data[i] == '\n')
                    LineBreaks--; // If deleting line breaks, alter the line count
            }
            delete nodeToDelete;
            nodeToDelete = nullptr; // Dereferences to avoid memory leak
        }
    }
    // Updates after final change in lines
    mLines += LineBreaks;

    // Creates node to either store the new data or just becomes the first node on the right if no data
    Node* newDataNode;
    if (!data.empty()) {
        newDataNode = new Node(data, firstRightNode,anchorNode->startIndex + anchorNode->data.length(),
            anchorNode->line + LineBreaks);
        mLastNode = newDataNode;
        mLastIndex = startIndex;
    }
    else
        newDataNode = firstRightNode;

    // Attaches to our original anchor
    anchorNode->next = newDataNode;

    // If nodes exist after replacement, alter all nodes' start indexes and line numbers.
    if (firstRightNode) {
        prevNode = newDataNode;
        Node* nextNode = firstRightNode;
        while (nextNode) {
            nextNode->startIndex = prevNode->startIndex + prevNode->data.length();
            nextNode->line += LineBreaks;
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

// Counts number of nodes currently in piece table
int PieceTable::countNodes() const {
    int output = 0;
    Node* currNode = mPieces;
    while (currNode) {
        output += 1;
        currNode = currNode->next;
    }
    return output;
}

// Returns main node for node analysis. Should only ever be accessible during debug.
Node* PieceTable::mainNode() const {
    return mPieces;
}
