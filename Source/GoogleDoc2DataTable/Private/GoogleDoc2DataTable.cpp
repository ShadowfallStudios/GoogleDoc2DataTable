#include "GoogleDoc2DataTable.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"

#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Runtime/Engine/Classes/Kismet/DataTableFunctionLibrary.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "FGoogleDoc2DataTableModule"

void FGoogleDoc2DataTableModule::StartupModule()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FGoogleDoc2DataTableModule::DataTableContextMenuExtender));
	CBAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();
}

void FGoogleDoc2DataTableModule::ShutdownModule()
{
	if (!IsRunningCommandlet() && !IsRunningGame())
	{
		if (FContentBrowserModule* ContentBrowserModule = static_cast<FContentBrowserModule*>(FModuleManager::Get().GetModule(TEXT("ContentBrowser"))))
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			CBAssetMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == CBAssetExtenderDelegateHandle; });
		}
	}
}

TSharedRef<FExtender> FGoogleDoc2DataTableModule::DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || AssetDataList[0].AssetClassPath != UDataTable::StaticClass()->GetClassPathName())
	{
		return MakeShareable(new FExtender());
	}

	SelectedDataTable = Cast<UDataTable>(AssetDataList[0].GetAsset());

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddMenuExtension(
		"ImportedAssetActions",
		EExtensionHook::Before,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FGoogleDoc2DataTableModule::AddMenuEntry));

	return MenuExtender.ToSharedRef();
}

void FGoogleDoc2DataTableModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	if (!SelectedDataTable->SourceURL.IsEmpty())
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Load from Google doc"),
			FText::FromString("Load from Google doc"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FGoogleDoc2DataTableModule::ShowWindow))
		);
	}
}

void FGoogleDoc2DataTableModule::ShowWindow()
{
	SlowTask = new FSlowTask(100.0f, LOCTEXT("GoogleFetchTask", "Fetching data..."));
	SlowTask->Initialize();
	SlowTask->MakeDialog(true);
	
	GoogleDocsApi = NewObject<UGoogleDocsApi>(UGoogleDocsApi::StaticClass());
	GoogleDocsApi->OnResponseDelegate.BindLambda([&](FString Response)
	{
		if (!SlowTask->ShouldCancel())
		{
			UDataTableFunctionLibrary::FillDataTableFromCSVString(SelectedDataTable, Response);
			SlowTask->EnterProgressFrame(100.f, LOCTEXT("GoogleFetchSuccess", "Success"));
			SlowTask->Destroy();
		}
		else
		{
			SlowTask->Destroy();
		}
		delete SlowTask;
		SlowTask = nullptr;
	});
	GoogleDocsApi->SendRequest(SelectedDataTable->SourceURL);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGoogleDoc2DataTableModule, GoogleDoc2DataTable)