// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

class UContentBrowserAssetContextMenuContext;

class FSGFToolsMaterialInstanceConversion
{
public:
	static bool CanConvertSelection(const UContentBrowserAssetContextMenuContext& ContentBrowserContext);
	static void ConvertSelectionToNativeMaterialInstance(const UContentBrowserAssetContextMenuContext& ContentBrowserContext);
};
