#include <coroutine>
#include <iostream>
#include <print>


struct SimpleAwaiter {
    bool await_ready() const { return false; }  // Always suspend
    void await_suspend(std::coroutine_handle<>) {}  // No-op
    void await_resume() {}  // No-op
};


struct MyPromise {
    struct get_return_object() { return MyCoroutine(...); }
    auto initial_suspend() { return std::suspend_never{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    // ... other methods
};


int main() {
    std::print("Hello World");
    return 0;
}