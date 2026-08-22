#include "calculator.h"

#include <iomanip>
#include <sstream>

namespace {

// 12桁精度+末尾のゼロを自動的に削る書式(printfの%gに近い)で数値を整形する。
std::string FormatNumber(double value) {
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(12) << value;
    return oss.str();
}

}  // namespace

void Calculator::InputDigit(char digit) {
    if (hasError_) {
        return;
    }
    if (startingNewInput_) {
        currentInput_.clear();
        startingNewInput_ = false;
    }
    if (currentInput_ == "0") {
        currentInput_.clear();
    }
    currentInput_.push_back(digit);
}

void Calculator::InputDecimalPoint() {
    if (hasError_) {
        return;
    }
    if (startingNewInput_) {
        currentInput_ = "0";
        startingNewInput_ = false;
    }
    if (currentInput_.find('.') == std::string::npos) {
        currentInput_.push_back('.');
    }
}

void Calculator::CommitPendingOperation() {
    double value = 0.0;
    try {
        value = std::stod(currentInput_);
    } catch (const std::exception&) {
        value = 0.0;
    }

    if (pendingOperator_ == '\0') {
        accumulator_ = value;
        return;
    }

    switch (pendingOperator_) {
        case '+':
            accumulator_ += value;
            break;
        case '-':
            accumulator_ -= value;
            break;
        case '*':
            accumulator_ *= value;
            break;
        case '/':
            if (value == 0.0) {
                hasError_ = true;
                return;
            }
            accumulator_ /= value;
            break;
        default:
            break;
    }
}

void Calculator::InputOperator(char op) {
    if (hasError_) {
        return;
    }
    CommitPendingOperation();
    if (hasError_) {
        return;
    }
    pendingOperator_ = op;
    startingNewInput_ = true;
}

void Calculator::Equals() {
    if (hasError_) {
        return;
    }
    CommitPendingOperation();
    pendingOperator_ = '\0';
    startingNewInput_ = true;
}

void Calculator::Clear() {
    accumulator_ = 0.0;
    pendingOperator_ = '\0';
    currentInput_ = "0";
    startingNewInput_ = true;
    hasError_ = false;
}

std::string Calculator::Display() const {
    if (hasError_) {
        return "Error";
    }
    if (!startingNewInput_) {
        return currentInput_;
    }
    return FormatNumber(accumulator_);
}
