#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <string>

std::string ShowOpenFileDialog();
bool ShowSaveDialogAndWriteString(const std::string& contentToSave);

#endif
