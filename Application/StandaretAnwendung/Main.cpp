#include <Engine_Include.h>

#include <EventSystem.h>

HINSTANCE g_hInst;
HWND g_hWnd;

LPCWSTR windowName = L"Demo";
int screenWidth = 800, screenHeight = 600;

int rectSize = 256;

uint32_t id = 0;
ATOM				MyRegisterClass(HINSTANCE);
bool				CreateMainWnd(int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
MSG msg{ 0 };

struct mouseButtonEvent: public Event{
public:
    bool LButton;
    bool RButton;
    bool MButton;
    bool LButtonAlt;
    bool RButtonAlt;
    bool MButtonAlt;

    long mousePosX;
    long mousePosY;
    long mousePosXAlt;
    long mousePosYAlt;

    void emptyMethod() {};
};

struct mouseScrollEvent : public Event {
public:
    
    byte dir;

    void emptyMethod() {};
};

class DragMover : public EventListener {
private:
    static ComponentRegister<DragMover> reg;
public:

    Transform* transform;

    DragMover() {
        transform = NULL;
    }

    static bool getIsLonly() { return true; };
    static std::vector<std::string> getRequirements() {
        std::vector<std::string> requirements = std::vector<std::string>();
        requirements.push_back(typeid(Transform).name());
        return requirements;
    };
    void setRequirements(std::vector <Component*> toSet) {
        transform = (Transform*)toSet[0];
        EventSystem::getInstance()->addEventListener(this);
    };

    virtual bool onEvent(Event* e) {
        mouseScrollEvent* mse = dynamic_cast<mouseScrollEvent*>(e);
        mouseButtonEvent* mbe = dynamic_cast<mouseButtonEvent*>(e);
        if (mbe != NULL) {
            if (mbe->LButton && mbe->LButtonAlt) {
                transform->AddPosition(
                    (mbe->mousePosX - mbe->mousePosXAlt) * (transform->m_position.z + 1) * 2,
                    (mbe->mousePosYAlt - mbe->mousePosY) * (transform->m_position.z + 1) * 2,
                    0);
                return true;
            }
        }
        else if (mse != NULL)
        {
            transform->AddPosition(0, 0, ((int)mse->dir) - 1);
            if (transform->m_position.z < 0)
                transform->AddPosition(0, 0, -transform->m_position.z);
        }
        return false;
    }

    void set(SerializedObject& from) {};
    void afterSet() {};
    SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    SerializedObject serialize(SerializedObject& inTo) {
        inTo << typeid(DragMover).name();
        return inTo;
    };
};
ComponentRegister<DragMover> DragMover::reg(typeid(DragMover).name());

class ButtonChecker : public EventListener {
private:
    static ComponentRegister<ButtonChecker> reg;
public:
    DrawableRenderer* button;

    ButtonChecker() {
        button = NULL;
    }

    static bool getIsLonly() { return true; };
    static std::vector<std::string> getRequirements() {
        std::vector<std::string> requirements = std::vector<std::string>();
        requirements.push_back(typeid(DrawableRenderer).name());
        return requirements;
    };
    void setRequirements(std::vector <Component*> toSet) {
        button = (DrawableRenderer*)toSet[0];
        EventSystem::getInstance()->addEventListener(this);
    };

    virtual bool onEvent(Event* e) {
        if (dynamic_cast<mouseButtonEvent*>(e) != NULL) {
            mouseButtonEvent* m = (mouseButtonEvent*)e;
            if (m->LButton && !m->LButtonAlt) {
                button->checkButtons(true, false);
            }
            else if (!m->LButton && m->LButtonAlt) {
                button->checkButtons(false, true);
            }
            else
                button->checkButtons(false, false);
        }
        return false;
    }

    void set(SerializedObject& from) {};
    void afterSet() {};
    SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    SerializedObject serialize(SerializedObject& inTo) {
        inTo << typeid(DragMover).name();
        return inTo;
    };
};
ComponentRegister<ButtonChecker> ButtonChecker::reg(typeid(ButtonChecker).name());

class objectKlicker : public Button::ButtonActor {
private:
    static ComponentRegister<objectKlicker> reg;
public:

    bool klick;
    XMFLOAT4 onklick[2];

    static bool getIsLonly() { return false; };
    static std::vector<std::string> getRequirements() {
        std::vector<std::string> requirements = std::vector<std::string>();
        requirements.push_back(typeid(InstancedTeilSprite::Instance).name());
        return requirements;
    };
    void setRequirements(std::vector <Component*> toSet) {
        inst = (InstancedTeilSprite::Instance*)toSet[0];
    };

    virtual void klicked() {
        //toDo
    };

    void set(SerializedObject& from) {};
    void afterSet() {};
    SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    SerializedObject serialize(SerializedObject& inTo) {
        inTo << typeid(objectKlicker).name();
        return inTo;
    };

protected:
    InstancedTeilSprite::Instance* inst;
};
ComponentRegister<objectKlicker> objectKlicker::reg(typeid(objectKlicker).name());

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_hInst = hInstance;
    g_hWnd = NULL;
    MyRegisterClass(hInstance);
    
    if (!CreateMainWnd(nCmdShow))
        return -1;

    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"DrawableRendererClass";
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

bool CreateMainWnd(int nCmdShow)
{
    LTRACE("MAIN::CREATEWINDOW");
    RECT rc = { 0, 0, screenWidth, screenHeight };
    ::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = ::CreateWindow(L"DrawableRendererClass", windowName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, g_hInst, NULL);

    if (g_hWnd == NULL)
        return false;

    ::ShowWindow(g_hWnd, nCmdShow);
    ::UpdateWindow(g_hWnd);

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    if (message == WM_MOUSEWHEEL) {

    }
    else if (message == WM_PAINT) {
        hdc = ::BeginPaint(hWnd, &ps);
        ::EndPaint(hWnd, &ps);
    }
    else if (message == WM_SIZE) {
        screenWidth = LOWORD(lParam);
        screenHeight = HIWORD(lParam);
    }
    else if (message == WM_DESTROY) {
        ::PostQuitMessage(0);
    }
    return ::DefWindowProc(hWnd, message, wParam, lParam);
}