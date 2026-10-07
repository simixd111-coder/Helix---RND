#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "helix.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cwchar>
#include <vector>

namespace
{
constexpr int kRenderWidth = 640;
constexpr int kRenderHeight = 360;
constexpr float kBallRadius = 7.0f;
constexpr float kPaddleWidth = 92.0f;
constexpr float kPaddleHeight = 12.0f;
constexpr float kPaddleY = 28.0f;
constexpr size_t kBrickCount = 32;
constexpr UINT kFrameIntervalMs = 16;

struct Brick
{
    float x;
    float y;
    bool alive;
};

struct VisualVertex
{
    HxVec3 position;
    HxVec4 color;
};

struct App
{
    HWND window = nullptr;
    HDC backbuffer_dc = nullptr;
    HBITMAP backbuffer_bitmap = nullptr;
    HBITMAP previous_backbuffer = nullptr;
    int backbuffer_width = 0;
    int backbuffer_height = 0;
    bool booted = false;
    bool paused = true;
    bool game_over = false;
    HxWorld world = nullptr;
    HxCam camera = nullptr;
    HxMesh frame_mesh = nullptr;
    HxSkin skin = nullptr;
    std::array<HxColor, 7> palette{};
    std::array<Brick, kBrickCount> bricks{};
    std::vector<VisualVertex> vertices;
    std::vector<uint8_t> rgba;
    std::vector<uint8_t> bgra;
    std::chrono::steady_clock::time_point last_frame{};
    std::chrono::steady_clock::time_point fps_window_start{};
    unsigned int frames_in_window = 0;
    int fps = 0;
    float paddle_x = kRenderWidth * 0.5f;
    float ball_x = kRenderWidth * 0.5f;
    float ball_y = 58.0f;
    float ball_vx = 145.0f;
    float ball_vy = -185.0f;
    int score = 0;
    int lives = 3;
};

void reset_bricks(App& app)
{
    constexpr float brick_width = 56.0f;
    constexpr float gap = 10.0f;
    constexpr float first_x = (kRenderWidth - (8.0f * brick_width + 7.0f * gap)) * 0.5f + brick_width * 0.5f;
    for (size_t row = 0; row < 4; ++row)
    {
        for (size_t column = 0; column < 8; ++column)
        {
            Brick& brick = app.bricks[row * 8 + column];
            brick.x = first_x + static_cast<float>(column) * (brick_width + gap);
            brick.y = 286.0f - static_cast<float>(row) * 25.0f;
            brick.alive = true;
        }
    }
}

void reset_ball(App& app)
{
    app.paddle_x = kRenderWidth * 0.5f;
    app.ball_x = kRenderWidth * 0.5f;
    app.ball_y = 58.0f;
    app.ball_vx = 145.0f;
    app.ball_vy = -185.0f;
}

bool initialize_app(App& app)
{
    HxCfg config{};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    config.app_name = "Helix Visual Smoke Test";
    if (hx_boot(&config) != HX_OK)
        return false;
    app.booted = true;

    app.world = hx_make_world();
    app.camera = hx_make_cam3d();
    hx_set_cam_ortho(app.camera, 0.0f, static_cast<float>(kRenderWidth), 0.0f, static_cast<float>(kRenderHeight), -1.0f, 1.0f);

    const HxColor colors[] = {
        {0.055f, 0.075f, 0.14f, 1.0f},
        {0.12f, 0.32f, 0.50f, 1.0f},
        {0.12f, 0.72f, 0.82f, 1.0f},
        {0.28f, 0.80f, 0.59f, 1.0f},
        {0.95f, 0.66f, 0.26f, 1.0f},
        {0.94f, 0.36f, 0.47f, 1.0f},
        {0.89f, 0.93f, 1.0f, 1.0f},
    };
    std::copy(std::begin(colors), std::end(colors), app.palette.begin());
    app.skin = hx_make_skin(HX_WHITE, HX_SKIN_UNLIT | HX_SKIN_DOUBLE_SIDED);

    app.rgba.resize(static_cast<size_t>(kRenderWidth) * kRenderHeight * 4u);
    app.bgra.resize(app.rgba.size());
    reset_bricks(app);
    reset_ball(app);
    app.last_frame = std::chrono::steady_clock::now();
    app.fps_window_start = app.last_frame;
    return app.world && app.camera && app.skin;
}

void shutdown_app(App& app)
{
    if (app.backbuffer_dc)
    {
        if (app.previous_backbuffer)
            SelectObject(app.backbuffer_dc, app.previous_backbuffer);
        if (app.backbuffer_bitmap)
            DeleteObject(app.backbuffer_bitmap);
        DeleteDC(app.backbuffer_dc);
    }
    if (app.world)
        hx_drop_world(app.world);
    if (app.frame_mesh)
        hx_drop_mesh(app.frame_mesh);
    if (app.skin)
        hx_drop_skin(app.skin);
    if (app.camera)
        hx_drop_cam(app.camera);
    if (app.booted)
        hx_quit();
}

void add_rectangle(App& app, size_t color_index, float x, float y, float width, float height, float z = 0.0f)
{
    const float left = x - width * 0.5f - 1.0f;
    const float right = x + width * 0.5f + 1.0f;
    const float bottom = y - height * 0.5f - 1.0f;
    const float top = y + height * 0.5f + 1.0f;
    const HxColor color = app.palette[color_index];
    const HxVec4 vertex_color{color.r, color.g, color.b, color.a};
    const HxVec3 positions[] = {
        {left, bottom, z}, {right, bottom, z}, {right, top, z},
        {left, bottom, z}, {right, top, z}, {left, top, z},
    };
    for (const HxVec3& position : positions)
        app.vertices.push_back({position, vertex_color});
}

bool render_frame(App& app)
{
    hx_clear_world(app.world);
    if (app.frame_mesh)
    {
        hx_drop_mesh(app.frame_mesh);
        app.frame_mesh = nullptr;
    }
    app.vertices.clear();
    add_rectangle(app, 1, 10.0f, kRenderHeight * 0.5f, 3.0f, kRenderHeight - 24.0f, -0.6f);
    add_rectangle(app, 1, kRenderWidth - 10.0f, kRenderHeight * 0.5f, 3.0f, kRenderHeight - 24.0f, -0.6f);

    constexpr float brick_width = 56.0f;
    constexpr float brick_height = 17.0f;
    for (size_t i = 0; i < app.bricks.size(); ++i)
    {
        if (app.bricks[i].alive)
            add_rectangle(app, 2 + i / 8, app.bricks[i].x, app.bricks[i].y,
                          brick_width, brick_height, -0.3f);
    }

    add_rectangle(app, 2, app.paddle_x, kPaddleY, kPaddleWidth, kPaddleHeight, -0.1f);
    add_rectangle(app, 6, app.ball_x, app.ball_y, kBallRadius * 2.0f, kBallRadius * 2.0f, 0.0f);

    app.frame_mesh = hx_make_mesh(app.vertices.data(), app.vertices.size(), HX_VERT_POS | HX_VERT_COLOR,
                                  sizeof(VisualVertex), nullptr, 0, false, HX_PRIM_TRIANGLES);
    if (!app.frame_mesh)
        return false;
    HxMat4 identity;
    hx_make_mat4_identity(&identity);
    hx_add_mesh(app.world, app.frame_mesh, app.skin, &identity);

    if (hx_render_headless(kRenderWidth, kRenderHeight, app.world, app.camera, app.rgba.data(),
                           static_cast<size_t>(kRenderWidth) * 4u) != HX_OK)
        return false;

    for (size_t i = 0; i < app.rgba.size(); i += 4u)
    {
        uint8_t red = app.rgba[i];
        uint8_t green = app.rgba[i + 1u];
        uint8_t blue = app.rgba[i + 2u];
        if (red == 25 && green == 25 && blue == 38)
        {
            const HxColor background = app.palette[0];
            red = static_cast<uint8_t>(background.r * 255.0f);
            green = static_cast<uint8_t>(background.g * 255.0f);
            blue = static_cast<uint8_t>(background.b * 255.0f);
        }
        app.bgra[i] = blue;
        app.bgra[i + 1u] = green;
        app.bgra[i + 2u] = red;
        app.bgra[i + 3u] = 255;
    }
    return true;
}

void update_game(App& app)
{
    const auto now = std::chrono::steady_clock::now();
    const float dt = std::clamp(std::chrono::duration<float>(now - app.last_frame).count(), 0.0f, 0.05f);
    app.last_frame = now;
    if (app.paused || app.game_over)
        return;

    const bool left = (GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000);
    const bool right = (GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000);
    if (left != right)
        app.paddle_x += (right ? 1.0f : -1.0f) * 360.0f * dt;
    app.paddle_x = std::clamp(app.paddle_x, kPaddleWidth * 0.5f + 12.0f,
                              kRenderWidth - kPaddleWidth * 0.5f - 12.0f);

    app.ball_x += app.ball_vx * dt;
    app.ball_y += app.ball_vy * dt;
    if (app.ball_x < kBallRadius + 12.0f || app.ball_x > kRenderWidth - kBallRadius - 12.0f)
        app.ball_vx = -app.ball_vx;
    if (app.ball_y > kRenderHeight - kBallRadius - 12.0f)
        app.ball_vy = -std::abs(app.ball_vy);

    const float paddle_top = kPaddleY + kPaddleHeight * 0.5f;
    if (app.ball_vy < 0.0f && app.ball_y - kBallRadius <= paddle_top &&
        app.ball_y >= kPaddleY && std::abs(app.ball_x - app.paddle_x) <= kPaddleWidth * 0.5f + kBallRadius)
    {
        const float offset = (app.ball_x - app.paddle_x) / (kPaddleWidth * 0.5f);
        app.ball_y = paddle_top + kBallRadius;
        app.ball_vy = std::abs(app.ball_vy);
        app.ball_vx = offset * 230.0f;
    }

    constexpr float brick_width = 56.0f;
    constexpr float brick_height = 17.0f;
    for (Brick& brick : app.bricks)
    {
        if (!brick.alive)
            continue;
        if (std::abs(app.ball_x - brick.x) <= brick_width * 0.5f + kBallRadius &&
            std::abs(app.ball_y - brick.y) <= brick_height * 0.5f + kBallRadius)
        {
            brick.alive = false;
            app.ball_vy = -app.ball_vy;
            app.score += 10;
            break;
        }
    }

    if (app.ball_y < -kBallRadius)
    {
        --app.lives;
        if (app.lives <= 0)
            app.game_over = true;
        reset_ball(app);
    }
    if (std::none_of(app.bricks.begin(), app.bricks.end(), [](const Brick& brick) { return brick.alive; }))
    {
        app.score += 100;
        reset_bricks(app);
    }
}

void release_backbuffer(App& app)
{
    if (!app.backbuffer_dc)
        return;
    if (app.previous_backbuffer)
        SelectObject(app.backbuffer_dc, app.previous_backbuffer);
    if (app.backbuffer_bitmap)
        DeleteObject(app.backbuffer_bitmap);
    DeleteDC(app.backbuffer_dc);
    app.backbuffer_dc = nullptr;
    app.backbuffer_bitmap = nullptr;
    app.previous_backbuffer = nullptr;
    app.backbuffer_width = 0;
    app.backbuffer_height = 0;
}

bool ensure_backbuffer(HWND window, App& app, int width, int height)
{
    if (app.backbuffer_dc && app.backbuffer_width == width && app.backbuffer_height == height)
        return true;
    release_backbuffer(app);

    HDC window_dc = GetDC(window);
    if (!window_dc)
        return false;
    HDC memory_dc = CreateCompatibleDC(window_dc);
    HBITMAP bitmap = CreateCompatibleBitmap(window_dc, width, height);
    ReleaseDC(window, window_dc);
    if (!memory_dc || !bitmap)
    {
        if (bitmap)
            DeleteObject(bitmap);
        if (memory_dc)
            DeleteDC(memory_dc);
        return false;
    }

    HGDIOBJ previous = SelectObject(memory_dc, bitmap);
    if (!previous || previous == HGDI_ERROR)
    {
        DeleteObject(bitmap);
        DeleteDC(memory_dc);
        return false;
    }
    app.backbuffer_dc = memory_dc;
    app.backbuffer_bitmap = bitmap;
    app.previous_backbuffer = static_cast<HBITMAP>(previous);
    app.backbuffer_width = width;
    app.backbuffer_height = height;
    return true;
}

void record_rendered_frame(App& app)
{
    ++app.frames_in_window;
    const auto now = std::chrono::steady_clock::now();
    const double elapsed = std::chrono::duration<double>(now - app.fps_window_start).count();
    if (elapsed >= 0.5)
    {
        app.fps = static_cast<int>(app.frames_in_window / elapsed + 0.5);
        app.frames_in_window = 0;
        app.fps_window_start = now;
    }
}

void paint_window(HWND window, App& app)
{
    PAINTSTRUCT paint{};
    HDC dc = BeginPaint(window, &paint);
    RECT client{};
    GetClientRect(window, &client);
    const int width = static_cast<int>(std::max<LONG>(1, client.right));
    const int height = static_cast<int>(std::max<LONG>(1, client.bottom));
    const bool buffered = ensure_backbuffer(window, app, width, height);
    HDC frame_dc = buffered ? app.backbuffer_dc : dc;
    FillRect(frame_dc, &client, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = kRenderWidth;
    info.bmiHeader.biHeight = -kRenderHeight;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(frame_dc, 16, 72, static_cast<int>(std::max<LONG>(1, client.right - 32)),
                  static_cast<int>(std::max<LONG>(1, client.bottom - 88)),
                  0, 0, kRenderWidth, kRenderHeight,
                  app.bgra.data(), &info, DIB_RGB_COLORS, SRCCOPY);

    SetBkMode(frame_dc, TRANSPARENT);
    SetTextColor(frame_dc, RGB(235, 244, 255));
    HFONT font = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT old_font = static_cast<HFONT>(SelectObject(frame_dc, font));
    wchar_t status[160]{};
    std::swprintf(status, sizeof(status) / sizeof(status[0]),
                  L"HELIX RND 2.0  |  SCORE %04d  |  LIVES %d  |  FPS %02d",
                  app.score, app.lives, app.fps);
    TextOutW(frame_dc, 20, 18, status, lstrlenW(status));
    SetTextColor(frame_dc, RGB(149, 174, 204));
    const wchar_t* controls = L"Move: Left/Right or A/D    Pause: Space    Exit: Esc";
    TextOutW(frame_dc, 20, 44, controls, lstrlenW(controls));
    if (app.paused || app.game_over)
    {
        RECT banner{0, 220, client.right, 270};
        SetTextColor(frame_dc, app.game_over ? RGB(255, 140, 150) : RGB(255, 255, 255));
        DrawTextW(frame_dc, app.game_over ? L"GAME OVER - press Space to restart" : L"PAUSED - press Space to resume",
                  -1, &banner, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(frame_dc, old_font);
    DeleteObject(font);
    if (buffered)
        BitBlt(dc, 0, 0, width, height, frame_dc, 0, 0, SRCCOPY);
    EndPaint(window, &paint);
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    App* app = reinterpret_cast<App*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
        app = static_cast<App*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        app->window = window;
    }

    switch (message)
    {
        case WM_CREATE:
            SetTimer(window, 1, kFrameIntervalMs, nullptr);
            return 0;
        case WM_TIMER:
            update_game(*app);
            if (!render_frame(*app))
            {
                MessageBoxA(window, hx_last_error(), "Helix render failed", MB_OK | MB_ICONERROR);
                DestroyWindow(window);
                return 0;
            }
            record_rendered_frame(*app);
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_SIZE:
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE)
                DestroyWindow(window);
            else if (wparam == VK_SPACE)
            {
                if (app->game_over)
                {
                    app->score = 0;
                    app->lives = 3;
                    app->game_over = false;
                    reset_bricks(*app);
                    reset_ball(*app);
                }
                else
                    app->paused = !app->paused;
            }
            return 0;
        case WM_PAINT:
            paint_window(window, *app);
            return 0;
        case WM_DESTROY:
            KillTimer(window, 1);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wparam, lparam);
    }
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_command)
{
    App app;
    if (!initialize_app(app))
    {
        MessageBoxA(nullptr, hx_last_error(), "Helix visual test failed to initialize", MB_OK | MB_ICONERROR);
        shutdown_app(app);
        return 1;
    }

    WNDCLASSW window_class{};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    window_class.lpszClassName = L"HelixVisualSmokeTest";
    if (!RegisterClassW(&window_class))
    {
        shutdown_app(app);
        return 1;
    }

    RECT bounds{0, 0, 960, 610};
    AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
    HWND window = CreateWindowExW(0, window_class.lpszClassName, L"Helix RND 2.0 - Visual Smoke Test",
                                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  bounds.right - bounds.left, bounds.bottom - bounds.top,
                                  nullptr, nullptr, instance, &app);
    if (!window)
    {
        shutdown_app(app);
        return 1;
    }

    if (!render_frame(app))
    {
        MessageBoxA(window, hx_last_error(), "Helix render failed", MB_OK | MB_ICONERROR);
        DestroyWindow(window);
        shutdown_app(app);
        return 1;
    }
    record_rendered_frame(app);
    ShowWindow(window, show_command);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    shutdown_app(app);
    return static_cast<int>(message.wParam);
}
