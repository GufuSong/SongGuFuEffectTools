// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

class UContentBrowserAssetContextMenuContext;

class FSGFToolsMaterialInstanceConversion
{
public:
	// 行为：检查选择是否可转换；作用：判断当前内容浏览器选择是否包含单个 SGF 材质实例；输出：是否允许转换的布尔值。
	static bool CanConvertSelection(const UContentBrowserAssetContextMenuContext& ContentBrowserContext);

	// 行为：转换选择资产；作用：把选中的 SGF 材质实例另存为原生材质实例并复制设置；输出：无返回值，成功时创建并选中新资产。
	static void ConvertSelectionToNativeMaterialInstance(const UContentBrowserAssetContextMenuContext& ContentBrowserContext);
};
