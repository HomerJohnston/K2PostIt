// Creative commons. Do whatever you want with this file.

#pragma once

#include "Components/RichTextBlockDecorator.h"

class K2POSTIT_API  FK2PostItDecorator_Separator : public ITextDecorator
{
public:
	FK2PostItDecorator_Separator(FString InName, const FSlateColor& InColor);
	
	bool Supports( const FTextRunParseResults& RunInfo, const FString& Text ) const override;

	static TSharedRef<FK2PostItDecorator_Separator> Create(FString InName, const FSlateColor& InColor)
	{
		return MakeShareable(new FK2PostItDecorator_Separator(MoveTemp(InName), InColor));
	}

	//TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunInfo, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style) override;
	TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunInfo, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style) override;

	TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const;
	
	FString RunName;

	const FTextBlockStyle& TextStyle;

	FSlateColor Color;
};