#include "tle94112-rpi.hpp"
#include "tle94112-platf-rpi.hpp"
#include <cstdio>
#include <bcm2835.h>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

struct Command
{
    int csPinIndex;
    std::string state_hb_pairs;
};

std::vector<Command> commandQueue;
std::atomic<bool> running(false);
std::mutex queueMutex;

void executePattern(std::vector<Tle94112Rpi> &controllers, Tle94112::HalfBridge *hbPins, Tle94112::HBState *states)
{
    while (true)
    {
        if (running.load())
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            for (const auto &cmd : commandQueue)
            {
                int cs = cmd.csPinIndex;
                std::istringstream pairStream(cmd.state_hb_pairs);
                std::string pair;

                controllers[cs].cs->enable();
                std::getline(pairStream, pair, ' '); // remove leading null
                while (std::getline(pairStream, pair, ' '))
                {
                    std::istringstream singlePair(pair);
                    std::string stateStr, hbStr;
                    if (!std::getline(singlePair, stateStr, ',') || !std::getline(singlePair, hbStr))
                    {
                        std::cerr << "ERROR parsing pair: " << pair << std::endl;
                        controllers[cs].cs->disable();
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        controllers[cs].cs->enable();
                        continue;
                    }

                    int state = std::atoi(stateStr.c_str());
                    int hbPin = std::atoi(hbStr.c_str()) - 1;

                    if (state >= 0 && state <= 2 && hbPin >= 0 && hbPin <= 11)
                    {
                        controllers[cs].configChip(hbPins[hbPin], states[state], Tle94112::TLE_NOPWM);
                    }
                    if (state ==2){
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                controllers[cs].timer->delayMilli(1);
            }
            // // ensure all half-bridges are set floating before delay
            // for (int i = 0; i < 8; i++)
            // {
            //     for (int hbPinIndex = 0; hbPinIndex < 12; hbPinIndex++)
            //     {
            //         controllers[i].configHB(hbPins[hbPinIndex], states[0], Tle94112::TLE_NOPWM);
            //     }
            // }
            // std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Cycle delay
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Idle wait
        }
    }
}

int main()
{
    uint8_t csPins[] = {
        TLE94112_PIN_CS0, TLE94112_PIN_CS1, TLE94112_PIN_CS2, TLE94112_PIN_CS3,
        TLE94112_PIN_CS4, TLE94112_PIN_CS5, TLE94112_PIN_CS6, TLE94112_PIN_CS7};

    Tle94112Rpi controllers[8] = {
        Tle94112Rpi(csPins[0]), Tle94112Rpi(csPins[1]), Tle94112Rpi(csPins[2]), Tle94112Rpi(csPins[3]),
        Tle94112Rpi(csPins[4]), Tle94112Rpi(csPins[5]), Tle94112Rpi(csPins[6]), Tle94112Rpi(csPins[7])};

    Tle94112::HBState states[] = {Tle94112::TLE_FLOATING, Tle94112::TLE_LOW, Tle94112::TLE_HIGH};
    Tle94112::HalfBridge hbPins[] = {
        Tle94112::TLE_HB1, Tle94112::TLE_HB2, Tle94112::TLE_HB3, Tle94112::TLE_HB4,
        Tle94112::TLE_HB5, Tle94112::TLE_HB6, Tle94112::TLE_HB7, Tle94112::TLE_HB8,
        Tle94112::TLE_HB9, Tle94112::TLE_HB10, Tle94112::TLE_HB11, Tle94112::TLE_HB12};

    for (int i = 0; i < 8; i++)
        controllers[i].begin();
    for (int i = 0; i < 8; i++)
        controllers[i].cs->init();
    for (int i = 0; i < 8; i++)
    {
        controllers[i].clearErrors();
        for (int hbPinIndex = 0; hbPinIndex < 12; hbPinIndex++)
        {
            controllers[i].configHB(hbPins[hbPinIndex], states[0], Tle94112::TLE_NOPWM);
        }
    }

    std::cout << "READY\n"
              << std::flush;
    if (!bcm2835_init())
    {
        std::cerr << "ERROR: Failed to initialize bcm2835\n";
        return 1;
    }

    std::vector<Tle94112Rpi> controllerVec(std::begin(controllers), std::end(controllers));
    std::thread execThread(executePattern, std::ref(controllerVec), hbPins, states);

    std::string input;
    while (std::getline(std::cin, input))
    {
        if (input.empty())
            continue;

        if (input == "START")
        {
            running.store(true);
            std::cout << "SUCCESS: Started pattern\nEND\n"
                      << std::flush;
            continue;
        }

        if (input == "STOP")
        {
            running.store(false);
            std::lock_guard<std::mutex> lock(queueMutex);
            commandQueue.clear();
            std::cout << "SUCCESS: Stopped pattern and cleared queue\nEND\n"
                      << std::flush;
            continue;
        }

        std::istringstream input_stream(input);
        std::string cs_pin_str, state_hb_pairs;
        if (!(input_stream >> cs_pin_str) || !std::getline(input_stream, state_hb_pairs))
        {
            std::cout << "ERROR: Invalid command format\nEND\n"
                      << std::flush;
            continue;
        }

        int csPinIndex = std::atoi(cs_pin_str.c_str());
        if (csPinIndex < 0 || csPinIndex > 7)
        {
            std::cout << "ERROR: Invalid chip select pin\nEND\n"
                      << std::flush;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            commandQueue.push_back({csPinIndex, state_hb_pairs});
        }

        std::cout << "SUCCESS: Queued command for CS" << csPinIndex << "\nEND\n"
                  << std::flush;
    }

    running.store(false);
    execThread.join();

    std::cout << "SHUTDOWN\n"
              << std::flush;
    for (int i = 0; i < 8; i++)
        controllers[i].end();
    bcm2835_close();
    return 0;
}
