// Unlicensed. This file is public domain.

#include "K2PostIt/Widgets/SGraphNode_K2PostIt.h"

#include "Editor.h"
#include "Animation/CurveHandle.h"
#include "Animation/CurveSequence.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "GraphEditorSettings.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"
#include "K2PostIt/Globals/K2PostItConstants.h"
#include "K2PostIt/K2PostItAsyncParser.h"
#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItProjectSettings.h"
#include "K2PostIt/K2PostItStyle.h"
#include "K2PostIt/Nodes/EdGraphNode_K2PostIt.h"
#include "K2PostIt/Widgets/SWindow_K2PostIt.h"
#include "Layout/ChildrenBase.h"
#include "Layout/Geometry.h"
#include "Layout/Margin.h"
#include "Math/Color.h"
#include "Misc/Attribute.h"
#include "Misc/Guid.h"
#include "SCommentBubble.h"
#include "ScopedTransaction.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"
#include "SlotBase.h"
#include "Styling/AppStyle.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Templates/Casts.h"
#include "TutorialMetaData.h"
#include "K2PostIt/Widgets/SK2PostItTItleTextBlock.h"
#include "Types/SlateEnums.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/Package.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Text/SRichTextBlock.h"

class FDragDropEvent;

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

namespace SCommentNodeDefs
{
	/** Size of the hit result border for the window borders */
	/* L, T, R, B */
	static const FSlateRect HitResultBorderSize(10,10,10,10);

	/** Minimum resize width for comment */
	static const float MinWidth = 30.0;

	/** Minimum resize height for comment */
	static const float MinHeight = 30.0;

	/** TitleBarColor = CommentColor * TitleBarColorMultiplier */
	static const float TitleBarColorMultiplier = 0.6f;

	/** Titlebar Offset - taken from the widget borders in UpdateGraphNode */
	static const FSlateRect TitleBarOffset(13,8,-3,0);
}

// ================================================================================================

