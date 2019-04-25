// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Input/DragAndDrop.h"
#include "Input/Reply.h"
#include "Layout/Margin.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "MotionFieldEditor/SMotionFieldTimeline.h"
#include "Types/SlateStructs.h"
#include "MotionField.h"
#include "ScopedTransaction.h"

class FUICommandList;
class SHorizontalBox;
class UAnimSequence;

namespace FMotionFieldUIConstants
{
	const float HandleWidth = 12.0f;
	const float FrameHeight = 48;
	const float HeightBeforeFrames = 16;
	const FMargin FramePadding(0.0f, 7.0f, 0.0f, 7.0f);
};


//////////////////////////////////////////////////////////////////////////
// SMotionFieldKeyframeWidget

class SMotionFieldKeyframeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldKeyframeWidget)
		: _SlateUnitsPerFrame(1)
		, _MotionFieldBeingEdited(nullptr)
	{}

	SLATE_ATTRIBUTE(float, SlateUnitsPerFrame)
		SLATE_ATTRIBUTE(class UMotionField*, MotionFieldBeingEdited)
		
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const float Time, const uint8 InTagIdx, TSharedPtr<FUICommandList> InCommandList, TSharedPtr<class FMotionFieldEditor> InMotionFieldEditor);

	// SWidget interface
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }


	FSlateColor GetSelectionColor() const;

	FSlateColor GetBorderColor() const;


protected:
	
	FText GetStepIndexText() const;


protected:
	float KeyTime;
	uint8 TagIdx;

	TAttribute<class UMotionField*> MotionFieldBeingEdited;
	//FOnMotionFieldKeyframeSelectionChanged OnSelectionChanged;
	TSharedPtr<FUICommandList> CommandList;
private:
	bool Tagged;
	TSharedPtr<class FMotionFieldEditor> MotionFieldEditorPtr;
};

//////////////////////////////////////////////////////////////////////////
// SMotionFieldTimelineTrack

class SMotionFieldTimelineTrack : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldTimelineTrack)
		: _SlateUnitsPerFrame(1)
		, _MotionFieldBeingEdited(nullptr)
		, _SourceAnimationAttr(nullptr)
	{}

	SLATE_ATTRIBUTE(float, SlateUnitsPerFrame)
		SLATE_ATTRIBUTE(class UMotionField*, MotionFieldBeingEdited)
		//SLATE_EVENT(FOnMotionFieldKeyframeSelectionChanged, OnSelectionChanged)
		SLATE_ATTRIBUTE(UAnimSequence*, SourceAnimationAttr)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TSharedPtr<FUICommandList> InCommandList, TSharedPtr<class FMotionFieldEditor> InMotionFieldEditorPtr, const uint8 InTagIdx);

	void Rebuild();



private:

	uint8 TagIdx;

	TAttribute<float> SlateUnitsPerFrame;
	TAttribute< class UMotionField* > MotionFieldBeingEdited;

	TSharedPtr<SHorizontalBox> MainBoxPtr;

	float HandleWidth;

	//FOnMotionFieldKeyframeSelectionChanged OnSelectionChanged;
	TSharedPtr<FUICommandList> CommandList;
	TAttribute<UAnimSequence*> SourceAnimationAttr;

	TSharedPtr<class FMotionFieldEditor> MotionFieldEditorPtr;
};
