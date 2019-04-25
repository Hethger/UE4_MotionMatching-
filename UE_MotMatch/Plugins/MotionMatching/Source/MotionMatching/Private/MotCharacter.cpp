

#include "MotCharacter.h"
#include "MotionMatching.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionStructs.h"
#include "Engine.h"
#include "MotCharacterMovementComponent.h"


// Sets default values
AMotCharacter::AMotCharacter(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
	:
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMotCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMotCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMotCharacter::OverrideRootMotion(const FTransform RootMotion)
{
	UMotCharacterMovementComponent * ChMovComp = Cast<UMotCharacterMovementComponent>(AMotCharacter::GetCharacterMovement());
	
	if (ChMovComp != NULL)
	{

		ChMovComp->OverridenRootMotion = RootMotion;
		ChMovComp->OverridenRootMotion.SetScale3D(GetMesh()->GetComponentToWorld().GetScale3D());
		ChMovComp->OverrideRootMotion = true;
		
	}
}