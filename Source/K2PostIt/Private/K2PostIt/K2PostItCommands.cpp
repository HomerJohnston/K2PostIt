// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItCommandsImpl.h"

#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/UICommandInfo.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/PlatformCrt.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ------------------------------------------------------------------------------------------------

void FK2PostItCommandsImpl::RegisterCommands()
{
	UI_COMMAND( CreateComment, "Create Comment", "Create a comment box", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Shift, EKeys::C))
}

// ------------------------------------------------------------------------------------------------

void FK2PostItCommands::Register()
{
	return FK2PostItCommandsImpl::Register();
}

// ------------------------------------------------------------------------------------------------

const FK2PostItCommandsImpl& FK2PostItCommands::Get()
{
	return FK2PostItCommandsImpl::Get();
}

// ------------------------------------------------------------------------------------------------

void FK2PostItCommands::BuildFindReferencesMenu(FMenuBuilder& MenuBuilder)
{

}

// ------------------------------------------------------------------------------------------------

void FK2PostItCommands::Unregister()
{
	return FK2PostItCommandsImpl::Unregister();
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
