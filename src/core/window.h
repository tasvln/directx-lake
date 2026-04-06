#pragma once

#include "utils/pch.h"

class Application;

class Window
{
    public:
        Window(HINSTANCE hInstance, WindowConfig& config, Application* app);
        ~Window();

        HWND getHwnd() const { 
            return hwnd; 
        }

        HWND createWindow(HINSTANCE hInstance);

        void onFullscreen();
        void setFullScreen(bool enable);

    private:
        static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        HWND hwnd = nullptr;
        WindowConfig config;

        RECT windowRect;
        WINDOWPLACEMENT windowPlacement = { 
            sizeof(WINDOWPLACEMENT) 
        };

        Application* app = nullptr;
        
};