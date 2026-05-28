// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SGFToolsEditor : ModuleRules
{
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
				"Slate",
				"SlateCore",
				"UnrealEd",
				"ToolMenus",
				"AssetTools",
				"AssetRegistry",
				"ContentBrowser",
				"SGFToolsRuntime"
			}
		);
	}
}
