// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionCharacter.h"

#include "MotionMatching.h"

#include "Engine.h"

#include "Components/SkeletalMeshComponent.h"

// Sets default values
AMotionCharacter::AMotionCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//////////////////////

	MotionField = NULL;

	PlayRate = 1.f;

	BlendTime = 0.3f;
	
	Responsiveness = 1.f;
	VelocityStrength = 1.f;
	PoseStrength = 1.f;
	
	SlotName = NAME_None;

	DrawDebug = false;
	/////////////

	CurrentAnimIndex = 0;
	
	CurrentAnimTime = 0.f;

	MotionKeyIndex = 0;

	DesiredTrajectory = FTrajectoryData();

	JointsData.Empty();
	
	Vel = FVector::ZeroVector;
	//////////////////////////////////////////
	

}


void AMotionCharacter::OnNewMotionKey_Implementation(int32 WinnerIndex, float Cost)
{
}

// Called when the game starts or when spawned
void AMotionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//PlayAnimStartingAtTime(0, 10.f);
}

// Called every frame
void AMotionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MotionUpdate(DeltaTime);

}

// Called to bind functionality to input
void AMotionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMotionCharacter::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	//Get all of our components  
	//TArray<UActorComponent*> MyComponents;
	//GetComponents(MyComponents);

	//Get the name of the property that was changed  
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// We test using GET_MEMBER_NAME_CHECKED so that if someone changes the property name  
	// in the future this will fail to compile and we can update it.  
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AMotionCharacter, MotionField)))
	{
		PostMotionFieldChanged();
	}

	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AMotionCharacter::PostMotionFieldChanged()
{
	JointsData.Empty();
	MotionKeyIndex = 0;
	CurrentAnimIndex = 0;
	CurrentAnimTime = 0.f;

	//ResetMontages();

	if (MotionField)
	{
		for (int i = 0; i < MotionField->MotionBones.Num(); i++)
		{
			JointsData.Add(FJointData());
		}
	}
}


void AMotionCharacter::MotionUpdate(const float DT)
{
	if (MotionField)
	{
		if (AreSkeletonsCompatible())
		{
			if (MotionField->MotionKeys.Num() > 0)
			{
				if (JointsData.Num() == MotionField->MotionBones.Num())
				{
					CurrentAnimTime += DT;

					FMotionKeyUtils::GetAnimVelocityAtTime(GetAnimAtIndex(CurrentAnimIndex), CurrentAnimTime, Vel);

					for (int i = 0; i < MotionField->MotionBones.Num(); i++)
					{
						FMotionKeyUtils::GetAnimJointData(GetAnimAtIndex(CurrentAnimIndex), CurrentAnimTime, MotionField->MotionBones[i], JointsData[i]);
					}


					float OutCost = 0.f;

					int Winner = MotionField->GetLowestCostMotionKey(Responsiveness, VelocityStrength, PoseStrength, DesiredTrajectory, JointsData, Vel, TArray<uint8>(), TArray<uint8>(), OutCost);

					if (Winner >= 0)
					{

						bool TheWinnerIsAtTheSameLocation =
							(MotionField->MotionKeys[Winner].SrcAnimIndex == CurrentAnimIndex)
							&&
							(fabs(MotionField->MotionKeys[Winner].StartTime - CurrentAnimTime) < 0.2f);

						if ((!TheWinnerIsAtTheSameLocation))// && (ChangeCounter <= MaxChanges))// && ((MotionField->MotionKeys[CurrentMotKeyIndex].EndTime + 0.0f) < CurrentAnimTime))
						{
							//FString JO = TEXT("DOING IT SUPOSEDLY  :: ") + FString::FromInt(Winner);
							//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, JO);


							CurrentAnimIndex = MotionField->MotionKeys[Winner].SrcAnimIndex;
							CurrentAnimTime = MotionField->MotionKeys[Winner].StartTime;
							MotionKeyIndex = Winner;

							PlayAnimStartingAtTime(CurrentAnimIndex, CurrentAnimTime, BlendTime);

							//FString JO = TEXT("New MotionKey :: ") + FString::FromInt(Winner) + TEXT("  :  With COST  :  ") + FString::SanitizeFloat(OutCost);
							//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, JO);

							OnNewMotionKey(Winner, OutCost);

							if (DrawDebug)
							{
								DrawDebugMotionData(0.1f);
							}

						}

					}
				}

			}
		}
	}

}

void AMotionCharacter::DrawDebugMotionData(const float LifeTime)
{
	const FTransform RootTM = GetMesh()->GetComponentToWorld();

	DrawDebugLine
	(
		GetWorld(),
		RootTM.GetTranslation(),
		RootTM.GetTranslation() + (RootTM.TransformVector(Vel.GetSafeNormal()) * Vel.Size()),
		FColor::Purple,
		false,
		LifeTime,
		15.f
	);

	DrawDebugLine
	(
		GetWorld(),
		RootTM.GetTranslation() + FVector(0.f, 0.f, 20.f),
		RootTM.GetTranslation() + ((RootTM.TransformVector(MotionField->MotionKeys[MotionKeyIndex].PresentVel.GetSafeNormal()) * MotionField->MotionKeys[MotionKeyIndex].PresentVel.Size()) + FVector(0.f, 0.f, 20.f)),
		FColor::Orange,
		false,
		LifeTime,
		10.f
	);


	for (int i = 0; i < MotionField->MotionBones.Num(); i++)
	{

		FVector LastPos = RootTM.TransformPosition(JointsData[i].BoneCSPos);
		FVector LastVel = RootTM.TransformVector(JointsData[i].BoneCSVel.GetSafeNormal()) * JointsData[i].BoneCSVel.Size();

		DrawDebugSphere(GetWorld(), LastPos, 10.f, 12, FColor::Yellow, false, LifeTime, 0, 0.5f);
		//FTransform BoneTM = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSocketTransform(MotionField->MotionBones[i], RTS_Component) * RootTM;
		//Context.AnimInstanceProxy->AnimDrawDebugSphere(BoneTM.GetTranslation(), 10.f, 12, FColor::Magenta, false, LifeTime, 0.5f);

		DrawDebugLine
		(
			GetWorld(),
			LastPos,
			LastPos + LastVel,
			FColor::Yellow,
			false,
			LifeTime,
			2.f
		);


		FVector WinnerPos = RootTM.TransformPosition(MotionField->MotionKeys[MotionKeyIndex].MotionJointData[i].BoneCSPos);
		FVector WinnerVel = RootTM.TransformVector(MotionField->MotionKeys[MotionKeyIndex].MotionJointData[i].BoneCSVel.GetSafeNormal()) * MotionField->MotionKeys[MotionKeyIndex].MotionJointData[i].BoneCSVel.Size();
		DrawDebugSphere(GetWorld(), WinnerPos, 10.f, 12, FColor::Green, false, LifeTime, 0, 2.f);
		DrawDebugLine
		(
			GetWorld(),
			WinnerPos,
			WinnerPos + WinnerVel,
			FColor::Green,
			false,
			LifeTime,
			2.f
		);

		DrawDebugDirectionalArrow
		(
			GetWorld(),
			LastPos + LastVel,
			WinnerPos + WinnerVel,
			10.f,
			FColor::Red,
			false,
			LifeTime,
			2.f
		);

		DrawDebugDirectionalArrow
		(
			GetWorld(),
			LastPos,
			WinnerPos,
			10.f,
			FColor::Red,
			false,
			LifeTime,
			2.f
		);
	}
}

bool AMotionCharacter::AreSkeletonsCompatible()
{
	if ((MotionField != NULL) && (GetMesh()->SkeletalMesh != NULL))
	{
		return MotionField->GetMotionFieldSkeleton() == GetMesh()->SkeletalMesh->Skeleton;
	}

	return false;
}

void AMotionCharacter::PlayAnimStartingAtTime(const int AnimIndex, const float AnimTime, const float InBlendTime)
{
	if (UAnimInstance* Instance = GetMesh()->GetAnimInstance())
	{
		if (UAnimSequence* Anim = GetAnimAtIndex(AnimIndex))//Montages.IsValidIndex(AnimIndex))
		{
			//Instance->Montage_Play(Montages[AnimIndex], PlayRate, EMontagePlayReturnType::MontageLength, AnimTime);
			Instance->PlaySlotAnimationAsDynamicMontage( Anim, SlotName, InBlendTime, 0.0f, PlayRate, 1, 0.f, AnimTime);
		
		//	FString JO = TEXT("DOING IT SUPOSEDLY  ::  PLAYING THE ANIM    ") + FString::FromInt(AnimIndex);// +Montages[AnimIndex]->SlotAnimTracks[0].SlotName.ToString();
			//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Yellow, JO);
		}
	}
}

UAnimSequence * AMotionCharacter::GetAnimAtIndex(const int AnimIndex)
{
	if (MotionField)
	{
		if (MotionField->IsValidSrcAnimIndex(AnimIndex))
		{
			return MotionField->GetSrcAnimAtIndex(AnimIndex);
		}
	}

	return NULL;
}
/*
void AMotionCharacter::ResetMontages()
{
	Montages.Empty();

	if (MotionField)
	{
		const int32 SrcAnimNum = MotionField->GetSrcAnimNum();

		
		//////////////////////////////


		for (int i = 0; i < SrcAnimNum; i++)
		{

			int32 AnimIndex = i;

			if (UAnimSequence* Asset = GetAnimAtIndex(AnimIndex))
			{
				// now play
				UAnimMontage* NewMontage = NewObject<UAnimMontage>();
				NewMontage->SetSkeleton(Asset->GetSkeleton());
				// add new track
				FSlotAnimationTrack& NewTrack = NewMontage->SlotAnimTracks[0];
				NewTrack.SlotName = SlotName;
				FAnimSegment NewSegment;
				NewSegment.AnimReference = Asset;
				NewSegment.AnimStartTime = 0.f;
				NewSegment.AnimEndTime = Asset->SequenceLength;
				NewSegment.AnimPlayRate = 1.f;
				NewSegment.StartPos = 0.f;
				NewSegment.LoopingCount = 1;
				NewMontage->SequenceLength = NewSegment.GetLength();
				NewTrack.AnimTrack.AnimSegments.Add(NewSegment);

				FCompositeSection NewSection;
				NewSection.SectionName = TEXT("Default");
				NewSection.SetTime(0.0f);

				// add new section
				NewMontage->CompositeSections.Add(NewSection);
				NewMontage->BlendIn.SetBlendTime(BlendTime);
				NewMontage->BlendOut.SetBlendTime(0.f);
				NewMontage->BlendOutTriggerTime = 0.f;

				Montages.Add(NewMontage);


			}
		}
	}
}
*/
