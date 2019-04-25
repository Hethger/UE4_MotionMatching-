// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionStructs.h"


float FJointData::CompareTo(const FJointData Other) const
{
	return (FVector::Dist(BoneCSPos, Other.BoneCSPos)) + (FVector::Dist(BoneCSVel, Other.BoneCSVel));
}

float FTrajectoryData::CompareTo(const FTrajectoryData Other, const int Per) const
{
	float Cost = 0.f;
	
	if ((TrajectoryPoints.Num() > 0) && (TrajectoryPoints.Num() == Other.TrajectoryPoints.Num()))
	{
		//// This settup assumess the Number of trajjectory points will be around 10 in order to get proper results,
		////could do this in a more consistent way
		/*
		int32 Numy = TrajectoryPoints.Num() / 3; 
		
		int32 Counter = 1;
		for (int32 i = 0; i < TrajectoryPoints.Num(); i++)
		{
			if (i >= ((Numy * Counter) - 1))
			{
				Cost += FVector::Dist(TrajectoryPoints[i].m_TM.GetTranslation(), Other.TrajectoryPoints[i].m_TM.GetTranslation());

				//This Here Adds Cost Based on difference in Rotation, for now is not used as it is not essential to get the initial successful results.
				//Cost += ((180.f) / PI * FMath::Acos(FVector::DotProduct(TrajectoryPoints[i].m_TM.GetRotation().GetForwardVector(), Other.TrajectoryPoints[i].m_TM.GetRotation().GetForwardVector())));

				Counter++;
			}
		}
		*/

		////....For Now Just compare the dist between the last trajectory points
		Cost += FVector::Dist(TrajectoryPoints.Last().m_TM.GetTranslation(), Other.TrajectoryPoints.Last().m_TM.GetTranslation());
	}

	return Cost;
}
