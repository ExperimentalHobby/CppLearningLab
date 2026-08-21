#include <iostream>
#include <stdexcept>
#include <string>

#include "display.h"
#include "fixed_queue.h"
#include "fixed_stack.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

template <typename T, std::size_t Capacity>
void PrintStackContents(FixedStack<T, Capacity>& stack) {
    // Popしながら取り出すため、内容確認用に別のスタックへ積み直す。
    FixedStack<T, Capacity> backup;
    std::cout << "  中身(Top->Bottom): ";
    while (!stack.Empty()) {
        const T value = stack.Pop();
        std::cout << ToDisplayString(value) << " ";
        backup.Push(value);
    }
    std::cout << std::endl;
    while (!backup.Empty()) {
        stack.Push(backup.Pop());
    }
}

void DemoFixedStack() {
    std::cout << "=== FixedStack<int, 4> ===" << std::endl;
    FixedStack<int, 4> intStack;
    for (int v : {10, 20, 30}) {
        intStack.Push(v);
    }
    std::cout << "Size=" << intStack.Size() << ", Top=" << intStack.Top() << std::endl;
    PrintStackContents(intStack);

    std::cout << "\n=== FixedStack<std::string, 3> ===" << std::endl;
    FixedStack<std::string, 3> stringStack;
    stringStack.Push("one");
    stringStack.Push("two");
    std::cout << "Size=" << stringStack.Size() << ", Top=" << stringStack.Top() << std::endl;
    PrintStackContents(stringStack);

    std::cout << "\n=== 容量超過・空アクセスの例外 ===" << std::endl;
    try {
        FixedStack<int, 2> smallStack;
        smallStack.Push(1);
        smallStack.Push(2);
        smallStack.Push(3);  // 容量2を超えるので例外
    } catch (const std::out_of_range& e) {
        std::cout << "catch(Push超過): " << e.what() << std::endl;
    }
    try {
        FixedStack<int, 2> emptyStack;
        emptyStack.Pop();  // 空からPopするので例外
    } catch (const std::out_of_range& e) {
        std::cout << "catch(Pop空): " << e.what() << std::endl;
    }
}

void DemoFixedQueue() {
    std::cout << "\n=== FixedQueue<int, 3> ===" << std::endl;
    FixedQueue<int, 3> queue;
    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);
    std::cout << "Front=" << queue.Front() << ", Size=" << queue.Size() << std::endl;

    std::cout << "Dequeue: " << queue.Dequeue() << std::endl;
    queue.Enqueue(4);  // リングバッファの折り返しを確認
    std::cout << "Dequeue: " << queue.Dequeue() << std::endl;
    std::cout << "Dequeue: " << queue.Dequeue() << std::endl;
    std::cout << "Dequeue: " << queue.Dequeue() << std::endl;
    std::cout << "Empty=" << std::boolalpha << queue.Empty() << std::endl;
}

void DemoFunctionTemplatesAndSpecialization() {
    std::cout << "\n=== 関数テンプレート: Max ===" << std::endl;
    std::cout << "Max(3, 7) = " << Max(3, 7) << std::endl;
    std::cout << "Max(3.5, 2.1) = " << Max(3.5, 2.1) << std::endl;
    std::cout << "Max(std::string(\"apple\"), std::string(\"banana\")) = "
              << Max(std::string("apple"), std::string("banana")) << std::endl;

    std::cout << "\n=== テンプレートの特殊化: ToDisplayString ===" << std::endl;
    std::cout << "ToDisplayString(42) = " << ToDisplayString(42) << " (汎用版)" << std::endl;
    std::cout << "ToDisplayString(3.14) = " << ToDisplayString(3.14) << " (汎用版)" << std::endl;
    std::cout << "ToDisplayString(true) = " << ToDisplayString(true) << " (bool特殊化)"
              << std::endl;
    std::cout << "ToDisplayString(std::string(\"hi\")) = " << ToDisplayString(std::string("hi"))
              << " (std::string特殊化)" << std::endl;
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    DemoFixedStack();
    DemoFixedQueue();
    DemoFunctionTemplatesAndSpecialization();

    return 0;
}
