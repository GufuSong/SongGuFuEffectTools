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

	//启动时调用,用于注册内容.
	virtual void StartupModule() override;

	////清理和卸载时调用,用于注册内容
	virtual void ShutdownModule() override;

private:

	//统一注册本插件支持的资产类型
	void RegisterAssetTypeActions();

	//注册单个资产类型行为
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> AssetTypeAction);

	//注册Content Browser菜单
	void RegisterMenus();

	//保存已经注册的IAssetTypeActions ;
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
};
