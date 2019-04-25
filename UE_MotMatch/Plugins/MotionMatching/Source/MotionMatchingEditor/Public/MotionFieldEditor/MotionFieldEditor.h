// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "UObject/GCObject.h"
#include "Framework/Docking/TabManager.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "MotionField.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"

class SMotionFieldEditorViewport;
class UDebugSkelMeshComponent;

class USkeletalMesh;
class UAnimSequence;
class SContextList;
//class SMotionFieldTimeline;

//////////////////////////////////////////////////////////////////////////
// FMotionFieldEditor FWorkflowCentricApplication


class FMotionFieldEditor : public FAssetEditorToolkit, public FGCObject
{
public:
	FMotionFieldEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	// End of FAssetEditorToolkit

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

public:
	void InitMotionFieldEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UMotionField* InitMotionField);

	UMotionField* GetMotionFieldBeingEdited() const { return MotionFieldBeingEdited; }
	UDebugSkelMeshComponent* GetPreviewComponentExtraction() const;
	UAnimSequence* GetSourceAnimation() const;
	bool SetPreviewComponentSkeletalMesh(USkeletalMesh* SkeletalMesh) const;
	bool SetExtractionAnimSequence(UAnimSequence* AnimSequence) const;

	
	//----------------------------------------------------------------------------------------------------------
	void AddRangeToTagHelper(const float RangeTime, const uint8 AtTagIdx);
	void RemoveRangeFromTagHelper(const float RangeTime, const uint8 AtTagIdx);
	bool IsTimeTagged(const float Time, const uint8 AtTagIdx);
	//-----------------------------------------------------------------------------------------------------

	void AddNewExtractionContext(TArray <UAnimSequence*> FromSequences);
	void DeleteExtractionContext(const int AtIndex);
	void SetCurrentExtractionContext(const int AtIndex);

	
	int CurrentExtractionContextIndex;

	void ProcessAllClicked();
	void ClearAllClicked();
	void SetPropertiesClicked();


	void SetCurrentFrame(int32 NewIndex)
	{
		const int32 TotalLengthInFrames = GetTotalFrameCount();
		if (TotalLengthInFrames > 0)
		{
			int32 ClampedIndex = FMath::Clamp<int32>(NewIndex, 0, TotalLengthInFrames);
			SetPlaybackPosition(ClampedIndex / GetFramesPerSecond());
		}
		else
		{
			SetPlaybackPosition(0.0f);
		}
	}

	void GoToTime(float ToTime)
	{
			SetPlaybackPosition(ToTime);
	}

	void MySetCurrentFrame(int32 NewIndex)
	{
		if (GetSourceAnimation())
		{
			SetPlaybackPosition(GetSourceAnimation()->GetTimeAtFrame(NewIndex));
		}
		else
		{
			SetPlaybackPosition(0.0f);
		}
	}


	bool IsKeyableFrame(const int32 FrameIndex)
	{
		if (GetSourceAnimation())
		{
			float Time = GetSourceAnimation()->GetTimeAtFrame(FrameIndex);
			if ((Time >= 1.0f) && (Time <= (GetSourceAnimation()->GetPlayLength() - 1.0f)))
			{
				return true;
			}
		}
		return false;
	}


	FText GetContextAnimationName(const int AtContextIndex);

	bool IsValidContext(const int AtContextIndex);

	bool IsExtractedFrame(const int FrameIndex);

	FString GetMotionFieldSkeletonName();

   
	bool AnimationAlreadyAddedToContextArray(const FName SequenceName);

	bool GetCurrentTrajectoryData(FTrajectoryData& PresentTrajectory, FTrajectoryData& FutureTrajectory, FTransform& PastTM);

	
	bool GetPendingTimelineRebuild()
	{
		return PendingTimelineRebuild;
	}

	void SetPendingTimelineRebuild(const bool In)
	{
		PendingTimelineRebuild = In;
	}


	//////////////---------------------------------------------------------------------------------


protected:
	UMotionField* MotionFieldBeingEdited;


	TSharedPtr<SMotionFieldEditorViewport> ViewportPtr;
	TSharedPtr <SContextList> ContextList;
	TSharedPtr<SVerticalBox> TagTimelineBoxPtr;
	
	// Selection set
	//int32 CurrentPendingKeyframe;

	// Range of times currently being viewed
	mutable float ViewInputMin;
	mutable float ViewInputMax;
	mutable float LastObservedSequenceLength;


	

protected:

	bool PendingTimelineRebuild = false;

	float GetFramesPerSecond() const
	{
		return 30.f;//return MotionFieldBeingEdited->GetFramesPerSecond();
	}

	int32 GetCurrentFrame() const
	{

		const int32 TotalLengthInFrames = GetTotalFrameCount();

		if (TotalLengthInFrames == 0)
		{
			return INDEX_NONE;
		}
		else
		{
			return FMath::Clamp<int32>((int32)(GetPlaybackPosition() * GetFramesPerSecond()), 0, TotalLengthInFrames);
		}

	}

	

protected:
	void BindCommands();
	void ExtendMenu();
	void ExtendToolbar();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	
	//void SetSelection(int32 NewSelection);
	//bool HasValidSelection() const;

	
	
	FReply OnClick_Forward();
	FReply OnClick_Forward_Step();
	FReply OnClick_Forward_End();
	FReply OnClick_Backward();
	FReply OnClick_Backward_Step();
	FReply OnClick_Backward_End();
	FReply OnClick_ToggleLoop();

	uint32 GetTotalFrameCount() const;
	uint32 GetTotalFrameCountPlusOne() const;
	float GetTotalSequenceLength() const;
	float GetPlaybackPosition() const;
	void SetPlaybackPosition(float NewTime);
	bool IsLooping() const;
	EPlaybackMode::Type GetPlaybackMode() const;

	float GetViewRangeMin() const;
	float GetViewRangeMax() const;
	void SetViewRange(float NewMin, float NewMax);

private:
	void InitTagHelpers();
	void ResetTagHelpers();
public:
	void RebuildTagTimelines();
};
