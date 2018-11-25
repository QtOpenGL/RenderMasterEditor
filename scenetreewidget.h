#ifndef SCENETREEWIDGET_H
#define SCENETREEWIDGET_H

#include "Engine.h"
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelection>
#include <memory.h>

using RENDER_MASTER::IGameObject;


class SceneManagerModel;


// This class handled addition gameobjects when engine raise event
//
class AddGameObjectHandler : RENDER_MASTER::IGameObjectEventSubscriber
{
	SceneManagerModel *_parent;
public:
	AddGameObjectHandler(SceneManagerModel *parent) : _parent(parent){}
	API Call(RENDER_MASTER::IGameObject *pGameObject) override;
};


// This class handled deletion gameobjects when engine raise event
//
class DeleteGameObjectHandler : RENDER_MASTER::IGameObjectEventSubscriber
{
	SceneManagerModel *_parent;
public:
	DeleteGameObjectHandler(SceneManagerModel *parent) : _parent(parent){}
	API Call(RENDER_MASTER::IGameObject *pGameObject) override;
};


//
//
class SceneManagerModel : public QAbstractItemModel
{
	Q_OBJECT

	friend DeleteGameObjectHandler;

	RENDER_MASTER::ISceneManager *sm = nullptr;

	std::unique_ptr<AddGameObjectHandler> _addObjectHandler;
	std::unique_ptr<DeleteGameObjectHandler> _deleteObjectHandler;

public:
	SceneManagerModel(QObject *parent);

	// Standart functions
	virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
	virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
	virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
	virtual QModelIndex parent( const QModelIndex &index ) const override;
	virtual QVariant data( const QModelIndex &index, int role ) const override;

	// Editing
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	// Drag & Drop
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const override;
	virtual QStringList mimeTypes() const override;
	virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

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

	SceneManagerModel *_model{nullptr};

public:
    explicit SceneTreeWidget(QWidget *parent = 0);
    ~SceneTreeWidget();

private:
    Ui::SceneTreeWidget *ui;

	void _add_game_object(RENDER_MASTER::IGameObject *go, int root, RENDER_MASTER::IGameObject *parent);

private slots:
	void onSelectionChangedFromEditor(const std::vector<IGameObject*>& selectedGameObjects);
	void _selectionChangedFromClick(const QItemSelection& selected, const QItemSelection& deselected);
	void _currentChanged(const QModelIndex &current, const QModelIndex &previous);
	void _doubleClickOnItem();
private slots:
	//void onEngineInited(RENDER_MASTER::ICore *pCore);
	void onEngineClosed(RENDER_MASTER::ICore *pCore);

};

#endif // SCENETREEWIDGET_H
