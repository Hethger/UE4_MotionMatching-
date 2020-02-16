
#pragma once

#include "Modules/ModuleManager.h"

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Modules/ModuleInterface.h"

#include "AssetTypeCategories.h"
#include "Engine/Texture2D.h"
#include "Editor.h"
#include "EditorModeRegistry.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"

#include "IAssetTypeActions.h"

#include "ISettingsModule.h"

#include "PropertyEditorDelegates.h"

class FToolBarBuilder;
class FMenuBuilder;

/**/
class FMotionMatchingEditorModule : public IModuleInterface
{
public:
	FMotionMatchingEditorModule()
		:MotionFieldAssetCategoryBit(EAssetTypeCategories::Misc)
	{
	}
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	
	virtual TSharedPtr<FExtensibilityManager> GetMotionFieldEditorToolBarExtensibilityManager() { return MotionFieldEditor_ToolBarExtensibilityManager; }
private:
	TSharedPtr<FExtensibilityManager> MotionFieldEditor_ToolBarExtensibilityManager;

	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
	void RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate);
	
	TSet< FName > RegisteredClassNames;
	EAssetTypeCategories::Type MotionFieldAssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

};