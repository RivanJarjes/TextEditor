#include <iostream>
#include "textBuffer.h"
#include "history.h"
#include "fileDialog.h"
#include <SFML/Graphics.hpp>
#include <fstream>

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
    Node* currentNode = mPieces;
    while (currentNode) {  // Goes through all nodes in order until the chain breaks
        output += currentNode->data;
        currentNode = currentNode->next;
    }
    return output;
}

std::string PieceTable::printSelection(int startIndex, int endIndex) const {
    std::string output = "";
    bool startReached = false;
    Node* currentNode = mPieces;
    while (currentNode) {  // Goes through all nodes in order until the chain breaks
        if (!startReached && startIndex <= currentNode->startIndex + currentNode->data.length()){
            if (endIndex <= currentNode->startIndex + currentNode->data.length()) {
                output += currentNode->data.substr(startIndex - currentNode->startIndex, 
                    endIndex - startIndex);
                break;
            }
            startReached = true;
            output += currentNode->data.substr(startIndex - currentNode->startIndex, 
                currentNode->data.length() - (startIndex - currentNode->startIndex));
        } else if (startReached && endIndex <= currentNode->startIndex + currentNode->data.length()) {
            output += currentNode->data.substr(0, (endIndex - currentNode->startIndex));
            break;
        } else if (startReached) 
            output += currentNode->data;

        currentNode = currentNode->next;
    }
    return output;
}

