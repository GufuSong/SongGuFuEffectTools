// Copyright Epic Games, Inc. All Rights Reserved.

#include "Material/SGFToolsMaterialInstanceHierarchyDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorSupportDelegates.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailGroup.h"
#include "IDetailCustomization.h"
#include "IDetailPropertyRow.h"
#include "IDetailsView.h"
#include "IMaterialEditor.h"
#include "IPropertyUtilities.h"
#include "Input/Reply.h"
#include "Material/SGFToolsMaterialInstance.h"
#include "MaterialEditor/DEditorParameterValue.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "MaterialEditorModule.h"
#include "MaterialPropertyHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/Optional.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/UnrealType.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SGFToolsMaterialInstanceHierarchyDetails"

namespace SGFTools::MaterialInstanceHierarchyDetails
{
	const FName TabId(TEXT("SGFTools_HierarchyDetails"));
}

namespace
{
	class FSGFToolsHierarchyDetailsExtension;

	struct FSGFToolsHierarchyDetailsState
	{
		TOptional<FName> SelectedGroup;
	};

	FDelegateHandle GMaterialInstanceEditorOpenedHandle;
	TArray<TWeakObjectPtr<USGFToolsMaterialInstance>> GPendingMaterialInstances;
	TArray<TSharedPtr<FSGFToolsHierarchyDetailsExtension>> GActiveExtensions;

	void GetShowHiddenParameters(bool& bShowHiddenParameters)
	{
		bShowHiddenParameters = false;
	}

	bool IsGlobalParameterGroup(const FEditorParameterGroup& ParameterGroup)
	{
		return ParameterGroup.GroupAssociation == EMaterialParameterAssociation::GlobalParameter
			&& ParameterGroup.GroupName != FMaterialPropertyHelpers::LayerParamName;
	}

	bool IsParameterVisibleInHierarchy(UMaterialEditorInstanceConstant* MaterialEditorInstance, UDEditorParameterValue* Parameter, bool bRespectOverridesFilter)
	{
		if (!MaterialEditorInstance || !Parameter)
		{
			return false;
		}

		if (Parameter->ParameterInfo.Association != EMaterialParameterAssociation::GlobalParameter)
		{
			return false;
		}

		if (!FMaterialPropertyHelpers::ShouldCreatePropertyRowForParameter(Parameter))
		{
			return false;
		}

		if (bRespectOverridesFilter)
		{
			return FMaterialPropertyHelpers::ShouldShowExpression(
				Parameter,
				MaterialEditorInstance,
				FGetShowHiddenParameters::CreateStatic(&GetShowHiddenParameters)) == EVisibility::Visible;
		}

		bool bIsCooked = false;
		if (MaterialEditorInstance->SourceInstance)
		{
			if (UMaterial* Material = MaterialEditorInstance->SourceInstance->GetMaterial())
			{
				bIsCooked = Material->GetPackage()->bIsCookedForEditor;
			}
		}

		return MaterialEditorInstance->VisibleExpressions.Contains(Parameter->ParameterInfo) || bIsCooked;
	}

	bool HasVisibleParametersInGroup(UMaterialEditorInstanceConstant* MaterialEditorInstance, const FEditorParameterGroup& ParameterGroup, bool bRespectOverridesFilter)
	{
		if (!IsGlobalParameterGroup(ParameterGroup))
		{
			return false;
		}

		for (UDEditorParameterValue* Parameter : ParameterGroup.Parameters)
		{
			if (IsParameterVisibleInHierarchy(MaterialEditorInstance, Parameter, bRespectOverridesFilter))
			{
				return true;
			}
		}

		return false;
	}

	UMaterialEditorInstanceConstant* FindMaterialEditorInstance(const TSharedRef<IMaterialEditor>& MaterialEditor)
	{
		const TArray<UObject*>* EditingObjects = MaterialEditor->GetObjectsCurrentlyBeingEdited();
		if (!EditingObjects)
		{
			return nullptr;
		}

		for (UObject* EditingObject : *EditingObjects)
		{
			if (UMaterialEditorInstanceConstant* MaterialEditorInstance = Cast<UMaterialEditorInstanceConstant>(EditingObject))
			{
				return MaterialEditorInstance;
			}
		}

		return nullptr;
	}

