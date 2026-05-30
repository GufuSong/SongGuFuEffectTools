// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

class IMaterialEditor;
class USGFToolsMaterialInstance;

namespace SGFTools::MaterialInstanceHierarchyDetails
{
	extern const FName TabId;

	void Startup();
	void Shutdown();

	void QueueEditorForMaterialInstance(USGFToolsMaterialInstance* MaterialInstance);
	void OpenForMaterialEditor(const TSharedRef<IMaterialEditor>& MaterialEditor);
}
