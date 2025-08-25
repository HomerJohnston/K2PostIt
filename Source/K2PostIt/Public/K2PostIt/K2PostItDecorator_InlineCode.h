// Unlicensed. This file is public domain.

#pragma once

#include "Components/RichTextBlockDecorator.h"

#include "K2PostIt/Widgets/SGraphNode_K2PostIt.h"

class UEdGraphNode_K2PostIt;

class K2POSTIT_API  FK2PostItDecorator_InlineCode : public ITextDecorator
{
public:
	FK2PostItDecorator_InlineCode(FString InName, TSharedPtr<SGraphNode_K2PostIt> InOwnerWidget);
	
	bool Supports( const FTextRunParseResults& RunInfo, const FString& Text ) const override;

	static TSharedRef<FK2PostItDecorator_InlineCode> Create(FString InName, TSharedPtr<SGraphNode_K2PostIt> InOwnerWidget)
	{
		return MakeShareable(new FK2PostItDecorator_InlineCode(MoveTemp(InName), InOwnerWidget));
	}

	//TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunInfo, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style) override;
	TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunInfo, const FString& OriginalText, const TSharedRef<FString>& ModelText, const ISlateStyle* Style) override;

	TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const;
	
	FString RunName;

	const FTextBlockStyle& TextStyle;

	TWeakPtr<SGraphNode_K2PostIt> OwnerWidget;

	UEdGraphNode_K2PostIt* GetOwner() const;
};
