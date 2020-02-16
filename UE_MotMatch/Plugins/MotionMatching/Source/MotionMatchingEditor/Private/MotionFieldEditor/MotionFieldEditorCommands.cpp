// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditor/MotionFieldEditorCommands.h"

//////////////////////////////////////////////////////////////////////////
// FMotionFieldEditorCommands

#define LOCTEXT_NAMESPACE "MotionFieldEditorCommands"

void FMotionFieldEditorCommands::RegisterCommands()
{
	UI_COMMAND(ProcessAll, "ProcessAll", "Processes all Motion Keys in all Animations", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ClearAll, "ClearAll", "Eliminates all Motion Keys", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(SetProperties, "SetProperties", "WARNING This will reset Tag Time Ranges", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(SetShowGrid, "Grid", "Displays the viewport grid.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(SetShowBounds, "Bounds", "Toggles display of the bounds of the static mesh.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(SetShowCollision, "Collision", "Toggles display of the simplified collision mesh of the static mesh, if one has been assigned.", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::C, EModifierKey::Alt));

	UI_COMMAND(SetShowPivot, "Show Pivot", "Display the pivot location of the static mesh.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(SetShowSockets, "Sockets", "Displays the MotionField sockets.", EUserInterfaceActionType::ToggleButton, FInputChord());

	UI_COMMAND(PickNewSpriteFrame, "Pick New Sprite", "Picks a new sprite for this key frame.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(EditSpriteFrame, "Edit Sprite", "Opens the sprite for this key frame in the Sprite Editor.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ShowInContentBrowser, "Show in Content Browser", "Shows the sprite for this key frame in the Content Browser.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
