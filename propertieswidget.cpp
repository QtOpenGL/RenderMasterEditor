#include "propertieswidget.h"
#include "editorglobal.h"
#include "scenetreewidget.h"
#include "ui_propertieswidget.h"
#include "QDoubleSpinBox"

using namespace RENDER_MASTER;

extern EditorGlobal *editor;

inline bool Approximately(float l, float r)
{
	const float eps = 0.000001f;
	return std::abs(l - r) < eps;
}

inline bool getFloatSpinbox(const QString& str, float &ret)
{
	bool ok;
	QLocale german(QLocale::German);
	ret = german.toFloat(str, &ok);
	return ok;
}


PropertiesWidget::PropertiesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertiesWidget)
{
    ui->setupUi(this);

	SetGameObject(nullptr);

	if (editor->sceneTreeWidget)
		onSceneTreeInited(editor->sceneTreeWidget);
	else
		connect(editor, &EditorGlobal::SceneTreeInited, this, &PropertiesWidget::onSceneTreeInited, Qt::DirectConnection);

}

PropertiesWidget::~PropertiesWidget()
{
    delete ui;
}

void PropertiesWidget::hideEvent(QHideEvent *event)
{

}

void PropertiesWidget::showEvent(QShowEvent *event)
{

}

void PropertiesWidget::SetGameObject(RENDER_MASTER::IGameObject *go)
{
	unsubscribeFromPreviousGO();

	_go = go;

	if (go)
	{
		subscribeToGO(go);

		ui->lineEdit->setEnabled(true);
		ui->groupBox->setEnabled(true);

		const char *name;
		go->GetName(&name);
		ui->lineEdit->setText(name);

		vec3 pos;
		go->GetPosition(&pos);
		ui->pos_x_sb->setValue((double)pos.x);
		ui->pos_y_sb->setValue((double)pos.y);
		ui->pos_z_sb->setValue((double)pos.z);

		quat rot;
		vec3 eulerAngles = rot.ToEuler();
		go->GetRotation(&rot);
		ui->rot_x_sb->setValue((double)eulerAngles.x);
		ui->rot_y_sb->setValue((double)eulerAngles.y);
		ui->rot_z_sb->setValue((double)eulerAngles.z);

		vec3 scale;
		go->GetScale(&scale);
		ui->scale_x_sb->setValue(scale.x);
		ui->scale_y_sb->setValue(scale.y);
		ui->scale_z_sb->setValue(scale.z);

	}else
	{
		ui->lineEdit->clear();
		ui->lineEdit->setEnabled(false);

		ui->pos_x_sb->clear();
		ui->pos_y_sb->clear();
		ui->pos_z_sb->clear();

		ui->rot_x_sb->clear();
		ui->rot_y_sb->clear();
		ui->rot_z_sb->clear();

		ui->scale_x_sb->clear();
		ui->scale_y_sb->clear();
		ui->scale_z_sb->clear();

		ui->groupBox->setEnabled(false);
	}
}

API PropertiesWidget::Call(vec3 *pPos)
{
    if (_go)
    {
        ui->pos_x_sb->setValue((double)pPos->x);
        ui->pos_y_sb->setValue((double)pPos->y);
        ui->pos_z_sb->setValue((double)pPos->z);
    }
	return S_OK;
}

void PropertiesWidget::onSceneTreeInited(SceneTreeWidget *sceneTree)
{
	// selection changed
	connect(sceneTree, &SceneTreeWidget::selectionChanged, this, &PropertiesWidget::onSelectionChanged, Qt::DirectConnection);	
}

void PropertiesWidget::onSelectionChanged(IGameObject *go)
{
	qDebug() << "PropertiesWidget::onSelectionChanged";
	SetGameObject(go);
}

void PropertiesWidget::unsubscribeFromPreviousGO()
{
	if (_go)
	{
		IPositionEvent *ev;
		_go->GetPositionEv(&ev);
		ev->Unsubscribe(this);

		for (auto& conn : _connections)
			QObject::disconnect(conn);

		_connections.clear();

		//ui->lineEdit->disconnect();

		//ui->pos_x_sb->disconnect();
		//ui->pos_y_sb->disconnect();
		//ui->pos_z_sb->disconnect();
	}
}

void PropertiesWidget::subscribeToGO(IGameObject *go)
{
	IPositionEvent *ev;
	go->GetPositionEv(&ev);
	ev->Subscribe(this);

	auto conn = connect(ui->lineEdit, &QLineEdit::editingFinished, [=]()
	{
		ui->lineEdit->clearFocus();
		 if (_go)
		 {
			 const QString newValue = ui->lineEdit->text();
			 _go->SetName(newValue.toUtf8().data());
		 }
	});
	_connections.emplace_back(conn);

	connectPosition(ui->pos_x_sb, 0);
	connectPosition(ui->pos_y_sb, 1);
	connectPosition(ui->pos_z_sb, 2);
}

void PropertiesWidget::connectPosition(MySpinBox *w, int xyz_offset)
{
	auto conn = connect(w, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), [=](const QString &newValue)
	{
		if (_go)
		{
			vec3 pos;
			_go->GetPosition(&pos);
			float new_x;
			if (getFloatSpinbox(newValue, new_x))
				if (!Approximately(pos.x, new_x))
				{
					pos.xyz[xyz_offset] = new_x;
					_go->SetPosition(&pos);
				}
		}
	});
	_connections.emplace_back(conn);
}
