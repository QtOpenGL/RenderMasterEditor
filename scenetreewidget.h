#ifndef SCENETREEWIDGET_H
#define SCENETREEWIDGET_H

#include "Engine.h"
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelection>


class RenderMasterSceneManagerAdapter : public QAbstractItemModel, RENDER_MASTER::IGameObjectEventSubscriber
{
	Q_OBJECT

	RENDER_MASTER::ISceneManager *sm{nullptr};

public:
	RenderMasterSceneManagerAdapter(QObject *parent);

	// STANDARD FUNCTIONS
	virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
	virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
	virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
	virtual QModelIndex parent( const QModelIndex &index ) const override;
	virtual QVariant data( const QModelIndex &index, int role ) const override;

	// EDIT
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	// DRAG & DROP
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const override;
	virtual QStringList mimeTypes() const override;
	virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

	// IGameObjectEventSubscriber interface
public:
	API Call(RENDER_MASTER::IGameObject *pGameObject) override;


private slots:
	void onEngineInited(RENDER_MASTER::ICore *pCore);
	void onEngineClosed(RENDER_MASTER::ICore *pCore);

};


namespace Ui {
class SceneTreeWidget;
}

class SceneTreeWidget : public QWidget
{
    Q_OBJECT

	RenderMasterSceneManagerAdapter *_model{nullptr};

public:
    explicit SceneTreeWidget(QWidget *parent = 0);
    ~SceneTreeWidget();

signals:
	void selectionChanged(RENDER_MASTER::IGameObject *pGameObject);

private:
    Ui::SceneTreeWidget *ui;

	void _add_game_object(RENDER_MASTER::IGameObject *go, int root, RENDER_MASTER::IGameObject *parent);

private slots:
	void _selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void _currentChanged(const QModelIndex &current, const QModelIndex &previous);


};

#endif // SCENETREEWIDGET_H
