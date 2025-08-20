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

void FK2PostIt_BaseBlock::SetParentWidget(TSharedPtr<SGraphNodeK2PostIt> GraphNodeK2PostIt)
{
	OwnerWidget = GraphNodeK2PostIt.ToWeakPtr();
}

TSharedPtr<SWidget> FK2PostIt_TextBlock::Draw() const
{
	return SNew(SBorder)
	.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.None))
	.Padding(0)
	.ForegroundColor_Lambda( [this] ()
	{
		FLinearColor Color = K2PostItColor::Noir;

		if (Owner.IsValid())
		{
			Color = K2PostItColor::GetNominalFontColor(Owner->CommentColor, K2PostItColor::White, K2PostItColor::Noir);
		}
		
		return Color;
	})
	[
		SNew(SRichTextBlock)
		.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Normal)
		.DecoratorStyleSet( &FK2PostItStyle::Get() )
		.Text(FText::FromString(Text))
		.LineHeightPercentage(K2PostIt::Constants::MarkdownPanelLineHeightSpacing)
		.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
		.WrapTextAt_Lambda( [this] ()
		{
			if (OwnerWidget.IsValid())
			{
				return (float)(OwnerWidget.Pin()->GetWrapAt());
			}
			return -1.0f;
		})
		+ SRichTextBlock::Decorator(FK2PostItDecorator_InlineCode::Create("K2PostIt.Code", Owner.Get()))
		+ SRichTextBlock::Decorator(SRichTextBlock::HyperlinkDecorator("browser", FSlateHyperlinkRun::FOnClick::CreateStatic(&K2PostIt::OnBrowserLinkClicked)))
	];
}

TSharedPtr<SWidget> FK2PostIt_SeparatorBlock::Draw() const
{
	return SNew(SBox)
	.HAlign(HAlign_Fill)
	.Padding(K2PostIt::Constants::Separator_SidePadding, K2PostIt::Constants::Separator_TopPadding, K2PostIt::Constants::Separator_SidePadding, K2PostIt::Constants::Separator_BottomPadding)
	[
		SNew(SSeparator)
		.Thickness(2)
		.SeparatorImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.Separator))
		.ColorAndOpacity_Lambda( [this] ()
		{
			if (Owner.IsValid())
			{
				if (Owner->CommentColor.GetLuminance() < K2PostIt::Constants::LuminanceDarkModeThreshold)
				{
					return K2PostItColor::DimWhite_SemiGlass;
				}
			}

			return K2PostItColor::Noir_SemiGlass;
		})
	];
}

TSharedPtr<SWidget> FK2PostIt_CodeBlock::Draw() const
{
	return SNew(SBox)
	.Padding(0, K2PostIt::Constants::CodeBlock_TopPadding, 0, K2PostIt::Constants::CodeBlock_BottomPadding)
	[
		// TODO this duplicates some code with K2PostItDecorator_InlineCode, I should pull out into something common
		SNew(SBorder)
		.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.CodeHighlightFill))
		.ForegroundColor_Lambda( [this] ()
		{
			FLinearColor Color = K2PostItColor::Noir;
	
			if (Owner.IsValid())
			{
				Color = K2PostItColor::GetNominalFontColor(Owner->CommentColor, K2PostItColor::White, K2PostItColor::Noir);
			}
			
			return Color;
		})
		.BorderBackgroundColor_Lambda( [this] ()
		{
			FLinearColor Color = K2PostItColor::White;
	
			if (Owner.IsValid())
			{
				float Alpha = Color.A;
				Color = Owner->CommentColor * K2PostIt::Constants::CodeBlock_BorderBackgroundColorMulti;
				Color.A = Alpha;
			}
			
			return Color;
		})
		.Padding(0)
		[
			SNew(SBorder)
			.Padding(K2PostIt::Constants::CodeBlock_InternalPadding)
			.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.CodeHighlightBorder))
			.BorderBackgroundColor_Lambda( [this] ()
			{
				FLinearColor Color = K2PostItColor::White;
	
				if (Owner.IsValid())
				{
					float Lum = Owner->CommentColor.GetLuminance() + K2PostIt::Constants::CodeBlock_BorderBrighten;
					Color = FLinearColor(Lum, Lum, Lum, 1.0f);
				}
			
				return Color;
			})
			[
				SNew(SRichTextBlock)
				.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_CodeBlock)
				.DecoratorStyleSet( &FK2PostItStyle::Get() )
				.Text(FText::FromString(Text))
				.LineHeightPercentage(K2PostIt::Constants::MarkdownPanelLineHeightSpacing)
				.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
				.WrapTextAt_Lambda( [this] ()
				{
					if (OwnerWidget.IsValid())
					{
						return (float)(OwnerWidget.Pin()->GetWrapAt());
					}
					return -1.0f;
				})
			]	
		]
	];
}