FCursorReply SGraphNode_K2PostIt::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	switch (MouseZone)
	{
		case CRWZ_NotInWindow:
			return FCursorReply::Cursor(EMouseCursor::Default);

		case CRWZ_InWindow:
			return FCursorReply::Cursor(EMouseCursor::Default);

		case CRWZ_LeftBorder:
		case CRWZ_RightBorder:
		case CRWZ_BottomLeftBorder:
		case CRWZ_BottomRightBorder:
			return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);

		case CRWZ_BottomBorder:
			return FCursorReply::Cursor(EMouseCursor::Default);

		case CRWZ_TopBorder:
			return FCursorReply::Cursor(EMouseCursor::ResizeUpDown);

		case CRWZ_TopLeftBorder:
			return FCursorReply::Cursor( EMouseCursor::ResizeSouthEast );

		case CRWZ_TopRightBorder:
			return FCursorReply::Cursor( EMouseCursor::ResizeSouthWest );

		case CRWZ_TitleBar:
			return FCursorReply::Cursor(EMouseCursor::CardinalCross);

		default:
			return FCursorReply::Unhandled();
	}
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::OnParseComplete()
{
	RebuildRichText();
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::Construct(const FArguments& InArgs, UEdGraphNode_K2PostIt* InNode)
{
	this->GraphNode = InNode;
	this->bIsSelected = false;

	// Set up animation
	{
		ZoomCurve = SpawnAnim.AddCurve(0, 0.1f);
		FadeCurve = SpawnAnim.AddCurve(0.15f, 0.15f);
	}

	// Cache these values so they do not force a re-build of the node next tick.
	CachedCommentTitle = GetNodeComment();
	CachedWidth = InNode->NodeWidth;

	this->UpdateGraphNode();

	// Pull out sizes
	UserSize.X = InNode->NodeWidth;
	UserSize.Y = InNode->NodeHeight;

	// Cache desired size so we cull correctly. We can do this as our ComputeDesiredSize ignores the layout scale.
	CacheDesiredSize(1.0f);

	MouseZone = CRWZ_NotInWindow;
	bUserIsDragging = false;

	InNode->OnBlocksUpdatedEvent.AddSP(this, &SGraphNode_K2PostIt::OnParseComplete);
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const FString CurrentCommentTitle = GetNodeComment();
	if (CurrentCommentTitle != CachedCommentTitle)
	{
		CachedCommentTitle = CurrentCommentTitle;
	}

	const int32 CurrentWidth = static_cast<int32>(UserSize.X);
	if (CurrentWidth != CachedWidth)
	{
		CachedWidth = CurrentWidth;
	}

	UEdGraphNode_K2PostIt* CommentNode = CastChecked<UEdGraphNode_K2PostIt>(GraphNode);
	if (bCachedBubbleVisibility != CommentNode->bCommentBubbleVisible_InDetailsPanel)
	{
		CommentBubble->UpdateBubble();
		bCachedBubbleVisibility = CommentNode->bCommentBubbleVisible_InDetailsPanel;
	}

	if (CachedFontSize != CommentNode->GetFontSize())
	{
		UpdateGraphNode();
	}

	if (TitleWidgetPanel->HasAnyUserFocusOrFocusedDescendants())
	{
		bTitleWidgetPanelFocused = true;
	}
	
	if (!bIsSelected)
	{
		bEditButtonClicked = false;
		bTitleWidgetPanelFocused = false;

		if (PreviewPanelRenderOpacity > 0)
		{
			PreviewPanelRenderOpacity -= 5.0f * InDeltaTime;

			if (PreviewPanelRenderOpacity <= 0)
			{
				PreviewPanelRenderOpacity = 0;
				PreviewPanelBox->SetContent(SNullWidget::NullWidget);
			}
		}

		HideQuickColorPalette();
	}
	else if (bEditButtonClicked && PreviewPanelRenderOpacity <= 1.0)
	{
		PreviewPanelRenderOpacity += 5.0f * InDeltaTime;
		PreviewPanelRenderOpacity = FMath::Clamp(PreviewPanelRenderOpacity, 0.0f, 1.0f);
	}

	UpdatePreviewPanelOpacity();
}

// ------------------------------------------------------------------------------------------------

FReply SGraphNode_K2PostIt::OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent )
{
	return FReply::Unhandled();
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::OnDragEnter( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent )
{

}

// ------------------------------------------------------------------------------------------------

float SGraphNode_K2PostIt::GetWrapAt() const
{
	return CachedWidth - 32.0f;
}

// ------------------------------------------------------------------------------------------------

FSlateColor SGraphNode_K2PostIt::ForegroundColor_TitleBorder() const
{
	FLinearColor Color = K2PostItColor::Error;

	const UEdGraphNode_K2PostIt* Owner = GetNodeObjAsK2PostIt();
						
	if (IsValid(Owner))
	{
		Color = K2PostItColor::GetNominalFontColor(Owner->CommentColor, K2PostItColor::White, K2PostItColor::Noir);
	}
						
	return Color;
}

// ------------------------------------------------------------------------------------------------

FMargin SGraphNode_K2PostIt::Padding_MarkdownPreviewPanel() const
{
	float MainPanelWidth = MainPanel->GetCachedGeometry().Size.X;

	const float InnerBorderPadding = K2PostIt::Constants::PreviewPanelPadding;
	const float Gap = K2PostIt::Constants::PreviewPanelGapToEditPanel;
	
	return FMargin(MainPanelWidth + Gap, 0, -MainPanelWidth - Gap - InnerBorderPadding, -9999);
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::RebuildRichText()
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("RebuildRichText"));
	if (UEdGraphNode_K2PostIt* CommentNode = GetNodeObjAsK2PostIt())
	{
		FormattedTextPanel->ClearChildren();

		// This is a bit ugly, I would rather this be const and some other way to inject this widget into the draw setup but
		for (TInstancedStruct<FK2PostIt_BaseBlock>& Block : CommentNode->GetBlocks())
		{
			Block.GetMutable<FK2PostIt_BaseBlock>().SetParentWidget(SharedThis(this));
		
			FormattedTextPanel->AddSlot()
			.AutoHeight()
			[
				Block.GetPtr<FK2PostIt_BaseBlock>()->Draw().ToSharedRef()
			];
		}
	}
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::ShowQuickColorPalette()
{
	const TArray<FLinearColor>& Colors = UK2PostItProjectSettings::GetQuickColorPaletteColors();
	
	TSharedRef<SHorizontalBox> NewPalette = SNew(SHorizontalBox);

	for (const FLinearColor Color : Colors)
	{
		NewPalette->AddSlot()
		[
			SNew(SButton)
			.ButtonStyle(FK2PostItStyle::Get(), K2PostItStyles.ButtonStyle_HoverHintOnly)
			.ContentPadding(4)
			.Cursor(EMouseCursor::Default)
			.OnClicked(this, &SGraphNode_K2PostIt::OnClicked_QuickColorPaletteColor, Color)
			[
				SNew(SBox)
				.WidthOverride(20)
				.HeightOverride(20)
				[
					SNew(SImage)
					.Image(FK2PostItStyle::GetImageBrush(K2PostItBrushes.Overlay_QuickColorPaletteShadow))
					.ColorAndOpacity(Color)
				]
			]
		];
	}

	QuickColorPalette = NewPalette;

	TitleWidgetPanel->AddSlot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.Padding(0, -44, 0, 0)
	[
		QuickColorPalette.ToSharedRef()
	];
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::HideQuickColorPalette()
{
	if (QuickColorPalette.IsValid())
	{
		TitleWidgetPanel->RemoveSlot(QuickColorPalette.ToSharedRef());
	}
}

// ------------------------------------------------------------------------------------------------

FReply SGraphNode_K2PostIt::OnClicked_QuickColorPaletteColor(const FLinearColor NewColor)
{
	UEdGraphNode_K2PostIt* Node = GetNodeObjAsK2PostIt();

	if (Node)
	{
		FScopedTransaction Transaction(TEXT("K2PostIt"), LOCTEXT("ChangeCommentColor_TransactionText", "Change Comment Color"), Node);
		Node->Modify();

		Node->CommentColor = NewColor;

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

// ------------------------------------------------------------------------------------------------

const UEdGraphNode_K2PostIt* SGraphNode_K2PostIt::GetNodeObjAsK2PostIt() const
{
	return Cast<UEdGraphNode_K2PostIt>(GetNodeObj());
}

// ------------------------------------------------------------------------------------------------

UEdGraphNode_K2PostIt* SGraphNode_K2PostIt::GetNodeObjAsK2PostIt()
{
	return Cast<UEdGraphNode_K2PostIt>(GetNodeObj());
}

// ------------------------------------------------------------------------------------------------

FReply SGraphNode_K2PostIt::OnClicked_EditIcon()
{
	bEditButtonClicked = true;
	bFocusAssigned = false;

	UEdGraphNode* Node = GetNodeObj();
	Node->GetGraph()->SelectNodeSet( {Node} );

	TSharedRef<SWindow_K2PostIt> NewPreviewPanelWindow = SNew(SWindow_K2PostIt)
	.CreateTitleBar(false)
	.InitialOpacity(0.0)
	.SizingRule(ESizingRule::Autosized)
	.LayoutBorder(.0f)
	[
		SNew(SBox)
		.WidthOverride(120.0f)
		[
			SNew(SBorder)
			.BorderImage( FK2PostItStyle::GetImageBrush(K2PostItBrushes.PreviewPaneBorder))
			.ColorAndOpacity( FLinearColor::White )
			.BorderBackgroundColor( this, &SGraphNode_K2PostIt::GetMarkdownPreviewPaneColor )
			.Padding(8)
			[
				FormattedTextPanel.ToSharedRef()
			]
		]
	];

	PreviewPanelRenderOpacity = -0.1f;
	PreviewPanelBox->SetContent(NewPreviewPanelWindow);
	PreviewPanelWindow = NewPreviewPanelWindow;

	ShowQuickColorPalette();

	// Oh my god is this for real? It takes two whole ass ticks for the graph to realize this thing is selected after you click on it.
	GEditor->GetTimerManager()->SetTimerForNextTick( [this]
	{
		GEditor->GetTimerManager()->SetTimerForNextTick( [this]
		{
			FSlateApplication::Get().SetAllUserFocus(CommentTextSource, EFocusCause::SetDirectly);
			
			FModifierKeysState CtrlKey(0, 0, 1, 0, 0, 0, 0, 0, 0);
			
			FKeyEvent EndKey(EKeys::End, CtrlKey, 0, false, 0, 0);
			FSlateApplication::Get().ProcessKeyDownEvent(EndKey);
			FSlateApplication::Get().ProcessKeyUpEvent(EndKey);
		} );
	} );
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------

int32 SGraphNode_K2PostIt::WidgetIndex_CommentTextPane() const
{
	const int32 EditMode = 0;
	const int32 MarkupMode = 1;

	/*
	if (bTitleWidgetPanelFocused)
	{
		return EditMode;
	}
	*/
	
	if (UK2PostItProjectSettings::GetMarkdownDisabledByDefault())
	{
		if (!GetNodeObjAsK2PostIt()->bEnableMarkdownRendering)
		{
			return EditMode;
		}
	}
	else
	{
		if (GetNodeObjAsK2PostIt()->bDisableMarkdownRendering)
		{
			return EditMode;
		}
	}
	
	if ((bEditButtonClicked && bIsSelected) || CommentTextSource->HasKeyboardFocus())
	{
		return EditMode;
	}
	
	return MarkupMode;
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::UpdatePreviewPanelOpacity()
{
	if (PreviewPanelWindow.IsValid())
	{
		PreviewPanelWindow.Pin()->SetRenderOpacity(PreviewPanelRenderOpacity);

		PreviewPanelBox->SetWidthOverride(MainPanel->GetCachedGeometry().Size.X);
		PreviewPanelBox->SetMinDesiredHeight(FormattedTextPanel->GetDesiredSize().Y + 16);
	}
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::K2PostIt_OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	OnTextCommitted.ExecuteIfBound(InText, CommitInfo, GraphNode);
	
	UpdateErrorInfo();
	if (ErrorReporting.IsValid())
	{
		ErrorReporting->SetError(ErrorMsg);
	}

	// Open up the body text editor
	if (GetNodeObjAsK2PostIt()->ConsumeFirstPlacement() && CommitInfo == ETextCommit::OnEnter)
	{
		//GEditor->GetTimerManager()->SetTimerForNextTick( [this]
		//{
			OnClicked_EditIcon();
		//} );
	}
}

// ------------------------------------------------------------------------------------------------

bool SGraphNode_K2PostIt::IsNameReadOnly() const
{
	return !IsEditable.Get() || SGraphNode::IsNameReadOnly();
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::UpdateGraphNode()
{
	// No pins in a comment box
	InputPins.Empty();
	OutputPins.Empty();

	// Avoid standard box model too
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	// Remember if we should be showing the bubble
	UEdGraphNode_K2PostIt* CommentNode = CastChecked<UEdGraphNode_K2PostIt>(GraphNode);
	bCachedBubbleVisibility = CommentNode->bCommentBubbleVisible_InDetailsPanel;

	// We want the name of the blueprint as our name - we can find the node from the GUID
	UObject* Package = GraphNode->GetOutermost();
	UObject* LastOuter = GraphNode->GetOuter();
	while (LastOuter->GetOuter() != Package)
	{
		LastOuter = LastOuter->GetOuter();
	}

	SetupErrorReporting();

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);

	CachedFontSize = CommentNode->GetFontSize();

	CommentStyle = FAppStyle::Get().GetWidgetStyle<FInlineEditableTextBlockStyle>("Graph.CommentBlock.TitleInlineEditableText");
	CommentStyle.EditableTextBoxStyle.TextStyle.Font.Size = CachedFontSize;
	CommentStyle.TextStyle.Font.Size = CachedFontSize;

	SAssignNew(FormattedTextPanel, SVerticalBox);

	TAttribute<FMargin> PaddingAttribute_PreviewPane = TAttribute<FMargin>::CreateRaw(this, &SGraphNode_K2PostIt::Padding_MarkdownPreviewPanel);

	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );
	this->GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MainPanel, SBorder)
			.BorderImage( FK2PostItStyle::GetImageBrush(K2PostItBrushes.Border_K2PostItNode))
			.ColorAndOpacity( FLinearColor::White )
			.BorderBackgroundColor( this, &SGraphNode_K2PostIt::GetCommentBodyColor )
			.ForegroundColor(K2PostItColor::DarkGray)
			.Padding(4.0f, 4.0f, 4.0f, 8.0f)
			.AddMetaData<FGraphNodeMetaData>(TagMeta)
			.ToolTip(nullptr)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SVerticalBox)
					//.ToolTipText( this, &SGraphNode::GetNodeTooltip )
					+SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					[
						SAssignNew(TitleBar, SBorder)
						.BorderImage( FAppStyle::GetBrush("NoBorder") )
						.BorderBackgroundColor(K2PostItColor::White)
						.ForegroundColor(this, &SGraphNode_K2PostIt::ForegroundColor_TitleBorder)
						.Padding( FMargin(8,5,8,3) )
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							[
							
								SAssignNew(TitleWidgetPanel, SOverlay)
								+ SOverlay::Slot()
								[
									SAssignNew(InlineEditableText, SK2PostItTitleTextBlock)
									.Style( &CommentStyle )
									.ColorAndOpacity(this, &SGraphNode_K2PostIt::ColorAndOpacity_TitleText)
									.ShadowOffset(0)
									.ShadowColorAndOpacity(K2PostItColor::Transparent)
									.Text( this, &SGraphNode_K2PostIt::GetEditableNodeTitleAsText )
									.EmptyText(LOCTEXT("K2PostIt_CommentTitleDefaultText", "Comment"))
									.OnVerifyTextChanged(this, &SGraphNode_K2PostIt::OnVerifyNameTextChanged)
									.OnTextCommitted(this, &SGraphNode_K2PostIt::K2PostIt_OnNameTextCommited)
									.IsReadOnly( this, &SGraphNode_K2PostIt::IsNameReadOnly )
									.IsSelected( this, &SGraphNode_K2PostIt::IsSelectedExclusively )
									.WrapTextAt( this, &SGraphNode_K2PostIt::GetWrapAt )
									.MultiLine(true)
									.ModiferKeyForNewLine(EModifierKey::Shift)	
								]	
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 2.0f)
							[
								SNew(SSeparator)
								.Thickness(2)
								.ColorAndOpacity(K2PostItColor::DarkGray_Trans)
							]
						]
					]
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(1.0f)
					[
						ErrorReporting->AsWidget()
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SBox)
						.MinDesiredHeight(33)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
									
								SNew(SWidgetSwitcher)
								.WidgetIndex(this, &SGraphNode_K2PostIt::WidgetIndex_CommentTextPane)
								+ SWidgetSwitcher::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										SNew(SBorder)
										.BorderImage( FAppStyle::GetBrush("NoBorder") )
										.ForegroundColor_Lambda( [this] ()
										{
											FLinearColor Color = K2PostItColor::Error;

											UEdGraphNode_K2PostIt* Owner = GetNodeObjAsK2PostIt();
											
											if (IsValid(Owner))
											{
												Color = K2PostItColor::GetNominalFontColor(Owner->CommentColor, K2PostItColor::White, K2PostItColor::Noir);
											}
											
											return Color;
										})
										.VAlign(VAlign_Fill)
										.Padding(7, 8, 7, 8)
										[
											SAssignNew(CommentTextSource, SMultiLineEditableText)
											.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Editor)
											.Text(this, &SGraphNode_K2PostIt::Text_CommentTextSource)
											.OnTextChanged(this, &SGraphNode_K2PostIt::OnTextChanged_CommentTextSource)
											.OnTextCommitted(this, &SGraphNode_K2PostIt::OnTextCommitted_CommentTextSource)
											.RevertTextOnEscape(true)
											.WrapTextAt( this, &SGraphNode_K2PostIt::GetWrapAt )
										]
									]
								]
								+ SWidgetSwitcher::Slot()
								.Padding(8)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									FormattedTextPanel.ToSharedRef()
								]
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Top)
							.Padding(PaddingAttribute_PreviewPane)
							[
								SAssignNew(PreviewPanelBox, SBox)
								.Visibility(EVisibility::SelfHitTestInvisible)
							]
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(0, 0, 0, 0)
				[
					SNew(SButton)
					.ButtonStyle(FK2PostItStyle::Get(), K2PostItStyles.ButtonStyle_EditButton)
					.ContentPadding(0)
					.OnClicked(this, &SGraphNode_K2PostIt::OnClicked_EditIcon)
					.Visibility(this, &SGraphNode_K2PostIt::Visibility_EditButton)
					.Cursor(EMouseCursor::Default)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Icons.FilledCircle"))
						.BorderBackgroundColor(K2PostItColor::DarkGray_Glass)
						.Padding(8)
						[
							SNew(SImage)
							.Image(FK2PostItStyle::GetImageBrush(K2PostItBrushes.Icon_Edit))
							.DesiredSizeOverride(FVector2D(16))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
			]
		];

	// Create comment bubble
	CommentBubble = SNew(SCommentBubble)
	.GraphNode(GraphNode)
	.Text(this, &SGraphNode_K2PostIt::GetNodeComment)
	.OnTextCommitted(this, &SGraphNode_K2PostIt::K2PostIt_OnNameTextCommited)
	.ColorAndOpacity(this, &SGraphNode_K2PostIt::GetCommentBubbleColor )
	.AllowPinning(true)
	.EnableTitleBarBubble(false)
	.EnableBubbleCtrls(false)
	.GraphLOD(this, &SGraphNode::GetCurrentLOD)
	.InvertLODCulling(true)
	.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
	.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
	.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
	.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
	.VAlign(VAlign_Top)
	[
		CommentBubble.ToSharedRef()
	];

	RebuildRichText();
}

