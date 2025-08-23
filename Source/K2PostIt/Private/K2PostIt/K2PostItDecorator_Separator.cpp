// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItDecorator_Separator.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "Framework/Text/TextDecorators.h"
#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

FK2PostItDecorator_Separator::FK2PostItDecorator_Separator(FString InName, const FSlateColor& InColor)
	: TextStyle(FK2PostItStyle::Get().GetWidgetStyle<FTextBlockStyle>(K2PostItStyles.TextStyle_Normal))
{
	RunName = InName;
	Color = InColor;
}

// ------------------------------------------------------------------------------------------------

bool FK2PostItDecorator_Separator::Supports(const FTextRunParseResults& RunInfo, const FString& Text) const
{
	return ( RunInfo.Name == RunName );
}

// ------------------------------------------------------------------------------------------------

TSharedRef<ISlateRun> FK2PostItDecorator_Separator::Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style)
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

	TextLayout.Get().SetJustification(ETextJustify::Center);
	
	TSharedPtr<ISlateRun> SlateRun = FSlateWidgetRun::Create(TextLayout, RunInfo, ModelText, WidgetRunInfo, ModelRange);
	
	return SlateRun.ToSharedRef();
}

// ------------------------------------------------------------------------------------------------

TSharedPtr<SWidget> FK2PostItDecorator_Separator::CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const
{
	return SNew(SBox)
	.HAlign(HAlign_Fill)
	.Padding(0, 0, 0, 8)
	[
		SNew(SSeparator)
		.Thickness(2)
		.ColorAndOpacity(K2PostItColor::LightGray_Glass)
	];
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE