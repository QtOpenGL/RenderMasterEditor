#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <vector>
#include <memory>
#include "Engine.h"
#include <QWidget>
#include "myspinbox.h"

class SceneTreeWidget;

using namespace RENDER_MASTER;

namespace Ui {
class PropertiesWidget;
}

class PropertiesWidget : public QWidget, RENDER_MASTER::IPositionEventSubscriber, RENDER_MASTER::IRotationEventSubscriber
{
    Q_OBJECT

	//std::vector<ResourcePtr<IGameObject>> _selectedGameObjects;
	ResourcePtr<IGameObject> _go;

public:
    explicit PropertiesWidget(QWidget *parent = 0);
    ~PropertiesWidget();

protected:
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;

public:
	void SetGameObject(IResource *go);
	API Call(OUT vec3 *pos) override;
	API Call(OUT quat *rot) override;

private slots:
	void onSceneTreeInited(SceneTreeWidget*);
	void onSelectionChanged(const std::vector<IResource*>& selectedGameObjects);

private:
    Ui::PropertiesWidget *ui;

	std::vector<QMetaObject::Connection> _connections;

	void unsubscribeFromPreviousGO();
	void subscribeToGO(RENDER_MASTER::IGameObject *go);

	void connectPosition(MySpinBox *w, int xyz_offset);
	void connectRotation(MySpinBox *w, int xyz_offset);
	void fmod360(MySpinBox *w);

private:
	bool block{false};
	bool signalBlocked{false};
};

#endif // PROPERTIESWIDGET_H
