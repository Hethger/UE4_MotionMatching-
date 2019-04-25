// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditorViewportClient.h"



#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "Engine/CollisionProfile.h"
#include "Utils.h"

#include "Engine.h"

#include "CanvasTypes.h"

#include "AssetEditorModeManager.h"


#include "Classes/AnimPreviewInstance.h"

#include "Animation/DebugSkelMeshComponent.h"
#include "Components/PoseableMeshComponent.h"


#include "Components/PostProcessComponent.h"
#include "Engine/Texture.h"
#include "Engine/TextureCube.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"


#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"

#include "Editor/AdvancedPreviewScene/Public/AssetViewerSettings.h"

#include "Engine/StaticMesh.h"

#include "MotionFieldEditor.h"

#include "MotionKeyUtils.h"


#define LOCTEXT_NAMESPACE "MotionFieldEditor"

FMotionFieldEditorViewportClientRoot::FMotionFieldEditorViewportClientRoot(const TWeakPtr<class SEditorViewport>& InEditorViewportWidget)
	: FEditorViewportClient(new FAssetEditorModeManager(), nullptr, InEditorViewportWidget)
{
	bOwnsModeTools = true;


	//ModifyCheckerboardTextureColors();
	//@TODO: ModeTools->SetToolkitHost


	SetViewModes(VMI_Lit, VMI_Lit);
	SetViewportType(LVT_Perspective);


	// Get the correct general direction of the perspective mode; the distance doesn't matter much as we've queued up a deferred zoom that will calculate a much better distance

	SetInitialViewTransform(LVT_Perspective, FVector(-300, 300, 150), FRotator(0, -45.f, 0), 0.0f);
}

FMotionFieldEditorViewportClientRoot::~FMotionFieldEditorViewportClientRoot()
{
}

void FMotionFieldEditorViewportClient::RequestFocusOnSelection(bool bInstant)
{
	FocusViewportOnBox(GetDesiredFocusBounds(), true);
}

//////////////////////////////////////////////////////////////////////////
// FMotionFieldEditorViewportClient

