// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SGFToolsRuntime : ModuleRules
{
	// 行为：构造运行时模块规则；作用：配置运行时模块的 PCH 和依赖；输出：无返回值，生成可供 Unreal Build Tool 使用的模块规则对象。
	public SGFToolsRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
