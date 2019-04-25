
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "Animation/AnimNode_AssetPlayerBase.h"

#include "Animation/AnimInstanceProxy.h"
#include "MotionField.h"
#include "AnimNode_MotionField.generated.h"
/**
*
*/



/*This was the Node That I was using before but now Just use it to test AnimGraph Node code*/
USTRUCT(BlueprintInternalUseOnly)
struct MOTIONMATCHING_API FAnimNode_MotionField : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionField)
		UMotionField* MotionField;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		FTrajectoryData DesiredTrajectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float Responsiveness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float PoseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float ChangeTimeLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		int ChangeAmountLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		bool DrawDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float BlendTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		bool AuthorSwitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		bool Switch;

public:

	FAnimNode_MotionField();


	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime();
	virtual float GetCurrentAssetTimePlayRateAdjusted();
	virtual float GetCurrentAssetLength();
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void OverrideAsset(UAnimationAsset* NewAsset) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface



protected:

	UPROPERTY()
		int CurrentAnimIndex;
	UPROPERTY()
		float CurrentAnimTime;

	//////////////////BLEND STUFF
	UPROPERTY()
		float BlendTimer;
	UPROPERTY()
		TArray <FTransform> LastBones;
	UPROPERTY()
		int CurrentMotKeyIndex;
	////////////////////////////////////////// TRAJECTORY STUFF
	/*
	UPROPERTY()
	float TrajectoryTimer;
	UPROPERTY()
	TArray <FTransform> TrajectoryPoints;
	UPROPERTY()
	FTrajectoryData PresentTrajectory;
	*/
	////////////////////////////////////////// JOINT DATA
	UPROPERTY()
		TArray <FJointData> JointsData;
	UPROPERTY()
		FVector Vel;
	//////////////////////////////////////////
	UPROPERTY()
		bool bLoopAnimation;
	UPROPERTY()
		int ChangeCounter;
	UPROPERTY()
		float ChangeTimer;
	////
	UPROPERTY()
		float PlayRate;
	UPROPERTY()
		bool LastSwitch;
	UPROPERTY()
		bool Pause;

private:

	UAnimSequence* GetCurrentAnim();

	void PreEvaluate(const FAnimationUpdateContext& Context);

	void CheckSwitch();

	FVector GetBoneCSTMFromPose(FPoseContext & Output, const FName BoneName);

	void DrawPresentTrajectory(const FAnimationUpdateContext& Context);
	void DrawTrajectoryPoint(const FAnimationUpdateContext& Context);
	void DrawJointData(const FAnimationUpdateContext& Context);

	void Compute();

	void MotionUpdate(const FAnimationUpdateContext& Context);


	void GetBlendPose(FCompactPose& OutPose, /*out*/ FBlendedCurve& OutCurve);

	//TArray <FVector> PastCSPos;
};
