// Fill out your copyright notice in the Description page of Project Settings.

#include "SMotionFieldTimeline.h"


#include "Rendering/DrawElements.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Colors/SColorBlock.h"
#include "EditorStyleSet.h"
#include "MotionField.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "ScopedTransaction.h"
#include "MotionFieldEditor/MotionFieldEditorCommands.h"
#include "MotionKey.h"
#include "MotionFieldEditor/SMotionFieldTimelineTrack.h"

#include "MotionFieldEditor.h"

#include "Animation/AnimSequence.h"

#define LOCTEXT_NAMESPACE "MotionFieldEditor"

//////////////////////////////////////////////////////////////////////////
// Inline widgets

//#include "MotionFieldEditor/STimelineHeader.h"

//////////////////////////////////////////////////////////////////////////
// SMotionFieldTimeline

void SMotionFieldTimeline::Construct(const FArguments& InArgs, TSharedPtr<FUICommandList> InCommandList, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor, const uint8 InTagIdx)
{
	MotionFieldBeingEdited = InArgs._MotionFieldBeingEdited;
	PlayTime = InArgs._PlayTime;

	TagIdx = InTagIdx;

	SourceAnimationAttr = InArgs._SourceAnimationAttr;
	CommandList = InCommandList;
	MotionFieldEditorPtr = InMotionFieldEditor;
	SlateUnitsPerFrame = 120.0f;

	BackgroundPerFrameSlices = SNew(SHorizontalBox);
	
	TimelineTrack = SNew(SMotionFieldTimelineTrack, CommandList, MotionFieldEditorPtr, TagIdx)
		.SlateUnitsPerFrame(this, &SMotionFieldTimeline::GetSlateUnitsPerFrame)
		.MotionFieldBeingEdited(MotionFieldBeingEdited)
		//.OnSelectionChanged(OnSelectionChanged)
		.SourceAnimationAttr(this, &SMotionFieldTimeline::GetSourceAnimationAttr);
	

	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(.9f)
		[
			SNew(SOverlay)

	+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			
	+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		[
			SNew(SBox)
			.HeightOverride(FMotionFieldUIConstants::FrameHeight)
		[
			TimelineTrack.ToSharedRef()
		]
		]
		]

	// Empty MotionField instructions
	+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Visibility(this, &SMotionFieldTimeline::NoFramesWarningVisibility)
		.Text(LOCTEXT("EmptyAnimInstruction", "SelectAnAnimToSetTagRanges"))
		]
		]
	+ SHorizontalBox::Slot()//-----------------------
		.FillWidth(.1f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()//-----------------------
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
		    .Text(this, &SMotionFieldTimeline::GetTagTimelineText)
		]
		]

		]
		];

	//NumKeyFramesFromLastRebuild = (MotionField != nullptr) ? MotionField->MotionKeys.Num() : 0;
	UAnimSequence* SourceAnimation = SourceAnimationAttr.Get();
	NumKeyFramesFromLastRebuild = (SourceAnimation != nullptr) ? SourceAnimation->GetNumberOfFrames() : 0;
	NumFramesFromLastRebuild = (SourceAnimation != nullptr) ? SourceAnimation->GetNumberOfFrames() : 0;
	RebuildPerFrameBG();
}

UAnimSequence * SMotionFieldTimeline::GetSourceAnimationAttr() const
{
	if (SourceAnimationAttr.Get() != nullptr)
	{
		return SourceAnimationAttr.Get();
	}
	return nullptr;
}

void SMotionFieldTimeline::RebuildTimelineTrack() const
{
	TimelineTrack.Get()->Rebuild();
}

int32 SMotionFieldTimeline::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const float CurrentTimeSecs = PlayTime.Get();
	UAnimSequence* SourceAnimation = SourceAnimationAttr.Get();
	const float TotalTimeSecs = (SourceAnimation != nullptr) ? SourceAnimation->GetPlayLength() : 0.0f;

	const float SlateTotalDistance = AllottedGeometry.GetLocalSize().X * 0.9f;// SlateUnitsPerFrame * TotalNumFrames;
	const float CurrentTimeXPos = (CurrentTimeSecs / TotalTimeSecs) * SlateTotalDistance;// (((CurrentTimeSecs / TotalTimeSecs) * SlateTotalDistance) - AnimationScrollBarPosition) + FMath::Clamp((AllottedGeometry.GetLocalSize().X + AnimationScrollBarPosition) - SlateTotalDistance, 0.0f, AnimationScrollBarPosition);

	// Draw a line for the current scrub cursor
	++LayerId;
	TArray<FVector2D> LinePoints;
	LinePoints.Add(FVector2D(CurrentTimeXPos, 0.f));
	LinePoints.Add(FVector2D(CurrentTimeXPos, AllottedGeometry.GetLocalSize().Y));


	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		LinePoints,
		ESlateDrawEffect::None,
		FLinearColor::Red
	);

	return LayerId;
}

