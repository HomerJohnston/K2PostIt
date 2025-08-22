// Unlicensed. This file is public domain.

#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "K2Node.h"
#include "K2PostIt/Widgets/SGraphNode_K2PostIt.h"
#include "Math/Color.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"
#include "Textures/SlateIcon.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
#include "InstancedStruct.h"
#else
#include "StructUtils/InstancedStruct.h"
#endif

#include "EdGraphNode_K2PostIt.generated.h"

class UEdGraphNode_K2PostIt;
class INameValidatorInterface;
class UEdGraphPin;
class UObject;
struct FPropertyChangedEvent;
struct Rect;
class SGraphNode_K2PostIt;
struct FK2PostIt_BaseBlock;
class FK2PostItAsyncParser;

typedef TArray<class UObject*> FCommentNodeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRegexPatternUpdated);

// ================================================================================================

UCLASS()
class K2POSTIT_API UEdGraphNode_K2PostIt : public UK2Node
{
public:
	GENERATED_BODY()

public:
	UEdGraphNode_K2PostIt(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Color to style comment with */
	UPROPERTY(EditAnywhere, Category = "Comment")
	FLinearColor CommentColor;

	/** Only effective if "disable markdown by default" is ***not*** set in project settings. */
	UPROPERTY(EditAnywhere, Category = "Comment")
	bool bDisableMarkdownRendering = false;

	/** Only effective if "disable markdown by default" is set in project settings. */
	UPROPERTY(EditAnywhere, Category = "Comment")
	bool bEnableMarkdownRendering = false;
	
	/** Size of the text in the comment box */
	UPROPERTY(EditAnywhere, Category = "Comment", meta=(ClampMin=8, ClampMax=64))
	int32 TitleFontSize;
	
	/** Whether to show a zoom-invariant comment bubble when zoomed out (making the comment readable at any distance). */
	UPROPERTY(EditAnywhere, Category = "Comment", meta=(DisplayName="Show Bubble When Zoomed"))
	uint32 bCommentBubbleVisible_InDetailsPanel:1;

	/** Whether to use Comment Color to color the background of the comment bubble shown when zoomed out. */
	UPROPERTY(EditAnywhere, Category = "Comment", meta=(DisplayName="Color Bubble", EditCondition = "bCommentBubbleVisible_InDetailsPanel"))
	uint32 bColorCommentBubble:1;

	/** comment Depth */
	UPROPERTY()
	int32 CommentDepth;

	UPROPERTY()
	FText CommentText;

	UPROPERTY()
	TArray<TInstancedStruct<FK2PostIt_BaseBlock>> Blocks;

	TSharedPtr<FK2PostItAsyncParser> ActiveParser;
	
	TSharedPtr<FK2PostItAsyncParser> QueuedParser;

	TMulticastDelegate<void()> OnParseCompleteEvent;
	
public:

	//~ Begin UObject Interface
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool IsSelectedInEditor() const override;

	void PostLoad() override;
	//~ End UObject Interface

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override {}
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeCommentColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldOverridePinNames() const override { return true; }
	virtual FText GetPinNameOverride(const UEdGraphPin& Pin) const override;
	virtual void ResizeNode(const FVector2D& NewSize) override;
	virtual void PostPlacedNewNode() override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual TSharedPtr<class INameValidatorInterface> MakeNameValidator() const override;
	virtual FString GetDocumentationLink() const override;
	virtual FString GetDocumentationExcerptName() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	/** Return whether the node's properties display in the blueprint details panel */
	bool ShouldShowNodeProperties() const override { return true; }

	bool IsNodePure() const override { return true; }
	
	/** Create a visual widget to represent this node in a graph editor or graph panel.  If not implemented, the default node factory will be used. */
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;

	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	//~ End UEdGraphNode Interface

	/** Return the font size of the comment */
	virtual int32 GetFontSize() const { return TitleFontSize; }
	
	void SetCommentText(const FText& Text);

	/** Override the default selection state of this graph node */
	enum class ESelectionState : uint8 { Inherited, Selected, Deselected };
	void SetSelectionState(const ESelectionState InSelectionState);

	void OnParseComplete(TArray<TInstancedStruct<FK2PostIt_BaseBlock>> NewBlocks);
	
private:
	/** Nodes currently within the region of the comment */
	TArray<TObjectPtr<class UObject>>	NodesUnderComment;

	/** Constructing FText strings can be costly, so we cache the node's tooltip */
	FNodeTextCache CachedTooltip;

	/** Override the default selection state of this graph node */
	ESelectionState SelectionState = ESelectionState::Inherited;
};

// Code that registers 'C' to add comment...
// UI_COMMAND( CreateComment, "Create Comment", "Create a comment box", EUserInterfaceActionType::Button, FInputChord(EKeys::C))