// Copyright Epic Games, Inc. All Rights Reserved.

#include "Material/SGFToolsMaterialInstanceFactory.h"

#include "AssetTypeCategories.h"
#include "Material/SGFToolsMaterialInstance.h"
#include "Materials/MaterialInterface.h"

#define LOCTEXT_NAMESPACE "SGFToolsMaterialInstanceFactory"

// 行为：构造 SGF 材质实例工厂；作用：声明工厂创建 USGFToolsMaterialInstance 并允许新建后编辑；输出：无返回值，初始化后的工厂对象。
USGFToolsMaterialInstanceFactory::USGFToolsMaterialInstanceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = USGFToolsMaterialInstance::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

// 行为：创建新资产；作用：实例化 SGF 材质实例并把可选初始材质设为 Parent；输出：新建的材质实例对象指针。
UObject* USGFToolsMaterialInstanceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USGFToolsMaterialInstance* MaterialInstance = NewObject<USGFToolsMaterialInstance>(InParent, InClass, InName, Flags);

	if (MaterialInstance)
	{
		if (InitialParent)
		{
			MaterialInstance->SetParentEditorOnly(InitialParent);
		}
	}

	return MaterialInstance;
}

// 行为：获取工厂显示名；作用：提供新建资产菜单中显示的 SGF 材质实例名称；输出：本地化文本。
FText USGFToolsMaterialInstanceFactory::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "SGF Material Instance");
}

// 行为：获取新建菜单分类；作用：把 SGF 材质实例放入材质分类；输出：材质分类位掩码。
uint32 USGFToolsMaterialInstanceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Materials;
}

// 行为：获取默认资产名；作用：为内容浏览器新建资产提供初始命名；输出：默认资产名字符串。
FString USGFToolsMaterialInstanceFactory::GetDefaultNewAssetName() const
{
	return TEXT("MI_SGF_MaterialInstance");
}

#undef LOCTEXT_NAMESPACE
