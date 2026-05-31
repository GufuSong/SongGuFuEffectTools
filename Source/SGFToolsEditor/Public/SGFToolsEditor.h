// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"

class IAssetTools;
class IAssetTypeActions;

class FSGFToolsEditorModule : public IModuleInterface
{
public:

	// 行为：启动编辑器模块；作用：注册资产类型、详情面板扩展和内容浏览器菜单；输出：无返回值。
	virtual void StartupModule() override;

	// 行为：关闭编辑器模块；作用：注销菜单、资产类型行为和详情面板扩展；输出：无返回值。
	virtual void ShutdownModule() override;

private:

	// 行为：注册全部资产类型行为；作用：集中添加本插件支持的自定义资产类型；输出：无返回值。
	void RegisterAssetTypeActions();

	// 行为：注册单个资产类型行为；作用：把指定资产行为交给 AssetTools 并记录句柄；输出：无返回值。
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> AssetTypeAction);

	// 行为：注册内容浏览器菜单；作用：扩展材质和 SGF 材质实例的右键菜单项；输出：无返回值。
	void RegisterMenus();

	//保存已经注册的IAssetTypeActions ;
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
};
