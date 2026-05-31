// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSGFToolsRuntimeModule : public IModuleInterface
{
public:
	// 行为：启动运行时模块；作用：执行模块加载时的初始化入口；输出：无返回值。
	virtual void StartupModule() override;

	// 行为：关闭运行时模块；作用：执行模块卸载时的清理入口；输出：无返回值。
	virtual void ShutdownModule() override;
};
