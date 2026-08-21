#include "display.h"

template <>
std::string ToDisplayString<bool>(const bool& value) {
    return value ? "true" : "false";
}

template <>
std::string ToDisplayString<std::string>(const std::string& value) {
    return "\"" + value + "\"";
}
