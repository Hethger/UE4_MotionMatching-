
#include "AnimNode_MotionField.h"

#include "MotionMatching.h"
#include "Engine/Engine.h"
#include "AnimationRuntime.h"



FAnimNode_MotionField::FAnimNode_MotionField()
	: MotionField(NULL)
	, DesiredTrajectory(FTrajectoryData())
	, Responsiveness(1.f)
	, PoseStrength(1.f)
	, ChangeTimeLimit(0.1f)
	, ChangeAmountLimit(2)
	, DrawDebug(false)
	//, NumPointsPerSecond(10)
	, BlendTime(0.3f)
	, CurrentAnimIndex(0)

	

	

	


	
	, ChangeTimer(0.f)

	, LastSwitch(false)
	, Pause(false)
{
	Vel = FVector(0.f);//, PresentTrajectory(FTrajectoryData())
	CurrentAnimTime = 2.f;
	PlayRate=(1.f);
	bLoopAnimation = (true);
    Switch = (false);
	ChangeCounter = (0);
	AuthorSwitch = (false);

	//TrajectoryPoints.Empty();
	LastBones.Empty();
	JointsData.Empty();
}

float FAnimNode_MotionField::GetCurrentAssetTime()
{
	return CurrentAnimTime;
}

float FAnimNode_MotionField::GetCurrentAssetTimePlayRateAdjusted()
{
	UAnimSequence* Sequence = GetCurrentAnim();

	float EffectivePlayrate = PlayRate * (Sequence ? Sequence->RateScale : 1.0f);
	return (EffectivePlayrate < 0.0f) ? GetCurrentAssetLength() - CurrentAnimTime : CurrentAnimTime;
}

float FAnimNode_MotionField::GetCurrentAssetLength()
{

	UAnimSequence* Sequence = GetCurrentAnim();

	return Sequence ? Sequence->SequenceLength : 0.0f;

}

void FAnimNode_MotionField::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{

	FAnimNode_Base::Initialize_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);

	InternalTimeAccumulator = 0.f;

	UAnimSequence* Sequence = GetCurrentAnim();
	/*
	for (int i = 0; i < NumPointsPerSecond; i++)
	{
	TrajectoryPoints.Add(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(TEXT("root"), RTS_World));
	}
	*/

	for (int i = 0; i < MotionField->MotionBones.Num(); i++)
	{
		FJointData JD = FJointData();
		FVector Vel = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetPhysicsLinearVelocity(MotionField->MotionBones[i]);
		float Size = Vel.Size();
		JD.BoneCSVel = Context.AnimInstanceProxy->GetComponentTransform().InverseTransformVectorNoScale(Vel.GetSafeNormal()) * Size;
		JD.BoneCSPos = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(MotionField->MotionBones[i], RTS_Component).GetTranslation();

		JointsData.Add(JD);
	}


	if (Sequence != NULL)
	{
		InternalTimeAccumulator = FMath::Clamp(0.f, 0.f, Sequence->SequenceLength);

		if (0.f == 0.f && (PlayRate * Sequence->RateScale) < 0.0f)
		{
			InternalTimeAccumulator = Sequence->SequenceLength;

		}
	}



	//BasePose.Initialize(Context);
}

void FAnimNode_MotionField::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{

	//BasePose.CacheBones(Context);
}

void FAnimNode_MotionField::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	//FString JO = TEXT("TIME COMPARISON  :: TESTO  ") + FString::SanitizeFloat( + Context.AnimInstanceProxy->GetDeltaSeconds()) + TEXT(" : This the real : ") + FString::SanitizeFloat(CopyTime);
	//GEngine->AddOnScreenDebugMessage(-1, 1.02f, FColor::Yellow, JO); Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(TEXT("root"), RTS_World).GetTranslation()


	//Context.AnimInstanceProxy->AnimDrawDebugLine(FVector(0.f), FVector(1000.f), FColor::Green, false, 1.f, 10.f);

	//CheckPending();
	CheckSwitch();

	MotionUpdate(Context);



	//Compute();

	if ((GetCurrentAnim() != NULL) && (Context.AnimInstanceProxy->IsSkeletonCompatible(GetCurrentAnim()->GetSkeleton())))
	{
		InternalTimeAccumulator = FMath::Clamp(CurrentAnimTime, 0.f, GetCurrentAnim()->SequenceLength);

		CreateTickRecordForNode(Context, GetCurrentAnim(), bLoopAnimation, PlayRate);
	}

}


