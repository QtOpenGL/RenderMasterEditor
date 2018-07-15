#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QWidget>
#include <QFileSystemModel>

#include "Engine.h"

namespace Ui {
class ProjectView;
}

class ProjectView : public QWidget
{
    Q_OBJECT

    QFileSystemModel _model;

public:
    explicit ProjectView(QWidget *parent = 0);
    ~ProjectView();

private:
    Ui::ProjectView *ui;

private slots:
    void onEngineInited(RENDER_MASTER::ICore *pCore);
    void onEngineClosed(RENDER_MASTER::ICore *pCore);
};

#endif // PROJECTVIEW_H
