// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePakLoader_init() {}
	PAKLOADER_API UFunction* Z_Construct_UDelegateFunction_PakLoader_DownloadPakDelegate__DelegateSignature();
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_PakLoader;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_PakLoader()
	{
		if (!Z_Registration_Info_UPackage__Script_PakLoader.OuterSingleton)
		{
			static UObject* (*const SingletonFuncArray[])() = {
				(UObject* (*)())Z_Construct_UDelegateFunction_PakLoader_DownloadPakDelegate__DelegateSignature,
			};
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/PakLoader",
				SingletonFuncArray,
				UE_ARRAY_COUNT(SingletonFuncArray),
				PKG_CompiledIn | 0x00000000,
				0xF0916F6B,
				0x3C4432E9,
				METADATA_PARAMS(nullptr, 0)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_PakLoader.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_PakLoader.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_PakLoader(Z_Construct_UPackage__Script_PakLoader, TEXT("/Script/PakLoader"), Z_Registration_Info_UPackage__Script_PakLoader, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xF0916F6B, 0x3C4432E9));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
