#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

struct Action {
    std::string data;
    std::string removedData;
    int startIndex;
    int endIndex;
    bool multipleActions;

    Action(const std::string& data, const std::string& removedData, 
        int startIndex, int endIndex);
};

class History {
private:
    std::vector<Action*> undoActions;
    std::vector<Action*> redoActions;
public:
    void addToHistory(std::string& data, std::string& removedData, 
        int startIndex, int endIndex, bool addToEnd = false);

    Action* getLastAction();

    Action* getLastUndo();

    int historyLength();

    void clearHistory();
};

#endif