void FAnimNode_MotionField::Evaluate_AnyThread(FPoseContext & Output)
{
	// Evaluate the input

	//
	//FPoseContext Pose2(Output);
	//BasePose.Evaluate(Pose1);
	//BasePose.Evaluate(Pose2);
	//const float TrueAlpha = FMath::Clamp((1.0f - (AccumulatedTime / 0.25f)), 0.f, 1.f);

	if (UAnimSequence* Sequence = GetCurrentAnim())
	{
		Sequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(CurrentAnimTime, true));
		/*
		if ((LastBones.Num() > 0) && (BlendTime > 0.f))
		{


		FPoseContext Pose1(Output);
		Sequence->GetAnimationPose(Pose1.Pose, Pose1.Curve, FAnimExtractContext(CurrentAnimTime, true));

		FPoseContext Pose2(Output);
		Pose2.Pose.CopyBonesFrom(LastBones);
		FAnimationRuntime::BlendTwoPosesTogether(Pose1.Pose, Pose2.Pose, Pose1.Curve, Pose2.Curve, BlendTimer / BlendTime, Output.Pose, Output.Curve);

		/*
		if (UAnimSequence* LastSequence = GetLastAnim())
		{
		LastSequence->GetAnimationPose(Pose2.Pose, Pose2.Curve, FAnimExtractContext(LastAnimTime, true));

		FAnimationRuntime::BlendTwoPosesTogether(Pose1.Pose, Pose2.Pose, Pose1.Curve, Pose2.Curve, BlendTimer / BlendTime, Output.Pose, Output.Curve);
		}
		else
		{
		Sequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(CurrentAnimTime, true));
		}


		}
		else
		{
		Sequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(CurrentAnimTime, true));
		}

		if (bChangeAnim)
		{
		LastBones = Output.Pose.GetBones();
		}
		*/
	}
	else
	{
		Output.ResetToRefPose();

	}

	//PostEvaluate(Output);
	//GetBlendPose(Output.Pose, Output.Curve);

}

void FAnimNode_MotionField::OverrideAsset(UAnimationAsset * NewAsset)
{
	if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(NewAsset))
	{

	}
}




void FAnimNode_MotionField::GatherDebugData(FNodeDebugData & DebugData)
{

	UAnimSequence* Sequence = GetCurrentAnim();

	FString DebugLine = DebugData.GetNodeName(this);
	DebugLine += FString::Printf(TEXT("('%s' Play Time: %.3f)"), Sequence ? *Sequence->GetName() : TEXT("NULL"), InternalTimeAccumulator);
	DebugData.AddDebugItem(DebugLine, true);
	//  BasePose.GatherDebugData(DebugData);
}

UAnimSequence * FAnimNode_MotionField::GetCurrentAnim()
{
	if (MotionField)
	{
		if (MotionField->IsValidSrcAnimIndex(CurrentAnimIndex))
		{
			return MotionField->GetSrcAnimAtIndex(CurrentAnimIndex);
		}
	}
	return NULL;
}

