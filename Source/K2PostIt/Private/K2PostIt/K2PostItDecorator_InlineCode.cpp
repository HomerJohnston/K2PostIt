// Creative commons. Do whatever you want with this file.

#include "K2PostIt/K2PostItDecorator_InlineCode.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "K2PostIt/EdGraphNode_K2PostIt.h"
#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItStyle.h"
#include "Widgets/Text/SRichTextBlock.h"


FK2PostItDecorator_InlineCode::FK2PostItDecorator_InlineCode(const FTextBlockStyle& Style)
	:	TextStyle(Style)
{
}

FK2PostItDecorator_InlineCode::FK2PostItDecorator_InlineCode(FString InName, UEdGraphNode_K2PostIt* InOwner)
	: TextStyle(FK2PostItStyle::Get().GetWidgetStyle<FTextBlockStyle>(K2PostItStyles.TextStyle_Normal))
{
	RunName = InName;
	Owner = InOwner;
}

bool FK2PostItDecorator_InlineCode::Supports(const FTextRunParseResults& RunInfo, const FString& Text) const
{
	return ( RunInfo.Name == RunName );
}

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

TSharedPtr<SWidget> FK2PostItDecorator_InlineCode::CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const
{
	return SNew(SBox)
	.Padding(2, -3, 2, -3)
	[
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
				Color = Owner->CommentColor * 5;
				Color.A = Alpha;
			}
			
			return Color;
		})
		.Padding(0)
		[
			SNew(SBorder)
			.Padding(3, 1, 3, 1)
			.BorderImage(FK2PostItStyle::GetImageBrush(K2PostItBrushes.CodeHighlightBorder))
			.BorderBackgroundColor_Lambda( [this] ()
			{
				FLinearColor Color = K2PostItColor::White;
	
				if (Owner.IsValid())
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
				.LineHeightPercentage(1.1f)
				.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
				.AutoWrapText(true)
				/*
				SNew(STextBlock)
				.TextStyle(&TextStyle)
				.Text(RunInfo.Content)
				*/
			]
		]
	];
}
