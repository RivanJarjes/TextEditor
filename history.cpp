#include <vector>
#include "history.h"

Action::Action(const std::string& data, const std::string& removedData, int startIndex, int endIndex) 
    : data(data), removedData(removedData), startIndex(startIndex), endIndex(endIndex), multipleActions(false) {}

void History::addToHistory(std::string& data, std::string& removedData, int startIndex, int endIndex, 
        bool addToEnd) {
    while (!redoActions.empty()) {
        delete redoActions.back();
        redoActions.pop_back();
    }
    if (addToEnd) {
        if ((isalpha(data[0]) && (isalpha(undoActions.back()->data.back()))) ||
                (data.back() == undoActions.back()->data.back()) ||
                (undoActions.back()->data.length() == 1 && undoActions.back()->data[0] == ' ')){
            undoActions.back()->data += data;
            undoActions.back()->multipleActions = true;
            return;
        }
        else if (data.empty() && undoActions.back()->data.empty() && endIndex == undoActions.back()->startIndex) {
            undoActions.back()->startIndex = startIndex;
            undoActions.back()->removedData = removedData + undoActions.back()->removedData;
            undoActions.back()->multipleActions = true;
            return;
        }
    }
    undoActions.push_back(new Action(data, removedData, startIndex, endIndex));
    if (undoActions.size() > 100) {
        undoActions.front() = std::move(undoActions.back());
        delete undoActions.back();
        undoActions.pop_back();
    }
}

Action* History::getLastAction() {
    if (undoActions.empty())
        return nullptr;
    Action* lastAction = undoActions.back();
    undoActions.pop_back();
    redoActions.push_back(lastAction);
    return lastAction;
}

Action* History::getLastUndo() {
    if (redoActions.empty())
        return nullptr;
    Action* lastAction = redoActions.back();
    redoActions.pop_back();
    undoActions.push_back(lastAction);
    return lastAction;
}

int History::historyLength() {
    return undoActions.size();
}

void History::clearHistory() {
    while (!undoActions.empty()) {
        delete undoActions.back();
        undoActions.pop_back();
    }
    while (!redoActions.empty()) {
        delete redoActions.back();
        redoActions.pop_back();
    }
}