FMotionFieldEditorViewportClient::FMotionFieldEditorViewportClient(const TAttribute<UMotionField*>& InMotionFieldBeingEdited, TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
	:MotionFieldEditorPtr(InMotionFieldEditorPtr)
{
	MotionFieldBeingEdited = InMotionFieldBeingEdited;
	//MotionFieldBeingEditedLastFrame = MotionFieldBeingEdited.Get();
	PreviewScene = &OwnedPreviewScene;

	SetRealtime(true);

	// Create a render component for the sprite being edited
	AnimatedRenderComponent = NewObject<UDebugSkelMeshComponent>();
	AnimatedRenderComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);


	if (USkeletalMesh* SkelMesh = MotionFieldBeingEdited.Get()->GetMotionFieldSkeleton()->GetPreviewMesh())
	{
		AnimatedRenderComponent->SetSkeletalMesh(SkelMesh);
	}

	//RootMotionMode = ERootMotionMode::NoRootMotionExtraction;
	//AnimatedRenderComponent->SetMotionField(MotionFieldBeingEdited.Get());
	AnimatedRenderComponent->UpdateBounds();

	PreviewScene->AddComponent(AnimatedRenderComponent.Get(), FTransform::Identity);

	//	AnimatedRenderComponent->PreviewInstance->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
	//AnimatedRenderComponent->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
	////////////////////////////////OW BABY///////////////////////////////////////////////////////////////////////////////////////
	float InFloorOffset = 40.f;

	DefaultSettings = UAssetViewerSettings::Get();
	int32 CurrentProfileIndex = 0;// DefaultSettings->Profiles.IsValidIndex(CurrentProfileIndex) ? GetDefault<UEditorPerProjectUserSettings>()->AssetViewerProfileIndex : 0;
	ensureMsgf(DefaultSettings->Profiles.IsValidIndex(CurrentProfileIndex), TEXT("Invalid default settings pointer or current profile index"));
	FPreviewSceneProfile& Profile = DefaultSettings->Profiles[CurrentProfileIndex];

	const FTransform Transform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1));

	// Add and set up sky light using the set cube map texture
	static const auto CVarSupportStationarySkylight = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.SupportStationarySkylight"));
	const bool bUseSkylight = CVarSupportStationarySkylight->GetValueOnAnyThread() != 0;
	if (bUseSkylight)
	{
		SkyLightComponent = NewObject<USkyLightComponent>();
		SkyLightComponent->Cubemap = Profile.EnvironmentCubeMap.Get();
		SkyLightComponent->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
		SkyLightComponent->Mobility = EComponentMobility::Movable;
		SkyLightComponent->bLowerHemisphereIsBlack = false;
		SkyLightComponent->Intensity = Profile.SkyLightIntensity;
		PreviewScene->AddComponent(SkyLightComponent, Transform);
		SkyLightComponent->UpdateSkyCaptureContents(PreviewScene->GetWorld());
	}
	else
	{
		SphereReflectionComponent = NewObject<USphereReflectionCaptureComponent>();
		SphereReflectionComponent->Cubemap = Profile.EnvironmentCubeMap.Get();
		SphereReflectionComponent->ReflectionSourceType = EReflectionSourceType::SpecifiedCubemap;
		SphereReflectionComponent->Brightness = Profile.SkyLightIntensity;
		PreviewScene->AddComponent(SphereReflectionComponent, Transform);
		SphereReflectionComponent->UpdateReflectionCaptureContents(PreviewScene->GetWorld());
	}


	// Large scale to prevent sphere from clipping
	const FTransform SphereTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(2000));
	SkyComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());

	// Set up sky sphere showing hte same cube map as used by the sky light
	UStaticMesh* SkySphere = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/AssetViewer/Sphere_inversenormals.Sphere_inversenormals"), NULL, LOAD_None, NULL);
	check(SkySphere);
	SkyComponent->SetStaticMesh(SkySphere);

	UMaterial* SkyMaterial = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/AssetViewer/M_SkyBox.M_SkyBox"), NULL, LOAD_None, NULL);
	check(SkyMaterial);

	InstancedSkyMaterial = NewObject<UMaterialInstanceConstant>(GetTransientPackage());
	InstancedSkyMaterial->Parent = SkyMaterial;

	UTextureCube* DefaultTexture = LoadObject<UTextureCube>(NULL, TEXT("/Engine/MapTemplates/Sky/SunsetAmbientCubemap.SunsetAmbientCubemap"));
	InstancedSkyMaterial->SetTextureParameterValueEditorOnly(FName("SkyBox"), (Profile.EnvironmentCubeMap.Get() != nullptr) ? Profile.EnvironmentCubeMap.Get() : DefaultTexture);
	InstancedSkyMaterial->SetScalarParameterValueEditorOnly(FName("CubemapRotation"), Profile.LightingRigRotation / 360.0f);
	InstancedSkyMaterial->SetScalarParameterValueEditorOnly(FName("Intensity"), Profile.SkyLightIntensity);
	InstancedSkyMaterial->PostLoad();
	SkyComponent->SetMaterial(0, InstancedSkyMaterial);
	PreviewScene->AddComponent(SkyComponent, SphereTransform);

	PostProcessComponent = NewObject<UPostProcessComponent>();
	PostProcessComponent->Settings = Profile.PostProcessingSettings;
	PostProcessComponent->bUnbound = true;
	PreviewScene->AddComponent(PostProcessComponent, Transform);

	UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/AssetViewer/Floor_Mesh.Floor_Mesh"), NULL, LOAD_None, NULL);
	check(FloorMesh);
	FloorMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());
	FloorMeshComponent->SetStaticMesh(FloorMesh);

	FTransform FloorTransform(FRotator(0, 0, 0), FVector(0, 0, -(InFloorOffset)), FVector(4.0f, 4.0f, 1.0f));
	PreviewScene->AddComponent(FloorMeshComponent, FloorTransform);

	PreviewScene->SetLightDirection(Profile.DirectionalLightRotation);
	//////////////////////////////////////YEAH



	bShowPivot = false;
	bShowSockets = true;
	DrawHelper.bDrawGrid = true;// GetDefault<UMotionFieldEditorSettings>()->bShowGridByDefault;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);

	//RequestFocusOnSelection(true);
}


