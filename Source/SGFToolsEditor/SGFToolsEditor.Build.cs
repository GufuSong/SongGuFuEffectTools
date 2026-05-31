// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SGFToolsEditor : ModuleRules
{
	// 行为：构造编辑器模块规则；作用：配置编辑器模块的 PCH、公共依赖和私有依赖；输出：无返回值，生成可供 Unreal Build Tool 使用的模块规则对象。
	public SGFToolsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"ApplicationCore",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"ToolMenus",
				"AssetTools",
				"AssetRegistry",
				"ContentBrowser",
				"MaterialEditor",
				"PropertyEditor",
				"SGFToolsRuntime"
			}
		);
	}
}
