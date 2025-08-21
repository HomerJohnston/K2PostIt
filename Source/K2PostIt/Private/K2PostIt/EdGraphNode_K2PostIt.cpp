// Copyright Epic Games, Inc. All Rights Reserved.

#include "K2PostIt/EdGraphNode_K2PostIt.h"

#include "GraphEditorSettings.h"
#include "Internationalization/Internationalization.h"
#include "K2PostIt/SGraphNodeK2PostIt.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "Layout/SlateRect.h"
#include "Math/UnrealMathSSE.h"
#include "Misc/AssertionMacros.h"
#include "Styling/AppStyle.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/UnrealType.h"

#include <regex>
#include <string>
#include <Widgets/Text/SRichTextBlock.h>

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2PostIt/K2PostItAsyncParser.h"
#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItDecorator_InlineCode.h"
#include "K2PostIt/K2PostItProjectSettings.h"
#include "K2PostIt/K2PostItStyle.h"
#include "K2PostIt/Globals/K2PostItConstants.h"
#include "K2PostIt/Globals/K2PostItFunctions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

class UEdGraphPin;

#define LOCTEXT_NAMESPACE "EdGraph"

namespace FEdGraphNode_K2PostIt_Utils
{
	template<typename T>
	void SyncPropertyToValue(UEdGraphNode_K2PostIt* InNode, FProperty* InProperty, const T& InValue)
	{
		if (InNode && InProperty)
		{
			T* CurrentValuePtr = InProperty->ContainerPtrToValuePtr<T>(InNode);
			if (*CurrentValuePtr != InValue)
			{
				*CurrentValuePtr = InValue;

				FPropertyChangedEvent PropertyChangedEvent(InProperty, EPropertyChangeType::Unspecified);
				InNode->PostEditChangeProperty(PropertyChangedEvent);
			}
		}
	}
};



/////////////////////////////////////////////////////
// UEdGraphNode_K2PostIt

UEdGraphNode_K2PostIt::UEdGraphNode_K2PostIt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeWidth = 400;
	NodeHeight = 100;
	TitleFontSize = 18;
	CommentColor = FLinearColor(1.0, 0.95, 0.66, 0.95);

	bColorCommentBubble = false;

	bCommentBubblePinned = true;
	bCommentBubbleVisible = true;
	bCommentBubbleVisible_InDetailsPanel = true;
	bCanResizeNode = true;
	bCanRenameNode = true;
	CommentDepth = -1;
}

void UEdGraphNode_K2PostIt::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) 
{
	UEdGraphNode_K2PostIt* This = CastChecked<UEdGraphNode_K2PostIt>(InThis);
	for (auto It = This->NodesUnderComment.CreateIterator(); It; ++It)
	{
		Collector.AddReferencedObject(*It, This);
	}

	Super::AddReferencedObjects(InThis, Collector);
}

void UEdGraphNode_K2PostIt::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UEdGraphNode_K2PostIt, bCommentBubbleVisible_InDetailsPanel))
	{
		bCommentBubbleVisible = bCommentBubbleVisible_InDetailsPanel;
		bCommentBubblePinned = bCommentBubbleVisible_InDetailsPanel;
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UEdGraphNode_K2PostIt::PostPlacedNewNode()
{
	const UClass* NodeClass = GetClass();
	const UGraphEditorSettings* GraphEditorSettings = GetDefault<UGraphEditorSettings>();

	// This is done here instead of in the constructor so we can later change the default for newly placed
	// instances without changing all of the existing ones (due to delta serialization)
	FEdGraphNode_K2PostIt_Utils::SyncPropertyToValue(this, NodeClass->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UEdGraphNode_K2PostIt, CommentColor)), GraphEditorSettings->DefaultCommentNodeTitleColor);
	FEdGraphNode_K2PostIt_Utils::SyncPropertyToValue(this, NodeClass->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UEdGraphNode_K2PostIt, bCommentBubbleVisible_InDetailsPanel)), GraphEditorSettings->bShowCommentBubbleWhenZoomedOut);

	NodeComment = NSLOCTEXT("K2Node", "CommentBlock_NewEmptyComment", "Comment").ToString();
	CommentColor = GetDefault<UEdGraphNode_K2PostIt>()->CommentColor; // For some reason Unreal keeps making the new instance white. Blah.

	FSlateApplication::Get().SetAllUserFocus(SNullWidget::NullWidget, EFocusCause::SetDirectly);
}

FText UEdGraphNode_K2PostIt::GetTooltipText() const
{
	if (NodeComment.IsEmpty())
	{
		return NSLOCTEXT("K2Node", "K2PostItCommentBlock_Tooltip", "Hold Shift+C and click to place");
	}
	if (CachedTooltip.IsOutOfDate(this))
	{
		CachedTooltip.SetCachedText(FText::Format(NSLOCTEXT("K2Node", "CommentBlock_Tooltip", "Comment:\n{0}"), FText::FromString(NodeComment)), this);
	}
	return CachedTooltip;
}

FString UEdGraphNode_K2PostIt::GetDocumentationLink() const
{
	return TEXT("Shared/GraphNodes/Common");
}

