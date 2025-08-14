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
}

/////////////////////////////////////////////////////
// UEdGraphNode_K2PostIt

void UK2PostItMarkdownBinding::OpenURL(FString URL)
{
	FPlatformProcess::LaunchURL( *URL, nullptr, nullptr );
}

void UK2PostItMarkdownBinding::OpenAsset(FString URL)
{
	//MarkdownAssetStatics::TryToOpenAsset(URL);
}

UEdGraphNode_K2PostIt::UEdGraphNode_K2PostIt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeWidth = 400;
	NodeHeight = 100;
	TitleFontSize = 18;
	ContentFontSize = 12;
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
}

FText UEdGraphNode_K2PostIt::GetTooltipText() const
{
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

void UEdGraphNode_K2PostIt::SetCommentText(const FText& Text)
{
	CommentText = Text;
	
	RichText = PeasantTextToRichText(Text);
}

void UEdGraphNode_K2PostIt::SetSelectionState(const ESelectionState InSelectionState)
{
	SelectionState = InSelectionState;
}

TArray<FText> UEdGraphNode_K2PostIt::PeasantTextToRichText(const FText& PeasantText)
{
	TArray<FString> Results;

	FString Raw = PeasantText.ToString();

	TArray<FString> Separated;

	// --- Separated Blocks ---
	// ---
	{
		FString RawTemp = Raw;
		
		const FRegexPattern Pattern( R"((?m)^---{1,}$)" , ERegexPatternFlags::CaseInsensitive);
		FRegexMatcher Matcher(Pattern, RawTemp);
		while (Matcher.FindNext())
		{
			FString Before = Raw.Left(Matcher.GetMatchBeginning());
			RawTemp = Raw.RightChop(Matcher.GetMatchEnding());

			Separated.Add(Before);
		}

		Separated.Add(RawTemp);
	}

	for (FString& Block : Separated)
	{
		// For some reason unreal's regex parser sucks ass and sometimes ^ refuses to catch the start of a line, so process these backwards

		// --- Headers ---
		// ### H3
		{
			const FRegexPattern Pattern( R"((?m)^### (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Header3>%s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		
		// ## H2
		{
			const FRegexPattern Pattern( R"((?m)^## (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Header2>%s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		
		// # H1
		{
			const FRegexPattern Pattern( R"((?m)^# (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Header1>%s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		
		// --- Other markdown ---
	    // Bold: **text**
	    {
	        const FRegexPattern Pattern(TEXT(R"(\*\*(.+?)\*\*)"));
	        FRegexMatcher Matcher(Pattern, Block);
	        while (Matcher.FindNext())
	        {
	            FString Inner = Matcher.GetCaptureGroup(1);
	            FString Replacement = FString::Printf(TEXT("<K2PostIt.Bold>%s</>"), *Inner);
	            Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
	            Matcher = FRegexMatcher(Pattern, Block);
	        }
	    }

		// Underline: __text__
		{
			const FRegexPattern Pattern(TEXT(R"(__(.+?)__)"));
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Underline>%s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		
	    // Italic: _text_
	    {
	        const FRegexPattern Pattern(TEXT(R"(_(.+?)_)"));
	        FRegexMatcher Matcher(Pattern, Block);
	        while (Matcher.FindNext())
	        {
	            FString Inner = Matcher.GetCaptureGroup(1);
	            FString Replacement = FString::Printf(TEXT("<K2PostIt.Italic>%s</>"), *Inner);
	            Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
	            Matcher = FRegexMatcher(Pattern, Block);
	        }
	    }
		
		// Bullet: -- WHITE CIRCLE
		{
			const FRegexPattern Pattern( R"((?m)^--- (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet2>             \u25CB %s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		
		// Bullet: - BLACK CIRCLE
		{
			const FRegexPattern Pattern( R"((?m)^-- (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet1>        \u25CF %s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		
		
		///*
		// Bullet: -- BLACK DIAMOND MINUS WHITE X
		{
			const FRegexPattern Pattern( R"((?m)^- (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet1>  \u2756 %s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		//*/
		
		/*
		// Bullet: - BLACK CIRCLE
		{
			const FRegexPattern Pattern( R"((?m)^- (.+)$)" , ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Block);
			while (Matcher.FindNext())
			{
				FString Inner = Matcher.GetCaptureGroup(1);
				FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet1> \u25CF %s</>"), *Inner);
				Block = Block.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
				Matcher = FRegexMatcher(Pattern, Block);
			}
		}
		*/
	}
	
	TArray<FText> ResultsText;

	for (const FString& Block : Separated)
	{
		ResultsText.Add(FText::FromString(Block));
	}
	
    return ResultsText;
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
