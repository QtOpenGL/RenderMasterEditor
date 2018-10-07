#include "editorglobal.h"


EditorGlobal::EditorGlobal()
{
}

void EditorGlobal::ChangeSelection(const std::vector<IResource *> &selectedGameObjects)
{
	if (selectedGameObjects.size() == 0)
	{
		_someObejctSelected = 0;
		_numberSelectedObjects = 0;
		selectionChanged(std::vector<IResource*>());
		return;
	}

	_someObejctSelected = 1;

	_numberSelectedObjects = selectedGameObjects.size();

	IGameObject *go;
	selectedGameObjects[0]->GetPointer((void**)&go);
	go->GetModelMatrix(&_selectionCenterWorldTransform);

	selectionChanged(selectedGameObjects);
}
