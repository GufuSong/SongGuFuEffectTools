// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGFToolsRuntime.h"

#define LOCTEXT_NAMESPACE "FSGFToolsRuntimeModule"

// 行为：启动运行时模块；作用：保留运行时模块加载时的初始化入口；输出：无返回值。
void FSGFToolsRuntimeModule::StartupModule()
{
}

// 行为：关闭运行时模块；作用：保留运行时模块卸载时的清理入口；输出：无返回值。
void FSGFToolsRuntimeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSGFToolsRuntimeModule, SGFToolsRuntime)
