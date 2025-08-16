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

#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItDecorator_InlineCode.h"
#include "K2PostIt/K2PostItStyle.h"

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

void UK2PostItMarkdownBinding::OpenURL(FString URL)
{
	FPlatformProcess::LaunchURL( *URL, nullptr, nullptr );
}

void UK2PostItMarkdownBinding::OpenAsset(FString URL)
{
	//MarkdownAssetStatics::TryToOpenAsset(URL);
}

TSharedPtr<SWidget> FK2PostIt_TextBlock::Draw() const
{
	return SNew(SRichTextBlock)
	.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Normal)
	.DecoratorStyleSet( &FK2PostItStyle::Get() )
	.Text(FText::FromString(Text))
	.LineHeightPercentage(1.1f)
	.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
	.AutoWrapText(true)
	+ SRichTextBlock::Decorator(FK2PostItDecorator_InlineCode::Create("K2PostIt.Code", FLinearColor::Blue));
}

TSharedPtr<SWidget> FK2PostIt_SeparatorBlock::Draw() const
{
	return SNew(SBox)
	.HAlign(HAlign_Fill)
	.Padding(12, 12, 12, 12)
	[
		SNew(SSeparator)
		.Thickness(2)
		.ColorAndOpacity(K2PostItColor::LightGray_Glass)
	];
}

TSharedPtr<SWidget> FK2PostIt_CodeBlock::Draw() const
{
	return SNew(SBox)
	.Padding(0, 4, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ErrorReporting.EmptyBox"))
		.Padding(8, 8)
		[
			SNew(SRichTextBlock)
			.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_CodeBlock)
			.DecoratorStyleSet( &FK2PostItStyle::Get() )
			.Text(FText::FromString(Text))
			.LineHeightPercentage(1.1f)
			.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
			.AutoWrapText(true)	
		]
	];
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

void UEdGraphNode_K2PostIt::PostLoad()
{
	Super::PostLoad();

	if (!IsTemplate())
	{
		GetMutableDefault<URegexTester>()->OnRegexPatternUpdated.AddDynamic(this, &ThisClass::UpdateShitPlease);
	}
}

void UEdGraphNode_K2PostIt::SetCommentText(const FText& Text)
{
	CommentText = Text;
	
	PeasantTextToRichText(CommentText);
}

void UEdGraphNode_K2PostIt::SetSelectionState(const ESelectionState InSelectionState)
{
	SelectionState = InSelectionState;
}