// ------------------------------------------------------------------------------------------------

FVector2D SGraphNode_K2PostIt::ComputeDesiredSize( float ) const
{
	//float Height = TitleBar->GetDesiredSize().Y + ErrorReporting->AsWidget()->GetDesiredSize().Y + FormattedTextPanel->GetDesiredSize().Y;
	float Height = MainPanel->GetDesiredSize().Y;
	
	return FVector2D(UserSize.X, Height);
}

// ------------------------------------------------------------------------------------------------

FString SGraphNode_K2PostIt::GetNodeComment() const
{
	const FString Title = GetEditableNodeTitle();;
	return Title;
}

// ------------------------------------------------------------------------------------------------

FReply SGraphNode_K2PostIt::OnMouseButtonDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent )
{
	// If user double-clicked in the title bar area
	if(FindMouseZone(InMyGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition())) == CRWZ_TitleBar && IsEditable.Get())
	{
		// Request a rename
		RequestRename();

		// Set the keyboard focus
		if(!HasKeyboardFocus())
		{
			FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EFocusCause::SetDirectly);
		}

		return FReply::Handled();
	}
	else
	{
		// Otherwise let the graph handle it, to allow spline interactions to work when they overlap with a comment node
		return FReply::Unhandled();
	}
}

// ------------------------------------------------------------------------------------------------

