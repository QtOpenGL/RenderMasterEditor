#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWidget>

#include "Engine.h"

class EngineGlobal;

enum class LOG_TYPE
{
    LT_NORMAL,
    LT_WARNING,
    LT_FATAL
};

namespace Ui {
class Console;
}

class ConsoleWidget : public QWidget, RENDER_MASTER::ILogEventSubscriber
{
    Q_OBJECT

   RENDER_MASTER::ICore *pCore;

public:
    explicit ConsoleWidget(QWidget *parent = 0);
    ~ConsoleWidget();

public:
    void execute();
    void print(const char *pStr, LOG_TYPE lt = LOG_TYPE::LT_NORMAL);

public:
    API Call(const char *pSt, RENDER_MASTER::LOG_TYPE type) override;

private slots:
    void on_pushButton_clicked();
    void onEngineInited(RENDER_MASTER::ICore *pCore);
    void onEngineClosed(RENDER_MASTER::ICore *pCore);

private:
    Ui::Console *ui;
};

#endif // CONSOLE_H
