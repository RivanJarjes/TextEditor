# TextEditor
A basic text editor with a programming line gutter and MacOS compliant functions and shortcuts. Built in C++ with the [SFML library](https://github.com/SFML/SFML), Cocoa, and the [clip library](https://github.com/dacap/clip).
![image](https://github.com/user-attachments/assets/89b76445-8a6b-4022-9923-3c78fc008105)

## Functions
* Complete text system in a memory efficient buffer (Piece Table implementation with line-cached nodes, able to handle large documents)
* Full input system (mouse, shortcuts)
* Copy/Paste
* Uses Cocoa for file opening and saving on MacOS

## To-Do
* Color-change for keywords based on a given JSON file.
* Actual scrollbar
* More memory efficient by splitting large files into parts