int PieceTable::insert(int startIndex, const std::string& data) {
    //If tab, replace it with three spaces for simplicity sake
    std::string modifiedData = data;
    if (modifiedData == "\t") {
        int whiteSpace = 0; //Counts how much whitespace occurs before index @ line to have an even tab
        int line = getCurrentLine(startIndex);
        Node* currentNode = mPieces;
        while (currentNode) {
            if (currentNode->line == line){
                if (startIndex <= currentNode->startIndex + currentNode->data.length()) {
                    for (int i = 0; i < startIndex - currentNode->startIndex; i++) 
                        if (currentNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
                    break;
                }
                else 
                    for (int i = 0; i < currentNode->data.length(); i++) 
                        if (currentNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
            }
            else if(currentNode->line > line)
                break;
            currentNode = currentNode->next;
        }
        modifiedData = std::string(4 - whiteSpace % 4, ' '); // Adds an even amount of whitespace.
    }
    // If a character was just typed at the previous index AND the cursor did not move, it will just
    // be added to the previous node.
    if (mLastIndex != -1 && mLastIndex == startIndex - 1 && mLastNode && modifiedData.find('\n') == modifiedData.npos) {
        mLastNode->data += modifiedData;
        mLength += modifiedData.length();
        mLastIndex++;
        Node* currentNode = mLastNode->next;
        // Update the starting indexes of each node afterwards.
        while (currentNode) {
            currentNode->startIndex += modifiedData.length();
            currentNode = currentNode->next;
        }
        std::string emptyString = "";
        mHistory.addToHistory(modifiedData, emptyString, startIndex, startIndex,
            true);
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
        Node* currentNode = mPieces;
        while (currentNode) {
            if (currentNode->line == line){
                if (endIndex <= currentNode->startIndex + currentNode->data.length()) {
                    for (int i = 0; i < endIndex - currentNode->startIndex; i++) 
                        if (currentNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
                    break;
                }
                else 
                    for (int i = 0; i < currentNode->data.length(); i++) 
                        if (currentNode->data[i] == ' ')
                            whiteSpace++;
                        else
                            whiteSpace = 0;
            }
            else if(currentNode-> line > line)
                break;
            currentNode = currentNode->next;
        }
        if (whiteSpace && whiteSpace % 4 == 0) {
            replace(startIndex-3, endIndex, "");
            return 4;
        }
    }
    // Calls replace function without any data
    replace(startIndex, endIndex, "", false, false, false);
    return -(endIndex - startIndex);
}

// To be called when cursor moves to reset last saved node, thus will make a new node next insertion
// Even makes new node if user returns to original last node position, (will be) used for undo/redo procedure.
void PieceTable::resetNodeSave() {
    mLastIndex = -1;
    mLastNode = nullptr;
}

// Gets the current line the index is located at.
int PieceTable::getCurrentLine(int index) const{
    Node* currentNode = mPieces;
    while (currentNode) {
        if (index <= currentNode->startIndex + currentNode->data.length() || !currentNode->next) 
            return currentNode->line;
        currentNode = currentNode->next;
    }
    return 1;
}

// Gets the index relative to it's current line. Used for cursor location.
int PieceTable::relativeLineIndex(int index) const {
    Node* currentNode = mPieces;
    int currentLine = 1;
    int characterDisposition = 0;
    while (currentNode) {
        if (currentLine != currentNode->line) {
            characterDisposition = currentNode->startIndex + 1;
            currentLine = currentNode->line;
        }
        if (index <= currentNode->startIndex + currentNode->data.length() || !currentNode->next) 
            return index - characterDisposition ;
        currentNode = currentNode->next;
    }
    return 0;
}

// Gets the current node the index is located at. For debugging purposes mainly.
Node* PieceTable::getCurrentNode(int index) const {
    Node* currentNode = mPieces;
    while (currentNode) {
        if (index <= currentNode->startIndex + currentNode->data.length() || !currentNode->next) 
            return currentNode;
        currentNode = currentNode->next;
    }
    return nullptr;
}

// Gets the global index of a requested relative index on a specific line.
int PieceTable::indexOnLine(int index, int line) const {
    // If tries going above first line, just go to beginning.
    if (line == 0)
        return 0;
    if (line > mLines)
        return mLength;
    Node* currentNode = mPieces;
    int offset = 0;
    bool foundLine = false;
    while (currentNode) { 
        if (currentNode->line == line) {
            // Once reaches first node of specified line, adds it's offset.
            if (!foundLine) {
                offset += currentNode->startIndex;
                foundLine = true;
            }
            // Don't count new lines - ruins up and down arrow traversal
            if (currentNode->data.contains('\n'))
                offset++;
            // If found, return
            if (index + offset <= currentNode->startIndex + currentNode->data.length())
                return index + offset;
            // Otherwise, return last character of the line
            else if (!currentNode->next || currentNode->next->line != line)
                return currentNode->startIndex + currentNode->data.length();
        }
        currentNode = currentNode->next;
    }
    return mLength;
}

// Main text manipulation functionality
int PieceTable::replace(int startIndex, int endIndex, const std::string& data, bool isRecalled, bool ignoreUndo, bool resetNode) {
    if (startIndex < 0 || endIndex > mLength || startIndex > endIndex) 
       return 0;
    
    if (resetNode)
        resetNodeSave();

    // Counts how many linebreaks there are to calculate new lines, as well as changes tabs to 4 spaces
    int LineBreaks = 0;
    std::string modifiedData;
    std::string removedData = "";
    if (startIndex != endIndex && !ignoreUndo) {
        removedData = printSelection(startIndex, endIndex);
    }

    for (char c : data) {
        if (c == '\n') {
            LineBreaks++;
            modifiedData.push_back('\n');
        }
        else if (c == '\t') 
            modifiedData += "    ";
        else 
            modifiedData.push_back(c);
    }

    if ((LineBreaks == 1 && modifiedData[0] != '\n') || LineBreaks > 1) {
        int change = 0;
        int start = 0;
        int lastEndIndex = -1;

        mHistory.addToHistory(modifiedData, removedData, startIndex, endIndex, false);
        while (true) {
            int pos = modifiedData.find('\n', start + 1);
            if (pos == std::string::npos) {
                change += replace(lastEndIndex, lastEndIndex, 
                    modifiedData.substr(start), false, true);
                break;
            }
            if (lastEndIndex == -1) {
                change += replace(startIndex, endIndex, modifiedData.substr(start, (pos - start)), 
                    false, true);
                lastEndIndex = startIndex + (pos - start);
            }
            else {
                change += replace(lastEndIndex, lastEndIndex, 
                    modifiedData.substr(start, (pos - start)), false, true);
                lastEndIndex += (pos - start);
            }
            start = pos;
        }
        return change;
    }

    // Modify length, adds new text and removes displaced text
    int change =  modifiedData.length() - (endIndex - startIndex);
    mLength += change;


    Node* prevNode = nullptr;
    Node* currentNode = mPieces;

    // Finds startIndex location
    while (currentNode && startIndex > currentNode->startIndex + currentNode->data.length()){
        prevNode = currentNode;
        currentNode = prevNode->next;
    }

    // If located at the end, will just add a new node
    if (!currentNode) {
        if (!prevNode) 
            mPieces = new Node(modifiedData);
        else if (prevNode->data.empty()) // If last node is empty, attempt to replace it
            prevNode = new Node(modifiedData, nullptr, prevNode->startIndex + modifiedData.length(),
                prevNode->line + LineBreaks);
        else
            prevNode->next = new Node(modifiedData, nullptr, prevNode->startIndex + modifiedData.length(),
                prevNode->line + LineBreaks);
        mLines += LineBreaks;
        return data.length() - (endIndex - startIndex);
    }

    // If start index is located in the middle of the node, split that node into two.
    if (startIndex < currentNode->startIndex + currentNode->data.length()) {
        int offset = startIndex - currentNode->startIndex;
        int length = currentNode->data.length();
        std::string prevData;
        if (offset > -length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset > length) usedOffset = length;
            prevData = currentNode->data.substr(0, usedOffset);
        }
        else
            prevData = "";
        std::string nextData;
        if (offset < length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset < 0) usedOffset = 0;
            nextData = currentNode->data.substr(usedOffset);
        }
        else
            nextData = "";

        Node* newNode = new Node(nextData, currentNode->next, currentNode->startIndex + offset, 
            currentNode->line);
        currentNode->next = newNode;
        currentNode->data = prevData;
    }

    // Records last node before manipulation as an anchor to attach new nodes to later
    Node* anchorNode = currentNode;
    prevNode = anchorNode;
    currentNode = prevNode->next;

    // Adds all nodes to be replaced to deletion pile
    std::vector<Node*> toDelete = {};
    while (currentNode && endIndex >= currentNode->startIndex + currentNode->data.length()) {
        toDelete.push_back(currentNode);
        prevNode = currentNode;
        currentNode = prevNode->next;
    }

    // If end index is in the middle of a node, split the node and add the first half to the deletion pile
    if (currentNode && endIndex < currentNode->startIndex + currentNode->data.length()) {
        int offset = endIndex - currentNode->startIndex;
        int length = currentNode->data.length();
        std::string prevData;
        if (offset > -length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset > length) usedOffset = length;
            prevData = currentNode->data.substr(0, usedOffset);
        }
        else
            prevData = "";
        std::string nextData;
        if (offset < length){
            int usedOffset = offset >= 0 ? offset : length + offset;
            if (usedOffset < 0) usedOffset = 0;
            nextData = currentNode->data.substr(usedOffset);
        }
        else
            nextData = "";
        Node* newNode = new Node(nextData, currentNode->next, currentNode->startIndex + offset, 
            currentNode->line);
        currentNode->next = newNode;
        currentNode->data = prevData;
        toDelete.push_back(currentNode);
    }

    // Creates first node after end index
    Node* firstRightNode;
    if (!toDelete.empty())
        firstRightNode = toDelete.back()->next;// Either becomes first node after the deleted nodes
    else
        firstRightNode = anchorNode->next;     // Or first node after our original anchor point if no data to be added
    anchorNode->next = nullptr;

    // Deletes all nodes in pile
    int lineBreaksRemoved = 0;
    while (!toDelete.empty()) {
        Node* nodeToDelete = toDelete.back();
        toDelete.pop_back();
        if (nodeToDelete) {
            for (int i = 0; i < nodeToDelete->data.length(); i++) 
                if (nodeToDelete->data[i] == '\n')
                    lineBreaksRemoved++; // If deleting line breaks, alter the line count
            delete nodeToDelete;
            nodeToDelete = nullptr; // Dereferences to avoid memory leak
        }
    }
    // Updates after final change in lines
    mLines += LineBreaks - lineBreaksRemoved;

    // Creates node to either store the new data or just becomes the first node on the right if no data
    Node* newDataNode;
    if (!modifiedData.empty()) {
        int line = anchorNode->line + LineBreaks;
        if (line < anchorNode->line)
            line = anchorNode->line;
        newDataNode = new Node(modifiedData, firstRightNode,
            anchorNode->startIndex + anchorNode->data.length(), line);
        mLastNode = newDataNode;
    }
    else{
        newDataNode = firstRightNode;
        mLastNode = nullptr;
    }

    // Attaches to our original anchor
    anchorNode->next = newDataNode;

    // If nodes exist after replacement, alter all nodes' start indexes and line numbers.
    if (firstRightNode) {
        prevNode = newDataNode;
        Node* nextNode = firstRightNode;
        while (nextNode) {
            if (modifiedData.empty())
                nextNode->startIndex += change;
            else
                nextNode->startIndex = prevNode->startIndex + prevNode->data.length();
            nextNode->line += LineBreaks - lineBreaksRemoved;
            prevNode = nextNode;
            nextNode = prevNode->next;
        }
    }

    if (!ignoreUndo)
        mHistory.addToHistory(modifiedData, removedData, startIndex, endIndex, endIndex == mLastIndex && !mLastNode);
    
    mLastIndex = startIndex;

    return data.length() - (endIndex - startIndex);
}

