// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "Animation/AnimNode_AssetPlayerBase.h"

#include "Animation/AnimInstanceProxy.h"
#include "MotionField.h"
#include "AnimNode_MotionMatching.generated.h"
/**
*
*/

/*Helper Struct to hold the parameters for Blending Animations Inside the Motion Matching AnimGraph Node*/
USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FMotionAnim
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData")
		int32 AnimIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData")
		float Position = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData")
		float BlendTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData")
		float Weight = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData")
		bool Maxed;

	FMotionAnim()
	{
		AnimIndex = 0;
		Position = 0.f;
		BlendTime = 0.f;
		Weight = 0.f;
		Maxed = false;
	}

	FMotionAnim(const int32 InAnimIndex, const float StartTime)
	{
		AnimIndex = InAnimIndex;
		Position = StartTime;
		BlendTime = 0.f;
		Weight = 0.f;
		Maxed = false;
	}


	bool ApplyTime(const float DT, const float InBlendTime, const bool Main);

};


/*Currently This is where the process gets done, Both Finding the best Candidate Motion Key and Blending the Animations together along with their Root Motion*/
USTRUCT(BlueprintInternalUseOnly)
struct MOTIONMATCHING_API FAnimNode_MotionMatching : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData)
		UMotionField* MotionField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		FTrajectoryData DesiredTrajectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float Responsiveness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float VelocityStrength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float PoseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tags)
		TArray <uint8> TagsToLookFor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tags)
		TArray <uint8> TagsToIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float BlendTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float PlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float ChangeTimeLimit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		int32 MaxChanges;

public:

	FAnimNode_MotionMatching();


	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime();
	virtual float GetCurrentAssetTimePlayRateAdjusted();
	virtual float GetCurrentAssetLength();
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override; ////-Unused
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void OverrideAsset(UAnimationAsset* NewAsset) override; ////-Unused 
	virtual void GatherDebugData(FNodeDebugData& DebugData) override; 
	// End of FAnimNode_Base interface

	

protected:
	//////////////////
	UPROPERTY()
		int CurrentMotionKeyIndex;
	//////////////////////////////////////////
	UPROPERTY()
		int ChangeCounter;
	UPROPERTY()
		float ChangeTimer;
	///////////////////////////////////////////
	UPROPERTY()
		bool bLoopAnimation; //<-This is just here so Tick Record has a valid bool.
	////////////////////////////////////////////
	

private:

	UPROPERTY()
	TArray<FMotionAnim> m_Anims;

	/////-------------------------------------------------------------
	
	void PlayAnimStartingAtTime(const int32 AnimIndex, const float StartingTime);

	UAnimSequence* GetCurrentAnim();
	UAnimSequence* AnimAtIndex(const int32 Index);

	void MotionUpdate(const FAnimationUpdateContext& Context);
	void GetMotionData(TArray <FJointData>& OutJointData, FVector& OutVelocity);
	
	void Compute();

	void GetBlendPose(const float DT, FTransform& OutRootMotion, FCompactPose& OutPose, FBlendedCurve& OutCurve, FStackCustomAttributes& CustomAttributes);
};
