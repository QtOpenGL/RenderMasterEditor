#include "consolewidget.h"
#include "ui_consolewidget.h"
#include "EngineGlobal.h"

using namespace RENDER_MASTER;

extern EngineGlobal* eng;

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QWidget(parent, Qt::Tool),
    ui(new Ui::Console)
{
    ui->setupUi(this);

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &ConsoleWidget::execute, Qt::QueuedConnection);

    connect(eng, &EngineGlobal::EngineInited, this, &ConsoleWidget::onEngineInited, Qt::DirectConnection);
    connect(eng, &EngineGlobal::EngineBeforeClose, this, &ConsoleWidget::onEngineClosed, Qt::DirectConnection);

    eng->GetCore(pCore);
    ILogEvent *ev;
    pCore->GetLogPrintedEv(&ev);
    ev->Subscribe(this);
}

ConsoleWidget::~ConsoleWidget()
{
    delete ui;
}

void ConsoleWidget::execute()
{
    pCore->Log("hello from editor", RENDER_MASTER::LOG_TYPE::NORMAL);
    ui->lineEdit->clear();
}

void ConsoleWidget::print(const char *pStr, ::LOG_TYPE lt)
{
    QString color = "#dddddd";
    if (lt == ::LOG_TYPE::LT_WARNING)
        color = "#bbbb00";
    if (lt == ::LOG_TYPE::LT_FATAL)
        color = "#cc0000";

    QString str = QString("<font color=")+ color +QString(">") + QString(pStr) + QString("</font>");
    ui->plainTextEdit->appendHtml(str);
}

API ConsoleWidget::Call(const char *pStr, RENDER_MASTER::LOG_TYPE type)
{
    ::LOG_TYPE lt = ::LOG_TYPE::LT_NORMAL;
    if (type == RENDER_MASTER::LOG_TYPE::WARNING) lt = ::LOG_TYPE::LT_WARNING;
    if (type == RENDER_MASTER::LOG_TYPE::FATAL) lt = ::LOG_TYPE::LT_FATAL;
    print(pStr, lt);
    return S_OK;
}

void ConsoleWidget::on_pushButton_clicked()
{
    execute();
}

void ConsoleWidget::onEngineInited(ICore *pCore_)
{    
}

void ConsoleWidget::onEngineClosed(ICore *pCore)
{
    ILogEvent *ev;
    pCore->GetLogPrintedEv(&ev);
    ev->Unsubscribe(this);
}

