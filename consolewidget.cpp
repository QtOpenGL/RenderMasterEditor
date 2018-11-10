#include "consolewidget.h"
#include "ui_consolewidget.h"
#include "EngineGlobal.h"
#include "QToolTip"
#include "QCompleter"
#include "QTreeView"
#include <memory>

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

	completer_model = new QStandardItemModel();

	QTreeView *completer_view = new QTreeView;
	completer_view->setIndentation(0);
	completer_view->setUniformRowHeights(true);
	completer_view->installEventFilter(this);
	completer_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	auto header = completer_view->header();
	header->hide();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setStretchLastSection(true);

	QCompleter *completer = new QCompleter(completer_model, this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionColumn(0);
	completer->setPopup(completer_view);
	ui->lineEdit->setCompleter(completer);

    eng->GetCore(pCore);

	if (pCore)
		onEngineInited(pCore);
}

ConsoleWidget::~ConsoleWidget()
{
    delete ui;
}

void ConsoleWidget::execute()
{
	if (!pConsole)
		return;

	QString command = ui->lineEdit->text();
//	QStringList pieces = command.split( " " );
//	QString neededWord = pieces.value( pieces.length() - 2 );

//	std::unique_ptr<const char*[]> args = std::unique_ptr<const char*[]>(new const char*[pieces.length()]);
//	for(int i = 0; i < pieces.length() - 1; i++)
//	{
//		QString& arg = pieces.value( i + 1 );
//		args[i] = arg.toLatin1().data();
//	}

	//const char *pStr = command.toLatin1();

	pConsole->ExecuteCommand(command.toLatin1().data(), nullptr, 0);

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
	if (!pConsole)
		return;

	pCore->GetSubSystem((ISubSystem**)&pConsole, SUBSYSTEM_TYPE::CONSOLE);

	if (pConsole)
	{
		ILogEvent *ev;
		pConsole->GetLogPrintedEv(&ev);
		ev->Subscribe(this);

		uint commands;
		pConsole->GetCommands(&commands);

		QStandardItemModel &model = *completer_model;
		model.clear();

		for (int i = 0; i < commands; i++)
		{
			const char *name;
			pConsole->GetCommand(&name, i);
			model.setItem(i, 0, new QStandardItem(QString(name)));
		}
	}
}

void ConsoleWidget::onEngineClosed(ICore *pCore)
{
	if (!pConsole)
		return;

	ILogEvent *ev;
	pConsole->GetLogPrintedEv(&ev);
    ev->Unsubscribe(this);
	pConsole = nullptr;
}


void ConsoleWidget::on_lineEdit_textEdited(const QString &arg1)
{
	//print("dd", ::LOG_TYPE::LT_NORMAL);
	//QToolTip::showText(ui->plainTextEdit->mapToGlobal(QPoint()), tr("Invalid Input"));
}