FReply SGraphNode_K2PostIt::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bMouseClickEditingInterlock = true;
	
	return SGraphNodeResizable::OnMouseButtonDown(MyGeometry, MouseEvent);
}

// ------------------------------------------------------------------------------------------------

FReply SGraphNode_K2PostIt::OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	bMouseClickEditingInterlock = false;
	
	if ( (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) && bUserIsDragging )
	{
		bUserIsDragging = false;

		// End resize transaction
		ResizeTransactionPtr.Reset();

		// Update contained child Nodes
		HandleSelection( bIsSelected, true );

		return FReply::Handled().ReleaseMouseCapture();
	}
	else
	{
		return FReply::Handled().ReleaseMouseCapture();
	}
	//return FReply::Unhandled();
}

// ------------------------------------------------------------------------------------------------

int32 SGraphNode_K2PostIt::GetSortDepth() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>( GraphNode );
	return CommentNode ? CommentNode->CommentDepth : -1;
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::HandleSelection(bool bSelected, bool bUpdateNodesUnderComment) const
{
	const FVector2D NodeSize = GetDesiredSize();
	// we only want to do this after the comment has a valid desired size
	if( !NodeSize.IsZero() )
	{
		bIsSelected = bSelected;
	}
}

// ------------------------------------------------------------------------------------------------