void FMotionFieldEditorViewportClient::DrawCurrentTrajectory(FPrimitiveDrawInterface* PDI)
{
	if (MotionFieldEditorPtr.IsValid())
	{
		FTrajectoryData PresentTrajectory;
		FTrajectoryData FutureTrajectory;
		FTransform PastWorldTM;

		if (MotionFieldEditorPtr.Pin().Get()->GetCurrentTrajectoryData(PresentTrajectory, FutureTrajectory, PastWorldTM))
		{
			/*
			if (PresentTrajectory.TrajectoryPoints.Num() == 11)
			{
				PDI->DrawLine(FVector(0.f), FVector(1000.f), FColor::Green, SDPG_Foreground, 5.0f);
			}
			*/
			
			uint8 Depth = SDPG_World;

			FMotionKeyUtils::DrawTrajectoryData(PresentTrajectory, PastWorldTM, PDI, FColor::Red, Depth);

			FTransform WorldTM = AnimatedRenderComponent->GetSocketTransform(AnimatedRenderComponent->GetBoneName(0));

			FMotionKeyUtils::DrawTrajectoryData(FutureTrajectory, WorldTM, PDI, FColor::Green, Depth);
			/*
			float TrajectoryThickness = 2.f;

			FTransform PresentMidWorldTM = PresentTrajectory.MidTM * PastWorldTM;
			FTransform PresentEndWorldTM = PresentTrajectory.EndTM * PastWorldTM;
			//
			PDI->DrawLine(PresentEndWorldTM.GetLocation(), PresentMidWorldTM.GetLocation(), FColor::Red, Depth, TrajectoryThickness);
			PDI->DrawLine(PresentMidWorldTM.GetLocation(), PastWorldTM.GetLocation(), FColor::Red, Depth, TrajectoryThickness);

			FTransform WorldTM = AnimatedRenderComponent->GetSocketTransform(AnimatedRenderComponent->GetBoneName(0));

			FTransform FutureMidWorldTM = FutureTrajectory.MidTM * WorldTM;
			FTransform FutureEndWorldTM = FutureTrajectory.EndTM * WorldTM;
			//SDPG_World
			PDI->DrawLine(WorldTM.GetLocation(), FutureMidWorldTM.GetLocation(), FColor::Green, Depth, TrajectoryThickness);
			PDI->DrawLine(FutureMidWorldTM.GetLocation(), FutureEndWorldTM.GetLocation(), FColor::Green, Depth, TrajectoryThickness);
			
			FTransform FutureArrowTM = FTransform(FRotationMatrix::MakeFromX((FutureMidWorldTM.GetLocation() - WorldTM.GetLocation()).GetSafeNormal()).ToQuat());
			FutureArrowTM.SetTranslation(FutureMidWorldTM.GetLocation());
			DrawDirectionalArrow(PDI, FutureArrowTM.ToMatrixNoScale(), FColor::Green, 0.0f, 20.f, Depth, 2.f);
			*/
			
			
		}
	}
}




void FMotionFieldEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);

	const bool bIsHitTesting = Canvas.IsHitTesting();
	if (!bIsHitTesting)
	{
		Canvas.SetHitProxy(nullptr);
	}

	int32 YPos = 42;

	static const FText MotionFieldHelpStr = LOCTEXT("MotionFieldEditHelp", "MotionField editor\n\nAdd keys using the toolbar or by drag-dropping sprite assets\nChange the timeline scale using Ctrl+MouseWheel\nEdit keys using the handles and right-click menu\nReorder keys by dragging and dropping");
	/*
	if (MotionFieldBeingEdited.Get())
	{
		TArray <FName> MotBones = MotionFieldBeingEdited.Get()->KeyBones;
		if (MotBones.Num() > 0)
		{
			for (int i = 0; i < MotBones.Num(); i++)
			{
				FString JO = MotBones[i].ToString();
				GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, JO);
			}
		}
	}
	*/
	// Display tool help
	{
		FCanvasTextItem TextItem(FVector2D(6, YPos), MotionFieldHelpStr, GEngine->GetSmallFont(), FLinearColor::White);
		TextItem.EnableShadow(FLinearColor::Black);
		TextItem.Draw(&Canvas);
		YPos += 36;
	}

}


void FMotionFieldEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);

	if (bShowPivot)
	{
	//	FUnrealEdUtils::DrawWidget(View, PDI, AnimatedRenderComponent->GetComponentTransform().ToMatrixWithScale(), 0, 0, EAxisList::Screen, EWidgetMovementMode::WMM_Translate);
	}

	
	DrawCurrentTrajectory(PDI);

}

FBox FMotionFieldEditorViewportClient::GetDesiredFocusBounds() const
{

	return AnimatedRenderComponent->Bounds.GetBox();
	
}

void FMotionFieldEditorViewportClient::Tick(float DeltaSeconds)
{
	if (AnimatedRenderComponent.IsValid())
	{
		//UMotionField* MotionField = MotionFieldBeingEdited.Get();
		//if (MotionField != MotionFieldBeingEditedLastFrame.Get())
		//{
		//	AnimatedRenderComponent->SetMotionField(MotionField);
		
			AnimatedRenderComponent->UpdateBounds();

			FTransform ComponentTransform = FTransform::Identity;
			if (UAnimSequence* CurrentAnim = MotionFieldEditorPtr.Pin().Get()->GetSourceAnimation())
			{
				if (CurrentAnim->bEnableRootMotion)
				{
					CurrentAnim->GetBoneTransform(ComponentTransform, 0, AnimatedRenderComponent->GetPosition(), false);
				}
			}
			
			AnimatedRenderComponent->SetWorldTransform(ComponentTransform, false);
		
			//MotionFieldBeingEditedLastFrame = MotionField;
		//}
	}

	FMotionFieldEditorViewportClientRoot::Tick(DeltaSeconds);

	if (!GIntraFrameDebuggingGameThread)
	{
		OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}

bool FMotionFieldEditorViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	bool bHandled = false;

	// Pass keys to standard controls, if we didn't consume input
	return (bHandled) ? true : FEditorViewportClient::InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
}

FLinearColor FMotionFieldEditorViewportClient::GetBackgroundColor() const
{
	return FColor(55, 55, 55);
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
