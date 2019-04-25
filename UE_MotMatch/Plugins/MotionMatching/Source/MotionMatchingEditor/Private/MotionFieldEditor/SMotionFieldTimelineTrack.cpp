// Fill out your copyright notice in the Description page of Project Settings.

#include "SMotionFieldTimelineTrack.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "AssetData.h"
#include "Framework/Commands/GenericCommands.h"

#include "MotionMatchingStyle.h"

#include "MotionFieldEditor/MotionFieldEditorCommands.h"

#include "DragAndDrop/AssetDragDropOp.h"
#include "Editor.h"
#include "PropertyCustomizationHelpers.h"
#include "MotionField.h"
#include "Toolkits/AssetEditorManager.h"

#include "Animation/AnimSequence.h"

#include "MotionFieldEditor.h"

#define LOCTEXT_NAMESPACE "MotionFieldEditor"


void SMotionFieldKeyframeWidget::Construct(const FArguments& InArgs, const float Time, const uint8 InTagIdx, TSharedPtr<FUICommandList> InCommandList, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor)
{
	KeyTime = Time;
	TagIdx = InTagIdx;

	CommandList = MakeShareable(new FUICommandList);
	CommandList->Append(InCommandList.ToSharedRef());

	MotionFieldBeingEdited = InArgs._MotionFieldBeingEdited;
	//OnSelectionChanged = InArgs._OnSelectionChanged;
	MotionFieldEditorPtr = InMotionFieldEditor;
	
	Tagged = MotionFieldEditorPtr.Get()->IsTimeTagged(KeyTime, TagIdx);

	////////////////////////////////

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		[
			SNew(SBorder)
			.OnMouseButtonUp(this, &SMotionFieldKeyframeWidget::OnMouseButtonUp)//(FEditorStyle::Get(), "FlatButton")
		   // .ForegroundColor(this, &SMotionFieldKeyframeWidget::GetBorderColor)
		.BorderBackgroundColor(this, &SMotionFieldKeyframeWidget::GetBorderColor)
		[
		
			SNew(SVerticalBox)

	        + SVerticalBox::Slot()
		
		[
			SNew(STextBlock)
				.Text(this, &SMotionFieldKeyframeWidget::GetStepIndexText)
				.ColorAndOpacity(this, &SMotionFieldKeyframeWidget::GetSelectionColor)
				//.Visibility(this, &SMotionFieldKeyframeWidget::GetSelectionVisibility)
		]
		]
		]
		];
}

FReply SMotionFieldKeyframeWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Tagged = !Tagged;
	//MotionFieldEditorPtr.Get()->MySetCurrentFrame(FrameIndex);
	if (Tagged)
	{
		MotionFieldEditorPtr.Get()->AddRangeToTagHelper(KeyTime, TagIdx);
	}
	else
	{
		MotionFieldEditorPtr.Get()->RemoveRangeFromTagHelper(KeyTime, TagIdx);
	}
	MotionFieldEditorPtr.Get()->GoToTime(KeyTime);

	return FReply::Handled();
}

FSlateColor SMotionFieldKeyframeWidget::GetSelectionColor() const
{
	if (Tagged)
	{
		return FSlateColor(FLinearColor::FromSRGBColor(FColor::Green));
	}
	return FSlateColor(FLinearColor::FromSRGBColor(FColor::White));
}

FSlateColor SMotionFieldKeyframeWidget::GetBorderColor() const
{

	if (FMath::RandBool())//IsHovered())
	{
		return FSlateColor(FLinearColor::Red);
	}
	return FSlateColor(FLinearColor::White);
}




FText SMotionFieldKeyframeWidget::GetStepIndexText() const
{
	return FText::AsNumber(KeyTime);
}

//////////////////////////////////////////////////////////////////////////
// SMotionFieldTimelineTrack

void SMotionFieldTimelineTrack::Construct(const FArguments& InArgs, TSharedPtr<FUICommandList> InCommandList, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor, const uint8 InTagIdx)
{
	TagIdx = InTagIdx;
	CommandList = InCommandList;
	SlateUnitsPerFrame = InArgs._SlateUnitsPerFrame;
	MotionFieldBeingEdited = InArgs._MotionFieldBeingEdited;
	//OnSelectionChanged = InArgs._OnSelectionChanged;
	SourceAnimationAttr = InArgs._SourceAnimationAttr;
	MotionFieldEditorPtr = InMotionFieldEditor;

	ChildSlot
		[
			SAssignNew(MainBoxPtr, SHorizontalBox)
		];

	Rebuild();
}

void SMotionFieldTimelineTrack::Rebuild()
{
	MainBoxPtr->ClearChildren();
	
	if (UAnimSequence* SourceAnimation = SourceAnimationAttr.Get())
	{
		float SequenceLength = SourceAnimation->GetPlayLength();
		float AccumulatedTime = 0.f;
		float TimeStep = MotionFieldEditorPtr.Get()->GetMotionFieldBeingEdited()->GetTimeStep();

		while (AccumulatedTime <= SequenceLength)
		{
			MainBoxPtr->AddSlot()
				.FillWidth(0.5f)
				[
					SNew(SMotionFieldKeyframeWidget, AccumulatedTime, TagIdx ,CommandList, MotionFieldEditorPtr)
					.SlateUnitsPerFrame(this->SlateUnitsPerFrame)
				.MotionFieldBeingEdited(this->MotionFieldBeingEdited)
				//.OnSelectionChanged(this->OnSelectionChanged)
				];
			AccumulatedTime += TimeStep;
		}
	}
	
}

#undef LOCTEXT_NAMESPACE

