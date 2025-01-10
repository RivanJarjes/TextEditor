#import <Cocoa/Cocoa.h>
#include <string>
#include <fstream>

std::string ShowOpenFileDialog()
{
    // Create and configure the NSOpenPanel
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setTitle:@"Open a file"];
    [panel setMessage:@"Choose a file to open."];

    // Run the modal dialog
    if ([panel runModal] == NSModalResponseOK) {
        // Get the first selected file’s path
        NSURL *selectedURL = [[panel URLs] firstObject];
        if (selectedURL) {
            NSString *path = [selectedURL path];
            // Convert NSString to std::string
            return std::string([path UTF8String]);
        }
    }
    
    // Return empty string if user cancels or no file is selected
    return std::string();
}

bool ShowSaveDialogAndWriteString(const std::string& contentToSave)
{
    // Create a save panel
    NSSavePanel* savePanel = [NSSavePanel savePanel];

    // Configure the panel (optional settings)
    [savePanel setTitle:@"Save File"];
    [savePanel setPrompt:@"Save"];
    [savePanel setMessage:@"Choose where to save your text."];
    
    // Run the panel modally
    if ([savePanel runModal] == NSModalResponseOK) {
        // Get selected URL
        NSURL *selectedURL = [savePanel URL];
        if (selectedURL) {
            // Convert NSURL path (NSString) to std::string
            NSString *path = [selectedURL path];
            std::string filePath([path UTF8String]);

            // Write the string to the chosen path
            std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);
            if (!outFile.is_open()) {
                NSLog(@"Failed to open file for writing: %s", filePath.c_str());
                return false;
            }

            // Write the content
            outFile << contentToSave;
            outFile.close();

            return true;
        }
    }
    
    // User canceled or something went wrong
    return false;
}
