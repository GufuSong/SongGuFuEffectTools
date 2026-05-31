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

	// 行为：返回隐藏参数显示状态；作用：为材质属性帮助器提供不显示隐藏参数的委托；输出：通过引用写出 false。
	void GetShowHiddenParameters(bool& bShowHiddenParameters)
	{
		bShowHiddenParameters = false;
	}

	// 行为：判断是否为全局参数组；作用：过滤材质层参数组之外的全局参数组；输出：是否为可处理全局参数组的布尔值。
	bool IsGlobalParameterGroup(const FEditorParameterGroup& ParameterGroup)
	{
		return ParameterGroup.GroupAssociation == EMaterialParameterAssociation::GlobalParameter
			&& ParameterGroup.GroupName != FMaterialPropertyHelpers::LayerParamName;
	}

	// 行为：判断层级详情中参数是否可见；作用：综合参数关联类型、行创建规则和覆盖过滤状态筛选参数；输出：是否可见的布尔值。
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

	// 行为：判断参数组是否存在可见参数；作用：决定层级详情是否展示指定参数组；输出：是否包含可见参数的布尔值。
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

	// 行为：查找材质编辑器实例数据；作用：从材质编辑器当前编辑对象中取出 UMaterialEditorInstanceConstant；输出：编辑器实例指针，失败时为空。
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

	// 行为：查找 SGF 材质实例；作用：从材质编辑器当前编辑对象中取出 USGFToolsMaterialInstance；输出：SGF 材质实例指针，失败时为空。
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
		// 行为：构造层级详情自定义对象；作用：保存材质编辑器实例和共享筛选状态；输出：无返回值，生成详情自定义对象。
		FSGFToolsHierarchyMaterialInstanceDetails(UMaterialEditorInstanceConstant* InMaterialEditorInstance, TSharedRef<FSGFToolsHierarchyDetailsState> InState)
			: MaterialEditorInstance(InMaterialEditorInstance)
			, State(MoveTemp(InState))
		{
		}

		// 行为：定制详情面板；作用：隐藏原生参数组并生成层级详情、组筛选和预览属性；输出：无返回值。
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
		// 行为：收集可筛选参数组；作用：遍历材质参数组并记录存在可见参数的组名；输出：通过数组引用写出组名列表。
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

		// 行为：构建参数组筛选器；作用：创建全部/单组筛选的芯片控件；输出：筛选器 Widget 引用。
		TSharedRef<SWidget> BuildGroupSelector(const TArray<FName>& ChipGroups)
		{
			TWeakPtr<IPropertyUtilities> WeakPropertyUtilities = PropertyUtilities;
			TSharedRef<FSGFToolsHierarchyDetailsState> LocalState = State;

			// 行为：创建单个筛选芯片；作用：把组名、选中状态和点击刷新逻辑封装成复选框控件；输出：芯片 Widget 引用。
			auto MakeChip = [WeakPropertyUtilities, LocalState](const TOptional<FName> GroupName, const FText& Label)
			{
				return SNew(SBox)
					.Padding(FMargin(0.0f, 1.0f, 4.0f, 3.0f))
					[
						SNew(SCheckBox)
						.Style(FAppStyle::Get(), "DetailsView.SectionButton")
						// 行为：计算筛选芯片选中状态；作用：让 UI 显示当前组选中或全部选中状态；输出：复选框状态。
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
						// 行为：响应筛选芯片切换；作用：更新选中的参数组并刷新详情面板；输出：无返回值。
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

		// 行为：创建参数组详情控件；作用：按筛选条件生成每个可见参数组、复制粘贴动作和保存按钮；输出：无返回值。
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

		// 行为：创建单个参数组控件；作用：为组内每个可见参数生成属性行并套用材质参数显示逻辑；输出：无返回值。
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

		// 行为：添加保存实例行；作用：在详情面板中提供保存兄弟实例和子实例按钮；输出：无返回值。
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

		// 行为：添加预览相关属性；作用：把源材质实例的 PreviewMesh 和 AssetUserData 暴露到详情面板；输出：无返回值。
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

		// 行为：批量设置组参数覆盖状态；作用：启用或禁用指定组内全部参数覆盖并刷新编辑器；输出：无返回值。
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

		// 行为：复制参数值；作用：把指定参数组的覆盖状态和值序列化到剪贴板；输出：无返回值，剪贴板获得参数文本。
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

		// 行为：判断是否可复制参数值；作用：确认参数组有效且包含参数；输出：是否允许复制的布尔值。
		bool CanCopyParameterValues(int32 ParameterGroupIndex) const
		{
			return MaterialEditorInstance.IsValid()
				&& MaterialEditorInstance->ParameterGroups.IsValidIndex(ParameterGroupIndex)
				&& MaterialEditorInstance->ParameterGroups[ParameterGroupIndex].Parameters.Num() > 0;
		}

		// 行为：粘贴参数值；作用：从剪贴板解析覆盖状态和值并应用到指定参数组；输出：无返回值。
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

		// 行为：判断是否可粘贴参数值；作用：确认参数组可复制且剪贴板存在内容；输出：是否允许粘贴的布尔值。
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
		// 行为：构造层级详情扩展对象；作用：绑定材质编辑器、材质实例和共享筛选状态；输出：无返回值，生成扩展对象。
		FSGFToolsHierarchyDetailsExtension(TSharedRef<IMaterialEditor> InMaterialEditor, USGFToolsMaterialInstance* InMaterialInstance)
			: MaterialEditor(InMaterialEditor)
			, MaterialInstance(InMaterialInstance)
			, State(MakeShared<FSGFToolsHierarchyDetailsState>())
		{
		}

		// 行为：绑定编辑器事件；作用：监听页签注册、注销和编辑器关闭事件；输出：无返回值。
		void Bind()
		{
			if (TSharedPtr<IMaterialEditor> Editor = MaterialEditor.Pin())
			{
				RegisterTabSpawnersHandle = Editor->OnRegisterTabSpawners().AddSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::RegisterTabSpawner);
				UnregisterTabSpawnersHandle = Editor->OnUnregisterTabSpawners().AddSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::UnregisterTabSpawner);
				EditorClosedHandle = Editor->OnMaterialEditorClosed().AddSP(AsShared(), &FSGFToolsHierarchyDetailsExtension::HandleEditorClosed);
			}
		}

		// 行为：判断扩展是否属于指定编辑器；作用：避免对同一个材质编辑器重复创建扩展；输出：是否匹配的布尔值。
		bool IsForEditor(const TSharedRef<IMaterialEditor>& InMaterialEditor) const
		{
			return MaterialEditor.Pin() == InMaterialEditor;
		}

		// 行为：判断扩展是否已关闭；作用：帮助全局列表清理失效扩展；输出：是否关闭的布尔值。
		bool IsClosed() const
		{
			return bClosed || !MaterialEditor.IsValid();
		}

		// 行为：打开层级详情页签；作用：确保页签注册后在当前材质编辑器中唤起详情面板；输出：无返回值。
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

		// 行为：关闭扩展对象；作用：移除编辑器事件绑定并注销层级详情页签；输出：无返回值。
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
		// 行为：确保页签生成器已注册；作用：在打开页签前补注册层级详情页签；输出：无返回值。
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

		// 行为：注册层级详情页签生成器；作用：向材质编辑器 TabManager 添加自定义详情页签；输出：无返回值。
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

		// 行为：注销层级详情页签生成器；作用：从材质编辑器 TabManager 移除自定义详情页签；输出：无返回值。
		void UnregisterTabSpawner(const TSharedRef<FTabManager>& InTabManager)
		{
			InTabManager->UnregisterTabSpawner(SGFTools::MaterialInstanceHierarchyDetails::TabId);
			bRegisteredTabSpawner = false;
			RegisteredTabManager.Reset();
		}

		// 行为：处理编辑器关闭事件；作用：标记扩展已关闭以便后续清理；输出：无返回值。
		void HandleEditorClosed()
		{
			bClosed = true;
		}

		// 行为：生成层级详情页签；作用：创建承载层级详情控件的 SDockTab；输出：页签 Widget 引用。
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

		// 行为：创建层级详情控件；作用：生成自定义 DetailsView 并绑定过滤、刷新和详情定制逻辑；输出：详情 Widget 引用。
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
			// 行为：校验详情属性节点；作用：允许自定义 DetailsView 接受当前属性树；输出：始终为 true 的布尔值。
			DetailsView->SetCustomValidatePropertyNodesFunction(FOnValidateDetailsViewPropertyNodes::CreateLambda(
				[](const FRootPropertyNodeList& PropertyNodeList)
				{
					return true;
				}));

			DetailsView->RegisterInstancedCustomPropertyLayout(
				UMaterialEditorInstanceConstant::StaticClass(),
				// 行为：创建详情定制实例；作用：为材质编辑器实例生成层级详情自定义对象；输出：详情定制共享指针。
				FOnGetDetailCustomizationInstance::CreateLambda([MaterialEditorInstance, State = State]()
				{
					return MakeShared<FSGFToolsHierarchyMaterialInstanceDetails>(MaterialEditorInstance, State.ToSharedRef());
				}));

			TWeakPtr<IDetailsView> WeakDetailsView = DetailsView;
			DetailsView->SetCustomFilterLabel(LOCTEXT("ShowOverriddenOnly", "Show Only Overridden Parameters"));
			// 行为：切换仅显示覆盖参数过滤；作用：反转材质编辑器实例的覆盖过滤状态并刷新详情面板；输出：无返回值。
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

	// 行为：清理失效扩展；作用：从活动扩展列表移除已关闭或无效的扩展对象；输出：无返回值。
	void PruneInactiveExtensions()
	{
		// 行为：判断扩展是否应移除；作用：供 RemoveAll 过滤无效或已关闭扩展；输出：是否移除的布尔值。
		GActiveExtensions.RemoveAll([](const TSharedPtr<FSGFToolsHierarchyDetailsExtension>& Extension)
		{
			return !Extension.IsValid() || Extension->IsClosed();
		});
	}

	// 行为：处理材质实例编辑器打开事件；作用：把等待队列中的 SGF 材质实例绑定到刚打开的材质编辑器；输出：无返回值。
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

	// 行为：确保材质编辑器事件已注册；作用：按需监听 MaterialEditor 模块的材质实例编辑器打开事件；输出：无返回值。
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
	// 行为：启动层级详情扩展；作用：确保材质编辑器打开事件被监听；输出：无返回值。
	void Startup()
	{
		EnsureMaterialEditorDelegateRegistered();
	}

	// 行为：关闭层级详情扩展；作用：移除事件监听、清空等待队列并关闭全部活动扩展；输出：无返回值。
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

	// 行为：排队等待材质实例编辑器；作用：记录要打开层级详情页签的 SGF 材质实例并确保事件已注册；输出：无返回值。
	void QueueEditorForMaterialInstance(USGFToolsMaterialInstance* MaterialInstance)
	{
		if (MaterialInstance)
		{
			EnsureMaterialEditorDelegateRegistered();
			GPendingMaterialInstances.Add(MaterialInstance);
		}
	}

	// 行为：打开指定材质编辑器的层级详情面板；作用：复用已有扩展或创建新扩展并唤起自定义页签；输出：无返回值。
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
				// 行为：移除同实例等待项；作用：避免已打开的材质实例再次从等待队列创建扩展；输出：是否移除等待项的布尔值。
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