void FAnimNode_MotionField::PreEvaluate(const FAnimationUpdateContext & Context)
{
	if (Pause)
	{

		if (DrawDebug)
		{
			const float LifeTime = 0.03f;
			const FTransform RootTM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentToWorld();


			Context.AnimInstanceProxy->AnimDrawDebugLine
			(
				RootTM.GetTranslation(),
				RootTM.GetTranslation() + (RootTM.TransformVector(Vel.GetSafeNormal()) * Vel.Size()),
				FColor::Purple,
				false,
				LifeTime,
				5.f
			);


			Context.AnimInstanceProxy->AnimDrawDebugLine
			(
				RootTM.GetTranslation() + FVector(0.f, 0.f, 20.f),
				RootTM.GetTranslation() + ((RootTM.TransformVector(MotionField->MotionKeys[CurrentMotKeyIndex].PresentVel.GetSafeNormal()) * MotionField->MotionKeys[CurrentMotKeyIndex].PresentVel.Size()) + FVector(0.f, 0.f, 20.f)),
				FColor::Orange,
				false,
				LifeTime,
				2.f
			);


			for (int i = 0; i < MotionField->MotionBones.Num(); i++)
			{

				FVector LastPos = RootTM.TransformPosition(JointsData[i].BoneCSPos);
				FVector LastVel = RootTM.TransformVector(JointsData[i].BoneCSVel.GetSafeNormal()) * JointsData[i].BoneCSVel.Size();

				Context.AnimInstanceProxy->AnimDrawDebugSphere(LastPos, 10.f, 12, FColor::Yellow, false, LifeTime, 2.f);
				//FTransform BoneTM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(MotionField->MotionBones[i], RTS_Component) * RootTM;
				//Context.AnimInstanceProxy->AnimDrawDebugSphere(BoneTM.GetTranslation(), 10.f, 12, FColor::Magenta, false, LifeTime, 0.5f);

				Context.AnimInstanceProxy->AnimDrawDebugLine
				(
					LastPos,
					LastPos + LastVel,
					FColor::Yellow,
					false,
					LifeTime,
					2.f
				);


				FVector WinnerPos = RootTM.TransformPosition(MotionField->MotionKeys[CurrentMotKeyIndex].MotionJointData[i].BoneCSPos);
				FVector WinnerVel = RootTM.TransformVector(MotionField->MotionKeys[CurrentMotKeyIndex].MotionJointData[i].BoneCSVel.GetSafeNormal()) * MotionField->MotionKeys[CurrentMotKeyIndex].MotionJointData[i].BoneCSVel.Size();
				Context.AnimInstanceProxy->AnimDrawDebugSphere(WinnerPos, 10.f, 12, FColor::Green, false, LifeTime, .5f);
				Context.AnimInstanceProxy->AnimDrawDebugLine
				(
					WinnerPos,
					WinnerPos + WinnerVel,
					FColor::Green,
					false,
					LifeTime,
					2.f
				);

				Context.AnimInstanceProxy->AnimDrawDebugDirectionalArrow
				(
					LastPos,
					WinnerPos,
					10.f,
					FColor::Red,
					false,
					LifeTime,
					2.f
				);
				//FString JO = TEXT("DOING IT SUPOSEDLY  :: ") + FString::FromInt(Winner);
				//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, JO);

			}

		}
	}
	else
	{

		if (JointsData.Num() == MotionField->MotionBones.Num())
		{
			const float DT = Context.GetDeltaTime();
			FReferenceSkeleton RefSkel = Context.AnimInstanceProxy->GetSkeleton()->GetReferenceSkeleton();



			for (int i = 0; i < MotionField->MotionBones.Num(); i++)
			{
				int32 CurrentIndex = RefSkel.FindBoneIndex(MotionField->MotionBones[i]);
				FTransform BoneTM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(MotionField->MotionBones[i], RTS_Component);
				//FMotionKeyUtils::GetAnimBoneLocalTM(GetCurrentAnim(), CurrentAnimTime, CurrentIndex, BoneTM);

				const FVector Vel = BoneTM.GetTranslation() - JointsData[i].BoneCSPos;
				//Context.AnimInstanceProxy->GetSkelMeshComponent()->GetPhysicsLinearVelocity(MotionField->MotionBones[i]);

				JointsData[i].BoneCSVel = (Vel.GetSafeNormal()) * (Vel.Size() / DT);
				JointsData[i].BoneCSPos = BoneTM.GetTranslation();
			}

			const FTransform PredictedRM = GetCurrentAnim()->ExtractRootMotion(CurrentAnimTime - DT, DT, true);
			Vel = PredictedRM.GetTranslation().GetSafeNormal() * (PredictedRM.GetTranslation().Size() / DT);



			float OutCost = 0.f;
			int Winner = 0;// MotionField->GetLowestCostMotionKey(Responsiveness, PoseStrength, DesiredTrajectory, FTrajectoryData(), JointsData, Vel, OutCost);

			if (MotionField->MotionKeys.IsValidIndex(Winner))
			{

				bool TheWinnerIsAtTheSameLocation =
					(MotionField->MotionKeys[Winner].SrcAnimIndex == CurrentAnimIndex)
					&&
					(fabs(MotionField->MotionKeys[Winner].StartTime - CurrentAnimTime) < 0.2f);

				if ((!TheWinnerIsAtTheSameLocation) && (ChangeCounter <= ChangeAmountLimit))// && ((MotionField->MotionKeys[CurrentMotKeyIndex].EndTime + 0.0f) < CurrentAnimTime))
				{
					//FString JO = TEXT("DOING IT SUPOSEDLY  :: ") + FString::FromInt(Winner);
					//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, JO);


					if (AuthorSwitch)
					{
						Pause = true;
					}
					ChangeCounter += 1;

					CurrentAnimIndex = MotionField->MotionKeys[Winner].SrcAnimIndex;
					CurrentAnimTime = MotionField->MotionKeys[Winner].StartTime;
					CurrentMotKeyIndex = Winner;
					//bChangeAnim = true;



					if (DrawDebug)
					{
						const float LifeTime = 0.1f;
						const FTransform RootTM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentToWorld();


						Context.AnimInstanceProxy->AnimDrawDebugLine
						(
							RootTM.GetTranslation(),
							RootTM.GetTranslation() + (RootTM.TransformVector(Vel.GetSafeNormal()) * Vel.Size()),
							FColor::Purple,
							false,
							LifeTime,
							5.f
						);


						Context.AnimInstanceProxy->AnimDrawDebugLine
						(
							RootTM.GetTranslation() + FVector(0.f, 0.f, 20.f),
							RootTM.GetTranslation() + ((RootTM.TransformVector(MotionField->MotionKeys[Winner].PresentVel.GetSafeNormal()) * MotionField->MotionKeys[Winner].PresentVel.Size()) + FVector(0.f, 0.f, 20.f)),
							FColor::Orange,
							false,
							LifeTime,
							2.f
						);


						for (int i = 0; i < MotionField->MotionBones.Num(); i++)
						{

							FVector LastPos = RootTM.TransformPosition(JointsData[i].BoneCSPos);
							FVector LastVel = RootTM.TransformVector(JointsData[i].BoneCSVel.GetSafeNormal()) * JointsData[i].BoneCSVel.Size();

							Context.AnimInstanceProxy->AnimDrawDebugSphere(LastPos, 10.f, 12, FColor::Yellow, false, LifeTime, 2.f);
							//FTransform BoneTM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(MotionField->MotionBones[i], RTS_Component) * RootTM;
							//Context.AnimInstanceProxy->AnimDrawDebugSphere(BoneTM.GetTranslation(), 10.f, 12, FColor::Magenta, false, LifeTime, 0.5f);

							Context.AnimInstanceProxy->AnimDrawDebugLine
							(
								LastPos,
								LastPos + LastVel,
								FColor::Yellow,
								false,
								LifeTime,
								2.f
							);


							FVector WinnerPos = RootTM.TransformPosition(MotionField->MotionKeys[Winner].MotionJointData[i].BoneCSPos);
							FVector WinnerVel = RootTM.TransformVector(MotionField->MotionKeys[Winner].MotionJointData[i].BoneCSVel.GetSafeNormal()) * MotionField->MotionKeys[Winner].MotionJointData[i].BoneCSVel.Size();
							Context.AnimInstanceProxy->AnimDrawDebugSphere(WinnerPos, 10.f, 12, FColor::Green, false, LifeTime, 2.f);
							Context.AnimInstanceProxy->AnimDrawDebugLine
							(
								WinnerPos,
								WinnerPos + WinnerVel,
								FColor::Green,
								false,
								LifeTime,
								2.f
							);
							//FString JO = TEXT("DOING IT SUPOSEDLY  :: ") + FString::FromInt(Winner);
							//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, JO);

						}


					}

				}
			}
		}
	}
}

