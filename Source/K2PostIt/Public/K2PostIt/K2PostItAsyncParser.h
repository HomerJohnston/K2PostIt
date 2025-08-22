// Unlicensed. This file is public domain.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "Tasks/Task.h"

// ================================================================================================

class SGraphNode_K2PostIt;
class UEdGraphNode_K2PostIt;
class SWidget;
class FRegexMatcher;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
#include "InstancedStruct.h"
#else
#include "StructUtils/InstancedStruct.h"
#endif

#include "K2PostItAsyncParser.generated.h"

USTRUCT()
struct FK2PostIt_BaseBlock
{
	GENERATED_BODY()

public:
	FK2PostIt_BaseBlock() {}
	
	virtual ~FK2PostIt_BaseBlock() {}
	
	void SetParentWidget(TSharedPtr<SGraphNode_K2PostIt> GraphNodeK2PostIt);

	void SetOwnerNode(UEdGraphNode_K2PostIt* InOwnerNode);

protected:
	UPROPERTY()
	TWeakObjectPtr<UEdGraphNode_K2PostIt> Owner;

	TWeakPtr<SGraphNode_K2PostIt> OwnerWidget;

public:
	virtual TSharedPtr<SWidget> Draw() const { return nullptr; };
};

// ================================================================================================

USTRUCT()
struct FK2PostIt_TextBlock : public FK2PostIt_BaseBlock
{
	GENERATED_BODY()

public:
	FK2PostIt_TextBlock() : FK2PostIt_BaseBlock() {};

	FK2PostIt_TextBlock(const FString& InText) : Text(InText) {}

protected:
	UPROPERTY()
	FString Text;

public:
	TSharedPtr<SWidget> Draw() const override;

	FString& GetText() { return Text; }
	
	const FString& GetText() const { return Text; }
};

// ================================================================================================

USTRUCT()
struct FK2PostIt_SeparatorBlock : public FK2PostIt_BaseBlock
{
	GENERATED_BODY()

public:
	FK2PostIt_SeparatorBlock() {};

public:
	TSharedPtr<SWidget> Draw() const override;
};

// ================================================================================================

USTRUCT()
struct FK2PostIt_CodeBlock : public FK2PostIt_TextBlock
{
	GENERATED_BODY()

public:
	FK2PostIt_CodeBlock() {};

	FK2PostIt_CodeBlock(const FString& InText) : FK2PostIt_TextBlock(InText) { }

public:
	TSharedPtr<SWidget> Draw() const override;
};

USTRUCT()
struct FK2PostIt_BulletBlock : public FK2PostIt_TextBlock
{
	GENERATED_BODY()

public:
	FK2PostIt_BulletBlock() {};
	
	FK2PostIt_BulletBlock(uint8 InIndentLevel, const FString& InText) : FK2PostIt_TextBlock(InText), IndentLevel(InIndentLevel) { }

protected:
	UPROPERTY()
	uint8 IndentLevel = 0;

public:
	TSharedPtr<SWidget> Draw() const override;
};

// ================================================================================================

struct MyStringContainer
{
protected:
	MyStringContainer(FString InString) : String(InString) {}
	MyStringContainer(FString InString, bool bInParsed) : String(InString), bParsed(bInParsed) {}

public:
	static MyStringContainer MakeRaw(FString InString) { return MyStringContainer(InString); } 
	static MyStringContainer MakeParsed(FString InString) { return MyStringContainer(InString, true); }
	const FString& Get() const { return String; } 

protected:
	FString String;
	bool bParsed = false;

public:
	bool IsParsed() const { return bParsed; }
};

// ================================================================================================

using SomeFunc = TFunction<void(FRegexMatcher& Matcher, TArray<TInstancedStruct<FK2PostIt_BaseBlock>>& ReplacementBlocks)>;

using SomeFunc2 = TFunction<void(FRegexMatcher& Matcher, FString& Text)>;
	
// ================================================================================================

class FK2PostItAsyncParser : public TSharedFromThis<FK2PostItAsyncParser>
{
public:
	FK2PostItAsyncParser(const FString& InString);
	
	void RunParser();
	
	static void PeasantTextToRichText(const FString& PeasantText, TArray<TInstancedStruct<FK2PostIt_BaseBlock>>& Blocks);
	
	static void ProcessTextBlocks(FString RegexPattern, SomeFunc F, TArray<TInstancedStruct<FK2PostIt_BaseBlock>>& Blocks);

	using BlockArray = TArray<TInstancedStruct<FK2PostIt_BaseBlock>>;

	UE::Tasks::TTask<void> Task;

	TMulticastDelegate<void(BlockArray)> OnParseComplete;

	FString StringToParse;
};