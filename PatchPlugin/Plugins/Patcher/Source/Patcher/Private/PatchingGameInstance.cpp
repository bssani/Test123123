// Fill out your copyright notice in the Description page of Project Settings.


#include "PatchingGameInstance.h"
#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"

void UPatchingGameInstance::Init()
{
    Super::Init();

    // Construct PatchVersionURL
    TArray<FString> TempCdnBaseUrls;
    GConfig->GetArray(TEXT("/Script/Plugins.ChunkDownloader"), TEXT("CdnBaseUrls"), TempCdnBaseUrls, GGameIni);

    if (TempCdnBaseUrls.Num() > 0) {
        BaseUrl = TempCdnBaseUrls[0];

        PatchVersionURL = BaseUrl + "/ContentBuildId.txt";
    }

    // create a new Http request and bind the response callback
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UPatchingGameInstance::OnPatchVersionResponse);

    // configure and send the request
    Request->SetURL(PatchVersionURL);
    Request->SetVerb("GET");
    Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
    Request->SetHeader("Content-Type", TEXT("application-json"));
    if (!Request->ProcessRequest()) {
        UE_LOG(LogTemp, Display, TEXT("Failed HTTP request to ContentBuildId.txt. File Unreachable."));
    }
}

void UPatchingGameInstance::OnPatchVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr response, bool bWasSucessful)
{
    if (bWasSucessful) {
        if (!response.IsValid()) { // Pretty important to fix "Assertion failed: IsValid()" when Web Server is down!!!
            // HTTP request failed
            UE_LOG(LogTemp, Display, TEXT("Failed HTTP request response from ContentBuildId.txt file. File Unreachable."));
        }
        else if (EHttpResponseCodes::IsOk(response->GetResponseCode())) {
            // content build ID. Our Http response will provide this info from txt file. From Blueprint editable variable.
            FString ContentBuildId = response->GetContentAsString(); // Throws assertion error popup if the CDN is down, because there wasn't a reponse!!!
            UE_LOG(LogTemp, Display, TEXT("Patch Content ID Response: %s"), *ContentBuildId);
            // initialize the chunk downloader with chosen platform
            TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
            Downloader->Initialize(UGameplayStatics::GetPlatformName(), 8);

            // load the cached build ID
            Downloader->LoadCachedBuild(DeploymentName);

            // update the build manifest file
            TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {bIsDownloadManifestUpToDate = bSuccess; };
            Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
        }
        else {
            // HTTP request error
            UE_LOG(LogTemp, Display, TEXT("Failed HTTP request for ContentBuildId.txt."));
        }
    }
}

void UPatchingGameInstance::Shutdown()
{
    Super::Shutdown();
    // Shut down ChunkDownloader
    FChunkDownloader::Shutdown();
}

void UPatchingGameInstance::OnManifestUpdateComplete(bool bSuccess)
{
    bIsDownloadManifestUpToDate = bSuccess;
}

void UPatchingGameInstance::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
    //Get a reference to ChunkDownloader
    TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

    //Get the loading stats struct
    FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

    //Get the bytes downloaded and bytes to download
    BytesDownloaded = LoadingStats.BytesDownloaded;
    TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

    //Get the number of chunks mounted and chunks to download
    ChunksMounted = LoadingStats.ChunksMounted;
    TotalChunksToMount = LoadingStats.TotalChunksToMount;

    //Calculate the download and mount percent using the above stats
    if (BytesDownloaded != 0 || TotalBytesToDownload != 0)
    {
        DownloadPercent = ((float)BytesDownloaded / (float)TotalBytesToDownload) * 100.0f;
    }
    else
    {
        BytesDownloaded = 100.0f;
        TotalBytesToDownload = 100.0f;
    }

    if (MountPercent != 0 || TotalChunksToMount != 0)
    {
        MountPercent = ((float)ChunksMounted / (float)TotalChunksToMount) * 100.0f;
    }
    else
    {
        ChunksMounted = 100.0f;
        TotalChunksToMount = 100.0f;
    }
}

bool UPatchingGameInstance::PatchGame()
{
    // make sure the download manifest is up to date
    if (bIsDownloadManifestUpToDate)
    {
        // get the chunk downloader
        TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

        // This might not be necessary since QueryDB runs at begin play.
        Downloader->GetAllChunkIds(ChunkDownloadList);

        // report manifest file's chunk status
        for (int32 ChunkID : ChunkDownloadList)
        {
            int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
            UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);

        }

            TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnDownloadComplete(bSuccess); };
            Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

            // start loading mode. This outside of the if might fix Assertion failed: !"Pak cannot be unmounted with outstanding requests" 
            TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {OnLoadingModeComplete(bSuccess); };
            Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
        

        return true;
    }

    // you couldn't contact the server to validate your Manifest, so you can't patch
    UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

    return false;
}

void UPatchingGameInstance::OnLoadingModeComplete(bool bSuccess)
{
    OnDownloadComplete(bSuccess);
}

void UPatchingGameInstance::OnMountComplete(bool bSuccess)
{
    OnPatchComplete.Broadcast(bSuccess);
}

void UPatchingGameInstance::OnDownloadComplete(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Display, TEXT("Download complete"));

        // get the chunk downloader
        TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
        FJsonSerializableArrayInt DownloadedChunks;

        for (int32 ChunkID : ChunkDownloadList)
        {
            DownloadedChunks.Add(ChunkID);
        }

        //Mount the chunks
        TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
        Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

        OnPatchComplete.Broadcast(true);
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("Load process failed"));

        // call the delegate
        OnPatchComplete.Broadcast(false);
    }
}