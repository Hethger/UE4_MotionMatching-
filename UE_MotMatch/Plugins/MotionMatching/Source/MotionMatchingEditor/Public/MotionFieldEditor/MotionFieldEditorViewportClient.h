// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "InputCoreTypes.h"
#include "PreviewScene.h"
#include "MotionField.h"

#include "EditorViewportClient.h"
#include "SEditorViewport.h"

class FCanvas;
class UDebugSkelMeshComponent;
class UPoseableMeshComponent;

class UAssetViewerSettings;
class UMaterialInstanceConstant;
class UPostProcessComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class USphereReflectionCaptureComponent;
struct FPreviewSceneProfile;
struct FTrajectoryData;

struct FMotionKey;

class FMotionFieldEditorViewportClientRoot : public FEditorViewportClient
{
public:
	/** Constructor */
	explicit FMotionFieldEditorViewportClientRoot(const TWeakPtr<class SEditorViewport>& InEditorViewportWidget = nullptr);
	~FMotionFieldEditorViewportClientRoot();

	
};

//////////////////////////////////////////////////////////////////////////
// FFlipbookEditorViewportClient

class FMotionFieldEditorViewportClient : public FMotionFieldEditorViewportClientRoot
{
public:
	/** Constructor */
	FMotionFieldEditorViewportClient(const TAttribute<class UMotionField*>& InMotionFieldBeingEdited, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditorPtr);

	// FViewportClient interface
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	// End of FViewportClient interface

	// FEditorViewportClient interface
	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad) override;
	virtual FLinearColor GetBackgroundColor() const override;
	// End of FEditorViewportClient interface

	void ToggleShowPivot() { bShowPivot = !bShowPivot; Invalidate(); }
	bool IsShowPivotChecked() const { return bShowPivot; }

	void ToggleShowSockets() { bShowSockets = !bShowSockets; Invalidate(); }
	bool IsShowSocketsChecked() const { return bShowSockets; }

	UDebugSkelMeshComponent* GetPreviewComponent() const
	{
	//	return AnimatedRenderComponentEdit.Get();
		return AnimatedRenderComponent.Get();
	}

	// Called to request a focus on the current selection
	virtual void RequestFocusOnSelection(bool bInstant);

	void DrawCurrentTrajectory(FPrimitiveDrawInterface* PDI);

	void SetCurrentTrajectory(const FTrajectoryData In)
	{
		CurrentTrajectory = In;
	}
	
//	void SetCurrentMotionKey(const FMotionKey InMotionKey);

protected:
	// FPaperEditorViewportClient interface
	virtual FBox GetDesiredFocusBounds() const;
	// End of FPaperEditorViewportClient interface
	USkyLightComponent* SkyLightComponent;
	UStaticMeshComponent* SkyComponent;
	USphereReflectionCaptureComponent* SphereReflectionComponent;
	UMaterialInstanceConstant* InstancedSkyMaterial;
	UPostProcessComponent* PostProcessComponent;
	UStaticMeshComponent* FloorMeshComponent;
	UAssetViewerSettings* DefaultSettings;
private:

	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;


	FTrajectoryData CurrentTrajectory;
	// The preview scene
	FPreviewScene OwnedPreviewScene;

	// The MotionField being displayed in this client
	TAttribute<class UMotionField*> MotionFieldBeingEdited;

	// A cached pointer to the MotionField that was being edited last frame. Used for invalidation reasons.
	//TWeakObjectPtr<class UMotionField> MotionFieldBeingEditedLastFrame;

	// Render component for the sprite being edited
	TWeakObjectPtr<UDebugSkelMeshComponent> AnimatedRenderComponent;

	// Should we show the sprite pivot?
	bool bShowPivot;

	// Should we show sockets?
	bool bShowSockets;
};
