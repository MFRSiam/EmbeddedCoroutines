#include <coroutine>
#include <iostream>
#include <print>
#include "EmbCoroutines/EmbCoroutines.hpp"


// Awaitable: Waits for a single byte
struct ByteReader {
    char* value_ptr;

    bool await_ready() { return false; } // Always suspend to wait for data
    void await_suspend(std::coroutine_handle<>) {
        // In a real system, you would register this handle with the UART ISR here.
    }
    char await_resume() { return *value_ptr; }
};

// Global "register" to simulate hardware
char UART_DATA_REGISTER = 0;

// Helper to wait for next byte
auto next_byte() {
    struct Awaiter {
        bool await_ready() { return false; }
        // We pause execution here and return control to main loop
        void await_suspend(std::coroutine_handle<>) {}
        char await_resume() { return UART_DATA_REGISTER; }
    };
    return Awaiter{};
}

StaticTask protocol_parser() {
    printf("  [Co] Parser started.\n");

    while(true) {
        // STATE 1: Wait for Header 0xAA
        char byte = co_await next_byte();

        if (byte != (char)0xAA) {
            printf("  [Co] Invalid Header: %02x, waiting...\n", (unsigned char)byte);
            continue;
        }

        // STATE 2: Read Command
        char cmd = co_await next_byte();

        // STATE 3: Read Data
        char data = co_await next_byte();

        printf("  [Co] PACKET RECEIVED: Cmd: %d, Data: %d\n", cmd, data);
    }
}

int main() {
    printf("Starting System...\n");

    // 1. Create the coroutine.
    // This triggers 'operator new', allocates from static buffer,
    // runs to the first 'co_await', and returns the handle.
    StaticTask task = protocol_parser();

    // 2. Simulate Incoming Data Stream (0xAA, 0x01, 0x05, 0xFF...)
    char incoming_stream[] = { 0x00, 0xAA, 0x01, 0x50, 0xAA, 0x02, 0x99 };

    for (char b : incoming_stream) {
        printf("\n[HW] Receive IRQ: %02x\n", (unsigned char)b);

        // Load data into "register"
        UART_DATA_REGISTER = b;

        // Resume the coroutine (Tell it data is ready)
        if (!task.handle.done()) {
            task.handle.resume();
        }
    }

    return 0;
}