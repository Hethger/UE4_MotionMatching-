// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Animation/AnimSequence.h"

#include "MotionStructs.h"

#include "Math/UnrealMathUtility.h"
/**
*
*/

class UAnimSequence;
class FPrimitiveDrawInterface;
class UMotionField;
class USkeletalMeshComponent;

struct FMotionKey;

/**
 * 
 */
class MOTIONMATCHING_API FMotionKeyUtils
{
public:
	////////////////////////////////EXTRACTION AND BAKE FUNCTIONALITIES
	static void GetAnimVelocityAtTime(const UAnimSequence* InSequence, const float AtTime, FVector& OutVelocity);

	static void ExtractAnimSmoothedRotation(FQuat& OutRot, const UAnimSequence* InSequence, const float KeyTime);
	static void ExtractAnimTrajectory(FTrajectoryData& OutVelocityData, const UAnimSequence* InSequence, const float KeyTime);

	static void GetAnimBoneLocalTM(const UAnimSequence* InSequence, const float AtTime, const int BoneIndex, FTransform& OutTM);

	static void GetAnimBoneWorldTM(const UAnimSequence* InSequence, const float AtTime, const int BoneIndex, FTransform& OutTM);

	static void GetAnimBoneLocalVel(const UAnimSequence* InSequence, const float AtTime, const int BoneIndex, FVector& OutVel);

	static void GetAnimJointData(const UAnimSequence* InSequence, const float AtTime, const FName BoneName, FJointData& OutJointData);

	static void GetKeyPoseDataFromAnim(const UAnimSequence* InSequence, const float AtTime, const TArray<FName> KeyBones, FKeyPoseData& OutPoseData);
	
	static void GetDataFromAnimation
	(
		const UAnimSequence * InSequence,
		const float AtTime, 
		const TArray <FName> InMotionBoneNames,
		FTrajectoryData& OutPresentTrajectory,
		FTrajectoryData& OutFutureTrajectory,
		TArray <FJointData>& OutJointData
	);


	////////////////////////////////

	////////////////////////////////MOTION KEY EVALUATION FUNCTIONALITIES
	
	static float CompareTrajectories(const FTrajectoryData TrajectoryData_A, const FTrajectoryData TrajectoryData_B);
	static float CompareJointDatas(const TArray <FJointData> JointData_A, const TArray <FJointData> JointData_B);
	///////////////////////////////////

	///////////////////////////////// OTHER UTILITIES
	static bool MakeTrajectoryData(FTrajectoryData& OutTrajectory, const TArray<FTransform> PastTMs, const TArray<FVector> Velocities);

	static void DrawTrajectoryData(const FTrajectoryData Trajectory, const FTransform KeyTM, FPrimitiveDrawInterface* PDI, const FColor InColor, const uint8 Depth);

	static void BlendKeyPoses(const FKeyPoseData PoseA, const FKeyPoseData PoseB, const float Alpha, FKeyPoseData& OutBlendedKeyPose);

	static void MakeGoal(FTrajectoryData& OutGoal, const FTransform DesiredTransform, const float TargetUUS, const FTransform RootWorldTM);

	/////////////////////////////////////////-----------------


};