FReply SMotionFieldTimeline::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsControlDown())
	{
		const float DirectionScale = 0.08f;
		const float MinFrameSize = 16.0f;
		const float Direction = MouseEvent.GetWheelDelta();
		const float NewUnitsPerFrame = FMath::Max(MinFrameSize, SlateUnitsPerFrame * (1.0f + Direction * DirectionScale));
		SlateUnitsPerFrame = NewUnitsPerFrame;

		CheckForRebuild(/*bRebuildAll=*/ true);

		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

TSharedRef<SWidget> SMotionFieldTimeline::GenerateContextMenu()
{
	FMenuBuilder MenuBuilder(true, CommandList);
	MenuBuilder.BeginSection("KeyframeActions", LOCTEXT("KeyframeActionsSectionHeader", "Keyframe Actions"));

	// 		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);
	// 		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
	// 		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste);
//	MenuBuilder.AddMenuEntry(FMotionFieldEditorCommands::Get().AddNewFrame);

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

FReply SMotionFieldTimeline::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		TSharedRef<SWidget> MenuContents = GenerateContextMenu();
		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

void SMotionFieldTimeline::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	CheckForRebuild();
}
FReply SMotionFieldTimeline::OnKeyDown(const FGeometry & MyGeometry, const FKeyEvent & InKeyEvent)
{

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}
FReply SMotionFieldTimeline::OnKeyUp(const FGeometry & MyGeometry, const FKeyEvent & InKeyEvent)
{
	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}



/*
void SMotionFieldTimeline::SetMotionFieldEditorPtr(TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{
	if (InMotionFieldEditorPtr.IsValid())
	{
		MotionFieldEditorPtr = InMotionFieldEditorPtr;
	}
}
*/
void SMotionFieldTimeline::CheckForRebuild(bool bRebuildAll)
{
	UAnimSequence* SourceAnimation = SourceAnimationAttr.Get();
	const int32 NewNumKeyframes = (SourceAnimation != nullptr) ? SourceAnimation->GetNumberOfFrames() : 0;

	
	const bool PendingRebuild = MotionFieldEditorPtr.Get()->GetPendingTimelineRebuild();

	if ((NewNumKeyframes != NumKeyFramesFromLastRebuild) || PendingRebuild || bRebuildAll)
	{
		MotionFieldEditorPtr.Get()->SetPendingTimelineRebuild(false);

		NumKeyFramesFromLastRebuild = NewNumKeyframes;
		TimelineTrack->Rebuild();
	}
	
	//const int32 NewNumFrames = (SourceAnimation != nullptr) ? SourceAnimation->GetNumberOfFrames() : 0;;
	if ((NewNumKeyframes != NumFramesFromLastRebuild) || bRebuildAll)
	{
		NumFramesFromLastRebuild = NewNumKeyframes;
		//TimelineHeader->Rebuild();
		RebuildPerFrameBG();
	}
	
}

EVisibility SMotionFieldTimeline::NoFramesWarningVisibility() const
{
	UAnimSequence* SourceAnimation = SourceAnimationAttr.Get();
	
	const int32 TotalNumFrames = (SourceAnimation != nullptr) ? SourceAnimation->GetNumberOfFrames() : 0;;
	return (TotalNumFrames == 0) ? EVisibility::Visible : EVisibility::Collapsed;
	
}

void SMotionFieldTimeline::RebuildPerFrameBG()
{
	const FLinearColor BackgroundColors[2] = { FLinearColor(1.0f, 1.0f, 1.0f, 0.05f), FLinearColor(0.0f, 0.0f, 0.0f, 0.05f) };

	BackgroundPerFrameSlices->ClearChildren();
	for (int32 FrameIndex = 0; FrameIndex < NumFramesFromLastRebuild; ++FrameIndex)
	{
		const FLinearColor& BackgroundColorForFrameIndex = BackgroundColors[FrameIndex & 1];

		BackgroundPerFrameSlices->AddSlot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(SlateUnitsPerFrame)
			[
				SNew(SColorBlock)
				.Color(BackgroundColorForFrameIndex)
			]
			];
	}
}

