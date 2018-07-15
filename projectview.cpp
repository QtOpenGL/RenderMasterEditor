#include "projectview.h"
#include "ui_projectview.h"
#include "EngineGlobal.h"

using namespace RENDER_MASTER;

extern EngineGlobal* eng;

ProjectView::ProjectView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectView)
{
    ui->setupUi(this);

    connect(eng, &EngineGlobal::EngineInited, this, &ProjectView::onEngineInited, Qt::DirectConnection);
    connect(eng, &EngineGlobal::EngineBeforeClose, this, &ProjectView::onEngineClosed, Qt::DirectConnection);

    _model.setRootPath(QDir::currentPath());

    ui->treeView->setModel(&_model);
}

ProjectView::~ProjectView()
{
    delete ui;
}

void ProjectView::onEngineInited(RENDER_MASTER::ICore *pCore)
{
    QString projectDir = eng->GetProjectDir();

    QDir dir(projectDir);
    if (!dir.exists())
        qWarning("Dir doesn't exist");

    _model.setRootPath(dir.absolutePath());
    ui->treeView->setRootIndex(_model.index(projectDir));
}

void ProjectView::onEngineClosed(RENDER_MASTER::ICore *pCore)
{

}