TSharedPtr<SWidget> FK2PostIt_BulletBlock::Draw() const
{
	const FString Bullets[3] { TEXT("\u2756"), TEXT("\u25CF"), TEXT("\u25CB")};
	const float IndentFactor = K2PostIt::Constants::BulletIndentFactor;
	const int32 SpacesPerIndent = 2;
	
	int32 EffectiveIndentLevel = FMath::Clamp(IndentLevel / SpacesPerIndent, 0, 2);

	const float TotalIndent = IndentFactor * EffectiveIndentLevel;
	
	return SNew(SBorder)
	.Padding(TotalIndent, 4, 0, 4)
	.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.None))
	.ForegroundColor_Lambda( [this] ()
	{
		FLinearColor Color = K2PostItColor::Noir;

		if (Owner.IsValid())
		{
			Color = K2PostItColor::GetNominalFontColor(Owner->CommentColor, K2PostItColor::DimWhite, K2PostItColor::DeepGray);
		}
	
		return Color;
	})
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 0, 0)
		[
			SNew(SBox)
			.WidthOverride(K2PostIt::Constants::BulletSymbolWidth)
			.HAlign(HAlign_Left)
			[
				SNew(SRichTextBlock)
				.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Normal)
				.DecoratorStyleSet( &FK2PostItStyle::Get() )
				.Text(FText::FromString(Bullets[EffectiveIndentLevel]))
				.LineHeightPercentage(K2PostIt::Constants::MarkdownPanelLineHeightSpacing)
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SRichTextBlock)
			.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Normal)
			.DecoratorStyleSet( &FK2PostItStyle::Get() )
			.Text(FText::FromString(Text))
			.LineHeightPercentage(K2PostIt::Constants::MarkdownPanelLineHeightSpacing)
			.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
			.WrapTextAt_Lambda( [this, TotalIndent] ()
			{
				if (OwnerWidget.IsValid())
				{
					return (float)(OwnerWidget.Pin()->GetWrapAt() - TotalIndent - K2PostIt::Constants::BulletSymbolWidth);
				}
				return -1.0f;
			})
			+ SRichTextBlock::Decorator(FK2PostItDecorator_InlineCode::Create("K2PostIt.Code", Owner.Get()))
			+ SRichTextBlock::Decorator(SRichTextBlock::HyperlinkDecorator("browser", FSlateHyperlinkRun::FOnClick::CreateStatic(&K2PostIt::OnBrowserLinkClicked)))
		]
	];
}

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
	
	PeasantTextToRichText(CommentText);
}

void UEdGraphNode_K2PostIt::SetSelectionState(const ESelectionState InSelectionState)
{
	SelectionState = InSelectionState;
}