sf::Vector2i PieceTable::undo() {
    Action* action = mHistory.getLastAction();
    if (action){
        if (!action->data.empty())
            replace(action->startIndex, action->startIndex + action->data.length(), "", false,
                true);
        if (!action->removedData.empty())
            replace(action->startIndex, action->startIndex, action->removedData, false,
                true);
        int left = !action->multipleActions ? action->startIndex : -1;
        int right = action->startIndex + action->removedData.length();
        if (left != -1 && abs(left - right) == 1) 
            left = -1;
        return sf::Vector2i(left, right);
    }
    return sf::Vector2i(-1, -1);
}

int PieceTable::redo() {
    Action* action = mHistory.getLastUndo();
    if (action) {
        if (action->startIndex != action->endIndex)
            replace(action->startIndex, action->endIndex, "", false, true);
        if (!action->data.empty())
            replace(action->startIndex, action->startIndex, action->data, false, true);
        return action->startIndex + action->data.length();
    }
    return -1;
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
    Node* currentNode = mPieces;
    while (currentNode) {
        output += 1;
        currentNode = currentNode->next;
    }
    return output;
}

// Returns main node for node analysis. Should only ever be accessible during debug.
Node* PieceTable::mainNode() const {
    return mPieces;
}

void PieceTable::open(std::ifstream& file) {
    mHistory.clearHistory();
    remove(0, mLength);
    std::string line;
    while (std::getline(file, line)) {
        replace(mLength, mLength, line + '\n', false, true);
    }
}

