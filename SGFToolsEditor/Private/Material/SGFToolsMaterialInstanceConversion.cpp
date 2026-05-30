// Copyright Epic Games, Inc. All Rights Reserved.

#include "Material/SGFToolsMaterialInstanceConversion.h"

#include "AssetRegistry/AssetData.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserMenuContexts.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "Material/SGFToolsMaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"

#define LOCTEXT_NAMESPACE "SGFToolsMaterialInstanceConversion"

namespace
{
	FString MakeDefaultNativeMaterialInstanceName(const FString& SourceAssetName)
	{
		const FString SGFPrefix = TEXT("MI_SGF_");
		if (SourceAssetName.StartsWith(SGFPrefix))
		{
			return FString::Printf(TEXT("MI_%s"), *SourceAssetName.RightChop(SGFPrefix.Len()));
		}

		return FString::Printf(TEXT("%s_Native"), *SourceAssetName);
	}

	void CopyMaterialInstanceSettings(const UMaterialInstanceConstant* SourceInstance, UMaterialInstanceConstant* TargetInstance)
	{
		if (!SourceInstance || !TargetInstance)
		{
			return;
		}

		TargetInstance->PhysMaterial = SourceInstance->PhysMaterial;
		for (int32 Index = 0; Index < EPhysicalMaterialMaskColor::MAX; ++Index)
		{
			TargetInstance->PhysicalMaterialMap[Index] = SourceInstance->PhysicalMaterialMap[Index];
		}
		TargetInstance->PhysMaterialMask = SourceInstance->PhysMaterialMask;

		TargetInstance->SubsurfaceProfile = SourceInstance->SubsurfaceProfile;
		TargetInstance->SubsurfaceProfiles = SourceInstance->SubsurfaceProfiles;
		TargetInstance->SpecularProfiles = SourceInstance->SpecularProfiles;
		TargetInstance->NeuralProfile = SourceInstance->NeuralProfile;
		TargetInstance->SpecularProfileOverride = SourceInstance->SpecularProfileOverride;

		TargetInstance->NaniteOverrideMaterial = SourceInstance->NaniteOverrideMaterial;
		TargetInstance->bOverrideSubsurfaceProfile = SourceInstance->bOverrideSubsurfaceProfile;
		TargetInstance->bOverrideSpecularProfile = SourceInstance->bOverrideSpecularProfile;
		TargetInstance->bOverrideBlendableLocation = SourceInstance->bOverrideBlendableLocation;
		TargetInstance->bOverrideBlendablePriority = SourceInstance->bOverrideBlendablePriority;
		TargetInstance->BlendableLocationOverride = SourceInstance->BlendableLocationOverride;
		TargetInstance->BlendablePriorityOverride = SourceInstance->BlendablePriorityOverride;

		TargetInstance->ScalarParameterValues = SourceInstance->ScalarParameterValues;
		TargetInstance->VectorParameterValues = SourceInstance->VectorParameterValues;
		TargetInstance->DoubleVectorParameterValues = SourceInstance->DoubleVectorParameterValues;
		TargetInstance->TextureParameterValues = SourceInstance->TextureParameterValues;
		TargetInstance->TextureCollectionParameterValues = SourceInstance->TextureCollectionParameterValues;
		TargetInstance->ParameterCollectionParameterValues = SourceInstance->ParameterCollectionParameterValues;
		TargetInstance->RuntimeVirtualTextureParameterValues = SourceInstance->RuntimeVirtualTextureParameterValues;
		TargetInstance->SparseVolumeTextureParameterValues = SourceInstance->SparseVolumeTextureParameterValues;
		TargetInstance->FontParameterValues = SourceInstance->FontParameterValues;
		TargetInstance->UserSceneTextureOverrides = SourceInstance->UserSceneTextureOverrides;

		FMaterialInstanceBasePropertyOverrides BasePropertyOverrides = SourceInstance->BasePropertyOverrides;
		TargetInstance->UpdateStaticPermutation(SourceInstance->GetStaticParameters(), BasePropertyOverrides, true);
	}

	USGFToolsMaterialInstance* LoadSingleSGFToolsMaterialInstance(const UContentBrowserAssetContextMenuContext& ContentBrowserContext)
	{
		const FAssetData* SelectedAsset = ContentBrowserContext.GetSingleSelectedAssetOfType(USGFToolsMaterialInstance::StaticClass());
		if (!SelectedAsset)
		{
			return nullptr;
		}

		return Cast<USGFToolsMaterialInstance>(SelectedAsset->GetAsset());
	}

	void ShowConversionError(const FText& Message)
	{
		FMessageDialog::Open(EAppMsgType::Ok, Message, LOCTEXT("ConversionErrorTitle", "Convert SGF Material Instance"));
	}
}

bool FSGFToolsMaterialInstanceConversion::CanConvertSelection(const UContentBrowserAssetContextMenuContext& ContentBrowserContext)
{
	return ContentBrowserContext.GetSingleSelectedAssetOfType(USGFToolsMaterialInstance::StaticClass()) != nullptr;
}

void FSGFToolsMaterialInstanceConversion::ConvertSelectionToNativeMaterialInstance(const UContentBrowserAssetContextMenuContext& ContentBrowserContext)
{
	USGFToolsMaterialInstance* SourceInstance = LoadSingleSGFToolsMaterialInstance(ContentBrowserContext);
	if (!SourceInstance)
	{
		return;
	}

	UMaterialInterface* SourceParent = SourceInstance->Parent;
	if (!SourceParent)
	{
		ShowConversionError(LOCTEXT("MissingParent", "The selected SGF Material Instance has no parent material."));
		return;
	}

	const FString SourcePackageName = SourceInstance->GetOutermost()->GetName();
	const FString DefaultPath = FPackageName::GetLongPackagePath(SourcePackageName);
	const FString DefaultAssetName = MakeDefaultNativeMaterialInstanceName(SourceInstance->GetName());

	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SaveNativeMaterialInstanceDialogTitle", "Save Native Material Instance As");
	SaveAssetDialogConfig.DefaultPath = DefaultPath;
	SaveAssetDialogConfig.DefaultAssetName = DefaultAssetName;
	SaveAssetDialogConfig.AssetClassNames.Add(UMaterialInstanceConstant::StaticClass()->GetClassPathName());
	SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
	if (SaveObjectPath.IsEmpty())
	{
		return;
	}

	const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
	const FString SavePackagePath = FPackageName::GetLongPackagePath(SavePackageName);
	const FString SaveAssetName = FPackageName::GetLongPackageAssetName(SavePackageName);

	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = SourceParent;

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	UMaterialInstanceConstant* NativeInstance = Cast<UMaterialInstanceConstant>(
		AssetTools.CreateAsset(
			SaveAssetName,
			SavePackagePath,
			UMaterialInstanceConstant::StaticClass(),
			Factory
		)
	);

	if (!NativeInstance)
	{
		ShowConversionError(LOCTEXT("CreateNativeMaterialInstanceFailed", "Failed to create the native Material Instance."));
		return;
	}

	NativeInstance->PreEditChange(nullptr);
	CopyMaterialInstanceSettings(SourceInstance, NativeInstance);
	NativeInstance->PostEditChange();
	NativeInstance->MarkPackageDirty();

	TArray<UObject*> CreatedAssets;
	CreatedAssets.Add(NativeInstance);
	AssetTools.SyncBrowserToAssets(CreatedAssets);
}

#undef LOCTEXT_NAMESPACE
