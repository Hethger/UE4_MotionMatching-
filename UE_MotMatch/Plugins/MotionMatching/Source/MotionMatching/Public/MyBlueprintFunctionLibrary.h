// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Animation/AnimSequence.h"
#include "BonePose.h"

#include "MotionKeyUtils.h"

#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */

struct FBlendedCurve;
struct FCompactPose;
class UAnimSequence;
class UMotionField;

/*Blueprint Function Library to Test things in the editor*/
UCLASS()
class MOTIONMATCHING_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	
		UFUNCTION(BlueprintCallable, Category = "shit")
		static void PoseGetTest(const UAnimSequence* InSequence, const float AtTime, const bool bExtractRootMotion, TArray <FTransform>& OutBoneTransforms);

	UFUNCTION(BlueprintCallable, Category = "shit")
		static FTransform GetBoneCSTM(const UAnimSequence * InSequence, const float AtTime, const bool bUseRawData, const int BoneIndex);
	
	UFUNCTION(BlueprintCallable, Category = "shit")
		static FQuat GetQuat(const FTransform InTM);

	UFUNCTION(BlueprintCallable, Category = "shit")
		static bool BuildTrajectoryData(FTrajectoryData& OutTrajectory, const TArray <FTransform> PastTMs, const TArray <FVector> Velocities);
	UFUNCTION(BlueprintCallable, Category = "shit")
		static int EvaluateMotionFieldToData(UMotionField* MotionField, const float Responsiveness, const float PoseStrength, const FTrajectoryData DesiredTrajectory, const FTrajectoryData PresentTrajectory, const TArray <FJointData> PresentJointData, const FVector PresentVel, float & OutLowestCost);

	UFUNCTION(BlueprintCallable, Category = "shit")
		static void BuildGoal(FTrajectoryData& OutGoal, const FTransform DesiredTransform, const float TargetUUS, const FTransform RootWorldTM);

	UFUNCTION(BlueprintCallable, Category = "shit")
	static void GetMotionDataFromAnimation
	(
		const UAnimSequence * InSequence,
		const float AtTime,
		TArray <FName> InMotionBoneNames,
		FTrajectoryData& OutPresentTrajectory,
		FTrajectoryData& OutFutureTrajectory,
		TArray <FJointData>& OutJointData,
		FTransform& OutPastTMRel
	);
	UFUNCTION(BlueprintCallable, Category = "shit")
		static float CompareAnimFramesPoseCost
		(
			const UAnimSequence* InSequence,
			const TArray <FName> MotionBones,
			const int32 Frame1,
			const int32 Frame2
		);
};