	USGFToolsMaterialInstance* FindSGFToolsMaterialInstance(const TSharedRef<IMaterialEditor>& MaterialEditor)
	{
		const TArray<UObject*>* EditingObjects = MaterialEditor->GetObjectsCurrentlyBeingEdited();
		if (!EditingObjects)
		{
			return nullptr;
		}

		for (UObject* EditingObject : *EditingObjects)
		{
			if (USGFToolsMaterialInstance* MaterialInstance = Cast<USGFToolsMaterialInstance>(EditingObject))
			{
				return MaterialInstance;
			}
		}

		return nullptr;
	}

	class FSGFToolsHierarchyMaterialInstanceDetails : public IDetailCustomization
	{
	public:
		FSGFToolsHierarchyMaterialInstanceDetails(UMaterialEditorInstanceConstant* InMaterialEditorInstance, TSharedRef<FSGFToolsHierarchyDetailsState> InState)
			: MaterialEditorInstance(InMaterialEditorInstance)
			, State(MoveTemp(InState))
		{
		}

		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
		{
			PropertyUtilities = DetailLayout.GetPropertyUtilities();

			DetailLayout.HideProperty(TEXT("ParameterGroups"));
			DetailLayout.HideProperty(TEXT("MaterialLayersParameterValues"));
			DetailLayout.HideProperty(TEXT("bUseOldStyleMICEditorGroups"));

			if (!MaterialEditorInstance.IsValid())
			{
				return;
			}

			TArray<FName> ChipGroups;
			CollectChipGroups(ChipGroups);

			if (State->SelectedGroup.IsSet() && !ChipGroups.Contains(State->SelectedGroup.GetValue()))
			{
				State->SelectedGroup.Reset();
			}

			IDetailCategoryBuilder& GroupsCategory = DetailLayout.EditCategory(TEXT("ParameterGroups"), LOCTEXT("HierarchyDetailsTitle", "Hierarchy Details"));
			GroupsCategory.AddCustomRow(LOCTEXT("GroupFilter", "Group Filter"))
				.WholeRowContent()
				[
					BuildGroupSelector(ChipGroups)
				];

			TSharedRef<IPropertyHandle> ParameterGroupsProperty = DetailLayout.GetProperty(TEXT("ParameterGroups"));
			CreateGroupsWidget(ParameterGroupsProperty, GroupsCategory);

			AddPreviewingProperties(DetailLayout);
		}

	private:
		void CollectChipGroups(TArray<FName>& OutGroups) const
		{
			check(MaterialEditorInstance.IsValid());

			for (const FEditorParameterGroup& ParameterGroup : MaterialEditorInstance->ParameterGroups)
			{
				if (HasVisibleParametersInGroup(MaterialEditorInstance.Get(), ParameterGroup, false))
				{
					OutGroups.Add(ParameterGroup.GroupName);
				}
			}
		}

