// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGFToolsEditor.h"

#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "IAssetTools.h"
#include "Material/SGFToolsMaterialInstance.h"
#include "Material/SGFToolsMaterialInstanceAssetTypeActions.h"
#include "Material/SGFToolsMaterialInstanceConversion.h"
#include "Material/SGFToolsMaterialInstanceFactory.h"
#include "Material/SGFToolsMaterialInstanceHierarchyDetails.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Styling/AppStyle.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FSGFToolsEditorModule"

namespace
{
	// 行为：执行创建 SGF 材质实例命令；作用：从当前选中的材质资产批量创建自定义材质实例；输出：无返回值，创建成功时同步内容浏览器选中项。
	void ExecuteCreateSGFToolsMaterialInstance(const FToolMenuContext& MenuContext)
	{
		const UContentBrowserAssetContextMenuContext* ContentBrowserContext = UContentBrowserAssetContextMenuContext::FindContextWithAssets(MenuContext);
		if (!ContentBrowserContext)
		{
			return;
		}

		// 基于选中的 Material / Material Instance 创建资产：这里加载选中资产，是因为执行命令时才真正需要读取 Parent。
		TArray<UMaterialInterface*> SourceMaterials = ContentBrowserContext->LoadSelectedObjects<UMaterialInterface>();
		if (SourceMaterials.IsEmpty())
		{
			return;
		}

		IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
		TArray<UObject*> CreatedAssets;

		for (UMaterialInterface* SourceMaterial : SourceMaterials)
		{
			if (!SourceMaterial)
			{
				continue;
			}

			const FString SourcePackageName = SourceMaterial->GetOutermost()->GetName();
			const FString SourcePackagePath = FPackageName::GetLongPackagePath(SourcePackageName);
			const FString BasePackageName = FString::Printf(TEXT("%s/MI_SGF_%s"), *SourcePackagePath, *SourceMaterial->GetName());

			FString UniquePackageName;
			FString UniqueAssetName;
			AssetTools.CreateUniqueAssetName(BasePackageName, TEXT(""), UniquePackageName, UniqueAssetName);

			USGFToolsMaterialInstanceFactory* Factory = NewObject<USGFToolsMaterialInstanceFactory>();
			Factory->InitialParent = SourceMaterial;

			UObject* CreatedAsset = AssetTools.CreateAsset(
				UniqueAssetName,
				FPackageName::GetLongPackagePath(UniquePackageName),
				USGFToolsMaterialInstance::StaticClass(),
				Factory
			);

			if (CreatedAsset)
			{
				CreatedAssets.Add(CreatedAsset);
			}
		}

		if (!CreatedAssets.IsEmpty())
		{
			AssetTools.SyncBrowserToAssets(CreatedAssets);
		}
	}

	// 行为：执行转换为原生材质实例命令；作用：把当前选中的 SGF 材质实例交给转换流程处理；输出：无返回值。
	void ExecuteConvertSGFToolsMaterialInstanceToNative(const FToolMenuContext& MenuContext)
	{
		const UContentBrowserAssetContextMenuContext* ContentBrowserContext = UContentBrowserAssetContextMenuContext::FindContextWithAssets(MenuContext);
		if (!ContentBrowserContext)
		{
			return;
		}

		FSGFToolsMaterialInstanceConversion::ConvertSelectionToNativeMaterialInstance(*ContentBrowserContext);
	}
}

// 行为：启动编辑器模块；作用：注册资产类型、层级详情扩展和内容浏览器菜单回调；输出：无返回值。
void FSGFToolsEditorModule::StartupModule()
{
	// Editor 模块启动时注册资产显示/打开逻辑，以及 Content Browser 右键菜单扩展。
	RegisterAssetTypeActions();
	RegisterAssetTypeActions();
	SGFTools::MaterialInstanceHierarchyDetails::Startup();
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSGFToolsEditorModule::RegisterMenus));
}

