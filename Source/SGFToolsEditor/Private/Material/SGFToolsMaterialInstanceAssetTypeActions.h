// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AssetTypeActions_Base.h"

class FSGFToolsMaterialInstanceAssetTypeActions : public FAssetTypeActions_Base
{
public:
	// 行为：获取资产类型名称；作用：提供内容浏览器显示的资产名称；输出：本地化文本。
	virtual FText GetName() const override;

	// 行为：获取资产类型颜色；作用：定义内容浏览器中该资产类型的标识颜色；输出：颜色值。
	virtual FColor GetTypeColor() const override;

	// 行为：获取支持的资产类；作用：声明此资产行为绑定到 SGF 材质实例类；输出：UClass 指针。
	virtual UClass* GetSupportedClass() const override;

	// 行为：获取资产分类；作用：把 SGF 材质实例归入材质分类；输出：资产分类位掩码。
	virtual uint32 GetCategories() override;

	// 行为：打开资产编辑器；作用：使用材质实例编辑器打开选中的 SGF 材质实例并启用层级详情面板；输出：无返回值。
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
};