		TSharedRef<SWidget> BuildGroupSelector(const TArray<FName>& ChipGroups)
		{
			TWeakPtr<IPropertyUtilities> WeakPropertyUtilities = PropertyUtilities;
			TSharedRef<FSGFToolsHierarchyDetailsState> LocalState = State;

			auto MakeChip = [WeakPropertyUtilities, LocalState](const TOptional<FName> GroupName, const FText& Label)
			{
				return SNew(SBox)
					.Padding(FMargin(0.0f, 1.0f, 4.0f, 3.0f))
					[
						SNew(SCheckBox)
						.Style(FAppStyle::Get(), "DetailsView.SectionButton")
						.IsChecked_Lambda([LocalState, GroupName]()
						{
							if (!GroupName.IsSet())
							{
								return LocalState->SelectedGroup.IsSet() ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
							}

							return LocalState->SelectedGroup.IsSet() && LocalState->SelectedGroup.GetValue() == GroupName.GetValue()
								? ECheckBoxState::Checked
								: ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([WeakPropertyUtilities, LocalState, GroupName](ECheckBoxState NewState)
						{
							if (NewState != ECheckBoxState::Checked)
							{
								return;
							}

							if (GroupName.IsSet())
							{
								LocalState->SelectedGroup = GroupName.GetValue();
							}
							else
							{
								LocalState->SelectedGroup.Reset();
							}

							if (TSharedPtr<IPropertyUtilities> Utilities = WeakPropertyUtilities.Pin())
							{
								Utilities->ForceRefresh();
							}
						})
						[
							SNew(STextBlock)
							.TextStyle(FAppStyle::Get(), "SmallText")
							.Text(Label)
						]
					];
			};

			TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox)
				.UseAllottedSize(true);

			WrapBox->AddSlot()
			[
				MakeChip(TOptional<FName>(), LOCTEXT("AllGroups", "All"))
			];

			for (const FName GroupName : ChipGroups)
			{
				WrapBox->AddSlot()
				[
					MakeChip(TOptional<FName>(GroupName), FText::FromName(GroupName))
				];
			}

			return WrapBox;
		}

		void CreateGroupsWidget(TSharedRef<IPropertyHandle> ParameterGroupsProperty, IDetailCategoryBuilder& GroupsCategory)
		{
			bool bShowSaveButtons = false;

			for (int32 GroupIdx = 0; GroupIdx < MaterialEditorInstance->ParameterGroups.Num(); ++GroupIdx)
			{
				FEditorParameterGroup& ParameterGroup = MaterialEditorInstance->ParameterGroups[GroupIdx];
				if (!HasVisibleParametersInGroup(MaterialEditorInstance.Get(), ParameterGroup, true))
				{
					continue;
				}

				bShowSaveButtons = true;

				if (State->SelectedGroup.IsSet() && State->SelectedGroup.GetValue() != ParameterGroup.GroupName)
				{
					continue;
				}

				IDetailGroup& DetailGroup = GroupsCategory.AddGroup(ParameterGroup.GroupName, FText::FromName(ParameterGroup.GroupName), false, false);
				FUIAction CopyAction(
					FExecuteAction::CreateSP(this, &FSGFToolsHierarchyMaterialInstanceDetails::OnCopyParameterValues, GroupIdx),
					FCanExecuteAction::CreateSP(this, &FSGFToolsHierarchyMaterialInstanceDetails::CanCopyParameterValues, GroupIdx));
				FUIAction PasteAction(
					FExecuteAction::CreateSP(this, &FSGFToolsHierarchyMaterialInstanceDetails::OnPasteParameterValues, GroupIdx),
					FCanExecuteAction::CreateSP(this, &FSGFToolsHierarchyMaterialInstanceDetails::CanPasteParameterValues, GroupIdx));

				FDetailWidgetRow& HeaderRow = DetailGroup.HeaderRow()
					.CopyAction(CopyAction)
					.PasteAction(PasteAction)
					.NameContent()
					[
						SNew(STextBlock)
						.Text(FText::FromName(DetailGroup.GetGroupName()))
					];

				CreateSingleGroupWidget(ParameterGroup, ParameterGroupsProperty->GetChildHandle(GroupIdx), DetailGroup);

				HeaderRow.AddCustomContextMenuAction(
					FUIAction(FExecuteAction::CreateSP(this, &FSGFToolsHierarchyMaterialInstanceDetails::SetGroupOverrideEnabled, GroupIdx, true)),
					LOCTEXT("ToggleParametersEnable", "Enable All Parameters"),
					LOCTEXT("ToggleParametersEnableTooltip", "Enable All Parameters in group"),
					FSlateIcon());

				HeaderRow.AddCustomContextMenuAction(
					FUIAction(FExecuteAction::CreateSP(this, &FSGFToolsHierarchyMaterialInstanceDetails::SetGroupOverrideEnabled, GroupIdx, false)),
					LOCTEXT("ToggleParametersDisable", "Disable All Parameters"),
					LOCTEXT("ToggleParametersDisableTooltip", "Disable All Parameters in group"),
					FSlateIcon());
			}

			if (bShowSaveButtons)
			{
				AddSaveInstanceRow(GroupsCategory);
			}
		}

		void CreateSingleGroupWidget(FEditorParameterGroup& ParameterGroup, TSharedPtr<IPropertyHandle> ParameterGroupProperty, IDetailGroup& DetailGroup)
		{
			if (!ParameterGroupProperty.IsValid())
			{
				return;
			}

			TSharedPtr<IPropertyHandle> ParametersArrayProperty = ParameterGroupProperty->GetChildHandle(TEXT("Parameters"));
			if (!ParametersArrayProperty.IsValid())
			{
				return;
			}

			FMaterialParameterPropertyRowOverrides RowOverrides;
			RowOverrides.ShowHiddenDelegate = FGetShowHiddenParameters::CreateStatic(&GetShowHiddenParameters);

			for (int32 ParamIdx = 0; ParamIdx < ParameterGroup.Parameters.Num(); ++ParamIdx)
			{
				UDEditorParameterValue* Parameter = ParameterGroup.Parameters[ParamIdx];
				if (!IsParameterVisibleInHierarchy(MaterialEditorInstance.Get(), Parameter, false))
				{
					continue;
				}

				TSharedPtr<IPropertyHandle> ParameterProperty = ParametersArrayProperty->GetChildHandle(ParamIdx);
				TSharedPtr<IPropertyHandle> ParameterValueProperty = ParameterProperty.IsValid() ? ParameterProperty->GetChildHandle(TEXT("ParameterValue")) : nullptr;
				if (!ParameterProperty.IsValid() || !ParameterValueProperty.IsValid() || !ParameterValueProperty->IsValidHandle())
				{
					continue;
				}

				IDetailPropertyRow& PropertyRow = DetailGroup.AddPropertyRow(ParameterValueProperty.ToSharedRef());
				FMaterialPropertyHelpers::ConfigurePropertyRowForParameter(PropertyRow, Parameter, ParameterValueProperty, MaterialEditorInstance.Get(), RowOverrides);
				FMaterialPropertyHelpers::SetPropertyRowParameterWidget(PropertyRow, Parameter, ParameterValueProperty, MaterialEditorInstance.Get());
			}
		}

		void AddSaveInstanceRow(IDetailCategoryBuilder& GroupsCategory)
		{
			FDetailWidgetRow& SaveInstanceRow = GroupsCategory.AddCustomRow(LOCTEXT("SaveInstances", "Save Instances"));

			UMaterialInterface* SourceInstance = MaterialEditorInstance->SourceInstance;
			UMaterialInterface* ParentMaterial = MaterialEditorInstance->SourceInstance ? MaterialEditorInstance->SourceInstance->Parent.Get() : nullptr;
			UObject* LocalEditorInstance = MaterialEditorInstance.Get();

			FOnClicked OnChildButtonClicked = FOnClicked::CreateStatic(&FMaterialPropertyHelpers::OnClickedSaveNewMaterialInstance, SourceInstance, LocalEditorInstance);
			FOnClicked OnSiblingButtonClicked = FOnClicked::CreateStatic(&FMaterialPropertyHelpers::OnClickedSaveNewMaterialInstance, ParentMaterial, LocalEditorInstance);

			SaveInstanceRow.ValueContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNullWidget::NullWidget
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("SaveSibling", "Save Sibling"))
						.HAlign(HAlign_Center)
						.IsEnabled(ParentMaterial != nullptr)
						.OnClicked(OnSiblingButtonClicked)
						.ToolTipText(LOCTEXT("SaveToSiblingInstance", "Save to Sibling Instance"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("SaveChild", "Save Child"))
						.HAlign(HAlign_Center)
						.IsEnabled(SourceInstance != nullptr)
						.OnClicked(OnChildButtonClicked)
						.ToolTipText(LOCTEXT("SaveToChildInstance", "Save to Child Instance"))
					]
				];
		}

