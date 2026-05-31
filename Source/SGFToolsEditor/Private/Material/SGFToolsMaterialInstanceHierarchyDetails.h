// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

class IMaterialEditor;
class USGFToolsMaterialInstance;

namespace SGFTools::MaterialInstanceHierarchyDetails
{
	extern const FName TabId;

	// 行为：启动层级详情扩展；作用：注册材质实例编辑器打开事件监听；输出：无返回值。
	void Startup();

	// 行为：关闭层级详情扩展；作用：移除事件监听并清理活动扩展对象；输出：无返回值。
	void Shutdown();

	// 行为：排队等待材质实例编辑器；作用：记录即将打开层级详情面板的 SGF 材质实例；输出：无返回值。
	void QueueEditorForMaterialInstance(USGFToolsMaterialInstance* MaterialInstance);

	// 行为：打开指定材质编辑器的层级详情面板；作用：找到或创建扩展并唤起自定义详情页签；输出：无返回值。
	void OpenForMaterialEditor(const TSharedRef<IMaterialEditor>& MaterialEditor);
}
