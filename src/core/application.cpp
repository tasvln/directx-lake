#include "application.h"
#include "window.h"

#include "engine/device.h"
#include "engine/command_queue.h"
#include "engine/swapchain.h"
#include "engine/shader.h"
#include "engine/pipeline.h"
#include "engine/resources/constant.h"

#include "engine/scene/camera.h"
#include "engine/scene/lighting.h"
#include "engine/scene/weather/clock.h"
#include "engine/scene/weather/sky.h"
#include "engine/scene/weather/fog.h"
#include "engine/scene/weather/system.h"
#include "engine/scene/weather/particles.h"

#include "utils/events.h"
#include "utils/keyboard.h"
#include "utils/frame_timer.h"

Application::Application(HINSTANCE hInstance, WindowConfig& config)
    : config(config)
{
    window = std::make_unique<Window>(hInstance, config, this);
    init();
}

Application::~Application()
{
    cleanUp();
}

// -------------------------------------------------------
// init — creates all GPU resources and scene objects
// Order matters: device → swapchain → resources → scene
// -------------------------------------------------------
void Application::init()
{
    scissorRect = CD3DX12_RECT(0, 0,
        static_cast<LONG>(config.width),
        static_cast<LONG>(config.height));

    viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
        static_cast<float>(config.width),
        static_cast<float>(config.height));

    device = std::make_unique<Device>(config.useWarp);

    directCommandQueue = std::make_unique<CommandQueue>(
        device->getDevice(),
        D3D12_COMMAND_LIST_TYPE_DIRECT
    );

    swapchain = std::make_unique<Swapchain>(
        window->getHwnd(),
        device->getDevice(),
        directCommandQueue->getCommandQueue(),
        config.width, config.height,
        FRAMEBUFFERCOUNT,
        device->getSupportTearingState()
    );

    currentBackBufferIndex = swapchain->getSwapchain()->GetCurrentBackBufferIndex();

    // --- Input ---
    keyboard = std::make_unique<Keyboard>();

    // --- Camera ---
    // Far plane based on model radius so nothing gets clipped
    camera1 = std::make_unique<Camera>(
        45.0f,
        static_cast<float>(config.width) / static_cast<float>(config.height),
        0.1f,
        5000.0f
    );

    // camera1->setPosition({ 0.0f, 80.0f, -200.0f }); 
    // camera1->lookAt({ 0.0f, 0.0f, 0.0f }); 
    // camera1->setFarPlane(5000.0f);

    // --- Lighting ---
    lighting1 = std::make_unique<Lighting>(device->getDevice());

    // --- Time of day — starts at sunrise, 5 min day cycle ---
    clock = std::make_unique<Clock>(0.25f, 300.0f);

    sky = std::make_unique<Sky>(device->getDevice(), DXGI_FORMAT_R8G8B8A8_UNORM);
    fog = std::make_unique<Fog>(device->getDevice());
    weather = std::make_unique<WeatherSystem>();
    particles = std::make_unique<Particles>(
        device->getDevice(), 
        directCommandQueue.get()
    );

    // waveSystem = std::make_unique<WaveSystem>(...);
    // rippleSystem = std::make_unique<RippleSystem>(...);
    // waterPass = std::make_unique<WaterPass>(...);
}

int Application::run()
{
    MSG msg = {};
    Timer timer;

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            timer.tick();

            UpdateEventArgs updateArgs(timer.getDeltaSeconds(), timer.getTotalSeconds());
            onUpdate(updateArgs);

            RenderEventArgs renderArgs(timer.getDeltaSeconds(), timer.getTotalSeconds());
            onRender(renderArgs);

            // Show FPS + time of day in title bar
            SetWindowTextW(window->getHwnd(),
                (std::wstring(config.appName)
                + L" - " + timer.getFPSString()
                + L" - " + clock->getTimeString()
                ).c_str()
            );
        }
    }

    return static_cast<int>(msg.wParam);
}

void Application::onUpdate(UpdateEventArgs& args)
{
    // --- Time of day drives lighting ---
    clock->update(static_cast<float>(args.elapsedTime));
    const TimeOfDayState& tod = clock->getState();

    lighting1->setEyePosition(camera1->getPosition());

    weather->update(static_cast<float>(args.elapsedTime));
    const WeatherState& ws = weather->getCurrent();

    sky->update(tod, camera1->getViewMatrix(), camera1->getProjectionMatrix(), static_cast<float>(args.totalTime), ws.cloudCoverage);

    lighting1->setEyePosition(camera1->getPosition());

    lighting1->setLight(
        0,
        LightType::Directional,
        { 0.0f, 0.0f, 0.0f },
        tod.sunDirection,
        0.0f, 0.0f, 0.0f,
        tod.sunColor,
        tod.sunIntensity * ws.sunMultiplier * 2.0f  // ← dimmed during storm
    );

    lighting1->updateGPU();

    XMMATRIX view       = camera1->getViewMatrix();
    XMMATRIX projection = camera1->getProjectionMatrix();

    viewProj = view * projection;
    keyboard->tick(); // must be last — updates prevKeys for isJustPressed
}

