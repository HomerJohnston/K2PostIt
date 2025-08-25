// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItCommands.h"

#include "BlueprintEditorModule.h"
#include "EdGraphSchema_K2.h"
#include "GraphEditor.h"
#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/UICommandInfo.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/PlatformCrt.h"
#include "InputCoreTypes.h"
#include "ScopedTransaction.h"
#include "EdGraph/EdGraph.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Text.h"
#include "K2PostIt/Nodes/EdGraphNode_K2PostIt.h"
#include "Kismet2/DebuggerCommands.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

void FK2PostItCommandsImpl::RegisterCommands()
{
	UI_COMMAND( CreateComment, "Create Comment", "Create a comment box", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::C))
	
	FPlayWorldCommands::GlobalPlayWorldActions->MapAction(
		CreateComment,
		FExecuteAction::CreateStatic(&FK2PostItCommands::OnCreateComment));
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

void FK2PostItCommands::OnCreateComment()
{
	TSharedPtr<SWidget> FocusedWidget = FSlateApplication::Get().GetUserFocusedWidget(0);

	if (!FocusedWidget.IsValid())
	{
		return;
	}

	// Try to find out if we're inside of a blueprint graph and the graph itself is of focus
	FWidgetPath WidgetPath;
	
	if (FSlateApplication::Get().GeneratePathToWidgetUnchecked(FocusedWidget.ToSharedRef(), WidgetPath))
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("-------------"));
		
		for (int32 i = 0; i < WidgetPath.Widgets.Num(); ++i)
		{
			const FArrangedWidget& Arranged = WidgetPath.Widgets[i];

			UE_LOG(LogTemp, VeryVerbose, TEXT("%s"), *Arranged.Widget->GetType().ToString());
			
			TSharedPtr<SGraphEditor> GraphEditor = StaticCastSharedPtr<SGraphEditor>(Arranged.Widget.ToSharedPtr());
			
			if (GraphEditor.IsValid() && GraphEditor->GetType() == FName("SGraphEditor")) // StaticCastSharedPtr does not handle type safety but will at least handle a null; we need to actually check the type
			{
				UEdGraph* Graph = GraphEditor->GetCurrentGraph();

				if (IsValid(Graph))
				{
					const class UEdGraphSchema* Schema = Graph->GetSchema();

					if (Schema && Schema->IsA(UEdGraphSchema_K2::StaticClass()))
					{
						UE_LOG(LogTemp, VeryVerbose, TEXT("Found graph, schema: %s"), *Schema->GetName());
						
						FVector2D Position = GraphEditor->GetPasteLocation();

						const FScopedTransaction Transaction(NSLOCTEXT("K2PostIt", "AddNode", "Add Node"));
						Graph->Modify();

						EObjectFlags Flags = RF_Transactional;
						
						UEdGraphNode_K2PostIt* NewNode = NewObject<UEdGraphNode_K2PostIt>(Graph, UEdGraphNode_K2PostIt::StaticClass(), NAME_None, Flags);
						Graph->AddNode(NewNode, true, true);

						NewNode->NodePosX = Position.X;
						NewNode->NodePosY = Position.Y;

						NewNode->AllocateDefaultPins();
						NewNode->ReconstructNode();

						NewNode->PostPlacedNewNode();
					}

					break;
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
