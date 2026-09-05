#include "hid_report.h"

#include <sstream>
#include <iomanip>

namespace hid {

HidReport ParseReport(const std::vector<uint8_t>& raw) {
    HidReport report;
    if (raw.empty()) {
        return report;
    }
    report.reportId = raw.front();
    report.data.assign(raw.begin() + 1, raw.end());
    return report;
}

std::string FormatBytes(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) {
            oss << ' ';
        }
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(bytes[i]);
    }
    return oss.str();
}

}  // namespace hid