void UEdGraphNode_K2PostIt::PeasantTextToRichText(const FText& PeasantText)
{
	Blocks.Empty();

	FString RawBlock = PeasantText.ToString();

	// ==========================================
	// The more difficult stuff. Break up into chunks. We do this first because it's annoying to find `Stuff` without wrecking ```Stuff``` (but I should fix this one day)

	// --- Separator
	{
		const FRegexPattern Pattern( R"((?m)(\r?\n)?^---{1,}$(\r?\n)?)" , ERegexPatternFlags::CaseInsensitive);
		FRegexMatcher Matcher(Pattern, RawBlock);
		int32 RunningIndex = 0;
		
		while (Matcher.FindNext())
		{
			if (Matcher.GetMatchBeginning() > 0)
			{
				FString Before = RawBlock.Mid(RunningIndex, Matcher.GetMatchBeginning() - RunningIndex);

				Blocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(Before));

				RunningIndex = Matcher.GetMatchEnding();	
			}

			Blocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_SeparatorBlock>());
		}

		if (RunningIndex < RawBlock.Len() - 1)
		{
			Blocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(RawBlock.RightChop(RunningIndex)));
		}
	}
	
	for (int32 i = 0; i < Blocks.Num(); ++i)
	{
		TInstancedStruct<FK2PostIt_BaseBlock>& TempBlock = Blocks[i];
		
		if (TempBlock.GetScriptStruct() == FK2PostIt_TextBlock::StaticStruct())
		{
			const FK2PostIt_TextBlock* TextBlock = TempBlock.GetPtr<FK2PostIt_TextBlock>();

			const FString& Text = TextBlock->GetText();

			//const FRegexPattern Pattern( R"((\r?\n)?```(?:\w+)?\n([\s\S]*?)\n```(\r?\n)?)" , ERegexPatternFlags::CaseInsensitive);
			//const FRegexPattern Pattern( R"(`{3}[\s\S]*?`{3})" , ERegexPatternFlags::CaseInsensitive);
			//const FRegexPattern Pattern( R"((?<=[^`]|^)(```?)([^`]+)\1(?=[^`]|$))" , ERegexPatternFlags::CaseInsensitive);
			const FRegexPattern Pattern( R"((?<=[^`]|^)(\r?\n```)(?:\r?\n)?([\s\S]*?)(?:\r?\n)?\1(?=[^`]|$)(\r?\n)?)" , ERegexPatternFlags::CaseInsensitive);

			const class URegexTester* RegexTester = GetDefault<URegexTester>();

			//const FRegexPattern Pattern( RegexTester->RegexPattern , ERegexPatternFlags::CaseInsensitive);
			
			
			FRegexMatcher Matcher(Pattern, Text);
			int32 RunningIndex = 0;

			TArray<TInstancedStruct<FK2PostIt_BaseBlock>> ReplacementBlocks;
			
			while (Matcher.FindNext())
			{
				if (Matcher.GetMatchBeginning() > 0)
				{
					FString Before = Text.Mid(RunningIndex, Matcher.GetMatchBeginning() - RunningIndex);
					ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(Before));

				}

				FString Code = Matcher.GetCaptureGroup(2);
				ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_CodeBlock>(Code));

				RunningIndex = Matcher.GetMatchEnding();	
			}

			if (RunningIndex < Text.Len() - 1)
			{
				FString After = Text.RightChop(RunningIndex);
				ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(After));
			}

			if (ReplacementBlocks.Num() > 0)
			{
				Blocks.RemoveAt(i);
				Blocks.Insert(ReplacementBlocks, i);
				i += ReplacementBlocks.Num() - 1;
			}
		}
	}
	
	for (int32 i = 0; i < Blocks.Num(); ++i)
	{
		TInstancedStruct<FK2PostIt_BaseBlock>& TempBlock = Blocks[i];

		if (TempBlock.GetScriptStruct() == FK2PostIt_TextBlock::StaticStruct())
		{
			FK2PostIt_TextBlock& TextBlock = TempBlock.GetMutable<FK2PostIt_TextBlock>();

			FString& Text = TextBlock.GetText();
			
			// ### H3
			{
				const FRegexPattern Pattern( R"((?m)^### (.+)$)" , ERegexPatternFlags::CaseInsensitive);
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Header3>%s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
			// ## H2
			{
				const FRegexPattern Pattern( R"((?m)^## (.+)$)" , ERegexPatternFlags::CaseInsensitive);
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Header2>%s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
			// # H1
			{
				const FRegexPattern Pattern( R"((?m)^# (.+)$)" , ERegexPatternFlags::CaseInsensitive);
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Header1>%s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
		    // **text** Bold
		    {
		        const FRegexPattern Pattern(TEXT(R"((?m)\*\*(.+?)\*\*)"));
		        FRegexMatcher Matcher(Pattern, Text);
		        while (Matcher.FindNext())
		        {
		            FString Inner = Matcher.GetCaptureGroup(1);
		            FString Replacement = FString::Printf(TEXT("<K2PostIt.Bold>%s</>"), *Inner);
		            Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
		            Matcher = FRegexMatcher(Pattern, Text);
		        }
		    }

			// __text__ Underline
			{
				const FRegexPattern Pattern(TEXT(R"((?m)__(.+?)__)"));
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Underline>%s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
		    // _text_ Italic
		    {
		        const FRegexPattern Pattern(TEXT(R"((?m)_(.+?)_)"));
		        FRegexMatcher Matcher(Pattern, Text);
		        while (Matcher.FindNext())
		        {
		            FString Inner = Matcher.GetCaptureGroup(1);
		            FString Replacement = FString::Printf(TEXT("<K2PostIt.Italic>%s</>"), *Inner);
		            Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
		            Matcher = FRegexMatcher(Pattern, Text);
		        }
		    }
			
			// `text` Code
			{
				const FRegexPattern Pattern(TEXT(R"((?m)`(.+?)`)"));
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Code>%s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
			//   - 3rd Bullet (white circle)
			{
				const FRegexPattern Pattern( R"((?m)^  - (.+)$)" , ERegexPatternFlags::CaseInsensitive);
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet2>             \u25CB %s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
			//  - 2nd Bullet (black circle)
			{
				const FRegexPattern Pattern( R"((?m)^ - (.+)$)" , ERegexPatternFlags::CaseInsensitive);
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet1>        \u25CF %s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
			
			// - 1st Bullet (black diamond minus white X)
			{
				const FRegexPattern Pattern( R"((?m)^- (.+)$)" , ERegexPatternFlags::CaseInsensitive);
				FRegexMatcher Matcher(Pattern, Text);
				while (Matcher.FindNext())
				{
					FString Inner = Matcher.GetCaptureGroup(1);
					FString Replacement = FString::Printf(TEXT("<K2PostIt.Bullet1>  \u2756 %s</>"), *Inner);
					Text = Text.Replace(*Matcher.GetCaptureGroup(0), *Replacement, ESearchCase::IgnoreCase);
					Matcher = FRegexMatcher(Pattern, Text);
				}
			}
		}
	}
}

void UEdGraphNode_K2PostIt::UpdateShitPlease()
{
	PeasantTextToRichText(CommentText);
	ReconstructNode();
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