		void AddPreviewingProperties(IDetailLayoutBuilder& DetailLayout)
		{
			if (!MaterialEditorInstance->SourceInstance)
			{
				return;
			}

			IDetailCategoryBuilder& PreviewingCategory = DetailLayout.EditCategory(TEXT("Previewing"), LOCTEXT("MICPreviewingCategoryTitle", "Previewing"));

			TArray<UObject*> ExternalObjects;
			ExternalObjects.Add(MaterialEditorInstance->SourceInstance);

			PreviewingCategory.AddExternalObjectProperty(ExternalObjects, TEXT("PreviewMesh"));

			IDetailCategoryBuilder& DefaultCategory = DetailLayout.EditCategory(NAME_None);
			DefaultCategory.AddExternalObjectProperty(ExternalObjects, TEXT("AssetUserData"), EPropertyLocation::Advanced);
		}

		void SetGroupOverrideEnabled(int32 GroupIndex, bool bShouldEnable)
		{
			if (!MaterialEditorInstance.IsValid() || !MaterialEditorInstance->ParameterGroups.IsValidIndex(GroupIndex))
			{
				return;
			}

			const FScopedTransaction Transaction(bShouldEnable
				? LOCTEXT("EnableAllParameters", "Enable All Material Instance Parameters")
				: LOCTEXT("DisableAllParameters", "Disable All Material Instance Parameters"));

			MaterialEditorInstance->Modify();

			FEditorParameterGroup& ParameterGroup = MaterialEditorInstance->ParameterGroups[GroupIndex];
			for (UDEditorParameterValue* Parameter : ParameterGroup.Parameters)
			{
				if (Parameter)
				{
					Parameter->Modify();
					Parameter->bOverride = bShouldEnable;
				}
			}

			MaterialEditorInstance->PostEditChange();
			FEditorSupportDelegates::RedrawAllViewports.Broadcast();

			if (TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
			{
				Utilities->ForceRefresh();
			}
		}

		void OnCopyParameterValues(int32 ParameterGroupIndex)
		{
			if (!MaterialEditorInstance.IsValid() || !MaterialEditorInstance->ParameterGroups.IsValidIndex(ParameterGroupIndex))
			{
				return;
			}

			FEditorParameterGroup& ParameterGroup = MaterialEditorInstance->ParameterGroups[ParameterGroupIndex];
			TStringBuilder<4096> CombinedValue;

			for (UDEditorParameterValue* Parameter : ParameterGroup.Parameters)
			{
				if (!Parameter)
				{
					continue;
				}

				const FName ParamName = Parameter->ParameterInfo.Name;
				const TCHAR* Prefix = CombinedValue.Len() == 0 ? TEXT("") : TEXT(",");

				if (FMaterialPropertyHelpers::IsOverriddenExpression(Parameter))
				{
					FProperty* ParameterValueProperty = Parameter->GetClass()->FindPropertyByName(TEXT("ParameterValue"));
					if (ParameterValueProperty)
					{
						FString ParameterValueString;
						if (ParameterValueProperty->ExportText_InContainer(0, ParameterValueString, Parameter, Parameter, Parameter, PPF_Copy))
						{
							CombinedValue.Appendf(
								TEXT("%s%s.Override=True,%s.Value=\"%s\""),
								Prefix,
								*ParamName.ToString(),
								*ParamName.ToString(),
								*ParameterValueString.ReplaceCharWithEscapedChar());
						}
					}
				}
				else
				{
					CombinedValue.Appendf(TEXT("%s%s.Override=False"), Prefix, *ParamName.ToString());
				}
			}

			if (CombinedValue.Len() > 0)
			{
				FPlatformApplicationMisc::ClipboardCopy(*CombinedValue);
			}
		}

		bool CanCopyParameterValues(int32 ParameterGroupIndex) const
		{
			return MaterialEditorInstance.IsValid()
				&& MaterialEditorInstance->ParameterGroups.IsValidIndex(ParameterGroupIndex)
				&& MaterialEditorInstance->ParameterGroups[ParameterGroupIndex].Parameters.Num() > 0;
		}

		void OnPasteParameterValues(int32 ParameterGroupIndex)
		{
			if (!MaterialEditorInstance.IsValid() || !MaterialEditorInstance->ParameterGroups.IsValidIndex(ParameterGroupIndex))
			{
				return;
			}

			FString ClipboardContent;
			FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
			if (ClipboardContent.IsEmpty())
			{
				return;
			}

			const FScopedTransaction Transaction(LOCTEXT("PasteMaterialInstanceParameters", "Paste Material Instance Parameters"));
			MaterialEditorInstance->Modify();

			for (UDEditorParameterValue* Parameter : MaterialEditorInstance->ParameterGroups[ParameterGroupIndex].Parameters)
			{
				if (!Parameter)
				{
					continue;
				}

				Parameter->Modify();

				const FName ParamName = Parameter->ParameterInfo.Name;
				const FString OverrideKey = FString::Printf(TEXT("%s.Override="), *ParamName.ToString());
				bool bParsedOverride = false;
				if (!FParse::Bool(*ClipboardContent, *OverrideKey, bParsedOverride))
				{
					continue;
				}

				Parameter->bOverride = bParsedOverride;
				if (!bParsedOverride)
				{
					continue;
				}

				const FString ValueKey = FString::Printf(TEXT("%s.Value="), *ParamName.ToString());
				FString ParsedValueString;
				if (FParse::Value(*ClipboardContent, *ValueKey, ParsedValueString))
				{
					ParsedValueString = ParsedValueString.ReplaceEscapedCharWithChar();
					FProperty* ParameterValueProperty = Parameter->GetClass()->FindPropertyByName(TEXT("ParameterValue"));
					if (ParameterValueProperty)
					{
						ParameterValueProperty->ImportText_InContainer(*ParsedValueString, Parameter, Parameter, PPF_Copy);
					}
				}
			}

			MaterialEditorInstance->PostEditChange();
			FEditorSupportDelegates::RedrawAllViewports.Broadcast();

			if (TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
			{
				Utilities->ForceRefresh();
			}
		}

		bool CanPasteParameterValues(int32 ParameterGroupIndex) const
		{
			if (!CanCopyParameterValues(ParameterGroupIndex))
			{
				return false;
			}

			FString ClipboardContent;
			FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
			return !ClipboardContent.IsEmpty();
		}

	private:
		TWeakObjectPtr<UMaterialEditorInstanceConstant> MaterialEditorInstance;
		TSharedRef<FSGFToolsHierarchyDetailsState> State;
		TWeakPtr<IPropertyUtilities> PropertyUtilities;
	};

	class FSGFToolsHierarchyDetailsExtension : public TSharedFromThis<FSGFToolsHierarchyDetailsExtension>
	{
	public:
		FSGFToolsHierarchyDetailsExtension(TSharedRef<IMaterialEditor> InMaterialEditor, USGFToolsMaterialInstance* InMaterialInstance)
			: MaterialEditor(InMaterialEditor)
			, MaterialInstance(InMaterialInstance)
			, State(MakeShared<FSGFToolsHierarchyDetailsState>())
		{
		}

		void Bind()
		{
			if (TSharedPtr<IMaterialEditor> Editor = MaterialEditor.Pin())
			{
				RegisterTabSpawnersHandle = Editor->OnRegisterTabSpawners().AddSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::RegisterTabSpawner);
				UnregisterTabSpawnersHandle = Editor->OnUnregisterTabSpawners().AddSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::UnregisterTabSpawner);
				EditorClosedHandle = Editor->OnMaterialEditorClosed().AddSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::HandleEditorClosed);
			}
		}

		bool IsForEditor(const TSharedRef<IMaterialEditor>& InMaterialEditor) const
		{
			return MaterialEditor.Pin() == InMaterialEditor;
		}

		bool IsClosed() const
		{
			return bClosed || !MaterialEditor.IsValid();
		}

		void OpenTab()
		{
			EnsureTabSpawnerRegistered();

			if (TSharedPtr<IMaterialEditor> Editor = MaterialEditor.Pin())
			{
				if (TSharedPtr<FTabManager> TabManager = Editor->GetTabManager())
				{
					TabManager->TryInvokeTab(SGFTools::MaterialInstanceHierarchyDetails::TabId);
				}
			}
		}

		void Shutdown()
		{
			if (TSharedPtr<IMaterialEditor> Editor = MaterialEditor.Pin())
			{
				Editor->OnRegisterTabSpawners().Remove(RegisterTabSpawnersHandle);
				Editor->OnUnregisterTabSpawners().Remove(UnregisterTabSpawnersHandle);
				Editor->OnMaterialEditorClosed().Remove(EditorClosedHandle);
			}

			if (TSharedPtr<FTabManager> TabManager = RegisteredTabManager.Pin())
			{
				TabManager->UnregisterTabSpawner(SGFTools::MaterialInstanceHierarchyDetails::TabId);
			}

			bClosed = true;
		}

	private:
		void EnsureTabSpawnerRegistered()
		{
			if (bRegisteredTabSpawner)
			{
				return;
			}

			if (TSharedPtr<IMaterialEditor> Editor = MaterialEditor.Pin())
			{
				if (TSharedPtr<FTabManager> TabManager = Editor->GetTabManager())
				{
					RegisterTabSpawner(TabManager.ToSharedRef());
				}
			}
		}

		void RegisterTabSpawner(const TSharedRef<FTabManager>& InTabManager)
		{
			if (!MaterialInstance.IsValid() || bRegisteredTabSpawner)
			{
				return;
			}

			InTabManager->RegisterTabSpawner(
				SGFTools::MaterialInstanceHierarchyDetails::TabId,
				FOnSpawnTab::CreateSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::SpawnHierarchyDetailsTab))
				.SetDisplayName(LOCTEXT("HierarchyDetailsTab", "Hierarchy Details"))
				.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

			RegisteredTabManager = InTabManager;
			bRegisteredTabSpawner = true;
		}

		void UnregisterTabSpawner(const TSharedRef<FTabManager>& InTabManager)
		{
			InTabManager->UnregisterTabSpawner(SGFTools::MaterialInstanceHierarchyDetails::TabId);
			bRegisteredTabSpawner = false;
			RegisteredTabManager.Reset();
		}

		void HandleEditorClosed()
		{
			bClosed = true;
		}

		TSharedRef<SDockTab> SpawnHierarchyDetailsTab(const FSpawnTabArgs& Args)
		{
			return SNew(SDockTab)
				.Label(LOCTEXT("HierarchyDetailsTabLabel", "Hierarchy Details"))
				[
					SNew(SBorder)
					.Padding(4.0f)
					[
						CreateHierarchyDetailsWidget()
					]
				];
		}

		TSharedRef<SWidget> CreateHierarchyDetailsWidget()
		{
			TSharedPtr<IMaterialEditor> Editor = MaterialEditor.Pin();
			if (!Editor.IsValid())
			{
				return SNew(STextBlock)
					.Text(LOCTEXT("EditorUnavailable", "Material Instance editor is unavailable."));
			}

			UMaterialEditorInstanceConstant* MaterialEditorInstance = FindMaterialEditorInstance(Editor.ToSharedRef());
			if (!MaterialEditorInstance)
			{
				return SNew(STextBlock)
					.Text(LOCTEXT("EditorInstanceUnavailable", "Hierarchy Details is waiting for the Material Instance Details data."));
			}

			FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

			FDetailsViewArgs DetailsViewArgs;
			DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
			DetailsViewArgs.bHideSelectionTip = true;
			DetailsViewArgs.bShowModifiedPropertiesOption = false;
			DetailsViewArgs.bShowCustomFilterOption = true;

			TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
			DetailsView->SetCustomValidatePropertyNodesFunction(FOnValidateDetailsViewPropertyNodes::CreateLambda(
				[](const FRootPropertyNodeList& PropertyNodeList)
				{
					return true;
				}));

			DetailsView->RegisterInstancedCustomPropertyLayout(
				UMaterialEditorInstanceConstant::StaticClass(),
				FOnGetDetailCustomizationInstance::CreateLambda([MaterialEditorInstance, State = State]()
				{
					return MakeShared<FSGFToolsHierarchyMaterialInstanceDetails>(MaterialEditorInstance, State.ToSharedRef());
				}));

			TWeakPtr<IDetailsView> WeakDetailsView = DetailsView;
			DetailsView->SetCustomFilterLabel(LOCTEXT("ShowOverriddenOnly", "Show Only Overridden Parameters"));
			DetailsView->SetCustomFilterDelegate(FSimpleDelegate::CreateLambda([MaterialEditorInstance, WeakDetailsView]()
			{
				if (!MaterialEditorInstance)
				{
					return;
				}

				MaterialEditorInstance->bShowOnlyOverrides = !MaterialEditorInstance->bShowOnlyOverrides;
				if (TSharedPtr<IDetailsView> PinnedDetailsView = WeakDetailsView.Pin())
				{
					PinnedDetailsView->ForceRefresh();
				}
			}));

			DetailsView->SetObject(MaterialEditorInstance, true);
			return DetailsView;
		}

	private:
		TWeakPtr<IMaterialEditor> MaterialEditor;
		TWeakObjectPtr<USGFToolsMaterialInstance> MaterialInstance;
		TSharedPtr<FSGFToolsHierarchyDetailsState> State;
		TWeakPtr<FTabManager> RegisteredTabManager;
		FDelegateHandle RegisterTabSpawnersHandle;
		FDelegateHandle UnregisterTabSpawnersHandle;
		FDelegateHandle EditorClosedHandle;
		bool bRegisteredTabSpawner = false;
		bool bClosed = false;
	};

	void PruneInactiveExtensions()
	{
		GActiveExtensions.RemoveAll([](const TSharedPtr<FSGFToolsHierarchyDetailsExtension>& Extension)
		{
			return !Extension.IsValid() || Extension->IsClosed();
		});
	}

	void HandleMaterialInstanceEditorOpened(TWeakPtr<IMaterialEditor> WeakMaterialEditor)
	{
		if (GPendingMaterialInstances.IsEmpty())
		{
			return;
		}

		TWeakObjectPtr<USGFToolsMaterialInstance> PendingMaterialInstance = GPendingMaterialInstances[0];
		GPendingMaterialInstances.RemoveAt(0);

		TSharedPtr<IMaterialEditor> MaterialEditor = WeakMaterialEditor.Pin();
		if (!MaterialEditor.IsValid() || !PendingMaterialInstance.IsValid())
		{
			return;
		}

		TSharedRef<FSGFToolsHierarchyDetailsExtension> Extension = MakeShared<FSGFToolsHierarchyDetailsExtension>(MaterialEditor.ToSharedRef(), PendingMaterialInstance.Get());
		Extension->Bind();
		GActiveExtensions.Add(Extension);
	}

	void EnsureMaterialEditorDelegateRegistered()
	{
		if (GMaterialInstanceEditorOpenedHandle.IsValid() || !FModuleManager::Get().IsModuleLoaded(TEXT("MaterialEditor")))
		{
			return;
		}

		IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>(TEXT("MaterialEditor"));
		GMaterialInstanceEditorOpenedHandle = MaterialEditorModule.OnMaterialInstanceEditorOpened().AddStatic(&HandleMaterialInstanceEditorOpened);
	}
}

