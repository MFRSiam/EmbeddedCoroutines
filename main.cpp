#include <coroutine>
#include <iostream>
#include <print>

bool byte_available();
uint8_t read_byte();
void send_byte(uint8_t);


struct ProtocolTask {
    struct promise_type {
        static constexpr std::size_t FRAME_SIZE = 256;

        static uint8_t pool[FRAME_SIZE];
        static bool used;

        static void* operator new(std::size_t size) {
            if (used || size > FRAME_SIZE) {
                return nullptr; // allocation failure
            }
            used = true;
            return pool;
        }

        static void operator delete(void*, std::size_t) {
            used = false;
        }

        ProtocolTask get_return_object() {
            return ProtocolTask{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;

    void resume() {
        if (!handle.done()) {
            handle.resume();
        }
    }
};


ProtocolTask protocol() {
    uint8_t checksum = 0;

    while (true) {
        // Wait for start byte
        uint8_t b;
        do {
            co_await std::suspend_always{};
            if (!byte_available()) continue;
            b = read_byte();
        } while (b != 0xAA);

        // Read payload
        checksum = 0;
        for (int i = 0; i < 4; ++i) {
            do {
                co_await std::suspend_always{};
            } while (!byte_available());

            uint8_t data = read_byte();
            checksum ^= data;
        }

        // Send response
        send_byte(checksum);
    }
}

int main() {
    auto task = protocol();

    while (true) {
        task.resume();
        // other embedded work
    }

    return EXIT_SUCCESS;
}