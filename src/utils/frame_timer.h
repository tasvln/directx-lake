#pragma once
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

class Timer {
public:
    Timer() {
        startTime = std::chrono::high_resolution_clock::now();
        previousTime = startTime;
    }

    void tick() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        elapsedTime += deltaTime;
        frameCount++;

        // Update FPS every second
        if (elapsedTime.count() >= 1.0) {
            FPS = frameCount / elapsedTime.count();
            frameCount = 0;
            elapsedTime = std::chrono::duration<double>(0);
        }
    }

    // Delta time in seconds
    double getDeltaSeconds() const { return deltaTime.count(); }
    double getDeltaMilliseconds() const { return deltaTime.count() * 1e3; }
    double getDeltaMicroseconds() const { return deltaTime.count() * 1e6; }
    double getDeltaNanoseconds() const { return deltaTime.count() * 1e9; }

    // Total time since start
    double getTotalSeconds() const {
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startTime).count();
    }

    // FPS
    double getFPS() const { return FPS; }

    std::wstring getFPSString() const {
        std::wstringstream ss;
        ss << L"FPS: " << std::fixed << std::setprecision(2) << FPS;
        return ss.str();
    }

private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point previousTime;
    std::chrono::duration<double> deltaTime{0};
    std::chrono::duration<double> elapsedTime{0};
    int frameCount = 0;
    double FPS = 0.0;
};
