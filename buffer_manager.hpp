#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>

/**
 * @class BufferManager
 * @brief Manages the file data buffer and disk persistence.
 */
class BufferManager {
private:
    std::vector<uint8_t> buffer;
    std::string filename;
    bool modified;

public:
    BufferManager() : filename(""), modified(false) {}

    /**
     * Loads a file into the internal vector.
     */
    bool loadFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return false;

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        buffer.resize(size);
        if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            filename = path;
            modified = false;
            return true;
        }
        return false;
    }

    /**
     * Saves the current buffer back to the original file.
     */
    bool saveFile() {
        if (filename.empty()) return false;
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        modified = false;
        return true;
    }

    // Accessors
    uint8_t getByte(size_t offset) const {
        return (offset < buffer.size()) ? buffer[offset] : 0;
    }

    void setByte(size_t offset, uint8_t value) {
        if (offset >= buffer.size()) {
            buffer.resize(offset + 1); // Ensure buffer exists for the test
        }
        buffer[offset] = value;
        modified = true;
    }

    size_t getSize() const { return buffer.size(); }
    bool isModified() const { return modified; }
    std::string getFilename() const { return filename; }
};

#endif