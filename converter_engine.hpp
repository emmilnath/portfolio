#ifndef CONVERTER_ENGINE_HPP
#define CONVERTER_ENGINE_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <bitset>

/**
 * @class ConverterEngine
 * @brief Handles conversion between raw bytes and Hex/Bin/Char representations.
 */
class ConverterEngine {
private:
    /**
     * Internal helper to ensure a byte is within the standard ASCII range.
     * Returns the byte itself if valid, otherwise returns the '.' character.
     */
    static uint8_t convertNonAscii(uint8_t byte) {
        if (byte > 127) {
            return static_cast<uint8_t>('.');
        }
        return byte;
    }

public:
    /**
     * Converts a byte to a 2-character Hex string (e.g., "0F").
     */
    static std::string byteToHex(uint8_t byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int) convertNonAscii(byte);
        return ss.str();
    }

    /**
     * Converts a byte to an 8-character Binary string (e.g., "00001111").
     */
    static std::string byteToBin(uint8_t byte) {
        return std::bitset<8>(convertNonAscii(byte)).to_string();
    }

    /**
     * Converts a byte to a printable character or a dot if non-printable.
     */
    static char byteToChar(uint8_t byte) {
        return static_cast<char>(convertNonAscii(byte));
    }

    /**
     * Parses a 2-char hex string back to a byte.
     * @return pair: {success, value}
     */
    static std::pair<bool, uint8_t> hexToByte(const std::string& hex) {
        try {
            unsigned int x;
            std::stringstream ss;
            ss << std::hex << hex;
            if (!(ss >> x)) return {false, 0};
            return {true, static_cast<uint8_t>(x)};
        } catch (...) {
            return {false, 0};
        }
    }

    /**
     * Parses an 8-char binary string back to a byte.
     */
    static std::pair<bool, uint8_t> binToByte(const std::string& bin) {
        try {
            if (bin.length() != 8) return {false, 0};
            return {true, static_cast<uint8_t>(std::bitset<8>(bin).to_ulong())};
        } catch (...) {
            return {false, 0};
        }
    }

    /**
     * Converts a single character directly to its byte value.
     * Useful for character-mode editing.
     */
    static uint8_t charToByte(char c) {
        return static_cast<uint8_t>(c);
    }
};

#endif