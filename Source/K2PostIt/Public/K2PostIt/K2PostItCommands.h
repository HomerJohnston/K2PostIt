// Unlicensed. This file is public domain.

#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/Platform.h"
#include "Internationalization/Internationalization.h"
#include "Styling/AppStyle.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"

class FUICommandInfo;

class FK2PostItCommandsImpl : public TCommands<FK2PostItCommandsImpl>
{
public:

	FK2PostItCommandsImpl()
		: TCommands<FK2PostItCommandsImpl>( TEXT("K2PostIt"), NSLOCTEXT("Contexts", "K2PostIt", "K2 PostIt"), NAME_None, FAppStyle::GetAppStyleSetName() )
	{
	}

	virtual ~FK2PostItCommandsImpl()
	{
	}

	void RegisterCommands() override;

	// Create a comment node
	TSharedPtr< FUICommandInfo > CreateComment;
};

class FK2PostItCommands
{
public:
	static void Register();

	static const FK2PostItCommandsImpl& Get();
	
	/** Build "Find References" submenu when a context allows for it */
	static void BuildFindReferencesMenu(FMenuBuilder& MenuBuilder);

	static void Unregister();

	static void OnCreateComment();
};
