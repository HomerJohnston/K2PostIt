// Unlicensed. This file is public domain.

#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Framework/Text/ITextDecorator.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "HAL/Platform.h"
#include "Input/Reply.h"
#include "K2PostIt/Nodes/EdGraphNode_K2PostIt.h"
#include "Layout/SlateRect.h"
#include "Math/Vector2D.h"
#include "Runtime/Launch/Resources/Version.h"
#include "SGraphNodeResizable.h"
#include "SNodePanel.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SBox;
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

class K2POSTIT_API SGraphNode_K2PostIt : public SGraphNodeResizable
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_K2PostIt){}
	SLATE_END_ARGS()

	//~ Begin SWidget Interface
	FReply OnMouseButtonDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent ) override;
	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	FReply OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	void OnDragEnter( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	//~ SWidget Interface

	//~ Begin SNodePanel::SNode Interface
	const FSlateBrush* GetShadowBrush(bool bSelected) const override;
	void GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const override;
	bool ShouldAllowCulling() const override { return true; }
	int32 GetSortDepth() const override;
	void EndUserInteraction() const override;
	FString GetNodeComment() const override;
	//~ SNodePanel::SNode Interface

	//~ Begin SPanel Interface
	FVector2D ComputeDesiredSize(float) const override;
	//~ SPanel Interface

	//~ Begin SGraphNode Interface
	bool IsNameReadOnly() const override;
	FSlateColor GetCommentColor() const override { return GetCommentBodyColor(); }
	//~ SGraphNode Interface

	//~ Begin SGraphNodeResizable Interface
	FCursorReply OnCursorQuery( const FGeometry& MyGeometry, const FPointerEvent& CursorEvent ) const override;
	//~ SGraphNodeResizable Interface
	
	
	void OnParseComplete();
	void Construct( const FArguments& InArgs, UEdGraphNode_K2PostIt* InNode );

	/** return if the node can be selected, by pointing given location */
	bool CanBeSelected( const FVector2D& MousePositionInNode ) const override;

	/** return size of the title bar */
	FVector2D GetDesiredSizeForMarquee() const override;

	/** return rect of the title bar */
	FSlateRect GetTitleRect() const override;

protected:
	void OnTextChanged_CommentTextSource(const FText& Text);

	EVisibility Visibility_EditButton() const;
	FSlateColor ColorAndOpacity_TitleText() const;
	//~ Begin SGraphNode Interface
	void UpdateGraphNode() override;
	void PopulateMetaTag(class FGraphNodeMetaData* TagMeta) const override;

	/**
	 * Helper method to update selection state of comment and any nodes 'contained' within it
	 * @param bIsSelected If true comment is being selected, false otherwise
	 * @param bUpdateNodesUnderComment If true then force the rebuild of the list of nodes under the comment
	 */
	void HandleSelection(bool bIsSelected, bool bUpdateNodesUnderComment = false) const;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
	/** called when user is moving the comment node */
	void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;
#else
	void MoveTo(const FVector2f& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;
#endif
	
	//~ Begin SGraphNodeResizable Interface
	float GetTitleBarHeight() const override;
	FSlateRect GetHitTestingBorder() const override;
	FVector2D GetNodeMaximumSize() const override;
	//~ Begin SGraphNodeResizable Interface

	/** @return the color to tint the comment body */
	FSlateColor GetCommentBodyColor() const;

	FSlateColor GetMarkdownPreviewPaneColor() const;

	/** @return the color to tint the title bar */
	FSlateColor GetCommentTitleBarColor() const;

	/** @return the color to tint the comment bubble */
	FSlateColor GetCommentBubbleColor() const;

	FText Text_CommentTextSource() const;

	void OnTextCommitted_CommentTextSource(const FText& Text, ETextCommit::Type Arg);

	static FSlateWidgetRun::FWidgetRunInfo GetWidgetThing(const FTextRunInfo& RunInfo, const ISlateStyle* Style);
	
public:
	
	/** Returns the width to wrap the text of the comment at */
	float GetWrapAt() const;

private:
	/** The comment bubble widget (used when zoomed out) */
	TSharedPtr<SCommentBubble> CommentBubble;

	TSharedPtr<SMultiLineEditableText> CommentTextSource;

	bool bMouseClickEditingInterlock = false;

	bool bEditButtonClicked = false;

	bool bTitleWidgetPanelFocused = false;
	
	bool bFocusAssigned = false;

	float PreviewPanelRenderOpacity = 0.0f;
	
	/** The current selection state of the comment */
	mutable bool bIsSelected = false;

	/** the title bar, needed to obtain it's height */
	TSharedPtr<SBorder> TitleBar;

	TSharedPtr<SBorder> MainPanel;

	TSharedPtr<SBox> PreviewPanelBox;

	TWeakPtr<SWindow> PreviewPanelWindow;

	TSharedPtr<SWebBrowserView> WebBrowser;

	TSharedPtr<SVerticalBox> FormattedTextPanel;

	TSharedPtr<SWidget> QuickColorPalette;

	TSharedPtr<SOverlay> TitleWidgetPanel;
	
	FSlateColor ForegroundColor_TitleBorder() const;

	FMargin Padding_MarkdownPreviewPanel() const;

protected:
	/** cached comment title */
	FString CachedCommentTitle;

	/** cached font size */
	int32 CachedFontSize = 0;
	
	/** Was the bubble desired to be visible last frame? */
	mutable bool bCachedBubbleVisibility = false;

private:
	/** cached comment title */
	float CachedWidth = 0.0f;

	/** Local copy of the comment style */
	FInlineEditableTextBlockStyle CommentStyle;

	void RebuildRichText();
	
	void ShowQuickColorPalette();

	void HideQuickColorPalette();
	
	FReply OnClicked_QuickColorPaletteColor(const FLinearColor NewColor);

	const UEdGraphNode_K2PostIt* GetNodeObjAsK2PostIt() const;
	
	UEdGraphNode_K2PostIt* GetNodeObjAsK2PostIt();

	FReply OnClicked_EditIcon();

	int32 WidgetIndex_CommentTextPane() const;

	void UpdatePreviewPanelOpacity();
	
	/* Called when text is committed on the node */
	void K2PostIt_OnNameTextCommited ( const FText& InText, ETextCommit::Type CommitInfo ) ;
};