FString UEdGraphNode_K2PostIt::GetDocumentationExcerptName() const
{
	return TEXT("UEdGraphNode_K2PostIt");
}

FSlateIcon UEdGraphNode_K2PostIt::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FLinearColor::White;
	
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "Icons.Comment");
	return Icon;
}

TSharedPtr<SGraphNode> UEdGraphNode_K2PostIt::CreateVisualWidget()
{
	return SNew(SGraphNodeK2PostIt, this);
}

void UEdGraphNode_K2PostIt::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(UEdGraphNode_K2PostIt::StaticClass());

	check(NodeSpawner);

	auto CustomizeMessageNodeLambda = [] (UEdGraphNode* NewNode, bool bIsTemplateNode)
	{
		UEdGraph* OuterGraph = NewNode->GetGraph();
		check(OuterGraph != nullptr);
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(OuterGraph);
		check(Blueprint != nullptr);

		const float OldNodePosX = static_cast<float>(NewNode->NodePosX);
		const float OldNodePosY = static_cast<float>(NewNode->NodePosY);
		const float OldHalfHeight = NewNode->NodeHeight / 2.f;
		const float OldHalfWidth  = NewNode->NodeWidth  / 2.f;
		
		static const float DocNodePadding = 50.0f;
		FSlateRect Bounds(OldNodePosX - OldHalfWidth, OldNodePosY - OldHalfHeight, OldNodePosX + OldHalfWidth, OldNodePosY + OldHalfHeight);
		FKismetEditorUtilities::GetBoundsForSelectedNodes(Blueprint, Bounds, DocNodePadding);
	};

	NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeMessageNodeLambda);

	NodeSpawner->DefaultMenuSignature.MenuName = INVTEXT("Add Comment (K2PostIt)...");

	UClass* ActionKey = GetClass();
	
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UEdGraphNode_K2PostIt::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if(TitleType == ENodeTitleType::MenuTitle)
	{
		return NSLOCTEXT("K2Node", "NoComment_ListTitle", "Add Comment...");
	}
	else if(TitleType == ENodeTitleType::ListView)
	{
		return NSLOCTEXT("K2Node", "CommentBlock_ListTitle", "Comment");	
	}

	return FText::FromString(NodeComment);
}

FText UEdGraphNode_K2PostIt::GetPinNameOverride(const UEdGraphPin& Pin) const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

FLinearColor UEdGraphNode_K2PostIt::GetNodeCommentColor() const
{
	// Only affects the 'zoomed out' comment bubble color, not the box itself
	return (bColorCommentBubble)
		? CommentColor 
		: FLinearColor::White;
}

void UEdGraphNode_K2PostIt::ResizeNode(const FVector2D& NewSize)
{
	if (bCanResizeNode) 
	{
		NodeHeight = UE::LWC::FloatToIntCastChecked<int32>(NewSize.Y);
		NodeWidth = UE::LWC::FloatToIntCastChecked<int32>(NewSize.X);
	}
}

void UEdGraphNode_K2PostIt::OnRenameNode(const FString& NewName)
{
	NodeComment = NewName;
	CachedTooltip.MarkDirty();
}

TSharedPtr<class INameValidatorInterface> UEdGraphNode_K2PostIt::MakeNameValidator() const
{
	// Comments can be duplicated, etc...
	return MakeShareable(new FDummyNameValidator(EValidatorResult::Ok));
}

bool UEdGraphNode_K2PostIt::IsSelectedInEditor() const
{
	if (SelectionState == ESelectionState::Inherited)
	{
		return Super::IsSelectedInEditor();
	}
	
	return SelectionState == ESelectionState::Selected;
}

void UEdGraphNode_K2PostIt::PostLoad()
{
	Super::PostLoad();
}

void UEdGraphNode_K2PostIt::SetCommentText(const FText& Text)
{
	CommentText = Text;

	if (ActiveParser.IsValid())
	{
		QueuedParser = MakeShared<FK2PostItAsyncParser>(Text.ToString());
		QueuedParser->OnParseComplete.AddUObject(this, &ThisClass::OnParseComplete);
	}
	else
	{
		ActiveParser = MakeShared<FK2PostItAsyncParser>(Text.ToString());
		ActiveParser->OnParseComplete.AddUObject(this, &ThisClass::OnParseComplete);
		
		ActiveParser->RunParser();	
	}
}

void UEdGraphNode_K2PostIt::SetSelectionState(const ESelectionState InSelectionState)
{
	SelectionState = InSelectionState;
}

void UEdGraphNode_K2PostIt::OnParseComplete(TArray<TInstancedStruct<FK2PostIt_BaseBlock>> NewBlocks)
{
	Blocks = NewBlocks;

	for (TInstancedStruct<FK2PostIt_BaseBlock>& Block : Blocks)
	{
		FK2PostIt_BaseBlock& BlockInstance = Block.GetMutable<FK2PostIt_BaseBlock>();
		
		BlockInstance.SetOwnerNode(this);
	}
	
	ActiveParser = nullptr;

	OnParseCompleteEvent.Broadcast();

	if (QueuedParser.IsValid())
	{
		ActiveParser = QueuedParser;
		ActiveParser->RunParser();

		QueuedParser = nullptr;
	}
}



/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
