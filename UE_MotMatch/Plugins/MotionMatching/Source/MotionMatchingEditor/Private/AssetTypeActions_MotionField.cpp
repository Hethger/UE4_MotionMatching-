

#include "AssetTypeActions_MotionField.h"
#include "MotionMatchingEditor.h"

#include "MotionFieldEditor/MotionFieldEditor.h"

#include "MotionField.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FFlipbookAssetTypeActions
/*
FAssetTypeActions_MotionField::FAssetTypeActions_MotionField(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}
*/
FText FAssetTypeActions_MotionField::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_MotionField", "Motion Field");
}

FColor FAssetTypeActions_MotionField::GetTypeColor() const
{
	return FColor::Magenta;
}

UClass* FAssetTypeActions_MotionField::GetSupportedClass() const
{
	return UMotionField::StaticClass();
}

void FAssetTypeActions_MotionField::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UMotionField* MotionField = Cast<UMotionField>(*ObjIt))
		{
			//FSimpleAssetEditor::CreateEditor(Mode, Mode == EToolkitMode::WorldCentric ? EditWithinLevelEditor : TSharedPtr<IToolkitHost>(), MotionField);
			
			TSharedRef<FMotionFieldEditor> NewFlipbookEditor(new FMotionFieldEditor());
			NewFlipbookEditor->InitMotionFieldEditor(Mode, EditWithinLevelEditor, MotionField);
			
		}
	}
}

uint32 FAssetTypeActions_MotionField::GetCategories()
{
	return EAssetTypeCategories::Animation;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE