// Copyright Epic Games, Inc. All Rights Reserved.

#include "K2PostIt.h"

#include "K2PostIt/K2PostItStyle.h"

#define LOCTEXT_NAMESPACE "FK2PostItModule"

void FK2PostItModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FK2PostItStyle::Initialize();
	FK2PostItStyle::ReloadTextures();
}

void FK2PostItModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FK2PostItModule, K2PostIt)