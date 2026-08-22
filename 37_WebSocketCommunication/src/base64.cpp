#include "base64.h"

namespace ws {

std::string Base64Encode(const std::string& data) {
    static const char kTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= data.size()) {
        const unsigned int chunk = (static_cast<unsigned char>(data[i]) << 16) |
                                    (static_cast<unsigned char>(data[i + 1]) << 8) |
                                    static_cast<unsigned char>(data[i + 2]);
        result += kTable[(chunk >> 18) & 0x3F];
        result += kTable[(chunk >> 12) & 0x3F];
        result += kTable[(chunk >> 6) & 0x3F];
        result += kTable[chunk & 0x3F];
        i += 3;
    }

    const size_t remaining = data.size() - i;
    if (remaining == 1) {
        const unsigned int chunk = static_cast<unsigned char>(data[i]) << 16;
        result += kTable[(chunk >> 18) & 0x3F];
        result += kTable[(chunk >> 12) & 0x3F];
        result += "==";
    } else if (remaining == 2) {
        const unsigned int chunk =
            (static_cast<unsigned char>(data[i]) << 16) | (static_cast<unsigned char>(data[i + 1]) << 8);
        result += kTable[(chunk >> 18) & 0x3F];
        result += kTable[(chunk >> 12) & 0x3F];
        result += kTable[(chunk >> 6) & 0x3F];
        result += "=";
    }

    return result;
}

}  // namespace ws