void SMotionFieldTimeline::AnimationScrollBar_OnUserScrolled(float ScrollOffset)
{
	AnimationScrollBarPosition = ScrollOffset;
}

FText SMotionFieldTimeline::GetTagTimelineText() const
{
	return FText::FromString(MotionFieldEditorPtr.Get()->GetMotionFieldBeingEdited()->GetTagAtIndex(TagIdx));
}

//////////////////////////////////////////////////////////////////////////


USetMotionFieldPropertiesSettings::USetMotionFieldPropertiesSettings(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	MotionFieldTimeStep = 0.1f;
	MotionFieldTags.Empty();
}

void SSetPropertiesDialog::Construct(const FArguments & InArgs, UMotionField* SourceMotionField, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor)
{
	
	SourceMotFil = SourceMotionField;
	MotionFieldEditorPtr = InMotionFieldEditor;
	SetMotionFieldPropertiesSettings = NewObject<USetMotionFieldPropertiesSettings>();

	SetMotionFieldPropertiesSettings->AddToRoot();

	SourceMotFil->GetMotionFieldProperties(SetMotionFieldPropertiesSettings->MotionFieldTimeStep, SetMotionFieldPropertiesSettings->MotionFieldTags);

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(/*bUpdateFromSelection=*/ false, /*bLockable=*/ false, /*bAllowSearch=*/ false, /*InNameAreaSettings=*/ FDetailsViewArgs::HideNameArea, /*bHideSelectionTip=*/ true);
	MainPropertyView = EditModule.CreateDetailView(DetailsViewArgs);
	MainPropertyView->SetObject(SetMotionFieldPropertiesSettings);




	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryTop"))
		.Padding(FMargin(1.0f, 1.0f, 1.0f, 0.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.Padding(1.0f)
		.AutoHeight()
		[
			MainPropertyView.ToSharedRef()
		]
	+ SVerticalBox::Slot()
		.Padding(1.0f)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(1)
		+ SUniformGridPanel::Slot(0, 0)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
		.ForegroundColor(FLinearColor::White)
		.Text(LOCTEXT("SetButton", "Set"))
		.OnClicked(this, &SSetPropertiesDialog::SetClicked)
		]
	+ SUniformGridPanel::Slot(1, 0)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.ForegroundColor(FLinearColor::White)
		.Text(LOCTEXT("CancelButton", "Cancel"))
		.OnClicked(this, &SSetPropertiesDialog::CancelClicked)
		]
		]
		]
		]
		];
}

SSetPropertiesDialog::~SSetPropertiesDialog()
{
}

bool SSetPropertiesDialog::ShowWindow(UMotionField * SourceMotionField, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor)
{

	const FText TitleText = NSLOCTEXT("MotionMatching", "Set_Properties", "Set Motion Field Properties");
	// Create the window to pick the class
	TSharedRef<SWindow> SetPropertiesWindow = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(550.f, 375.f))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SSetPropertiesDialog> AddNewContextDialog = SNew(SSetPropertiesDialog, SourceMotionField, InMotionFieldEditor);

	SetPropertiesWindow->SetContent(AddNewContextDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();

	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(SetPropertiesWindow, RootWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(SetPropertiesWindow);
	}

	return false;
}

FReply SSetPropertiesDialog::SetClicked()
{
	if (SetMotionFieldPropertiesSettings->MotionFieldTimeStep > 0.f)
	{
		SourceMotFil->SetProperties(SetMotionFieldPropertiesSettings->MotionFieldTimeStep, SetMotionFieldPropertiesSettings->MotionFieldTags);

		MotionFieldEditorPtr.Get()->RebuildTagTimelines();

		CloseContainingWindow();

		return FReply::Handled();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("WrongTimeStep", "TimeStep must be more than 0"));
		return FReply::Unhandled();
	}

	return FReply::Handled();
}

FReply SSetPropertiesDialog::CancelClicked()
{
	CloseContainingWindow();
	return FReply::Handled();
}

void SSetPropertiesDialog::CloseContainingWindow()
{
	FWidgetPath WidgetPath;
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared(), WidgetPath);
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}


#undef LOCTEXT_NAMESPACE
