// Creative commons. Do whatever you want with this file.

#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "EdGraphNode_K2PostIt.h"
#include "HAL/Platform.h"
#include "Input/Reply.h"
#include "Layout/SlateRect.h"
#include "Math/Vector2D.h"
#include "SGraphNodeResizable.h"
#include "SNodePanel.h"
#include "Framework/Text/ITextDecorator.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SMultiLineEditableText;
class SWebBrowserView;
class FDragDropEvent;
class SBorder;
class SCommentBubble;
class SGraphNode;
class UEdGraphNode_K2PostIt;
struct FGeometry;
struct FPointerEvent;
struct FSlateBrush;

class K2POSTIT_API SGraphNodeK2PostIt : public SGraphNodeResizable
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeK2PostIt){}
	SLATE_END_ARGS()

	//~ Begin SWidget Interface
	virtual FReply OnMouseButtonDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDragEnter( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	//~ End SWidget Interface

	//~ Begin SNodePanel::SNode Interface
	virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;
	virtual void GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const override;
	virtual bool ShouldAllowCulling() const override { return true; }
	virtual int32 GetSortDepth() const override;
	virtual void EndUserInteraction() const override;
	virtual FString GetNodeComment() const override;
	//~ End SNodePanel::SNode Interface

	//~ Begin SPanel Interface
	virtual FVector2D ComputeDesiredSize(float) const override;
	//~ End SPanel Interface

	//~ Begin SGraphNode Interface
	virtual bool IsNameReadOnly() const override;
	virtual FSlateColor GetCommentColor() const override { return GetCommentBodyColor(); }
	//~ End SGraphNode Interface

	void Construct( const FArguments& InArgs, UEdGraphNode_K2PostIt* InNode );

	/** return if the node can be selected, by pointing given location */
	virtual bool CanBeSelected( const FVector2D& MousePositionInNode ) const override;

	/** return size of the title bar */
	virtual FVector2D GetDesiredSizeForMarquee() const override;

	/** return rect of the title bar */
	virtual FSlateRect GetTitleRect() const override;

protected:
	void OnPostItCommentTextChanged(const FText& Text);

	//~ Begin SGraphNode Interface
	virtual void UpdateGraphNode() override;
	virtual void PopulateMetaTag(class FGraphNodeMetaData* TagMeta) const override;

	/**
	 * Helper method to update selection state of comment and any nodes 'contained' within it
	 * @param bSelected	If true comment is being selected, false otherwise
	 * @param bUpdateNodesUnderComment If true then force the rebuild of the list of nodes under the comment
	 */
	void HandleSelection(bool bIsSelected, bool bUpdateNodesUnderComment = false) const;

	/** called when user is moving the comment node */
	virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;

	//~ Begin SGraphNodeResizable Interface
	virtual float GetTitleBarHeight() const override;
	virtual FSlateRect GetHitTestingBorder() const override;
	virtual FVector2D GetNodeMaximumSize() const override;
	//~ Begin SGraphNodeResizable Interface

	/** @return the color to tint the comment body */
	FSlateColor GetCommentBodyColor() const;

	FSlateColor GetMarkdownPreviewPaneColor() const;

	/** @return the color to tint the title bar */
	FSlateColor GetCommentTitleBarColor() const;

	/** @return the color to tint the comment bubble */
	FSlateColor GetCommentBubbleColor() const;

	FText GetCommentText() const;

	void OnPostItCommentTextCommitted(const FText& Text, ETextCommit::Type Arg);

	static FSlateWidgetRun::FWidgetRunInfo GetWidgetThing(const FTextRunInfo& RunInfo, const ISlateStyle* Style);
	
private:
	
	/** Returns the width to wrap the text of the comment at */
	float GetWrapAt() const;

	/** The comment bubble widget (used when zoomed out) */
	TSharedPtr<SCommentBubble> CommentBubble;

	TSharedPtr<SMultiLineEditableText> CommentTextSource;

	bool bMouseClickEditingInterlock = false;

	bool bEditButtonClicked = false;

	float PreviewPanelRenderOpacity = 0.0f;
	
	/** The current selection state of the comment */
	mutable bool bIsSelected;

	/** the title bar, needed to obtain it's height */
	TSharedPtr<SBorder> TitleBar;

	TSharedPtr<SBorder> MainPanel;

	TSharedPtr<SBox> PreviewPanelBox;

	TWeakPtr<SWindow> PreviewPanelWindow;
	
	TSharedPtr<SWebBrowserView> WebBrowser;

	TSharedPtr<SVerticalBox> FormattedTextPanel;
	
	FMargin Padding_MarkdownPreviewPanel() const;

protected:
	/** cached comment title */
	FString CachedCommentTitle;

	/** cached font size */
	int32 CachedFontSize;

	/** Was the bubble desired to be visible last frame? */
	mutable bool bCachedBubbleVisibility;

private:
	/** cached comment title */
	float CachedWidth;

	/** Local copy of the comment style */
	FInlineEditableTextBlockStyle CommentStyle;

	void RebuildRichText();

	UEdGraphNode_K2PostIt* GetNodeObjAsK2PostIt();

	FReply OnClicked_EditIcon();
};
