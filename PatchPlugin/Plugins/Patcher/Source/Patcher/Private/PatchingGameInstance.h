// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "PatchingGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);

UCLASS()
class UPatchingGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    // Overrides
    virtual void Init() override;
    virtual void Shutdown() override;
    const FString DeploymentName = "PatchingCDN"; // DefaultGame.ini  /Script/Plugins.ChunkDownloader 에 있는 이름

public:
    UFUNCTION(BlueprintPure, Category = "Patching|Stats")
    void GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

public:
    // Delegates
    // Fired when the patching process succeeds or fails
    UPROPERTY(BlueprintAssignable, Category = "Patching");
    FPatchCompleteDelegate OnPatchComplete;

public:
    // Starts the game patching process. Returns false if the patching manifest is not up to date. */
    UFUNCTION(BlueprintCallable, Category = "Patching")
    bool PatchGame();

    void OnPatchVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr response, bool bWasSucessful);

protected:
    //Tracks if our local manifest file is up to date with the one hosted on our website
    UPROPERTY(BlueprintReadOnly, Category = "Patching");
    bool bIsDownloadManifestUpToDate;

protected:
    //Called when the chunk download process finishes
    void OnManifestUpdateComplete(bool bSuccess);

protected:
    // List of Chunk IDs to try and download
    TArray<int32> ChunkDownloadList;

    // Base URL for the Deployment
    FString BaseUrl;
    // URL variable to ContentBuildId.txt file
    FString PatchVersionURL;

protected:
    // Called when the chunk download process finishes
    void OnDownloadComplete(bool bSuccess);

    // Called whenever ChunkDownloader's loading mode is finished
    void OnLoadingModeComplete(bool bSuccess);

    // Called when ChunkDownloader finishes mounting chunks
    void OnMountComplete(bool bSuccess);

};
