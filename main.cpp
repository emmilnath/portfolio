#include "buffer_manager.hpp"
#include "converter_engine.hpp"
#include "display_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>

/**
 * @brief Unit Tests for Phase 2 Verification
 */
void runTests() {
    std::cout << "Tests werden durchgeführt...\n";

    // Test 1: Converter Logic
    // Test ascii values
    assert(ConverterEngine::byteToHex('A') == "41");
    assert(ConverterEngine::byteToBin('A') == "01000001");
    assert(ConverterEngine::byteToChar('A') == 'A');

    // // Test non-ascii values
    assert(ConverterEngine::byteToHex(254) == "2E");
    assert(ConverterEngine::byteToHex(255) == "2E");
    assert(ConverterEngine::byteToBin(254) == "00101110");
    assert(ConverterEngine::byteToBin(255) == "00101110");
    assert(ConverterEngine::byteToChar(254) == '.');
    assert(ConverterEngine::byteToChar(255) == '.');
    
    // Testing Hex String to Byte
    auto hexRes = ConverterEngine::hexToByte("A0");
    assert(hexRes.first == true && hexRes.second == 0xA0);

    // Testing Binary String to Byte
    auto binRes = ConverterEngine::binToByte("10101010");
    assert(binRes.first == true && binRes.second == 0xAA);
    auto binResFail = ConverterEngine::binToByte("101"); // Too short
    assert(binResFail.first == false);

    // // Testing Character to Byte
    assert(ConverterEngine::charToByte('Z') == 90);
    assert(ConverterEngine::charToByte(' ') == 32);

    // Test 2: Buffer Management
    BufferManager bm;
    bm.setByte(0, 0xDE);
    assert(bm.getByte(0) == 0xDE);
    assert(bm.isModified() == true);
    
    std::cout << "Alle Tests sind erfolgreich durchgelaufen!\n";
}

/**
 * @brief Main Entry Point
 */
int main(int argc, char* argv[]) {
    // Check for test flag
    if (argc > 1 && std::string(argv[1]) == "--test") {
        runTests();
        return 0;
    }

    BufferManager buffer;
    DisplayEngine view;
    
    std::string path;
    if (argc > 1) {
        path = argv[1];
    } else {
        std::cout << "Bitte einen Dateipfad eingeben: ";
        std::getline(std::cin, path);
    }

    if (!buffer.loadFile(path)) {
        std::cout << "Keine Datei gefunden. Ein neues buffer erstellen? (j/n): ";
        char choice;
        std::cin >> choice;
        if (choice == 'j' || choice == 'J') {
            for(int i=0; i<1024; ++i) buffer.setByte(i, (i % 26) + 'A'); 
        } else {
            return 1;
        }
        std::cin.ignore(); // Clear newline
    }

    bool running = true;
    while (running) {
        view.render(buffer);
        
        char cmd = view.getCommand();
        int rowWidth = view.getBytesPerRow();

        switch (cmd) {
            case 'q': case 'Q': 
                running = false; 
                break;
            case 's': case 'S': 
                buffer.saveFile();
                break;
            case 'U': // Up Arrow
                view.moveCursor(-rowWidth, buffer);
                break;
            case 'D': // Down Arrow
                view.moveCursor(rowWidth, buffer);
                break;
            case 'L': // Left Arrow
                view.moveCursor(-1, buffer);
                break;
            case 'R': // Right Arrow
                view.moveCursor(1, buffer);
                break;
            case 'h': case 'H': {
                view.setMode(EditMode::HEX);
                view.render(buffer); // Refresh to show Hex primary view
                std::cout << "\nEinen 2-digit Hex Wert eingeben: ";
                std::string newHex;
                std::cin >> newHex;
                auto res = ConverterEngine::hexToByte(newHex);
                if (res.first) {
                    buffer.setByte(view.getCursorOffset(), res.second);
                }
                break;
            }
            case 'b': case 'B': {
                view.setMode(EditMode::BIN);
                view.render(buffer); // Refresh to show bin primary view
                std::cout << "\nEinen 8-bit Binary Wert eingeben: ";
                std::string newBin;
                std::cin >> newBin;
                auto res = ConverterEngine::binToByte(newBin);
                if (res.first) {
                    buffer.setByte(view.getCursorOffset(), res.second);
                }
                break;
            }
            case 'c': case 'C': {
                view.setMode(EditMode::CHAR);
                view.render(buffer); // Refresh to show char primary view
                std::cout << "\nEinen ASCII char eingeben: ";
                char newChar;
                std::cin >> newChar;
                buffer.setByte(view.getCursorOffset(), ConverterEngine::charToByte(newChar));
                break;
            }
            case '1': {
                view.setMode(EditMode::HEX);
                view.render(buffer); // Refresh to show Hex primary view
                break;
            }
            case '2': {
                view.setMode(EditMode::BIN);
                view.render(buffer); // Refresh to show bin primary view
                break;
            }
            case '3': {
                view.setMode(EditMode::CHAR);
                view.render(buffer); // Refresh to show char primary view
                break;
            }
        }
    }

    return 0;
}