// 行为：关闭编辑器模块；作用：注销菜单回调、层级详情扩展和已注册资产类型行为；输出：无返回值。
void FSGFToolsEditorModule::ShutdownModule()
{
	// 关闭或热重载时清理注册项，避免菜单和 AssetTypeActions 留下失效指针。
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	SGFTools::MaterialInstanceHierarchyDetails::Shutdown();

	if (FAssetToolsModule::IsModuleLoaded())
	{
		IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
		for (const TSharedRef<IAssetTypeActions>& AssetTypeAction : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(AssetTypeAction);
		}
	}

	RegisteredAssetTypeActions.Empty();
}

// 行为：注册全部资产类型行为；作用：让 AssetTools 识别 SGF 材质实例资产；输出：无返回值。
void FSGFToolsEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	RegisterAssetTypeAction(AssetTools, MakeShared<FSGFToolsMaterialInstanceAssetTypeActions>());
}

// 行为：注册单个资产类型行为；作用：向 AssetTools 注册行为并保存到模块生命周期列表；输出：无返回值。
void FSGFToolsEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> AssetTypeAction)
{
	AssetTools.RegisterAssetTypeActions(AssetTypeAction);
	RegisteredAssetTypeActions.Add(AssetTypeAction);
}

// 行为：注册内容浏览器菜单；作用：为材质和 SGF 材质实例添加创建、转换菜单入口；输出：无返回值。
void FSGFToolsEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// 右键已有 Material / Material Instance 时显示此菜单；空白处新建则由 USGFToolsMaterialInstanceFactory 提供。
	UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UMaterialInterface::StaticClass());
	FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");

	// 行为：构建创建菜单动态入口；作用：向材质右键菜单添加创建 SGF 材质实例命令；输出：无返回值。
	Section.AddDynamicEntry("SGFTools_CreateMaterialInstance_Dynamic", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		const TAttribute<FText> Label = LOCTEXT("CreateSGFToolsMaterialInstance", "创建 SGF Material Instance");
		const TAttribute<FText> ToolTip = LOCTEXT("CreateSGFToolsMaterialInstanceTooltip", "创建一个 SGFTools 自定义 Material Instance，并把当前选中的材质设为 Parent。");
		const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.MaterialInstanceActor");
		const FToolMenuExecuteAction UIAction = FToolMenuExecuteAction::CreateStatic(&ExecuteCreateSGFToolsMaterialInstance);

		InSection.AddMenuEntry("SGFTools_CreateMaterialInstance", Label, ToolTip, Icon, UIAction);
	}));

	UToolMenu* SGFToolsMaterialInstanceMenu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(USGFToolsMaterialInstance::StaticClass());
	FToolMenuSection& SGFToolsMaterialInstanceSection = SGFToolsMaterialInstanceMenu->FindOrAddSection("GetAssetActions");

	// 行为：构建转换菜单动态入口；作用：在可转换的 SGF 材质实例右键菜单中添加转换命令；输出：无返回值。
	SGFToolsMaterialInstanceSection.AddDynamicEntry("SGFTools_ConvertMaterialInstance_Dynamic", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		const UContentBrowserAssetContextMenuContext* ContentBrowserContext = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection);
		if (!ContentBrowserContext || !FSGFToolsMaterialInstanceConversion::CanConvertSelection(*ContentBrowserContext))
		{
			return;
		}

		const TAttribute<FText> Label = LOCTEXT("ConvertSGFToolsMaterialInstanceToNative", "转换为原生 Material Instance");
		const TAttribute<FText> ToolTip = LOCTEXT("ConvertSGFToolsMaterialInstanceToNativeTooltip", "创建一个普通原生 Material Instance 副本，并复制当前 SGF Material Instance 的 Parent 和参数覆盖。");
		const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.MaterialInstanceActor");
		const FToolMenuExecuteAction UIAction = FToolMenuExecuteAction::CreateStatic(&ExecuteConvertSGFToolsMaterialInstanceToNative);

		InSection.AddMenuEntry("SGFTools_ConvertMaterialInstanceToNative", Label, ToolTip, Icon, UIAction);
	}));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSGFToolsEditorModule, SGFToolsEditor)
