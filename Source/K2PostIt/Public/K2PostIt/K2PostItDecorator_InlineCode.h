// Creative commons. Do whatever you want with this file.

#pragma once

#include "Components/RichTextBlockDecorator.h"

class UEdGraphNode_K2PostIt;

class K2POSTIT_API  FK2PostItDecorator_InlineCode : public ITextDecorator
{
public:
	FK2PostItDecorator_InlineCode(FString InName, UEdGraphNode_K2PostIt* InOwner);
	
	bool Supports( const FTextRunParseResults& RunInfo, const FString& Text ) const override;

	static TSharedRef<FK2PostItDecorator_InlineCode> Create(FString InName, UEdGraphNode_K2PostIt* InOwner)
	{
		return MakeShareable(new FK2PostItDecorator_InlineCode(MoveTemp(InName), InOwner));
	}

	//TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunInfo, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style) override;
	TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunInfo, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style) override;

	TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const;
	
	FString RunName;

	const FTextBlockStyle& TextStyle;

	TWeakObjectPtr<UEdGraphNode_K2PostIt> Owner;
};
