// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Material/SGFToolsMaterialInstance.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeSGFToolsMaterialInstance() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UMaterialInstanceConstant();
SGFTOOLSRUNTIME_API UClass* Z_Construct_UClass_USGFToolsMaterialInstance();
SGFTOOLSRUNTIME_API UClass* Z_Construct_UClass_USGFToolsMaterialInstance_NoRegister();
UPackage* Z_Construct_UPackage__Script_SGFToolsRuntime();
// ********** End Cross Module References **********************************************************

// ********** Begin Class USGFToolsMaterialInstance ************************************************
FClassRegistrationInfo Z_Registration_Info_UClass_USGFToolsMaterialInstance;
UClass* USGFToolsMaterialInstance::GetPrivateStaticClass()
{
	using TClass = USGFToolsMaterialInstance;
	if (!Z_Registration_Info_UClass_USGFToolsMaterialInstance.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("SGFToolsMaterialInstance"),
			Z_Registration_Info_UClass_USGFToolsMaterialInstance.InnerSingleton,
			StaticRegisterNativesUSGFToolsMaterialInstance,
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
	return Z_Registration_Info_UClass_USGFToolsMaterialInstance.InnerSingleton;
}
UClass* Z_Construct_UClass_USGFToolsMaterialInstance_NoRegister()
{
	return USGFToolsMaterialInstance::GetPrivateStaticClass();
}
struct Z_Construct_UClass_USGFToolsMaterialInstance_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \xe6\x9d\xbe\xe8\xb0\xb7\xe7\xa6\x8f\xe7\x89\xb9\xe6\x95\x88\xe9\x80\x9a \xe5\x8f\x82\xe6\x95\xb0\xe7\xae\xa1\xe7\x90\x86\xe5\x8a\x9f\xe8\x83\xbd,\xe5\x85\xb7\xe5\xa4\x87\xe6\x9b\xb4\xe9\xab\x98\xe7\xba\xa7\xe7\x9a\x84\xe5\x8f\x82\xe6\x95\xb0UI\xe7\x95\x8c\xe9\x9d\xa2\xe3\x80\x82\n *\n * \n * \xe7\xbb\xa7\xe6\x89\xbf\xe8\x87\xaa UMaterialInstanceConstant\xe3\x80\x82\n */" },
#endif
		{ "HideCategories", "Object Thumbnail" },
		{ "IncludePath", "Material/SGFToolsMaterialInstance.h" },
		{ "ModuleRelativePath", "Public/Material/SGFToolsMaterialInstance.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xe6\x9d\xbe\xe8\xb0\xb7\xe7\xa6\x8f\xe7\x89\xb9\xe6\x95\x88\xe9\x80\x9a \xe5\x8f\x82\xe6\x95\xb0\xe7\xae\xa1\xe7\x90\x86\xe5\x8a\x9f\xe8\x83\xbd,\xe5\x85\xb7\xe5\xa4\x87\xe6\x9b\xb4\xe9\xab\x98\xe7\xba\xa7\xe7\x9a\x84\xe5\x8f\x82\xe6\x95\xb0UI\xe7\x95\x8c\xe9\x9d\xa2\xe3\x80\x82\n\n\n\xe7\xbb\xa7\xe6\x89\xbf\xe8\x87\xaa UMaterialInstanceConstant\xe3\x80\x82" },
#endif
	};
#endif // WITH_METADATA

// ********** Begin Class USGFToolsMaterialInstance constinit property declarations ****************
// ********** End Class USGFToolsMaterialInstance constinit property declarations ******************
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USGFToolsMaterialInstance>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_USGFToolsMaterialInstance_Statics
UObject* (*const Z_Construct_UClass_USGFToolsMaterialInstance_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UMaterialInstanceConstant,
	(UObject* (*)())Z_Construct_UPackage__Script_SGFToolsRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_USGFToolsMaterialInstance_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_USGFToolsMaterialInstance_Statics::ClassParams = {
	&USGFToolsMaterialInstance::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x009020A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_USGFToolsMaterialInstance_Statics::Class_MetaDataParams), Z_Construct_UClass_USGFToolsMaterialInstance_Statics::Class_MetaDataParams)
};
void USGFToolsMaterialInstance::StaticRegisterNativesUSGFToolsMaterialInstance()
{
}
UClass* Z_Construct_UClass_USGFToolsMaterialInstance()
{
	if (!Z_Registration_Info_UClass_USGFToolsMaterialInstance.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_USGFToolsMaterialInstance.OuterSingleton, Z_Construct_UClass_USGFToolsMaterialInstance_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_USGFToolsMaterialInstance.OuterSingleton;
}
USGFToolsMaterialInstance::USGFToolsMaterialInstance() {}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, USGFToolsMaterialInstance);
USGFToolsMaterialInstance::~USGFToolsMaterialInstance() {}
// ********** End Class USGFToolsMaterialInstance **************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h__Script_SGFToolsRuntime_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_USGFToolsMaterialInstance, USGFToolsMaterialInstance::StaticClass, TEXT("USGFToolsMaterialInstance"), &Z_Registration_Info_UClass_USGFToolsMaterialInstance, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(USGFToolsMaterialInstance), 703492812U) },
	};
}; // Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h__Script_SGFToolsRuntime_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h__Script_SGFToolsRuntime_1223297663{
	TEXT("/Script/SGFToolsRuntime"),
	Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h__Script_SGFToolsRuntime_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h__Script_SGFToolsRuntime_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