const FSlateBrush* SGraphNode_K2PostIt::GetShadowBrush(bool bSelected) const
{
	HandleSelection(bSelected);
	
	return bSelected
		? FK2PostItStyle::GetImageBrush(K2PostItBrushes.SelectionShadow_K2PostItNode)
		: FK2PostItStyle::GetImageBrush(K2PostItBrushes.Shadow_K2PostItNode);
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const
{
	/*
	const float Fudge = 3.0f;

	HandleSelection(bSelected);

	FOverlayBrushInfo HandleBrush = FAppStyle::GetBrush( TEXT("Graph.Node.Comment.Handle") );

	HandleBrush.OverlayOffset.X = WidgetSize.X - HandleBrush.Brush->ImageSize.X - Fudge;
	HandleBrush.OverlayOffset.Y = WidgetSize.Y - HandleBrush.Brush->ImageSize.Y - Fudge;

	Brushes.Add(HandleBrush);
	*/
	return SGraphNode::GetOverlayBrushes(bSelected, WidgetSize, Brushes);
}

// ------------------------------------------------------------------------------------------------

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
void SGraphNode_K2PostIt::MoveTo( const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty)
{
	SGraphNode::MoveTo(NewPosition, NodeFilter, bMarkDirty);
}
#else
void SGraphNode_K2PostIt::MoveTo( const FVector2f& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty)
{
	SGraphNode::MoveTo(NewPosition, NodeFilter, bMarkDirty);
}
#endif

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::EndUserInteraction() const
{
	// Find any parent comments and their list of child nodes
	const FVector2D NodeSize = GetDesiredSize();
	if( !NodeSize.IsZero() )
	{
		const FVector2D NodePosition = GetPosition();
		const FSlateRect CommentRect( NodePosition.X, NodePosition.Y, NodePosition.X + NodeSize.X, NodePosition.Y + NodeSize.Y );

		TSharedPtr<SGraphPanel> Panel = GetOwnerPanel();
		FChildren* PanelChildren = Panel->GetAllChildren();
		int32 NumChildren = PanelChildren->Num();
		static FString SGraphNodeK2PostItType = "SGraphNodeK2PostIt";

		for ( int32 NodeIndex=0; NodeIndex < NumChildren; ++NodeIndex )
		{
			const TSharedPtr<SGraphNode> SomeNodeWidget = StaticCastSharedRef<SGraphNode>(PanelChildren->GetChildAt(NodeIndex));

			UObject* GraphObject = SomeNodeWidget->GetObjectBeingDisplayed();
			if ( !GraphObject->IsA<UEdGraphNode_K2PostIt>() )
			{
				continue;
			}

			const FVector2D SomeNodePosition = SomeNodeWidget->GetPosition();
			const FVector2D SomeNodeSize = SomeNodeWidget->GetDesiredSize();

			const FSlateRect NodeGeometryGraphSpace(SomeNodePosition.X, SomeNodePosition.Y, SomeNodePosition.X + SomeNodeSize.X, SomeNodePosition.Y + SomeNodeSize.Y);
			if (FSlateRect::DoRectanglesIntersect(CommentRect, NodeGeometryGraphSpace))
			{
				// This downcast *should* be valid at this point, since we verified the GraphObject is a comment node
				TSharedPtr<SGraphNode_K2PostIt> CommentWidget = StaticCastSharedPtr<SGraphNode_K2PostIt>(SomeNodeWidget);
				CommentWidget->HandleSelection(CommentWidget->bIsSelected, true);
			}
		}
	}	
}

// ------------------------------------------------------------------------------------------------

float SGraphNode_K2PostIt::GetTitleBarHeight() const
{
	return TitleBar.IsValid() ? TitleBar->GetDesiredSize().Y : 0.0f;
}

// ------------------------------------------------------------------------------------------------

FSlateRect SGraphNode_K2PostIt::GetHitTestingBorder() const
{
	return SCommentNodeDefs::HitResultBorderSize;
}

// ------------------------------------------------------------------------------------------------

FVector2D SGraphNode_K2PostIt::GetNodeMaximumSize() const
{
	return FVector2D( UserSize.X + 100, UserSize.Y + 100 );
}

// ------------------------------------------------------------------------------------------------

FSlateColor SGraphNode_K2PostIt::GetCommentBodyColor() const
{
	if (UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode))
	{
		return CommentNode->CommentColor;
	}

	return FLinearColor::White;
}

// ------------------------------------------------------------------------------------------------

FSlateColor SGraphNode_K2PostIt::GetMarkdownPreviewPaneColor() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);

	FLinearColor Color;
	
	if (CommentNode)
	{
		Color = CommentNode->CommentColor;
	}
	else
	{
		Color = FLinearColor::White;
	}

	Color = K2PostItColor::Desaturate(Color, 0.1f);
	Color = K2PostItColor::Darken(Color, 0.9f);
	Color.A *= 0.8f;

	return Color;
}

// ------------------------------------------------------------------------------------------------

FSlateColor SGraphNode_K2PostIt::GetCommentTitleBarColor() const
{
	return K2PostItColor::Noir_Glass;

	/*
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);
	if (CommentNode)
	{
		const FLinearColor Color = CommentNode->CommentColor * SCommentNodeDefs::TitleBarColorMultiplier;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
	else
	{
		const FLinearColor Color = FLinearColor::White * SCommentNodeDefs::TitleBarColorMultiplier;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
	*/
}

// ------------------------------------------------------------------------------------------------

FSlateColor SGraphNode_K2PostIt::GetCommentBubbleColor() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);
	if (CommentNode)
	{
		const FLinearColor Color = CommentNode->bColorCommentBubble ?	(CommentNode->CommentColor * SCommentNodeDefs::TitleBarColorMultiplier) :
																		GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
	else
	{
		const FLinearColor Color = FLinearColor::White * SCommentNodeDefs::TitleBarColorMultiplier;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
}

// ------------------------------------------------------------------------------------------------

FText SGraphNode_K2PostIt::Text_CommentTextSource() const
{
	if (UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode))
	{
		return CommentNode->CommentText;
	}

	return FText::GetEmpty();
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::OnTextChanged_CommentTextSource(const FText& Text)
{
	if (UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode))
	{
		CommentNode->SetPreviewCommentText(Text);
	}
}

void SGraphNode_K2PostIt::OnTextCommitted_CommentTextSource(const FText& Text, ETextCommit::Type Arg)
{
	if (UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode))
	{
		if (Arg == ETextCommit::OnCleared)
		{
			CommentNode->AbortCommentEdit();
		}
		else
		{
			CommentNode->SetCommentText(Text);
		}
	}
	
}

// ------------------------------------------------------------------------------------------------

FSlateWidgetRun::FWidgetRunInfo SGraphNode_K2PostIt::GetWidgetThing(const FTextRunInfo& RunInfo, const ISlateStyle* Style)
{
	return FSlateWidgetRun::FWidgetRunInfo(SNew(STextBlock).Text(INVTEXT("HELLO WORLD")), 5);
}

// ------------------------------------------------------------------------------------------------

bool SGraphNode_K2PostIt::CanBeSelected(const FVector2D& MousePositionInNode) const
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
	const EResizableWindowZone InMouseZone = FindMouseZone(FVector2D(MousePositionInNode));
#else
	const EResizableWindowZone InMouseZone = FindMouseZone(FVector2f(MousePositionInNode));
#endif
	return CRWZ_TitleBar == InMouseZone;
}

// ------------------------------------------------------------------------------------------------

FVector2D SGraphNode_K2PostIt::GetDesiredSizeForMarquee() const
{
	const float TitleBarHeight = TitleBar.IsValid() ? TitleBar->GetDesiredSize().Y : 0.0f;
	return FVector2D(UserSize.X, TitleBarHeight);
}

// ------------------------------------------------------------------------------------------------

FSlateRect SGraphNode_K2PostIt::GetTitleRect() const
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
	const FVector2D NodePosition = GetPosition();
#else
	const FVector2f NodePosition = GetPosition2f();
#endif
	const FVector2D NodeSize  = TitleBar.IsValid() ? TitleBar->GetDesiredSize() : GetDesiredSize();
	return FSlateRect( NodePosition.X, NodePosition.Y, NodePosition.X + NodeSize.X, NodePosition.Y + NodeSize.Y ) + SCommentNodeDefs::TitleBarOffset;
}

// ------------------------------------------------------------------------------------------------


EVisibility SGraphNode_K2PostIt::Visibility_EditButton() const
{
	if (InlineEditableText->HasAnyUserFocusOrFocusedDescendants())
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

// ------------------------------------------------------------------------------------------------

FSlateColor SGraphNode_K2PostIt::ColorAndOpacity_TitleText() const
{
	if (GraphNode != nullptr)
	{
		// Trying to catch a non-reproducible crash in this function
		check(GraphNode->IsValidLowLevel());

		FText TitleText = GraphNode->GetNodeTitle(ENodeTitleType::EditableTitle);

		// TODO this should compare localized text
		if (TitleText.IsEmpty())//|| TitleText.ToString() == "Comment")
		{
			if (const UEdGraphNode_K2PostIt* Node = GetNodeObjAsK2PostIt())
			{
				return K2PostItColor::GetNominalFontColor(Node->CommentColor, K2PostItColor::Gray, K2PostItColor::Gray);				
			}

			return K2PostItColor::Gray_SemiTrans;
		}
	}
	
	return FSlateColor::UseForeground();
}

// ------------------------------------------------------------------------------------------------

void SGraphNode_K2PostIt::PopulateMetaTag(FGraphNodeMetaData* TagMeta) const
{
	if (GraphNode != nullptr)
	{
		// We want the name of the blueprint as our name - we can find the node from the GUID
		UObject* Package = GraphNode->GetOutermost();
		UObject* LastOuter = GraphNode->GetOuter();
		while (LastOuter->GetOuter() != Package)
		{
			LastOuter = LastOuter->GetOuter();
		}
		TagMeta->Tag = FName(*FString::Printf(TEXT("GraphNode_%s_%s"), *LastOuter->GetFullName(), *GraphNode->NodeGuid.ToString()));
		TagMeta->OuterName = LastOuter->GetFullName();
		TagMeta->GUID = GraphNode->NodeGuid;
		TagMeta->FriendlyName = FString::Printf(TEXT("%s in %s"), *GraphNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString(), *TagMeta->OuterName);
	}
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE