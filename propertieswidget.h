#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <vector>
#include <memory>
#include "Engine.h"
#include <QWidget>
#include "myspinbox.h"

class SceneTreeWidget;

namespace Ui {
class PropertiesWidget;
}

class PropertiesWidget : public QWidget, RENDER_MASTER::IPositionEventSubscriber
{
    Q_OBJECT

    RENDER_MASTER::IGameObject *_go{nullptr};

public:
    explicit PropertiesWidget(QWidget *parent = 0);
    ~PropertiesWidget();

protected:
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;

public:
	void SetGameObject(RENDER_MASTER::IGameObject *go);
    API Call(OUT vec3 *pPos) override;

private slots:
	void onSceneTreeInited(SceneTreeWidget*);
	void onSelectionChanged(RENDER_MASTER::IGameObject *go);

private:
    Ui::PropertiesWidget *ui;

	std::vector<QMetaObject::Connection> _connections;

	void unsubscribeFromPreviousGO();
	void subscribeToGO(RENDER_MASTER::IGameObject *go);

	void connectPosition(MySpinBox *w, int xyz_offset);
};

#endif // PROPERTIESWIDGET_H
