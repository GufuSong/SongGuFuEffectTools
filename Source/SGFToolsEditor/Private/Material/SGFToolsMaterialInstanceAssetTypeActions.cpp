// Copyright Epic Games, Inc. All Rights Reserved.

#include "Material/SGFToolsMaterialInstanceAssetTypeActions.h"

#include "AssetTypeCategories.h"
#include "IMaterialEditor.h"
#include "Material/SGFToolsMaterialInstance.h"
#include "Material/SGFToolsMaterialInstanceHierarchyDetails.h"
#include "MaterialEditorModule.h"

#define LOCTEXT_NAMESPACE "SGFToolsMaterialInstanceAssetTypeActions"

// 行为：获取资产类型名称；作用：提供内容浏览器显示的 SGF 材质实例名称；输出：本地化文本。
FText FSGFToolsMaterialInstanceAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "SGF Material Instance");
}

// 行为：获取资产类型颜色；作用：为 SGF 材质实例设置内容浏览器分类颜色；输出：绿色颜色值。
FColor FSGFToolsMaterialInstanceAssetTypeActions::GetTypeColor() const
{
	return FColor(0, 128, 0);
}

// 行为：获取支持的资产类；作用：让资产行为只作用于 USGFToolsMaterialInstance；输出：UClass 指针。
UClass* FSGFToolsMaterialInstanceAssetTypeActions::GetSupportedClass() const
{
	return USGFToolsMaterialInstance::StaticClass();
}

// 行为：获取资产分类；作用：将 SGF 材质实例放入材质资产分类；输出：材质分类位掩码。
uint32 FSGFToolsMaterialInstanceAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Materials;
}

// 行为：打开资产编辑器；作用：为每个选中的 SGF 材质实例创建材质实例编辑器并打开层级详情面板；输出：无返回值。
void FSGFToolsMaterialInstanceAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
	const EToolkitMode::Type ToolkitMode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		if (USGFToolsMaterialInstance* MaterialInstance = Cast<USGFToolsMaterialInstance>(Object))
		{
			SGFTools::MaterialInstanceHierarchyDetails::QueueEditorForMaterialInstance(MaterialInstance);
			TSharedRef<IMaterialEditor> MaterialEditor = MaterialEditorModule.CreateMaterialInstanceEditor(ToolkitMode, EditWithinLevelEditor, MaterialInstance);
			SGFTools::MaterialInstanceHierarchyDetails::OpenForMaterialEditor(MaterialEditor);
		}
	}
}

#undef LOCTEXT_NAMESPACE
