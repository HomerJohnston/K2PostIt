// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItDecorator_InlineCode.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItStyle.h"
#include "K2PostIt/Nodes/EdGraphNode_K2PostIt.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

FK2PostItDecorator_InlineCode::FK2PostItDecorator_InlineCode(FString InName, TSharedPtr<SGraphNode_K2PostIt> InOwnerWidget)
	: TextStyle(FK2PostItStyle::Get().GetWidgetStyle<FTextBlockStyle>(K2PostItStyles.TextStyle_Normal))
{
	RunName = InName;
	OwnerWidget = InOwnerWidget;
}

// ------------------------------------------------------------------------------------------------

bool FK2PostItDecorator_InlineCode::Supports(const FTextRunParseResults& RunInfo, const FString& Text) const
{
	return ( RunInfo.Name == RunName );
}

// ------------------------------------------------------------------------------------------------

TSharedRef<ISlateRun> FK2PostItDecorator_InlineCode::Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style)
{
	FTextRange ModelRange;
	ModelRange.BeginIndex = ModelText->Len();

	FTextRunInfo RunInfo(RunParseResult.Name, FText::FromString(OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex)));
	for (const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData)
	{
		RunInfo.MetaData.Add(Pair.Key, OriginalText.Mid(Pair.Value.BeginIndex, Pair.Value.EndIndex - Pair.Value.BeginIndex));
	}
	
	TSharedPtr<SWidget> DecoratorWidget = CreateDecoratorWidget(RunInfo, TextStyle);
	
	*ModelText += TEXT('\u200B'); // Zero-Width Breaking Space
	ModelRange.EndIndex = ModelText->Len();
	
	// Calculate the baseline of the text within the owning rich text
	// Requested on demand as the font may not be loaded right now
	const FSlateFontInfo Font = TextStyle.Font;
	const float ShadowOffsetY = FMath::Min(0.0f, TextStyle.ShadowOffset.Y);

	TAttribute<int16> GetBaseline = TAttribute<int16>::CreateLambda([Font, ShadowOffsetY]()
	{
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		
		return FontMeasure->GetBaseline(Font) - ShadowOffsetY;
	});
	
	FSlateWidgetRun::FWidgetRunInfo WidgetRunInfo(DecoratorWidget.ToSharedRef(), GetBaseline);

	TextLayout->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
	
	TSharedPtr<ISlateRun> SlateRun = FSlateWidgetRun::Create(TextLayout, RunInfo, ModelText, WidgetRunInfo, ModelRange);

	return SlateRun.ToSharedRef();
}

// ------------------------------------------------------------------------------------------------

TSharedPtr<SWidget> FK2PostItDecorator_InlineCode::CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const
{
	return SNew(SBox)
	.Padding(0, 0, 0, 0)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.CodeHighlightFill))
		.ForegroundColor_Lambda( [this] ()
		{
			FLinearColor Color = K2PostItColor::Noir;
	
			if (UEdGraphNode_K2PostIt* Owner = GetOwner())
			{
				Color = K2PostItColor::GetNominalFontColor(Owner->CommentColor, K2PostItColor::White, K2PostItColor::Noir);
			}
			
			return Color;
		})
		.BorderBackgroundColor_Lambda( [this] ()
		{
			FLinearColor Color = K2PostItColor::White;
	
			if (UEdGraphNode_K2PostIt* Owner = GetOwner())
			{
				float Alpha = Color.A;
				Color = Owner->CommentColor * 5;
				Color.A = Alpha;
			}
			
			return Color;
		})
		.Padding(0)
		[
			SNew(SBorder)
			.Padding(2, 1, 2, 1)
			.VAlign(VAlign_Bottom)
			.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.CodeHighlightBorder))
			.BorderBackgroundColor_Lambda( [this] ()
			{
				FLinearColor Color = K2PostItColor::White;
	
				if (UEdGraphNode_K2PostIt* Owner = GetOwner())
				{
					float Lum = Owner->CommentColor.GetLuminance() * 1.2 + 0.15;
					Color = FLinearColor(Lum, Lum, Lum, 1.0f);
				}
			
				return Color;
			})
			[
				SNew(SRichTextBlock)
				.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_CodeBlock)
				.DecoratorStyleSet( &FK2PostItStyle::Get() )
				.Text(RunInfo.Content)
				.LineHeightPercentage(1.0f) // TODO this should be a global 
				.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
				.AutoWrapText(false)
			]
		]
	];
}

UEdGraphNode_K2PostIt* FK2PostItDecorator_InlineCode::GetOwner() const
{
	if (OwnerWidget.IsValid())
	{
		UEdGraphNode_K2PostIt* OwnerNode = OwnerWidget.Pin()->GetNodeObjAsK2PostIt();

		if (IsValid(OwnerNode))
		{
			return OwnerNode;
		}
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE