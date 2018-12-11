#include "editorglobal.h"
#include "manipulators/ManipulatorTranslator.h"
#include "EngineGlobal.h"

extern EngineGlobal* eng;

API EditorGlobal::Call(vec3 *pos)
{
	Q_UNUSED( pos )
	_selectionGameObject->GetModelMatrix(&_selectionCenterWorldTransform);
	return S_OK;
}

EditorGlobal::EditorGlobal()
{
}

void EditorGlobal::ChangeSelection(const std::vector<IGameObject *> &selectedGameObjects)
{
	if (selectedGameObjects.size() == 0)
	{
		_someObejctSelected = 0;
		_numberSelectedObjects = 0;

		if (_selectionGameObject)
		{
			IPositionEvent *ev;
			_selectionGameObject->GetPositionEv(&ev);
			ev->Unsubscribe(dynamic_cast<IPositionEventSubscriber*>(this));

			_selectionGameObject->Release();
			_selectionGameObject = nullptr;
		}

		selectionChanged(std::vector<IGameObject*>());
	} else
	{
		if (_selectionGameObject != selectedGameObjects[0])
		{
			if (_selectionGameObject)
			{
				IPositionEvent *ev;
				_selectionGameObject->GetPositionEv(&ev);
				ev->Unsubscribe(dynamic_cast<IPositionEventSubscriber*>(this));

				_selectionGameObject->Release();
				_selectionGameObject = nullptr;
			}

			_someObejctSelected = 1;
			_numberSelectedObjects = (int)selectedGameObjects.size();

			_selectionGameObject = selectedGameObjects[0];
			_selectionGameObject->AddRef();
			_selectionGameObject->GetModelMatrix(&_selectionCenterWorldTransform);

			IPositionEvent *ev;
			_selectionGameObject->GetPositionEv(&ev);
			ev->Subscribe(dynamic_cast<IPositionEventSubscriber*>(this));

			selectionChanged(selectedGameObjects);
		}
	}
}

void EditorGlobal::RaiseFocusOnSelevtedObjects()
{
	if (_someObejctSelected > 0)
	{
		vec3 worldCeneter;
		worldCeneter.x = _selectionCenterWorldTransform.el_2D[0][3];
		worldCeneter.y = _selectionCenterWorldTransform.el_2D[1][3];
		worldCeneter.z = _selectionCenterWorldTransform.el_2D[2][3];

		AABB aabb = {-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f};

		emit OnFocusAtSelectded(worldCeneter, aabb);
	}
}

void EditorGlobal::ToggleManipulator(MANIPULATOR manipulator)
{
	if (manipulator == _manipulatorType)
		return;

	_manipulatorType = manipulator;

	RENDER_MASTER::ICore *core;
	eng->GetCore(core);

	switch (manipulator)
	{
		case MANIPULATOR::SELECT: _manipulator = nullptr; break;
		case MANIPULATOR::TRANSLATE: _manipulator = std::unique_ptr<IManipulator>(new ManipulatorTranslator(core)); break;
	}
}

