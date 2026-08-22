#include "csv_utils.h"

namespace csv {

std::vector<Row> ParseCsv(const std::string& content) {
    std::vector<Row> rows;
    Row currentRow;
    std::string field;
    bool inQuotes = false;
    bool rowHasContent = false;  // 現在の行に何か1文字でも書き込まれたか

    auto endField = [&]() {
        currentRow.push_back(field);
        field.clear();
    };
    auto endRow = [&]() {
        endField();
        rows.push_back(currentRow);
        currentRow.clear();
        rowHasContent = false;
    };

    for (size_t i = 0; i < content.size(); ++i) {
        const char c = content[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < content.size() && content[i + 1] == '"') {
                    field += '"';  // エスケープされた""は1個の"として扱う
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += c;
            }
            rowHasContent = true;
            continue;
        }

        switch (c) {
            case '"':
                inQuotes = true;
                rowHasContent = true;
                break;
            case ',':
                endField();
                rowHasContent = true;
                break;
            case '\r':
                break;  // \r\nの\rは無視し、\nだけを行区切りとして扱う
            case '\n':
                endRow();
                break;
            default:
                field += c;
                rowHasContent = true;
                break;
        }
    }

    // ファイル末尾に改行が無いまま終わった場合、最後の行を取りこぼさないようにする。
    if (rowHasContent) {
        endRow();
    }

    return rows;
}

namespace {

bool NeedsQuoting(const std::string& field) {
    return field.find(',') != std::string::npos || field.find('"') != std::string::npos ||
           field.find('\n') != std::string::npos || field.find('\r') != std::string::npos;
}

}  // namespace

std::string BuildCsvLine(const Row& fields) {
    std::string line;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) {
            line += ',';
        }
        const std::string& field = fields[i];
        if (NeedsQuoting(field)) {
            line += '"';
            for (const char c : field) {
                if (c == '"') {
                    line += "\"\"";
                } else {
                    line += c;
                }
            }
            line += '"';
        } else {
            line += field;
        }
    }
    return line;
}

}  // namespace csv
