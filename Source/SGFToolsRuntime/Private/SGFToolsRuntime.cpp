// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGFToolsRuntime.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FSGFToolsRuntimeModule"

// 行为：启动运行时模块；作用：保留运行时模块加载时的初始化入口；输出：无返回值。
void FSGFToolsRuntimeModule::StartupModule()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SGFTools"));
	if (Plugin.IsValid())
	{
		const FString ShaderDirectory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/SGFTools"), ShaderDirectory);
	}
}

// 行为：关闭运行时模块；作用：保留运行时模块卸载时的清理入口；输出：无返回值。
void FSGFToolsRuntimeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSGFToolsRuntimeModule, SGFToolsRuntime)
