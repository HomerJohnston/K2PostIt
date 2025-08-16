// Creative commons. Do whatever you want with this file.

#include "K2PostIt/K2PostItDecorator_InlineCode.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "K2PostIt/K2PostItStyle.h"


FK2PostItDecorator_InlineCode::FK2PostItDecorator_InlineCode(const FTextBlockStyle& Style)
	:	TextStyle(Style)
{
}

FK2PostItDecorator_InlineCode::FK2PostItDecorator_InlineCode(FString InName, const FSlateColor& InColor)
	: TextStyle(FK2PostItStyle::Get().GetWidgetStyle<FTextBlockStyle>(K2PostItStyles.TextStyle_Normal))
{
	RunName = InName;
	Color = InColor;
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

//	const FTextBlockStyle& TextStyle = Owner->GetCurrentDefaultTextStyle();

	
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
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ErrorReporting.EmptyBox"))
		[
			SNew(STextBlock)
			.Text(RunInfo.Content)
		];
}
