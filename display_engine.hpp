#ifndef DISPLAY_ENGINE_HPP
#define DISPLAY_ENGINE_HPP

#include "buffer_manager.hpp"
#include "converter_engine.hpp"
#include <iostream>
#include <vector>
#include <cstdio>
#include <iomanip>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

/**
 * @enum EditMode
 * @brief Tracks which representation is currently prioritized for viewing/editing.
 */
enum class EditMode { HEX, BIN, CHAR };

/**
 * @class DisplayEngine
 * @brief Renders the visual representation of the hex editor and handles TUI input.
 */
class DisplayEngine {
private:
    size_t cursorOffset;
    size_t windowStart; 
    const int hexBinBytesPerRow = 16;
    const int visibleRows = 20;
    EditMode currentMode;

    /**
     * Reads a single character from the terminal without waiting for Enter.
     */
    int getRawChar() {
#ifdef _WIN32
        return _getch();
#else
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
#endif
    }

public:
    DisplayEngine() : cursorOffset(0), windowStart(0), currentMode(EditMode::HEX) {}

    /**
     * Captures and interprets key presses, including arrow escape sequences.
     */
    char getCommand() {
        int ch = getRawChar();
        if (ch == 27) { // Escape sequence start
            if (getRawChar() == '[') {
                switch (getRawChar()) {
                    case 'A': return 'U'; // Up
                    case 'B': return 'D'; // Down
                    case 'C': return 'R'; // Right
                    case 'D': return 'L'; // Left
                }
            }
        }
        return static_cast<char>(ch);
    }

    void setMode(EditMode mode) { 
        currentMode = mode; 
        // Re-align window start for Hex/Bin modes
        if (currentMode != EditMode::CHAR) {
            windowStart = (cursorOffset / hexBinBytesPerRow) * hexBinBytesPerRow;
        } else {
            windowStart = 0; // Simplified scrolling for raw char mode
        }
    }

    /**
     * Helper to get the current row width based on mode.
     */
    int getBytesPerRow() const {
        return hexBinBytesPerRow;
    }

    /**
     * Moves the cursor and manages window scrolling.
     * In CHAR mode, "Down" calculates the offset to the next line in the actual file.
     */
    void moveCursor(int delta, const BufferManager& buffer) {
        size_t bufferSize = buffer.getSize();
        
        if (currentMode == EditMode::CHAR) {
            if (delta == 1 || delta == -1) {
                // Horizontal movement
                long long newOffset = static_cast<long long>(cursorOffset) + delta;
                if (newOffset >= 0 && newOffset < static_cast<long long>(bufferSize)) {
                    cursorOffset = static_cast<size_t>(newOffset);
                }
            } else if (delta > 0) {
                // Moving "Down": Find next newline
                for (size_t i = cursorOffset; i < bufferSize; ++i) {
                    if (buffer.getByte(i) == '\n') {
                        if (i + 1 < bufferSize) cursorOffset = i + 1;
                        break;
                    }
                }
            } else if (delta < 0) {
                // Moving "Up": Find previous newline
                if (cursorOffset > 0) {
                    size_t search = cursorOffset - 1;
                    // Skip current line's preceding newline if we are at the start of a line
                    if (search > 0 && buffer.getByte(search) == '\n') search--;
                    
                    while (search > 0 && buffer.getByte(search) != '\n') search--;
                    cursorOffset = (buffer.getByte(search) == '\n') ? search + 1 : 0;
                }
            }
        } else {
            // Standard Hex/Bin grid movement
            long long newOffset = static_cast<long long>(cursorOffset) + delta;
            if (newOffset >= 0 && newOffset < static_cast<long long>(bufferSize)) {
                cursorOffset = static_cast<size_t>(newOffset);
                if (cursorOffset < windowStart) {
                    windowStart = (cursorOffset / hexBinBytesPerRow) * hexBinBytesPerRow;
                } else if (cursorOffset >= windowStart + (visibleRows * hexBinBytesPerRow)) {
                    windowStart = ((cursorOffset / hexBinBytesPerRow) - visibleRows + 1) * hexBinBytesPerRow;
                }
            }
        }
    }

    /**
     * Renders the view. In CHAR mode, displays content linearly respecting newlines.
     */
    void render(const BufferManager& buffer) {
        // \033[H   : Move cursor to home (top-left)
        // \033[2J  : Clear visible screen
        // \033[3J  : Clear scrollback buffer (prevents scrolling up to old data)
        std::cout << "\033[H\033[2J\033[3J";
        std::cout << "File: " << buffer.getFilename() 
                  << (buffer.isModified() ? " [Modified]" : "") << "\n";
        
        std::string modeStr = (currentMode == EditMode::HEX) ? "HEX" : 
                              (currentMode == EditMode::BIN) ? "BINARY" : "CHAR";
        
        std::cout << "Mode: [" << modeStr << "]\n";
        std::cout << "--------------------------------------------------------\n";

        if (currentMode == EditMode::CHAR) {
            // Display as is (Raw text view)
            for (size_t i = 0; i < buffer.getSize(); ++i) {
                uint8_t b = buffer.getByte(i);
                if (i == cursorOffset) std::cout << "\033[7m";
                
                std::cout << static_cast<char>(b);

                if (i == cursorOffset) std::cout << "\033[0m";
            }
            // std::cout << "\n";
        } else {
            // Hex/Bin Grid View
            for (int r = 0; r < visibleRows; ++r) {
                size_t rowStart = windowStart + (r * hexBinBytesPerRow);
                if (rowStart >= buffer.getSize()) break;

                for (int c = 0; c < hexBinBytesPerRow; ++c) {
                    size_t current = rowStart + c;
                    if (current < buffer.getSize()) {
                        uint8_t b = buffer.getByte(current);
                        if (current == cursorOffset) std::cout << "\033[7m";

                        if (currentMode == EditMode::BIN) {
                            std::cout << ConverterEngine::byteToBin(b);
                        } else {
                            std::cout << ConverterEngine::byteToHex(b);
                        }

                        if (current == cursorOffset) std::cout << "\033[0m";
                        if (c < hexBinBytesPerRow - 1) std::cout << " ";
                    }
                }
                std::cout << "\n";
            }
        }

        // Detail Panel
        uint8_t selected = buffer.getByte(cursorOffset);
        std::cout << "\nSelected Offset (Dec): " << std::dec << cursorOffset << "\n";
        std::cout << "Current Value\t Hex: " << ConverterEngine::byteToHex(selected)
                  << " | Bin: " << ConverterEngine::byteToBin(selected)
                  << " | Char: " << ConverterEngine::byteToChar(selected) << "\n";
        
        std::cout << "Commands: [Arrows] Navigate\n"
                  << "\tView Mode\t[1] Hex\t[2] Bin\t[3] Char\n"
                  << "\tEdit Mode\t[H] Hex\t[B] Bin\t[C] Char\n"
                  << "\t[S] Save\t[Q] Quit\n";
        std::cout.flush(); // Ensure buffer is flushed for immediate display
    }

    size_t getCursorOffset() const { return cursorOffset; }
    EditMode getMode() const { return currentMode; }
};

#endif