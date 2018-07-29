#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QTimer>

#include "Engine.h"

#include <chrono>

class RENDER_MASTER::ICore;

class EngineGlobal : public QObject
{
    Q_OBJECT

    RENDER_MASTER::ICore *pCore;
    QTimer *timer;
    std::chrono::steady_clock::time_point start;

public:
    explicit EngineGlobal();

    void SetCore(RENDER_MASTER::ICore *pCoreIn) { pCore = pCoreIn; }
    void GetCore(RENDER_MASTER::ICore *&pCoreOut) { pCoreOut = pCore; }
    void Init(RENDER_MASTER::ICore *pCore);
    void BeforeClose();
    QString GetProjectDir();

	using CoreRender = RENDER_MASTER::ICoreRender;
	inline CoreRender *getCoreRender() { CoreRender* p; pCore->GetSubSystem((RENDER_MASTER::ISubSystem**) &p, RENDER_MASTER::SUBSYSTEM_TYPE::CORE_RENDER); return p; }

signals:
    void EngineInited(RENDER_MASTER::ICore *pCore);
    void EngineBeforeClose(RENDER_MASTER::ICore *pCore);
    void OnUpdate(float dt);
    void OnRender();

public slots:
     void _OnTimer();
};

#endif // ENGINE_H
