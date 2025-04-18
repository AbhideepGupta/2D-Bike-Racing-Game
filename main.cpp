#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#define TIMER_ID 1
#define INITIAL_OBSTACLE_COUNT 10
#define MAX_OBSTACLES 100

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

class Game {
public:
    HWND hwnd;
    int bikeX, bikeY;
    int obstacleX[MAX_OBSTACLES];
    int obstacleY[MAX_OBSTACLES];
    int obstacleCount;
    float speed;
    int score;
    int lives;
    GameState state;

    static const int laneCount = 5;
    int laneY[laneCount] = { 100, 250, 400, 550, 700 };

    Game() {
        Reset();
        state = MENU;
    }

    void Reset() {
        bikeX = 300;
        bikeY = 600;
        obstacleCount = INITIAL_OBSTACLE_COUNT;
        speed = 5.0f;
        score = 0;
        lives = 3;
        InitObstacles();
    }

    void InitObstacles() {
        for (int i = 0; i < obstacleCount; i++) {
            obstacleX[i] = 800 + rand() % 800;
            int lane = rand() % laneCount;
            int layerOffset = (i % 2) * 40;
            obstacleY[i] = laneY[lane] + layerOffset;
        }
    }

    void MoveBike() {
        for (int i = 0; i < obstacleCount; i++) {
            obstacleX[i] -= (int)speed;
            if (obstacleX[i] < -80) {
                obstacleX[i] = 800 + rand() % 800;
                int lane = rand() % laneCount;
                int layerOffset = (i % 2) * 40;
                obstacleY[i] = laneY[lane] + layerOffset;
                score++;
                if (score % 8 == 0) {
                    speed += 0.25f;
                }
                if (score % 10 == 0) {
                    if (obstacleCount < MAX_OBSTACLES) obstacleCount++;
                }
            }

            RECT bikeRect = { bikeX, bikeY, bikeX + 60, bikeY + 30 };
            RECT obsRect = { obstacleX[i], obstacleY[i], obstacleX[i] + 80, obstacleY[i] + 40 };

            RECT intersection;
            if (IntersectRect(&intersection, &bikeRect, &obsRect)) {
                lives--;
                if (lives <= 0) {
                    state = GAME_OVER;
                } else {
                    obstacleX[i] = 800 + rand() % 800;
                    int lane = rand() % laneCount;
                    int layerOffset = (i % 2) * 40;
                    obstacleY[i] = laneY[lane] + layerOffset;
                }
            }
        }
    }

    void DrawMenu(HDC hdc, int width, int height) {
        const char* msg1 = "1 - Start Game";
        const char* msg2 = "2 - Exit";
        TextOut(hdc, width / 2 - 50, height / 2 - 20, msg1, strlen(msg1));
        TextOut(hdc, width / 2 - 50, height / 2 + 10, msg2, strlen(msg2));
    }

    void DrawScene(HDC hdc) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right;
        int height = rect.bottom;

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(memDC, &rect, bgBrush);
        DeleteObject(bgBrush);

        if (state == MENU) {
            DrawMenu(memDC, width, height);
        } else {
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            SelectObject(memDC, hPen);
            SetBkMode(memDC, TRANSPARENT);
            for (int j = 100; j < height; j += 150) {
                for (int i = 0; i < width; i += 40) {
                    MoveToEx(memDC, i, j, NULL);
                    LineTo(memDC, i + 20, j);
                }
            }
            DeleteObject(hPen);

            HBRUSH bikeBody = CreateSolidBrush(RGB(0, 0, 255));
            SelectObject(memDC, bikeBody);
            Rectangle(memDC, bikeX, bikeY, bikeX + 60, bikeY + 30);
            DeleteObject(bikeBody);

            HPEN bikePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            SelectObject(memDC, bikePen);
            HBRUSH wheelBrush = CreateSolidBrush(RGB(0, 0, 0));
            SelectObject(memDC, wheelBrush);
            Ellipse(memDC, bikeX + 5, bikeY + 25, bikeX + 20, bikeY + 40);
            Ellipse(memDC, bikeX + 40, bikeY + 25, bikeX + 55, bikeY + 40);
            DeleteObject(wheelBrush);
            DeleteObject(bikePen);

            HPEN headlightPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
            SelectObject(memDC, headlightPen);
            HBRUSH headlightBrush = CreateSolidBrush(RGB(255, 255, 0));
            SelectObject(memDC, headlightBrush);
            POINT headlight[3] = {
                { bikeX + 60, bikeY + 10 },
                { bikeX + 70, bikeY + 5 },
                { bikeX + 70, bikeY + 25 }
            };
            Polygon(memDC, headlight, 3);
            DeleteObject(headlightBrush);
            DeleteObject(headlightPen);

            HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
            SelectObject(memDC, redBrush);
            for (int i = 0; i < obstacleCount; i++) {
                Rectangle(memDC, obstacleX[i], obstacleY[i], obstacleX[i] + 80, obstacleY[i] + 40);
            }
            DeleteObject(redBrush);

            char text[50];
            SetTextColor(memDC, RGB(0, 0, 0));
            SetBkMode(memDC, TRANSPARENT);
            sprintf(text, "Score: %d", score);
            TextOut(memDC, width - 150, 10, text, strlen(text));

            sprintf(text, "Lives: %d", lives);
            TextOut(memDC, width - 150, 30, text, strlen(text));

            if (state == GAME_OVER) {
                const char* msg = "Game Over! Press R to Restart";
                TextOut(memDC, width / 2 - 100, height / 2, msg, strlen(msg));
            }
        }

        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
    }
};

Game game;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        game.DrawScene(hdc);
        EndPaint(hwnd, &ps);
    } break;

    case WM_KEYDOWN:
        if (game.state == MENU) {
            if (wParam == '1') {
                game.state = PLAYING;
                game.Reset();
            } else if (wParam == '2') {
                PostQuitMessage(0);
            }
        } else if (game.state == PLAYING) {
            if (wParam == VK_UP) game.bikeY -= 10;
            if (wParam == VK_DOWN) game.bikeY += 10;
        } else if (game.state == GAME_OVER) {
            if (wParam == 'R') {
                game.state = PLAYING;
                game.Reset();
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_TIMER:
        if (game.state == PLAYING) {
            game.MoveBike();
        }
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "BikeRaceGame";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("BikeRaceGame", "2D Bike Racing Game", WS_OVERLAPPEDWINDOW,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);
    game.hwnd = hwnd;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    srand((unsigned)time(NULL));
    SetTimer(hwnd, TIMER_ID, 50, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
