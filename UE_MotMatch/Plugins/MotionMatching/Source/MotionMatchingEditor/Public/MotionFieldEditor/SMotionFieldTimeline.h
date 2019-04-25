// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"


#include "UObject/NoExportTypes.h"

#include"SMotionFieldTimeline.generated.h"


class FPaintArgs;
class FSlateWindowElementList;
class FUICommandList;
class SMotionFieldTimelineTrack;
class SHorizontalBox;
class STimelineHeader;
class FMotionFieldEditor;

// Called when the selection changes
//DECLARE_DELEGATE(FOnFramesExtractedDelegate);

class SMotionFieldTimeline : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldTimeline)
		: _MotionFieldBeingEdited(nullptr)
		, _PlayTime(0)
		, _SourceAnimationAttr(nullptr)
	{}

	SLATE_ATTRIBUTE(class UMotionField*, MotionFieldBeingEdited)
		SLATE_ATTRIBUTE(float, PlayTime)
		SLATE_ATTRIBUTE(class UAnimSequence*, SourceAnimationAttr)
		//SLATE_EVENT(FOnFramesExtractedDelegate, OnFramesExtracted)
		SLATE_END_ARGS()
		//, TSharedPtr <FMotionFieldEditor> InMotionFieldEditor
		void Construct(const FArguments& InArgs, TSharedPtr<FUICommandList> InCommandList, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor,const uint8 InTagIdx);

	// SWidget interface
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	virtual FReply OnKeyDown(const FGeometry & MyGeometry, const FKeyEvent & InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry & MyGeometry, const FKeyEvent & InKeyEvent) override;
	// End of SWidget interface

	//void SetMotionFieldEditorPtr(TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr);

	UAnimSequence* GetSourceAnimationAttr() const;


	void RebuildTimelineTrack() const;


private:

	uint8 TagIdx;

	void RebuildPerFrameBG();

	TSharedRef<SWidget> GenerateContextMenu();
	EVisibility NoFramesWarningVisibility() const;
	void CheckForRebuild(bool bRebuildAll = false);

	float GetSlateUnitsPerFrame() const { return SlateUnitsPerFrame; }

	void AnimationScrollBar_OnUserScrolled(float ScrollOffset);

private:
	TSharedPtr<SHorizontalBox> BackgroundPerFrameSlices;
	TSharedPtr<class STimelineHeader> TimelineHeader;
	TSharedPtr<class SMotionFieldTimelineTrack> TimelineTrack;

	TAttribute<class UMotionField*> MotionFieldBeingEdited;
	TAttribute<float> PlayTime;
	TAttribute<class UAnimSequence*> SourceAnimationAttr;

	TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr;

	TSharedPtr<FUICommandList> CommandList;

	//FOnFramesExtractedDelegate OnFramesExtracted;

	float SlateUnitsPerFrame;
	float AnimationScrollBarPosition;

	// Observer values (used to detect if the widget needs to be rebuilt)
	int32 NumFramesFromLastRebuild;
	int32 NumKeyFramesFromLastRebuild;

	
protected:
	FText GetTagTimelineText() const;
};


UCLASS()
class MOTIONMATCHINGEDITOR_API USetMotionFieldPropertiesSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Category = SearchSettings, EditAnywhere, meta = (HideAlphaChannel))
		float MotionFieldTimeStep;
	UPROPERTY(Category = SearchSettings, EditAnywhere, meta = (HideAlphaChannel))
		TArray <FString> MotionFieldTags;
	USetMotionFieldPropertiesSettings(const FObjectInitializer& ObjectInitializer);
};

class MOTIONMATCHINGEDITOR_API SSetPropertiesDialog : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSetPropertiesDialog) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UMotionField* SourceMotionField, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor);

	~SSetPropertiesDialog();

	static bool ShowWindow(UMotionField* SourceMotionField, TSharedPtr<FMotionFieldEditor> InMotionFieldEditor);


private:

	TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr;
	UMotionField* SourceMotFil;

	class USetMotionFieldPropertiesSettings* SetMotionFieldPropertiesSettings;

	FReply SetClicked();

	FReply CancelClicked();

	void CloseContainingWindow();

	TSharedPtr<class IDetailsView> MainPropertyView;
};
