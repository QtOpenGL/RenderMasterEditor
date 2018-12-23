#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "dialog.h"

#include "advanceddockingsystem/include/API.h"
#include "advanceddockingsystem/include/ContainerWidget.h"
#include "advanceddockingsystem/include/SectionContent.h"

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
	void on_actionAbout_Render_Master_triggered();
	void on_actionactionManipulatorRotate_triggered(bool checked);
	void on_actionManipulatorScale_triggered(bool checked);
	void on_actionactionManipulatorTransform_triggered(bool checked);
	void on_actionSave_scene_triggered();
	void on_actionLoad_Scene_triggered();
	void on_actionClose_Scene_triggered();

	void clone_node();
	void shaders_reload();

	void on_actionactionCursorSelect_triggered(bool checked);

protected:
    virtual void closeEvent(QCloseEvent* e);
	virtual void keyPressEvent(QKeyEvent *event) override;
	//virtual void contextMenuEvent(QContextMenuEvent* e);

private:

	void switch_button(QAction *action);

    Ui::MainWindow *ui;
    Dialog *dg;
    ADS_NS::ContainerWidget* _container;
};

#endif // MAINWINDOW_H
