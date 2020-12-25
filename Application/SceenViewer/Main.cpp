#define TraceOn
#define WarnOn
#define ErrorOn

#include <NetzwerkSumm.h>
#include <Engine_Include.h>

#include <EventSystem.h>

HINSTANCE g_hInst;
HWND g_hWnd;

LPCWSTR windowName = L"Demo";
int screenWidth = 800, screenHeight = 600;

int rectSize = 256;

uint32_t id = 0;
int addGameobjectDisplayer(
    int counter,
    int sidecount,
    std::vector<XMFLOAT4>& rects1,
    ObjectStorrage* storrageInstance,
    ObjectStorrage* inspectedSceenStorrage,
    InstancedTeilSprite* instanceRenderer,
    StringRenderer* stringRenderer,
    Dx11Base* buttonChecker,
    GameObject* it);
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
        klick = !klick;
        inst->rect = onklick[(int)klick];
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
    
    ObjectStorrage* storrageInstance = ObjectStorrage::getInstance();

    GameObject* renderObject = storrageInstance->createGameObject();
    renderObject->name = "rendererObject";
    DrawableRenderer* mainRenderer = renderObject->addComponent<DrawableRenderer>();
    InstancedTeilSprite* instanceRenderer = mainRenderer->gameobject->addComponent<InstancedTeilSprite>();
    instanceRenderer->m_shaderPath = "Assets\\Shader\\ShaderTexture_01.hlsl";
    instanceRenderer->m_texturePath = "Assets\\ColoredRects.png";
    StringRenderer* stringRenderer = mainRenderer->gameobject->addComponent<StringRenderer>();
    stringRenderer->m_shaderPath = "Assets\\Shader\\ShaderTexture_01.hlsl";
    stringRenderer->m_texturePath = "Assets\\Ascii.png";
    stringRenderer->m_tabSize = 10;
    stringRenderer->m_endLineSize = 20;
    stringRenderer->m_charSizeMultiplier = 1;
    std::vector<Factory::float6> rects0 = Factory::charVectorBuilder("Assets\\charStellen.txt");
    for (int i = 0; i < 127; i++) {
        stringRenderer->m_alphabetRects[i] = rects0[i];
    }
    for (int i = 128; i < 256; i++) {
        stringRenderer->m_alphabetRects[i] = { 0,0,0,0,0,0 };
    }
    renderObject->addComponent<ButtonChecker>();


    GameObject* kamerObject = storrageInstance->createGameObject();
    kamerObject->name = "kameraObject"; 
    Transform* mainKameraTransform = kamerObject->addComponent<Transform>();
    Kamera* mainKamera = kamerObject->addComponent<Kamera>();
    mainKamera->transform = mainKameraTransform;
    mainKamera->setProjection(screenWidth, screenHeight, 100.0f, 0.1f);
    mainKamera->setOrthogonal(false);
    mainRenderer->setKamera(mainKamera);
    kamerObject->addComponent<DragMover>();


    GameObject* fpsObject = storrageInstance->createGameObject();
    fpsObject->name = "fpsObject";

    mainRenderer->addDrawable(instanceRenderer);
    mainRenderer->addDrawable(stringRenderer);

    //SerializedObject so = storrageInstance->serialize();

    //writeToFile(so, "ObjectLoader.bin");

    ObjectStorrage inspectedSceenStorrage = ObjectStorrage();

    /*SerializedObject otherSO = readSerializedObjectFromFile("ObjectLoader.bin");

    std::string name;

    otherSO >> name;

    inspectedSceenStorrage.set(otherSO);
    inspectedSceenStorrage.afterSet();*/

    const int objCount = 500;

    GameObject* next[objCount];

    next[0] = inspectedSceenStorrage.createGameObject();

    for (int i = 1; i < objCount; i++) {
        next[i] = inspectedSceenStorrage.createGameObject();
        int p = rand() % i;
            next[i]->setParrent(next[p]);
    }

    std::vector<XMFLOAT4> rects1 = Factory::RectVectorBuilder("Assets\\ColoredRectsCoords.txt");

    std::map<int,GameObject*>* allObject = inspectedSceenStorrage.getAll();
    int counter = 0;
    for (std::map<int, GameObject*>::iterator it = allObject->begin(); it != allObject->end(); ++it) {
        if (it->second->getParrentID() == EMPTY_ID) {
            counter += addGameobjectDisplayer(counter, 0, rects1, storrageInstance, &inspectedSceenStorrage, instanceRenderer, stringRenderer, mainRenderer, it->second);
            counter++;
        }
    }

    /*
    SerializedObject so = readFromFile("sceen.bin");

    std::string name;
    so >> name;

    ObjectStorrage::getInstance()->set(so);
    ObjectStorrage::getInstance()->afterSet();*/

    if (!mainRenderer->Initialize(g_hWnd, g_hInst))
        return -1;

    POINT mPoint;
    GetCursorPos(&mPoint);
    ScreenToClient(g_hWnd, &mPoint);

    mouseButtonEvent* e = new mouseButtonEvent();
    e->destroy = false;
    e->usable = true;

    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        e->LButtonAlt = e->LButton;
        e->RButtonAlt = e->RButton;
        e->MButtonAlt = e->MButton;
        //e->LButton = GetKeyState(VK_LBUTTON) & 0x01;
        //e->RButton = GetKeyState(VK_RBUTTON) & 0x02;  toggle state
        //e->MButton = GetKeyState(VK_MBUTTON) & 0x10;

        e->LButton = GetKeyState(VK_LBUTTON) & 0x80;
        e->RButton = GetKeyState(VK_RBUTTON) & 0x80;
        e->MButton = GetKeyState(VK_MBUTTON) & 0x80;

        e->mousePosXAlt = mPoint.x;
        e->mousePosYAlt = mPoint.y;

        GetCursorPos(&mPoint);
        ScreenToClient(g_hWnd, &mPoint);

        e->mousePosX = mPoint.x;
        e->mousePosY = mPoint.y;

        e->updateFireEvent();

        EventSystem::getInstance()->update();

        mainRenderer->Render();
    }
    
    mainRenderer->Terminate();

    return (int)msg.wParam;
}

