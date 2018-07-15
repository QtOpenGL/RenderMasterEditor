#include "EngineGlobal.h"

using namespace RENDER_MASTER;

EngineGlobal::EngineGlobal()
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(_OnTimer()));
    timer->start(0);
    start = std::chrono::steady_clock::now();
}

void EngineGlobal::Init(RENDER_MASTER::ICore *_pCore)
{
    pCore = _pCore;
    emit EngineInited(pCore);
}

void EngineGlobal::BeforeClose()
{
    emit EngineBeforeClose(pCore);
}

QString EngineGlobal::GetProjectDir()
{
    char *pDataDir;
    pCore->GetDataDir(&pDataDir);
    return QString(pDataDir);
}

void EngineGlobal::_OnTimer()
{
    static const float upd_interv = 1.0f;
    static float accum = 0.0f;

    std::chrono::duration<float> _durationSec = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
    float dt = _durationSec.count();

    accum += dt;

    if (accum > upd_interv)
    {
        accum = 0.0f;
        int fps = static_cast<int>(1.0f / dt);
        std::string fps_str = std::to_string(fps);
        std::wstring fps_strw = std::wstring(L"Test [") + std::wstring(fps_str.begin(), fps_str.end()) + std::wstring(L"]");
        static char buf[40];
        //sprintf(buf, "FPS = %i", fps);
       // pCore->Log(buf, RENDER_MASTER::LOG_TYPE::NORMAL);
    }

    start = std::chrono::steady_clock::now();

    OnUpdate(dt);
    OnRender();
}
