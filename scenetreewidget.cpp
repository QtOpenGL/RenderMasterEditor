#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include "scenetreewidget.h"
#include "EngineGlobal.h"
#include "editorglobal.h"
#include "ui_scenetreewidget.h"
#include <unordered_set>

using namespace RENDER_MASTER;
using namespace std;

extern EngineGlobal* eng;
extern EditorGlobal *editor;


RenderMasterSceneManagerAdapter::RenderMasterSceneManagerAdapter(QObject *parent)  : QAbstractItemModel( parent )
{
	connect(eng, &EngineGlobal::EngineInited, this, &RenderMasterSceneManagerAdapter::onEngineInited, Qt::DirectConnection);
	connect(eng, &EngineGlobal::EngineBeforeClose, this, &RenderMasterSceneManagerAdapter::onEngineClosed, Qt::DirectConnection);
}

int RenderMasterSceneManagerAdapter::rowCount(const QModelIndex &parent) const
{
	if (!sm) return 0;

	if( parent.isValid() )
	{
		return 0;
	}

	IGameObjectEvent *pGameObjectEvent;
	sm->GetGameObjectAddedEvent(&pGameObjectEvent);
	pGameObjectEvent->Subscribe((IGameObjectEventSubscriber*)this);

	uint rootGameObjects;
	sm->GetChilds(&rootGameObjects, nullptr);

	return rootGameObjects;
}

int RenderMasterSceneManagerAdapter::columnCount(const QModelIndex &parent ) const
{
	return 1;
}

QModelIndex RenderMasterSceneManagerAdapter::index(int row, int column, const QModelIndex &parent ) const
{
	if (!sm) return QModelIndex();

//  if( !parent.isValid() )
//	parentObject = m_root;
//  else
//	parentObject = static_cast<QObject*>( parent.internalPointer() );

	if( parent.isValid() )
	{
		return QModelIndex();
	}

	uint rootGameObjects;
	sm->GetChilds(&rootGameObjects, nullptr);

	if( 0 <= row && row < rootGameObjects )
	{
		IGameObject *go{nullptr};
		sm->GetChild(&go, nullptr, row);
		return createIndex( row, column, go );
	}
	else
		return QModelIndex();
}

QModelIndex RenderMasterSceneManagerAdapter::parent(const QModelIndex &index) const
{
  //if( !index.isValid() )
	return QModelIndex();

//  QObject *indexObject = static_cast<QObject*>( index.internalPointer() );
//  QObject *parentObject = indexObject->parent();

//  if( parentObject == m_root )
//	return QModelIndex();

//  QObject *grandParentObject = parentObject->parent();

//  return createIndex( grandParentObject->children().indexOf( parentObject ), 0, parentObject );
}

QVariant RenderMasterSceneManagerAdapter::data( const QModelIndex &index, int role) const
{
  if( !index.isValid() )
	return QVariant();

  if( role == Qt::DisplayRole )
  {
	switch( index.column() )
	{
	case 0:
	{
		IGameObject *go = static_cast<IGameObject*>( index.internalPointer() );
		const char *name;
		go->GetName(&name);
		return QVariant(name);
	}
	break;

//    case 1:
//      return static_cast<QObject*>( index.internalPointer() )->metaObject()->className();

	default:
	  break;
	}
  }
  else if( role == Qt::ToolTipRole )
  {
	switch( index.column() )
	{
	case 0:
	  return QString( "The name of the object." );

//    case 1:
//      return QString( "The name of the class." );

	default:
	  break;
	}
  }

  else if (role == Qt::DecorationRole)
  {
		return QIcon(":/icons/cube.png");
  }

  return QVariant();
}
bool RenderMasterSceneManagerAdapter::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole && value.toString() != "")
	{
		IGameObject *go = static_cast<IGameObject*>( index.internalPointer() );
		go->SetName( value.toString().toUtf8().data() );
		emit dataChanged(index, index);
		return true;
	}
	return false;
}



////////////////////////////////////
// DRAG & DROP
////////////////////////////////////

Qt::ItemFlags RenderMasterSceneManagerAdapter::flags(const QModelIndex &index) const
{
	 Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

	 if (index.isValid())
		 return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | defaultFlags;
	 else
		 return Qt::ItemIsDropEnabled |  Qt::ItemIsEditable | defaultFlags;
}

QMimeData* RenderMasterSceneManagerAdapter::mimeData(const QModelIndexList &indexes) const
{
	Q_ASSERT(false); // not impl
	return nullptr;

//	QMimeData *mimeDataPtr = new QMimeData();
//	QByteArray encodedData;
//	QDataStream stream(&encodedData, QIODevice::WriteOnly);

//	qDebug() << "drag ";

//	foreach (QModelIndex index, indexes)
//	{
//		if (index.isValid())
//		{
//		   stream << qint64(index.internalPointer());
//		}
//	}

//	mimeDataPtr->setData("application/vnd.text.list", encodedData);
//	return mimeDataPtr;
}

QStringList RenderMasterSceneManagerAdapter::mimeTypes() const
 {
	 QStringList types;
	 types << "application/vnd.text.list";
	 return types;
 }

void print(std::unordered_set<QObject*>& objs)
{
	QString str;
	foreach(QObject* q, objs)
	{
		str += q->objectName();
		str += " ";
	}
	qDebug() << "decoded data: " << str;
}

