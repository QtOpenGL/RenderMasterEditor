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

void PropertiesWidget::SetGameObject(IGameObject *go)
{
	unsubscribeFromPreviousGO();

	_go = go;

	if (_go)
	{
		subscribeToGO(_go);

		ui->lineEdit->setEnabled(true);
		ui->groupBox->setEnabled(true);

		const char *name;
		_go->GetName(&name);
		ui->lineEdit->setText(name);

		vec3 pos;
		_go->GetPosition(&pos);
		ui->pos_x_sb->setValue((double)pos.x);
		ui->pos_y_sb->setValue((double)pos.y);
		ui->pos_z_sb->setValue((double)pos.z);

		quat rot;
		vec3 eulerAngles = rot.ToEuler();
		_go->GetRotation(&rot);
		ui->rot_x_sb->setValue((double)eulerAngles.x);
		ui->rot_y_sb->setValue((double)eulerAngles.y);
		ui->rot_z_sb->setValue((double)eulerAngles.z);

		vec3 scale;
		_go->GetScale(&scale);
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

API PropertiesWidget::Call(quat *rot)
{
	if (_go && !block)
	{
		vec3 eulerAngles = rot->ToEuler();
		ui->rot_x_sb->setValue((double)eulerAngles.x);
		ui->rot_y_sb->setValue((double)eulerAngles.y);
		ui->rot_z_sb->setValue((double)eulerAngles.z);
	}
	return S_OK;
}

void PropertiesWidget::onSceneTreeInited(SceneTreeWidget *sceneTree)
{
	// selection changed
	connect(editor, &EditorGlobal::selectionChanged, this, &PropertiesWidget::onSelectionChanged, Qt::DirectConnection);
}

void PropertiesWidget::onSelectionChanged(const std::vector<IGameObject*>& selectedGameObjects)
{
	qDebug() << "PropertiesWidget::onSelectionChanged";
	if (!editor->IsSomeObjectSelected() || editor->GetNumberSelectedObjects() > 1)
		SetGameObject(nullptr);
	else
	{
		SetGameObject(selectedGameObjects.at(0));
	}
}

void PropertiesWidget::unsubscribeFromPreviousGO()
{
	if (_go)
	{
		IPositionEvent *ev;
		_go->GetPositionEv(&ev);
		ev->Unsubscribe(dynamic_cast<IPositionEventSubscriber*>(this));

		IRotationEvent *rotEv;
		_go->GetRotationEv(&rotEv);
		rotEv->Unsubscribe(dynamic_cast<IRotationEventSubscriber*>(this));

		for (auto& conn : _connections)
			QObject::disconnect(conn);

		_connections.clear();

		_go->Release();
	}
}

void PropertiesWidget::subscribeToGO(IGameObject *go)
{
	go->AddRef();

	IPositionEvent *ev;
	go->GetPositionEv(&ev);
	ev->Subscribe(dynamic_cast<IPositionEventSubscriber*>(this));

	IRotationEvent *rotEv;
	go->GetRotationEv(&rotEv);
	rotEv->Subscribe(dynamic_cast<IRotationEventSubscriber*>(this));

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

	connectRotation(ui->rot_x_sb, 0);
	connectRotation(ui->rot_y_sb, 1);
	connectRotation(ui->rot_z_sb, 2);
}

void PropertiesWidget::connectPosition(MySpinBox *w, int xyz_offset)
{
	auto conn = connect(w, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), [=](const QString &newValue)
	{
		if (_go)
		{
			vec3 pos;
			_go->GetPosition(&pos);
			float newValueFloat;
			if (getFloatSpinbox(newValue, newValueFloat))
				if (!Approximately(pos.x, newValueFloat))
				{
					pos.xyz[xyz_offset] = newValueFloat;
					_go->SetPosition(&pos);
				}
		}
	});
	_connections.emplace_back(conn);
}

void PropertiesWidget::fmod360(MySpinBox *w)
{
	signalBlocked = true;
	float f = w->value();
	float f1 = fmod(f, 360.0f);
	if (f != f1)
		w->setValue(f1);
	signalBlocked = false;
}

void PropertiesWidget::connectRotation(MySpinBox *w, int xyz_offset)
{
	auto conn = connect(w, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), [&](const QString &newValueStr)
	{
		if (_go && !signalBlocked)
		{
			quat rot;
			_go->GetRotation(&rot);
			float newValueFloat;
			if (getFloatSpinbox(newValueStr, newValueFloat))
			{
				// quat -> euler
				//vec3 euler = rot.ToEuler();

				//qDebug() << "X=" << euler.x << " Y=" << euler.y;
				//euler.xyz[xyz_offset] = newValueFloat;

				//fmod360(w);

				float x = ui->rot_x_sb->value();
				float y = ui->rot_y_sb->value();
				float z = ui->rot_z_sb->value();

				vec3 euler = {x, y, z};

				// euler -> quat
				quat newRot = quat(euler);
				block = true;
				_go->SetRotation(&newRot);
				block = false;
			}
		}
	});
	_connections.emplace_back(conn);
}
