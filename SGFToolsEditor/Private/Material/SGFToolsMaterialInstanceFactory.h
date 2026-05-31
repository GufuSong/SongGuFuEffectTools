// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"

#include "SGFToolsMaterialInstanceFactory.generated.h"

class UMaterialInterface;

/**
 * SGFTools 材质实例资产的编辑器创建工厂。
 *
 * Factory 属于 Editor-only：它只负责在内容浏览器里创建资产，不应该放进 Runtime。
 */
UCLASS()
class USGFToolsMaterialInstanceFactory : public UFactory
{
	GENERATED_BODY()

public:
	// 行为：构造 SGF 材质实例工厂；作用：初始化支持的资产类和新建后编辑标记；输出：无返回值，生成可创建 SGF 材质实例的工厂对象。
	USGFToolsMaterialInstanceFactory(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	TObjectPtr<UMaterialInterface> InitialParent;

	// 行为：创建新资产；作用：生成 SGF 材质实例并按需设置初始 Parent；输出：新建的 UObject 资产指针。
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	// 行为：获取工厂显示名；作用：提供新建资产菜单中的工厂名称；输出：本地化文本。
	virtual FText GetDisplayName() const override;

	// 行为：获取新建菜单分类；作用：把 SGF 材质实例工厂归入材质分类；输出：资产分类位掩码。
	virtual uint32 GetMenuCategories() const override;

	// 行为：获取默认资产名；作用：为新建 SGF 材质实例提供初始命名；输出：默认资产名字符串。
	virtual FString GetDefaultNewAssetName() const override;
};
