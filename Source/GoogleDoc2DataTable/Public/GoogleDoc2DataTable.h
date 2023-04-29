#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"

#include "GoogleDocsApi.h"

class FGoogleDoc2DataTableModule : public IModuleInterface
{

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FSlowTask* SlowTask = nullptr;
	UDataTable* SelectedDataTable = nullptr;
	UGoogleDocsApi* GoogleDocsApi = nullptr;
	FDelegateHandle CBAssetExtenderDelegateHandle;

	void ShowWindow();
	void AddMenuEntry(FMenuBuilder& MenuBuilder);

	TSharedRef<FExtender> DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList);
};