void FAnimNode_MotionField::CheckSwitch()
{
	if (Switch != LastSwitch)
	{
		LastSwitch = Switch;

		Pause = !Pause;
	}

	if (Pause)
	{
		PlayRate = 0.f;
	}
	else
	{
		PlayRate = 1.f;
	}

}


FVector FAnimNode_MotionField::GetBoneCSTMFromPose(FPoseContext & Output, const FName BoneName)
{
	FReferenceSkeleton RefSkel = Output.AnimInstanceProxy->GetSkeleton()->GetReferenceSkeleton();

	int32 CurrentIndex = RefSkel.FindBoneIndex(BoneName);

	FCompactPoseBoneIndex CmpctIndex(CurrentIndex);

	FTransform BoneTM = Output.Pose[CmpctIndex];


	while (RefSkel.GetParentIndex(CurrentIndex) != 0)
	{
		int ParentIndex = RefSkel.GetParentIndex(CurrentIndex);
		FCompactPoseBoneIndex ParentCmpctIndex(CurrentIndex);
		FTransform ParentTM = Output.Pose[ParentCmpctIndex];

		BoneTM = BoneTM * ParentTM;

		CurrentIndex = ParentIndex;
	}


	return BoneTM.GetTranslation();
}

void FAnimNode_MotionField::DrawPresentTrajectory(const FAnimationUpdateContext& Context)
{
	/*
	FVector Begin = TrajectoryPoints[0].GetTranslation();
	FVector Mid = FVector::ZeroVector;// (PresentTrajectory.MidTM * TrajectoryPoints[0]).GetTranslation();
	FVector End = (PresentTrajectory.EndTM * TrajectoryPoints[0]).GetTranslation();

	float Lifetime = 0.1f;

	Context.AnimInstanceProxy->AnimDrawDebugSphere(Begin, 10.f, 6, FColor::Red, false, Lifetime, 2.f);
	Context.AnimInstanceProxy->AnimDrawDebugSphere(Mid, 10.f, 6, FColor::Red, false, Lifetime, 2.f);
	Context.AnimInstanceProxy->AnimDrawDebugSphere(End, 10.f, 6, FColor::Red, false, Lifetime, 2.f);

	Context.AnimInstanceProxy->AnimDrawDebugLine
	(
	Begin,
	Mid,
	FColor::Red,
	false,
	Lifetime,
	2.f
	);

	Context.AnimInstanceProxy->AnimDrawDebugLine
	(
	Mid,
	End,
	FColor::Red,
	false,
	Lifetime,
	2.f
	);
	*/
}

void FAnimNode_MotionField::DrawTrajectoryPoint(const FAnimationUpdateContext & Context)
{
	/*
	if (TrajectoryPoints.Num() > 1)
	{

	for (int i = 1; i < TrajectoryPoints.Num(); i++)
	{
	Context.AnimInstanceProxy->AnimDrawDebugLine(TrajectoryPoints[i].GetTranslation(), TrajectoryPoints[i - 1].GetTranslation(), FColor::Red, false, 0.02f, 2.f);
	//FString JO = TEXT("DOING IT SUPOSEDLY  :: ");
	//GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, JO);
	}

	}
	*/
}

void FAnimNode_MotionField::DrawJointData(const FAnimationUpdateContext & Context)
{
	FTransform RootTM = Context.AnimInstanceProxy->GetComponentTransform();
	for (int i = 0; i < JointsData.Num(); i++)
	{
		FVector BonePos = RootTM.TransformPosition(JointsData[i].BoneCSPos);
		FVector BoneVel = RootTM.TransformVector(JointsData[i].BoneCSVel);
		Context.AnimInstanceProxy->AnimDrawDebugLine(BonePos, BonePos + BoneVel, FColor::Blue, false, 0.02f, 2.f);
	}
}

void FAnimNode_MotionField::MotionUpdate(const FAnimationUpdateContext& Context)
{
	//DoCheck()
	const float DT = Context.GetDeltaTime();
	if (!Pause)
	{


		CurrentAnimTime += DT;
		BlendTimer = FMath::Clamp(BlendTimer + DT, 0.f, BlendTime);

		if (ChangeCounter > ChangeAmountLimit)
		{
			ChangeTimer += DT;
		}

		if (ChangeTimer > ChangeTimeLimit)
		{
			ChangeTimer = 0.f;
			ChangeCounter = 0;

			FString JO = TEXT("CHANGE TIMER CHANGED");
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, JO);
		}
	}
	PreEvaluate(Context);

	/*
	if (TrajectoryTimer > (1.f / NumPointsPerSecond))
	{
	TrajectoryTimer = 0.f;

	FTransform TM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(TEXT("root"), RTS_World);

	TM.Accumulate(Context.AnimInstanceProxy->GetSkelMeshComponent()->ConvertLocalRootMotionToWorld(GetCurrentAnim()->ExtractRootMotion(CurrentAnimTime - DT, DT, true)));

	TrajectoryPoints.Add(TM);

	if (TrajectoryPoints.Num() > NumPointsPerSecond)
	{
	TrajectoryPoints.RemoveAt(0);
	//Context.AnimInstanceProxy->QueuedDrawDebugItems.Empty();
	}

	FMotionKeyUtils::MakeTrajectoryData(PresentTrajectory, TrajectoryPoints, TArray <FVector>());

	DrawPresentTrajectory(Context);

	}
	*/

	/*
	if (CurrentAnimIndex == 0)
	{
	if (CurrentAnimTime > 5.f)
	{
	PendingAnimIndex = 1;
	PendingAnimTime = 12.f;
	bChangeAnim = true;
	}
	}
	else if (CurrentAnimTime > 14.f)
	{

	PendingAnimIndex = 0;
	PendingAnimTime = 4.f;
	bChangeAnim = true;
	}
	*/
}


