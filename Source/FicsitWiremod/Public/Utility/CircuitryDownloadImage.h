﻿// 

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Subsystem/ModSubsystem.h"
#include "CircuitryDownloadImage.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FCircuitryDownloadImageDelegate, UTexture*, Texture, bool, Success);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCircuitryImageDownloadTaskFinished, const FString& Url, UTexture* Texture, bool Success);
/**
 * 
 */
UCLASS()
class FICSITWIREMOD_API UCircuitryDownloadImage : public UObject
{
	GENERATED_BODY()

public:
	void StartDownload(const FString& URL);

	void HandleImageDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	void OnFail()
	{
		RemoveFromRoot();
		Finished.Broadcast(Url, nullptr, false);
	}
public:
	FString Url;
	FCircuitryImageDownloadTaskFinished Finished;
};

USTRUCT()
struct FCircuitryImageDownloadTask
{
	GENERATED_BODY()

	UPROPERTY()
	bool Finished = false;

	UPROPERTY()
	bool Success = false;

	UPROPERTY()
	FDateTime FinishedAt = FDateTime::MinValue();

	UPROPERTY()
	UTexture* Texture = nullptr;
	
	TArray<FCircuitryDownloadImageDelegate> Delegates;

	void AddDelegate(const FCircuitryDownloadImageDelegate& Delegate)
	{
		if(Finished) Delegate.ExecuteIfBound(Texture, Success);
		else Delegates.Add(Delegate);
	}

	void OnDownloadFinished(UTexture* ResultTexture, bool DownloadSuccess)
	{
		Finished = true;
		Success = DownloadSuccess;
		FinishedAt = FDateTime::Now();
		Texture = ResultTexture;

		for(auto& Delegate : Delegates) Delegate.ExecuteIfBound(ResultTexture, DownloadSuccess);

		Delegates.Empty();
	}
};

UCLASS()
class ACircuitryImageStorage : public AModSubsystem
{
	GENERATED_BODY()

	ACircuitryImageStorage()
	{
		ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnClient;
	}

public:
	UFUNCTION(BlueprintCallable)
	void DownloadImage(const FString& Url, FCircuitryDownloadImageDelegate OnFinish, bool AllowCache = true)
	{
		if(AllowCache)
		{
			if(auto CachedTask = Cache.Find(Url))
			{
				CachedTask->AddDelegate(OnFinish);
				return;
			}
		}

		//If the url starts with "asset://" then we should return whatever we find in that path
		if(Url.StartsWith("asset://"))
		{
			FString AssetPath = Url;
			AssetPath.ReplaceInline(*FString("asset://"), *FString());
			const auto Texture = Cast<UTexture>(FSoftObjectPath(AssetPath).TryLoad());

			OnFinish.ExecuteIfBound(Texture, Texture != nullptr);
			
			return;
		}
		//However for backwards compatibility we should check for old path that didn't contain that string.
		//If we don't find anything then it's probably just a url without a protocol specified, so we shouldn't return null.
		else if(!Url.Contains("://"))
		{
			const auto Texture = Cast<UTexture>(FSoftObjectPath(Url).TryLoad());
			if(Texture)
			{
				OnFinish.ExecuteIfBound(Texture, true);
				return;
			}
		}
		
		UCircuitryDownloadImage* DownloadTask = NewObject<UCircuitryDownloadImage>();
		auto CacheObject = FCircuitryImageDownloadTask();
		CacheObject.AddDelegate(OnFinish);
		DownloadTask->Finished.AddUObject(this, &ACircuitryImageStorage::OnDownloadFinished);
		
		Cache.Add(Url, CacheObject);
		
		DownloadTask->StartDownload(Url);
	}

	void OnDownloadFinished(const FString& Url, UTexture* ResultTexture, bool DownloadSuccess)
	{
		auto CachedTask = Cache.Find(Url);
		if(!CachedTask) return;
		CachedTask->OnDownloadFinished(ResultTexture, DownloadSuccess);
	}
	
	UPROPERTY()
	TMap<FString, FCircuitryImageDownloadTask> Cache;
};
