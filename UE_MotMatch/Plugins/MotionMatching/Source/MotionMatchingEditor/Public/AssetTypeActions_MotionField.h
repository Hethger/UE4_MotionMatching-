
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

/**
*
*/
/*AssetTypeActions class for the Motion Field so it can have a distinctive color and opens the Motion Field editor*/
class FAssetTypeActions_MotionField : public FAssetTypeActions_Base
{
public:
//	FAssetTypeActions_MotionField(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;
	// End of IAssetTypeActions interface

};