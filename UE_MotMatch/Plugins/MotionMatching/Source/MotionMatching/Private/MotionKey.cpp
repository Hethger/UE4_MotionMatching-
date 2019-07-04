// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionKey.h"

#include "AnimationRuntime.h"

FMotionKey::FMotionKey()
{
	//KeyPose = FKeyPoseData();
	SrcAnimIndex = INDEX_NONE;
	StartTime = 0.f;

	TagsIdx.Empty();

	FutureTrajectory = FTrajectoryData();

	MotionJointData.Empty();
	
	SrcAnimationName = NAME_None;

}

float FMotionKey::ComputeMotionKeyCost(const float Responsiveness, const float VelocityStrength, const float PoseStrength, const FTrajectoryData DesiredTrajectory, const TArray <FJointData> PresentJointData, const FVector InPresentVel)
{
	float Cost = 0.f;

	Cost += FVector::Dist(PresentVel, InPresentVel) * VelocityStrength;
	
	
	Cost += DesiredTrajectory.CompareTo(FutureTrajectory, 1) * Responsiveness;
	
	
	check(PresentJointData.Num() == MotionJointData.Num());
	
	float PoseCost = 0.f;

	for (int i = 0; i < PresentJointData.Num(); i++)
	{
		PoseCost += PresentJointData[i].CompareTo(MotionJointData[i]);
	}

	PoseCost *= PoseStrength;

	return PoseCost + Cost;
}


void FMotionKey::ExtractDataFromAnimation(const UAnimSequence * InSequence, const int AtSrcAnimIndex, const float AtSrcStartTime, TArray <FName> InMotionBoneNames)
{
	if (InSequence)
	{
		SrcAnimIndex = AtSrcAnimIndex;

		FReferenceSkeleton RefSkel = InSequence->GetSkeleton()->GetReferenceSkeleton();

		StartTime = AtSrcStartTime;

		for (int i = 0; i < InMotionBoneNames.Num(); i++)
		{
			FJointData NewJointData = FJointData();
			FMotionKeyUtils::GetAnimJointData(InSequence, StartTime, InMotionBoneNames[i], NewJointData);
			MotionJointData.Add(NewJointData);
		}

		FMotionKeyUtils::GetAnimVelocityAtTime(InSequence, StartTime, PresentVel);

		FMotionKeyUtils::ExtractAnimTrajectory(FutureTrajectory, InSequence, StartTime);

#if WITH_EDITOR
			SrcAnimationName = InSequence->GetFName();
#endif // WITH_EDITOR
	}

}

