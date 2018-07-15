#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "dialog.h"

#include "ads/API.h"
#include "ads/ContainerWidget.h"
#include "ads/SectionContent.h"

class TreeModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_actionNew_scene_triggered();

    void on_actionExit_triggered();

    void on_actionBuild_Standalone_triggered();

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void contextMenuEvent(QContextMenuEvent* e);

private:
    Ui::MainWindow *ui;
    Dialog *dg;
    ADS_NS::ContainerWidget* _container;
};

#endif // MAINWINDOW_H
