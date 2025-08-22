// Unlicensed. This file is public domain.

#include "K2PostIt.h"

#include "BlueprintEditorModule.h"
#include "K2PostIt/K2PostItCommandsImpl.h"
#include "K2PostIt/K2PostItStyle.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "FK2PostItModule"

void FK2PostItModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FK2PostItStyle::Initialize();
	FK2PostItStyle::ReloadTextures();
	
	PluginCommands = MakeShareable(new FUICommandList);
	
	FK2PostItCommands::Register();
	
	PluginCommands->MapAction(
		FK2PostItCommandsImpl::Get().CreateComment,
		FExecuteAction::CreateStatic(&FK2PostItCommands::OnCreateComment),
		FCanExecuteAction());


	//FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");

	TSharedPtr<FExtender> Extender = MakeShareable(new FExtender);

	Extender->AddMenuExtension("BlueprintEditor", EExtensionHook::First, PluginCommands, FMenuExtensionDelegate());

	//BlueprintEditorModule.GetMenuExtensibilityManager()->AddExtender(Extender);
//	BlueprintEditorModule.GetToolBarExtensibilityManager()

	const FString ConfigSection = TEXT("BlueprintSpawnNodes");
	const FString SettingName = TEXT("Node");
	TArray< FString > NodeSpawns;
	GConfig->GetArray(*ConfigSection, *SettingName, NodeSpawns, GEditorPerProjectIni);

	const FString DefaultSettingData = TEXT("(Class=/Script/K2PostIt.EdGraphNode_K2PostIt Key=C Shift=true Ctrl=false Alt=false)");
	
	if (!NodeSpawns.Contains(DefaultSettingData))
	{
		NodeSpawns.Add("(Class=/Script/K2PostIt.EdGraphNode_K2PostIt Key=C Shift=true Ctrl=false Alt=false)");		
		GConfig->SetArray(TEXT("BlueprintSpawnNodes"), TEXT("Node"), NodeSpawns, GEditorPerProjectIni);
	}
}

void FK2PostItModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FK2PostItModule, K2PostIt)