void Application::onRender(RenderEventArgs& args)
{
    auto commandList  = directCommandQueue->getCommandList();
    auto rtvHeap      = swapchain->getRTVHeap();
    auto dsvHeap      = swapchain->getDSVHeap();
    auto srvHeap      = swapchain->getSRVHeap();
    auto vsync        = device->getSupportTearingState();
    auto backBuffer   = swapchain->getBackBuffer(currentBackBufferIndex);

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    // Transition back buffer: present → render target
    transitionResource(commandList, backBuffer.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->getHeap()->GetCPUDescriptorHandleForHeapStart(),
        currentBackBufferIndex,
        rtvHeap->getDescriptorSize()
    );

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
        dsvHeap->getHeap()->GetCPUDescriptorHandleForHeapStart()
    );

    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Sky color comes from time of day — dark at night, blue at noon
    const TimeOfDayState& tod = clock->getState();
    const float clearColor[] = { tod.ambientColor.x, tod.ambientColor.y, tod.ambientColor.z, 1.0f };

    // const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    const WeatherState& ws = weather->getCurrent();

    particles->dispatch(
        commandList.Get(),
        camera1->getPosition(),
        viewProj,
        camera1->getCamRight(),
        camera1->getCamUp(),
        static_cast<float>(args.elapsedTime),
        ws.rainIntensity,
        ws.snowIntensity,
        ws.wind.direction,
        ws.wind.strength,
        0.0f
    );
    // waveSystem->dispatch(commandList.Get()); 

    sky->draw(commandList.Get(), fog->getGPUAddress());
    // waterPass->draw(commandList.Get());
    particles->draw(commandList.Get());

    // Transition back buffer: render target → present
    transitionResource(commandList, backBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    fenceValues[currentBackBufferIndex] = directCommandQueue->executeCommandList(commandList);

    INT syncInterval  = vsync ? 1 : 0;
    UINT presentFlags = !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    throwFailed(swapchain->getSwapchain()->Present(syncInterval, presentFlags));

    currentBackBufferIndex = swapchain->getSwapchain()->GetCurrentBackBufferIndex();

    // Wait for GPU to finish this frame before reusing its resources
    directCommandQueue->fenceWait(fenceValues[currentBackBufferIndex]);
}

void Application::transitionResource(
    ComPtr<ID3D12GraphicsCommandList2> commandList,
    ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState,
    D3D12_RESOURCE_STATES afterState)
{
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
    commandList->ResourceBarrier(1, &barrier);
}

void Application::onResize(ResizeEventArgs& args)
{
    if (!device || !swapchain) return;
    if (args.width == 0 || args.height == 0) return;

    if (config.width != args.width || config.height != args.height)
    {
        config.width  = std::max(1u, static_cast<unsigned int>(args.width));
        config.height = std::max(1u, static_cast<unsigned int>(args.height));

        directCommandQueue->flush();
        swapchain->resize(config.width, config.height);
        currentBackBufferIndex = swapchain->getSwapchain()->GetCurrentBackBufferIndex();

        camera1->setProjection(45.0f,
            float(config.width) / float(config.height),
            0.1f, 1000.0f);

        viewport    = { 0.0f, 0.0f, float(config.width), float(config.height), 0.0f, 1.0f };
        scissorRect = { 0, 0, static_cast<LONG>(config.width), static_cast<LONG>(config.height) };
    }
}

void Application::onMouseWheel(MouseWheelEventArgs& args)
{
    camera1->setFov(camera1->getFov() - args.wheelDelta * 0.05f);
}

void Application::onMouseMoved(MouseMotionEventArgs& args)
{
    if (!args.leftButton) return;

    if (camera1->getMode() == CameraMode::ThirdPerson)
    {
        cameraYaw   += static_cast<float>(args.relX) * 0.01f;
        cameraPitch -= static_cast<float>(args.relY) * 0.01f;
        cameraPitch  = std::clamp(cameraPitch, -XM_PIDIV4, XM_PIDIV2 - 0.05f);
    }
    else if (args.shift)
    {
        camera1->pan(static_cast<float>(args.relX), static_cast<float>(args.relY));
    }
    else
    {
        camera1->orbit(static_cast<float>(args.relX) * 0.01f, static_cast<float>(args.relY) * 0.01f);
    }
}

void Application::onKeyPressed(KeyEventArgs& e)
{
    keyboard->onKeyPressed(e.key);

    // F1-F4 = jump to specific time of day for testing
    // F5 = toggle pause
    // F6 - F10 = Weather Modes
    switch (e.key)
    {
        case KeyCode::Key::F1: clock->setTimeOfDay(0.25f); break; // sunrise
        case KeyCode::Key::F2: clock->setTimeOfDay(0.5f);  break; // noon
        case KeyCode::Key::F3: clock->setTimeOfDay(0.75f); break; // sunset
        case KeyCode::Key::F4: clock->setTimeOfDay(0.0f);  break; // midnight
        // case KeyCode::Key::F5: clock->setPaused(!clock->getState().isNight); break;
        case KeyCode::Key::F6:  weather->setWeather(WeatherType::Clear);    break;
        case KeyCode::Key::F7:  weather->setWeather(WeatherType::Rain);     break;
        case KeyCode::Key::F8:  weather->setWeather(WeatherType::Storm);    break;
        case KeyCode::Key::F9:  weather->setWeather(WeatherType::Snow);     break;
        case KeyCode::Key::F10: weather->setWeather(WeatherType::Blizzard); break;
        default: break;
    }
}

void Application::onKeyReleased(KeyEventArgs& e)
{
    keyboard->onKeyReleased(e.key);
}

void Application::cleanUp()
{
    // Flush GPU before releasing anything
    if (directCommandQueue)
        directCommandQueue->flush();

    // Release in reverse dependency order
    particles.reset();
    weather.reset();
    fog.reset();
    sky.reset();
    clock.reset();
    lighting1.reset();
    camera1.reset();
    swapchain.reset();
    directCommandQueue.reset();
    device.reset();
    window.reset();

    hwnd = nullptr;
    currentBackBufferIndex = 0;
}