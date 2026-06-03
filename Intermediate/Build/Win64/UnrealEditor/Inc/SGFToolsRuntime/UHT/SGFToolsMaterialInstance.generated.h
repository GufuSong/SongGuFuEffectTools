// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Material/SGFToolsMaterialInstance.h"

#ifdef SGFTOOLSRUNTIME_SGFToolsMaterialInstance_generated_h
#error "SGFToolsMaterialInstance.generated.h already included, missing '#pragma once' in SGFToolsMaterialInstance.h"
#endif
#define SGFTOOLSRUNTIME_SGFToolsMaterialInstance_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class USGFToolsMaterialInstance ************************************************
struct Z_Construct_UClass_USGFToolsMaterialInstance_Statics;
SGFTOOLSRUNTIME_API UClass* Z_Construct_UClass_USGFToolsMaterialInstance_NoRegister();

#define FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h_18_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSGFToolsMaterialInstance(); \
	friend struct ::Z_Construct_UClass_USGFToolsMaterialInstance_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend SGFTOOLSRUNTIME_API UClass* ::Z_Construct_UClass_USGFToolsMaterialInstance_NoRegister(); \
public: \
	DECLARE_CLASS2(USGFToolsMaterialInstance, UMaterialInstanceConstant, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SGFToolsRuntime"), Z_Construct_UClass_USGFToolsMaterialInstance_NoRegister) \
	DECLARE_SERIALIZER(USGFToolsMaterialInstance)


#define FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h_18_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USGFToolsMaterialInstance(); \
	/** Deleted move- and copy-constructors, should never be used */ \
	USGFToolsMaterialInstance(USGFToolsMaterialInstance&&) = delete; \
	USGFToolsMaterialInstance(const USGFToolsMaterialInstance&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USGFToolsMaterialInstance); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USGFToolsMaterialInstance); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(USGFToolsMaterialInstance) \
	NO_API virtual ~USGFToolsMaterialInstance();


#define FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h_15_PROLOG
#define FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h_18_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h_18_INCLASS_NO_PURE_DECLS \
	FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h_18_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class USGFToolsMaterialInstance;

// ********** End Class USGFToolsMaterialInstance **************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_SongGuFuTools_Plugins_SGFTools_Source_SGFToolsRuntime_Public_Material_SGFToolsMaterialInstance_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
