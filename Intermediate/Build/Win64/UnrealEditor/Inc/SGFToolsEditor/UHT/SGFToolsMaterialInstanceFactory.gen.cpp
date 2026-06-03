// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Material/SGFToolsMaterialInstanceFactory.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeSGFToolsMaterialInstanceFactory() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UMaterialInterface_NoRegister();
SGFTOOLSEDITOR_API UClass* Z_Construct_UClass_USGFToolsMaterialInstanceFactory();
SGFTOOLSEDITOR_API UClass* Z_Construct_UClass_USGFToolsMaterialInstanceFactory_NoRegister();
UNREALED_API UClass* Z_Construct_UClass_UFactory();
UPackage* Z_Construct_UPackage__Script_SGFToolsEditor();
// ********** End Cross Module References **********************************************************

// ********** Begin Class USGFToolsMaterialInstanceFactory *****************************************
FClassRegistrationInfo Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory;
UClass* USGFToolsMaterialInstanceFactory::GetPrivateStaticClass()
{
	using TClass = USGFToolsMaterialInstanceFactory;
	if (!Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("SGFToolsMaterialInstanceFactory"),
			Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory.InnerSingleton,
			StaticRegisterNativesUSGFToolsMaterialInstanceFactory,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory.InnerSingleton;
}
UClass* Z_Construct_UClass_USGFToolsMaterialInstanceFactory_NoRegister()
{
	return USGFToolsMaterialInstanceFactory::GetPrivateStaticClass();
}
struct Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * SGFTools \xe6\x9d\x90\xe8\xb4\xa8\xe5\xae\x9e\xe4\xbe\x8b\xe8\xb5\x84\xe4\xba\xa7\xe7\x9a\x84\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe5\x88\x9b\xe5\xbb\xba\xe5\xb7\xa5\xe5\x8e\x82\xe3\x80\x82\n *\n * Factory \xe5\xb1\x9e\xe4\xba\x8e Editor-only\xef\xbc\x9a\xe5\xae\x83\xe5\x8f\xaa\xe8\xb4\x9f\xe8\xb4\xa3\xe5\x9c\xa8\xe5\x86\x85\xe5\xae\xb9\xe6\xb5\x8f\xe8\xa7\x88\xe5\x99\xa8\xe9\x87\x8c\xe5\x88\x9b\xe5\xbb\xba\xe8\xb5\x84\xe4\xba\xa7\xef\xbc\x8c\xe4\xb8\x8d\xe5\xba\x94\xe8\xaf\xa5\xe6\x94\xbe\xe8\xbf\x9b Runtime\xe3\x80\x82\n */" },
#endif
		{ "IncludePath", "Material/SGFToolsMaterialInstanceFactory.h" },
		{ "ModuleRelativePath", "Private/Material/SGFToolsMaterialInstanceFactory.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "SGFTools \xe6\x9d\x90\xe8\xb4\xa8\xe5\xae\x9e\xe4\xbe\x8b\xe8\xb5\x84\xe4\xba\xa7\xe7\x9a\x84\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe5\x88\x9b\xe5\xbb\xba\xe5\xb7\xa5\xe5\x8e\x82\xe3\x80\x82\n\nFactory \xe5\xb1\x9e\xe4\xba\x8e Editor-only\xef\xbc\x9a\xe5\xae\x83\xe5\x8f\xaa\xe8\xb4\x9f\xe8\xb4\xa3\xe5\x9c\xa8\xe5\x86\x85\xe5\xae\xb9\xe6\xb5\x8f\xe8\xa7\x88\xe5\x99\xa8\xe9\x87\x8c\xe5\x88\x9b\xe5\xbb\xba\xe8\xb5\x84\xe4\xba\xa7\xef\xbc\x8c\xe4\xb8\x8d\xe5\xba\x94\xe8\xaf\xa5\xe6\x94\xbe\xe8\xbf\x9b Runtime\xe3\x80\x82" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_InitialParent_MetaData[] = {
		{ "ModuleRelativePath", "Private/Material/SGFToolsMaterialInstanceFactory.h" },
	};
#endif // WITH_METADATA

// ********** Begin Class USGFToolsMaterialInstanceFactory constinit property declarations *********
	static const UECodeGen_Private::FObjectPropertyParams NewProp_InitialParent;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End Class USGFToolsMaterialInstanceFactory constinit property declarations ***********
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USGFToolsMaterialInstanceFactory>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics

// ********** Begin Class USGFToolsMaterialInstanceFactory Property Definitions ********************
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::NewProp_InitialParent = { "InitialParent", nullptr, (EPropertyFlags)0x0114000000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(USGFToolsMaterialInstanceFactory, InitialParent), Z_Construct_UClass_UMaterialInterface_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_InitialParent_MetaData), NewProp_InitialParent_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::NewProp_InitialParent,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::PropPointers) < 2048);
// ********** End Class USGFToolsMaterialInstanceFactory Property Definitions **********************
UObject* (*const Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UFactory,
	(UObject* (*)())Z_Construct_UPackage__Script_SGFToolsEditor,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::ClassParams = {
	&USGFToolsMaterialInstanceFactory::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::PropPointers),
	0,
	0x000000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::Class_MetaDataParams), Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::Class_MetaDataParams)
};
void USGFToolsMaterialInstanceFactory::StaticRegisterNativesUSGFToolsMaterialInstanceFactory()
{
}
UClass* Z_Construct_UClass_USGFToolsMaterialInstanceFactory()
{
	if (!Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory.OuterSingleton, Z_Construct_UClass_USGFToolsMaterialInstanceFactory_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, USGFToolsMaterialInstanceFactory);
USGFToolsMaterialInstanceFactory::~USGFToolsMaterialInstanceFactory() {}
// ********** End Class USGFToolsMaterialInstanceFactory *******************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsEditor_Private_Material_SGFToolsMaterialInstanceFactory_h__Script_SGFToolsEditor_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_USGFToolsMaterialInstanceFactory, USGFToolsMaterialInstanceFactory::StaticClass, TEXT("USGFToolsMaterialInstanceFactory"), &Z_Registration_Info_UClass_USGFToolsMaterialInstanceFactory, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(USGFToolsMaterialInstanceFactory), 4287314234U) },
	};
}; // Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsEditor_Private_Material_SGFToolsMaterialInstanceFactory_h__Script_SGFToolsEditor_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsEditor_Private_Material_SGFToolsMaterialInstanceFactory_h__Script_SGFToolsEditor_1558305423{
	TEXT("/Script/SGFToolsEditor"),
	Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsEditor_Private_Material_SGFToolsMaterialInstanceFactory_h__Script_SGFToolsEditor_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsEditor_Private_Material_SGFToolsMaterialInstanceFactory_h__Script_SGFToolsEditor_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
