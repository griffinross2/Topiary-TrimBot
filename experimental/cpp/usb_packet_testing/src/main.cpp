#include "common.h"
#include "packet.h"
#include "packet_engine.h"
#include "cobs.h"
#include "usb.h"
#include "file_receiver.h"

#include <csignal>
#include <random>
#include <chrono>

char rx_buf[512 + 1];
bool should_quit = false;

void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        should_quit = true;
    }
}

void quit()
{
    TRACE_PRINTF("SIGINT received, exiting...\n");
    usb_disconnect();
    usb_deinit();
    exit(0);
}

int main()
{
    signal(SIGINT, signal_handler);

    if (usb_init() < 0)
    {
        TRACE_PRINTF("Failed to initialize USB\n");
        return 1;
    }

    if (usb_connect() < 0)
    {
        TRACE_PRINTF("Failed to connect to device\n");
        return 1;
    }

    TRACE_PRINTF("Success!\n");

    auto start{std::chrono::steady_clock::now()};
    while (true)
    {
        usb_task();
        packet_engine_task();
        file_receiver_task();

        if (should_quit)
        {
            quit();
        }
    }

    return 0;
}