void FAnimNode_MotionField::Compute()
{
	/*
	if (MotionField)
	{
	if (Sequences.Num() > 0)
	{

	FTrajectoryData PresentTrajectory;
	FTrajectoryData FutureTrajectory;
	TArray <FJointData> PresentJointData;

	FMotionKeyUtils::GetDataFromAnimation
	(
	Sequences.Last(),
	SequencePositions.Last(),
	MotionField->MotionBones,
	PresentTrajectory,
	FutureTrajectory,
	PresentJointData
	);

	float OutCost;

	int Winner = MotionField->GetLowestCostMotionKey(Responsiveness, DesiredTrajectory, PresentTrajectory, PresentJointData, OutCost);

	if (MotionField->MotionKeys.IsValidIndex(Winner))
	{

	bool TheWinnerIsAtTheSameLocation =
	(MotionField->MotionKeys[Winner].SrcAnimIndex == MotionField->MotionKeys[MotionKeyIndex].SrcAnimIndex)
	&&
	(fabs(MotionField->MotionKeys[Winner].StartTime - SequencePositions.Last()) < 0.2f);

	if ((!TheWinnerIsAtTheSameLocation) && (Winner != MotionKeyIndex))
	{
	FString JO = TEXT("DOING IT SUPOSEDLY  :: ") + FString::FromInt(Winner);
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, JO);

	AddSequence(Winner);
	}

	}

	}
	else
	{
	AddSequence(0);
	}
	}
	else
	{
	Sequences.Empty();

	SequenceStarts.Empty();

	SequencePositions.Empty();

	SequencePlayTimes.Empty();
	SequenceWeights.Empty();

	InternalTimeAccumulator = 0.f;
	}
	*/
}


void FAnimNode_MotionField::GetBlendPose(FCompactPose & OutPose, FBlendedCurve & OutCurve)
{
	//SCOPE_CYCLE_COUNTER(STAT_BlendSpace_GetAnimPose);
	//FScopeCycleCounterUObject BlendSpaceScope(this);
	/*
	if (Sequences.Num() == 0)
	{
	OutPose.ResetToRefPose();
	return;
	}

	if ((Sequences.Num() == 1) || (BlendTime <= 0.f))
	{
	Sequences.Last()->GetAnimationPose(OutPose, OutCurve, FAnimExtractContext(SequencePositions.Last(), true));
	return;
	}

	const int32 NumPoses = Sequences.Num();

	TArray<FCompactPose, TInlineAllocator<8>> ChildrenPoses;
	ChildrenPoses.AddZeroed(NumPoses);

	TArray<FBlendedCurve, TInlineAllocator<8>> ChildrenCurves;
	ChildrenCurves.AddZeroed(NumPoses);

	TArray<float, TInlineAllocator<8>> ChildrenWeights;
	ChildrenWeights.AddZeroed(NumPoses);

	for (int32 ChildrenIdx = 0; ChildrenIdx<ChildrenPoses.Num(); ++ChildrenIdx)
	{
	ChildrenPoses[ChildrenIdx].SetBoneContainer(&OutPose.GetBoneContainer());
	ChildrenCurves[ChildrenIdx].InitFrom(OutCurve);
	}

	// get all child atoms we interested in
	for (int32 I = 0; I < Sequences.Num(); I++)
	{
	FCompactPose& Pose = ChildrenPoses[I];

	ChildrenWeights[I] = SequenceWeights[I];

	if (Sequences[I])
	{
	const float Time = FMath::Clamp<float>(SequencePositions[I], 0.f, Sequences[I]->SequenceLength);

	// first one always fills up the source one
	Sequences[I]->GetAnimationPose(Pose, ChildrenCurves[I], FAnimExtractContext(Time, true));
	}
	else
	{
	Pose.ResetToRefPose();
	}
	}


	TArrayView<FCompactPose> ChildrenPosesView(ChildrenPoses);
	//////COMPUTE CHILDREN WEIGHTS
	float Sum = 0.f;

	for (int i = 0; i < ChildrenWeights.Num(); i++)
	{
	Sum += ChildrenWeights[i];
	}

	for (int i = 0; i < ChildrenWeights.Num(); i++)
	{
	ChildrenWeights[i] = ChildrenWeights[i] / Sum;
	}


	FAnimationRuntime::BlendPosesTogether(ChildrenPosesView, ChildrenCurves, ChildrenWeights, OutPose, OutCurve);


	// Once all the accumulation and blending has been done, normalize rotations.
	OutPose.NormalizeRotations();
	*/
}