namespace SGFTools::MaterialInstanceHierarchyDetails
{
	void Startup()
	{
		EnsureMaterialEditorDelegateRegistered();
	}

	void Shutdown()
	{
		if (GMaterialInstanceEditorOpenedHandle.IsValid() && FModuleManager::Get().IsModuleLoaded(TEXT("MaterialEditor")))
		{
			IMaterialEditorModule& MaterialEditorModule = FModuleManager::GetModuleChecked<IMaterialEditorModule>(TEXT("MaterialEditor"));
			MaterialEditorModule.OnMaterialInstanceEditorOpened().Remove(GMaterialInstanceEditorOpenedHandle);
		}

		GMaterialInstanceEditorOpenedHandle.Reset();
		GPendingMaterialInstances.Empty();

		for (const TSharedPtr<FSGFToolsHierarchyDetailsExtension>& Extension : GActiveExtensions)
		{
			if (Extension.IsValid())
			{
				Extension->Shutdown();
			}
		}

		GActiveExtensions.Empty();
	}

	void QueueEditorForMaterialInstance(USGFToolsMaterialInstance* MaterialInstance)
	{
		if (MaterialInstance)
		{
			EnsureMaterialEditorDelegateRegistered();
			GPendingMaterialInstances.Add(MaterialInstance);
		}
	}

	void OpenForMaterialEditor(const TSharedRef<IMaterialEditor>& MaterialEditor)
	{
		PruneInactiveExtensions();

		bool bOpened = false;
		for (const TSharedPtr<FSGFToolsHierarchyDetailsExtension>& Extension : GActiveExtensions)
		{
			if (Extension.IsValid() && Extension->IsForEditor(MaterialEditor))
			{
				Extension->OpenTab();
				bOpened = true;
				break;
			}
		}

		if (!bOpened)
		{
			if (USGFToolsMaterialInstance* MaterialInstance = FindSGFToolsMaterialInstance(MaterialEditor))
			{
				GPendingMaterialInstances.RemoveAll([MaterialInstance](const TWeakObjectPtr<USGFToolsMaterialInstance>& PendingMaterialInstance)
				{
					return !PendingMaterialInstance.IsValid() || PendingMaterialInstance.Get() == MaterialInstance;
				});

				TSharedRef<FSGFToolsHierarchyDetailsExtension> Extension = MakeShared<FSGFToolsHierarchyDetailsExtension>(MaterialEditor, MaterialInstance);
				Extension->Bind();
				GActiveExtensions.Add(Extension);
				Extension->OpenTab();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