int addGameobjectDisplayer(
        int counter, 
        int sidecount, 
        std::vector<XMFLOAT4>& rects1,
        ObjectStorrage* storrageInstance, 
        ObjectStorrage* inspectedSceenStorrage,
        InstancedTeilSprite* instanceRenderer, 
        StringRenderer* stringRenderer,
        Dx11Base* buttonChecker,
        GameObject* it) {
    GameObject* temp[3];
    Transform* tempTransform[3];
    InstancedTeilSprite::Instance* tempInstance;
    Button* button;
    objectKlicker* klicker;
    StringRenderer::Instance* textInstance;
    for (int i = 0; i < 3; i++) {
        temp[i] = storrageInstance->createGameObject();
        tempTransform[i] = temp[i]->addComponent<Transform>();
    }
    temp[1]->setParrent(temp[0]);
    tempTransform[1]->m_parrent = tempTransform[0];
    temp[2]->setParrent(temp[0]);
    tempTransform[2]->m_parrent = tempTransform[0];

    tempTransform[0]->SetPosition(sidecount * 80, screenHeight - 80 * counter, 0);
    tempTransform[0]->SetScale(8, 8, 1);

    tempInstance = temp[1]->addComponent<InstancedTeilSprite::Instance>();
    tempInstance->rect = rects1[2];
    tempTransform[1]->SetScale(64, 64, 1);
    instanceRenderer->addInstance(tempInstance);
    button = temp[1]->addComponent<Button>();
    buttonChecker->addButton(button);
    klicker = temp[1]->addComponent<objectKlicker>();
    klicker->onklick[0] = rects1[2];
    klicker->onklick[1] = rects1[3];
    button->addActor(klicker);

    textInstance = temp[2]->addComponent<StringRenderer::Instance>();
    textInstance->string = (it->getID() + '0');
    tempTransform[2]->SetPosition(-16, 16, 0);
    //stringRenderer->addInstance(textInstance);

    int childDepth = 0;

    std::vector<int> *children = it->getChilds();
    for (int i = 0; i < children->size(); i++) {
        childDepth += addGameobjectDisplayer(
            counter + childDepth,
            sidecount + 1, 
            rects1, 
            storrageInstance, 
            inspectedSceenStorrage, 
            instanceRenderer, 
            stringRenderer, 
            buttonChecker,
            inspectedSceenStorrage->getById(children->at(i))
        );
        childDepth++;
    }
    return std::max(childDepth - 1, 0);
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
    LTRACE("MAIN::STARTED");
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
        mouseScrollEvent* e = new mouseScrollEvent();
        e->destroy = true;
        e->usable = false;
        e->dir = (GET_WHEEL_DELTA_WPARAM(wParam) / 120) + 1;
        e->updateFireEvent();
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