void UEdGraphNode_K2PostIt::ProcessTextBlocks(FString RegexPattern, SomeFunc F)
{
	for (int32 i = 0; i < Blocks.Num(); ++i)
	{
		TInstancedStruct<FK2PostIt_BaseBlock>& CurrentBlock = Blocks[i];
		
		if (CurrentBlock.GetScriptStruct() == FK2PostIt_TextBlock::StaticStruct())
		{
			const FK2PostIt_TextBlock* TextBlock = CurrentBlock.GetPtr<FK2PostIt_TextBlock>();

			const FString& Text = TextBlock->GetText();
			
			TArray<TInstancedStruct<FK2PostIt_BaseBlock>> ReplacementBlocks;
			
			int32 RunningIndex = 0;

			const FRegexPattern Pattern(RegexPattern, ERegexPatternFlags::CaseInsensitive);
			FRegexMatcher Matcher(Pattern, Text);

			while (Matcher.FindNext())
			{
				if (Matcher.GetMatchBeginning() > 0)
				{
					FString Before = Text.Mid(RunningIndex, Matcher.GetMatchBeginning() - RunningIndex);

					if (Before.Len() > 0)
					{
						ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(this, Before));
					}
				}

				F(Matcher, ReplacementBlocks);

				RunningIndex = Matcher.GetMatchEnding();
			}

			if (RunningIndex < Text.Len())
			{
				FString After = Text.RightChop(RunningIndex);

				if (After.Len() > 0)
				{
					ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(this, After));					
				}
			}

			if (ReplacementBlocks.Num() > 0)
			{
				Blocks.RemoveAt(i, EAllowShrinking::No);
				Blocks.Insert(ReplacementBlocks, i);
				i += ReplacementBlocks.Num() - 1;
			}
		}
	}
}