void PieceTable::save() {
    std::string file = print();
    if (ShowSaveDialogAndWriteString(file)) {
        std::cout << "File was successfully saved.\n";
    } else {
        std::cout << "Save was canceled or failed.\n";
    }
}

//Test Cases
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

std::vector<std::array<int, 3>> PieceTable::getSelectionBoxes(int startIndex, int endIndex) {
    std::vector<std::array<int, 3>> output;
    int currentOffset = 0;
    int lastIndex = -1;
    Node* currNode = mPieces;
    auto pushLine = [&](int end) {
        output.push_back({lastIndex - (currNode->line > 1 && lastIndex > 0 ? 1 : 0), 
            end - (currNode->line > 1 ? 1 : 0), currNode->line});
    };

    while (currNode) {
        if (lastIndex == -1 && startIndex <= currNode->startIndex + currNode->data.length()) 
            lastIndex = startIndex - currentOffset;

        if (endIndex <= currNode->startIndex + currNode->data.length()) {
            pushLine(endIndex - currentOffset);
            lastIndex = -1;
            break;
        }
        if (!currNode->next)
            pushLine(static_cast<int>(currNode->data.length()) + currNode->startIndex - currentOffset);

        if (currNode->next && currNode->next->line != currNode->line) {
            if (lastIndex != -1) {
                pushLine(static_cast<int>(currNode->data.length()) + currNode->startIndex - currentOffset + 1);
                lastIndex = 0;
            }
            currentOffset = currNode->next->startIndex;
        }
        currNode = currNode->next;
    }
    return output;
}
