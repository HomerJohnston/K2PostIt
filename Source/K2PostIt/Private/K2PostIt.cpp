// Unlicensed. This file is public domain.

#include "K2PostIt.h"

#include "BlueprintEditorModule.h"
#include "K2PostIt/K2PostItCommands.h"
#include "K2PostIt/K2PostItStyle.h"
#include "Kismet2/DebuggerCommands.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "FK2PostIt"

// ------------------------------------------------------------------------------------------------

void FK2PostItModule::StartupModule()
{
	FK2PostItStyle::Initialize();

	FK2PostItCommands::Register();
}

// ------------------------------------------------------------------------------------------------

void FK2PostItModule::ShutdownModule()
{
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FK2PostItModule, K2PostIt)