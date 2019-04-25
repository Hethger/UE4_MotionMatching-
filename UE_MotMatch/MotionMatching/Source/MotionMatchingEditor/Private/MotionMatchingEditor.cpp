

#include "MotionMatchingEditor.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "LevelEditor.h"

#include "MotionMatchingStyle.h"

#include "AssetToolsModule.h"

#include "AssetTypeActions_MotionField.h"

#include "AddNewContextDialog.h"

static const FName MotionMatchingEditorTabName("MotionMatchingEditor");

#define LOCTEXT_NAMESPACE "FMotionMatchingEditorModule"

void FMotionMatchingEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	MotionFieldEditor_ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	FMotionMatchingStyle::Initialize();
	
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_MotionField()));
	

}

void FMotionMatchingEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	MotionFieldEditor_ToolBarExtensibilityManager.Reset();

	FMotionMatchingStyle::Shutdown();
	
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();
	
}


void FMotionMatchingEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

void FMotionMatchingEditorModule::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);

	RegisteredClassNames.Add(ClassName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMotionMatchingEditorModule, MotionMatchingEditor)