void UEdGraphNode_K2PostIt::PeasantTextToRichText(const FText& PeasantText)
{
	Blocks.Empty();

	Blocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_TextBlock>(this, PeasantText.ToString()));

	
	FString SeparatorBlockRegex = R"((?m)(\r?\n)?^---{1,}$(\r?\n)?)";
	SomeFunc SeparatorBlockParser = [this] (FRegexMatcher& Matcher, TArray<TInstancedStruct<FK2PostIt_BaseBlock>>& ReplacementBlocks)
	{
		ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_SeparatorBlock>(this));
	};
	ProcessTextBlocks(SeparatorBlockRegex, SeparatorBlockParser);

	
	FString CodeBlockRegex = R"((?m)(?<=[^`]|^)((?:\r?\n)?```)(?:.*)?(\r?\n)?([\s\S]*?)(?:\r?\n)?\1[ \t]*(?:\r?\n)?)";
	SomeFunc CodeBlockParser = [this] (FRegexMatcher& Matcher, TArray<TInstancedStruct<FK2PostIt_BaseBlock>>& ReplacementBlocks)
	{
		FString Code = Matcher.GetCaptureGroup(3);
		ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_CodeBlock>(this, Code));
	};
	ProcessTextBlocks(CodeBlockRegex, CodeBlockParser);


	FString BulletBlockRegex = R"((?m)(?:\r?\n)?^( {0}| {2}| {4})(-)\s(.*)$(?:\r?\n)?)";
	SomeFunc BulletBlockParser = [this] (FRegexMatcher& Matcher, TArray<TInstancedStruct<FK2PostIt_BaseBlock>>& ReplacementBlocks)
	{
		int32 HyphenCount = Matcher.GetCaptureGroup(1).Len();

		FString BulletText = Matcher.GetCaptureGroup(3);
		
		ReplacementBlocks.Add(TInstancedStruct<FK2PostIt_BaseBlock>::Make<FK2PostIt_BulletBlock>(this, HyphenCount, BulletText));
	};
	ProcessTextBlocks(BulletBlockRegex, BulletBlockParser);

	
	// Now process simpler inline markups
	
	for (int32 i = 0; i < Blocks.Num(); ++i)
	{
		TInstancedStruct<FK2PostIt_BaseBlock>& CurrentBlock = Blocks[i];

		if (CurrentBlock.GetScriptStruct()->IsChildOf(FK2PostIt_TextBlock::StaticStruct()))
		{
			FK2PostIt_TextBlock& TextBlock = CurrentBlock.GetMutable<FK2PostIt_TextBlock>();
			FString& Text = TextBlock.GetText();
			
			// Start off with one chunk. Then we'll process it with the rules.
			// Each rule will process the whole set from start to finish.
			// Any time any rule finds a match, we'll split that chunk up into start/middle/end and then keep going, starting from the new end.
			// The middle replaced section will become marked as "parsed" and skipped by future parsers.

			using ParserFunc = TFunction<FString(FRegexMatcher&)>;
			
			struct InlineParser
			{
				InlineParser(const FString& InRegex, TArray<UScriptStruct*> InValidBlocks, ParserFunc Parser)
					: Regex(InRegex)
					, ValidBlockTypes(InValidBlocks)
					, Func(Parser)
				{}
				FString Regex;
				TArray<UScriptStruct*> ValidBlockTypes;
				ParserFunc Func;
			};

			// Define all of the parsers. Order is important!
			TArray<InlineParser> Parsers
			{
				// TODO can I grab these three headers in one pass?
				// ### H3
				{
					R"((?m)^(?<!\\)### (.+)$)",
					{ FK2PostIt_TextBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						return FString::Printf(L"<K2PostIt.Header3>%s</>", *Matcher.GetCaptureGroup(1));
					}
				},
				
				// ### H2
				{
					R"((?m)^(?<!\\)## (.+)$)",
					{ FK2PostIt_TextBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						return FString::Printf(L"<K2PostIt.Header2>%s</>", *Matcher.GetCaptureGroup(1));
					}
				},
				
				// ### H1
				{
					R"((?m)^(?<!\\)# (.+)$)",
					{ FK2PostIt_TextBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						return FString::Printf(L"<K2PostIt.Header1>%s</>", *Matcher.GetCaptureGroup(1));
					}
				},
				
				// `text` Code
				{
					R"((?<!\\)`(.+?)(?<!\\)`)",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Code = Matcher.GetCaptureGroup(1);
						return FString::Printf(TEXT("<K2PostIt.Code>%s</>"), *Code);
					}
				},
				
				// [label](url) Website browser link
				{
					R"(\[(.*?)\]\((.*?)\))",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Label = Matcher.GetCaptureGroup(1);
						FString URL = Matcher.GetCaptureGroup(2);
						return FString::Printf(TEXT("<a id=\"browser\" href=\"%s\" style=\"K2PostItCommonHyperlink\">%s</>"), *URL, *Label);
					}
				},

				// https:// Website browser link
				{
					R"((?![^\s]*\.\.)(?:[a-z]{3,9}:\/\/?[\-;:&=\+\$,\w]+?[a-z0-9.-]+|[\/a-z0-9]+\.|[\-;:&=\+\$,\w]+@)[a-z0-9.-]+(?:(?:\/[\+~%\/.\w\-_]*)?\??[\-\+=&;%@.\w_]*#?[.!\/\\\w]*)?)",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString URL = Matcher.GetCaptureGroup(0);
						return FString::Printf(TEXT("<a id=\"browser\" href=\"%s\" style=\"K2PostItCommonHyperlink\">%s</>"), *URL, *URL);
					}
				},
				
				// ***text*** Bold Italic
				{
					R"((?<!\\)\*(?<!\\)\*(?<!\\)\*(.+?)(?<!\\)\*(?<!\\)\*(?<!\\)\*)",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return FString::Printf(TEXT("<K2PostIt.BoldItalic>%s</>"), *Inner);
					}
				},
				
				// **text** Bold
				{
					R"((?<!\\)\*(?<!\\)\*(.+?)(?<!\\)\*(?<!\\)\*)",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return FString::Printf(TEXT("<K2PostIt.Bold>%s</>"), *Inner);
					}
				},

				// *text* Italic
				{
					R"((?<!\\)\*(.+?)(?<!\\)\*)",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return FString::Printf(TEXT("<K2PostIt.Italic>%s</>"), *Inner);
					}
				},
				
				// __text__ Underline
				{
					R"((?<!\\)_(?<!\\)_(.+?)(?<!\\)_(?<!\\)_)",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return FString::Printf(TEXT("<K2PostIt.Underline>%s</>"), *Inner);
					}
				},
			
			    // \* unescape any remaining escaped characters
			    {
			        R"(\\(\*))",
			        { FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return "*";
					}
			    },
				
				// \` unescape any remaining escaped characters
				{
					R"(\\(\`))",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return "`";
					}
				},
				
				// \_ unescape any remaining escaped characters
				{
					R"(\\(\_))",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return "_";
					}
				},
				
				// \# unescape any remaining escaped characters
				{
					R"(\\(\#))",
					{ FK2PostIt_TextBlock::StaticStruct(), FK2PostIt_BulletBlock::StaticStruct() },
					[] (FRegexMatcher& Matcher) -> FString
					{
						FString Inner = Matcher.GetCaptureGroup(1);
						return "#";
					}
				}
			};

			// This is the starting point for processing inline parsers
			TArray<MyStringContainer> StringChunks = { MyStringContainer::MakeRaw(Text) };

			for (const InlineParser& Parser : Parsers)
			{
				TArray<UScriptStruct*> ValidBlockTypes = Parser.ValidBlockTypes;
				const UScriptStruct* CurrentBlockType = CurrentBlock.GetScriptStruct();

				// Make sure that the block we're currently processing is valid for this parser - for example, code blocks should not get processed by **bold** 
				const auto BlockTypeMatch = [CurrentBlockType] (const UScriptStruct* ParentBlockType)
				{
					return CurrentBlockType == ParentBlockType;
				};
				
				if (!ValidBlockTypes.ContainsByPredicate(BlockTypeMatch))
				{
					continue;
				}

				// Start parsing
				for (int32 j = 0; j < StringChunks.Num(); ++j)
				{
					MyStringContainer& Chunk = StringChunks[j];

					// If the chunk we're about to look at was already parsed, don't parse it again. This is a hack to keep this thing a bit simple - SRichTextBlock doesn't support nesting and neither do we.
					if (Chunk.IsParsed())
					{
						continue;
					}
					
					const FString& ChunkString = Chunk.Get();
					
					const FRegexPattern Pattern(Parser.Regex, ERegexPatternFlags::CaseInsensitive);
					FRegexMatcher Matcher(Pattern, ChunkString);

					if (Matcher.FindNext())
					{
						int32 current_j = j;
						TArray<MyStringContainer> ReplacementSegments;

						// We have a match - we're going to split this into three chunks (before the match, the match itself, and after the match)
						if (Matcher.GetMatchBeginning() > 0)
						{
							FString Before = ChunkString.Left(Matcher.GetMatchBeginning());
							ReplacementSegments.Emplace(MyStringContainer::MakeRaw(Before));
							++j;
						}

						// This is the actual parser doing its thing
						ReplacementSegments.Emplace( MyStringContainer::MakeParsed(Parser.Func(Matcher)) );

						// This is the leftover after the match, on the next for-loop we'll be looking at this chunk 
						if (Matcher.GetMatchEnding() < ChunkString.Len() - 1)
						{
							FString After = ChunkString.RightChop(Matcher.GetMatchEnding()); 
							ReplacementSegments.Emplace(MyStringContainer::MakeRaw(After));
						}

						// Pull out the single raw string and replace it with the three chunks. Note the middle chunk is "Parsed" and the Before/After chunks remain raw.
						StringChunks.RemoveAt(current_j, EAllowShrinking::No);
						StringChunks.Insert(ReplacementSegments, current_j);
					}
				}
			}

			// We're all done processing this block with all parsers. Turn it back into a contiguous string.
			auto ProcessedChunksToString = [] (const TArray<MyStringContainer>& StringChunks) -> FString
			{
				FString String;
			
				for (int32 i = 0; i < StringChunks.Num(); ++i)
				{
					const MyStringContainer& Container = StringChunks[i];
					String += Container.Get();
				}

				return String;
			};
			
			Text = ProcessedChunksToString(StringChunks);
		}
	}
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