void decode_objs(const QMimeData* data, std::unordered_set<QObject*>& objs)
{
	QByteArray encodedData = data->data("application/vnd.text.list");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	while (!stream.atEnd())
	{
		qint64 pointer;
		stream >> pointer;
		QObject *obj = reinterpret_cast<QObject*>(pointer);
		objs.emplace(obj);
	}
}

void add_childs(std::unordered_set<QObject*>& objs)
{
	for (QObject *c : objs)
	{
		for (QObject *cc : c->children())
			objs.emplace(cc);
	}
}

bool RenderMasterSceneManagerAdapter::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
//	if (action == Qt::IgnoreAction) {
//		return true;
//	}

//	int position;
//	QObject *parent_obj = m_root;

//	if (row != -1) {
//		position = row;
//	} else if (parent.isValid()) {
//		position = parent.row();
//		parent_obj = static_cast<QObject*>( parent.internalPointer() );
//	} else {
//		position = rowCount(QModelIndex());
//	}

//	{
//		std::unordered_set<QObject*> drop_objs;

//		decode_objs(data, drop_objs);

//		foreach(QObject* q, drop_objs)
//		{
//			q->setParent(parent_obj);
//		}

//		emit layoutChanged();

//		return true;
//	}

	Q_ASSERT(false); // not impl
	return false;
}

bool RenderMasterSceneManagerAdapter::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(parent);

	if (!data->hasFormat("application/vnd.text.list"))
		return false;

	if (column > 0)
		return false;

	if (parent.isValid())
	{
		QObject *parent_obj = static_cast<QObject*>( parent.internalPointer() );
		qDebug() << "canDropMimeData at=" << parent_obj->objectName() << " row=" << row << " column=" << column;

		std::unordered_set<QObject*> drop_objs;

		decode_objs(data, drop_objs);
		add_childs(drop_objs);
		print(drop_objs);

		if (drop_objs.find(parent_obj) != drop_objs.end())
			return false;
	}
	else
	{
		qDebug() << "canDropMimeData at=null row=" << row << " column=" << column;
	}

	return true;

}

void RenderMasterSceneManagerAdapter::onEngineInited(RENDER_MASTER::ICore *pCore)
{
	pCore->GetSubSystem((ISubSystem**)&sm, SUBSYSTEM_TYPE::SCENE_MANAGER);

//	for (int i = 0; i< rootGameObjects; i++)
//	{
//		IGameObject *go{nullptr};
//		sm->GetRootGameObject(&go, i);
//		_add_game_object(go, 1, nullptr);
//	}

	// dbg set first gameobject
	//if (go_number)
	//{
	//    IGameObject *go{nullptr};
	//    sm->GetGameObject(&go, 0);
	//    editor->propertiesWidget->SetGameObject(go);
	//}

	emit layoutChanged();
}

void RenderMasterSceneManagerAdapter::onEngineClosed(RENDER_MASTER::ICore *pCore)
{
	pCore = nullptr;
	sm = nullptr;
}

API RenderMasterSceneManagerAdapter::Call(IGameObject *pGameObject)
{
	emit layoutChanged();
	return S_OK;
}


///////////////////////////////
// SceneTreeWidget
///////////////////////////////

SceneTreeWidget::SceneTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneTreeWidget)
{
    ui->setupUi(this);

	ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->treeView->setDragEnabled(true);
	ui->treeView->setAcceptDrops(true);
	ui->treeView->setDropIndicatorShown(true);

	_model = new RenderMasterSceneManagerAdapter(nullptr);
	ui->treeView->setModel(_model);

	auto *selectionModel = ui->treeView->selectionModel();
	connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(_selectionChanged(const QItemSelection&,const QItemSelection&)));
	connect(selectionModel, SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(_currentChanged(const QModelIndex &, const QModelIndex &)));
}

SceneTreeWidget::~SceneTreeWidget()
{
	delete ui;
}

void SceneTreeWidget::_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	if (selected.indexes().size())
	{
		qDebug() << "selected" << selected.size() << "deselected" << deselected.size();

		const QModelIndex& index = selected.indexes().at(0);
		if (index.isValid())
		{
			IGameObject *go = static_cast<IGameObject*>( index.internalPointer() );
			if (go)
			{
				selectionChanged(go);
			}
		}
		//else
		//{
			//IGameObject *go = static_cast<IGameObject*>( index.internalPointer() );
		//	selectionChanged(go);
		//}


	}else
	{
		selectionChanged(nullptr);
		auto *selectionModel = ui->treeView->selectionModel();
		selectionModel->clear();
	}
}

void SceneTreeWidget::_currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (!current.isValid())
	{
		selectionChanged(nullptr);
		auto *selectionModel = ui->treeView->selectionModel();
		selectionModel->clear();
	}else
	{
		IGameObject *go = static_cast<IGameObject*>( current.internalPointer() );
		selectionChanged(go);
	}
}

//void SceneTreeWidget::_add_game_object(IGameObject *go, int root, IGameObject *parent)
//{
//	Q_ASSERT(parent == nullptr); // other - not implemented

//	const char *pName{nullptr};
//	go->GetName(&pName);

//	QIcon iC(":/icons/cube.png");

//	QTreeWidgetItem *it;
//	it = new QTreeWidgetItem();
//	it->setText(0, pName);
//	it->setIcon(0, iC);
//	it->setExpanded(true);

//	ui->treeWidget->insertTopLevelItem(0, it);
//}




