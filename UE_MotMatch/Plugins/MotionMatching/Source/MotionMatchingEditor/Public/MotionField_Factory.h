// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"

#include "Factories/Factory.h"
#include "MotionField.h"
#include "MotionField_Factory.generated.h"


 
 
UCLASS()
class MOTIONMATCHINGEDITOR_API UMotionFieldFactory : public UFactory
{
	GENERATED_BODY()
		
public:
	UMotionFieldFactory(const FObjectInitializer& ObjectInitializer);
	//~MotionField_Factory();


	UPROPERTY(EditAnywhere, Category = MotionFieldFactory)
		class USkeleton* TargetSkeleton;

	UPROPERTY(EditAnywhere, Category = MotionFieldFactory)
		TArray <FName> TargetMotionBones;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface	
	
};
