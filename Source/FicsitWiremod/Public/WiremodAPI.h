﻿// 

#pragma once

#include "CoreMinimal.h"
#include "WiremodReflection.h"
#include "WiremodUtils.h"
#include "Buildables/FGBuildable.h"
#include "Engine/DataTable.h"
#include "Engine/Private/DataTableJSON.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Subsystem/ModSubsystem.h"
#include "UObject/Object.h"
#include "WiremodAPI.generated.h"

USTRUCT(BlueprintType)
struct FWiremodAPIData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UDataTable*> ConnectionLists;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* FactoryGameList;
	
};

#define Ref UWiremodUtils::GetModReference

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class FICSITWIREMOD_API AWiremodAPI : public AModSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWiremodAPIData Data;
	
	FString ForceFileUsePrefix = "WMOD_FORCEUSE_";
	
	virtual void BeginPlay() override
	{
		ParseLists();
	}

	UFUNCTION(BlueprintCallable)
	FString ParseLists(bool ForceOverwrite = false)
	{
		FString ErrorList;

		
		auto path = FPaths::ProjectDir() + "/WiremodAPI";
		
		if(!FFileManagerGeneric::Get().DirectoryExists(*path))
		{
			FFileManagerGeneric::Get().MakeDirectory(*path);
			FString PlaceholderFile = 
			"This is a folder for wiremod API lists!\nDrop your JSON table in this folder and wiremod will attempt to add connections to buildings.\n"
			"The file name must match the mod reference! i.e. Wiremod's mod reference is \"FicsitWiremod\"\n"
			"In case wiremod already integrated the mod in question but you still want to use your own data list, you can force the file to be used as a data table by prefixing the file with \"" + ForceFileUsePrefix + "\"";

			FFileHelper::SaveStringToFile(PlaceholderFile, *FString(path + "/README.txt"));
		}


		TArray<FString> Files;
		FFileManagerGeneric::Get().FindFiles(Files, *path, *FString("json"));
		int successParse = 0;
		for (FString File : Files)
		{
			UDataTable* table = NewObject<UDataTable>();
			table->RowStruct = FBuildingConnections::StaticStruct();
	
			FString json;
			FFileHelper::LoadFileToString(json, *FString(path + "/" + File));
			auto parseErrors = table->CreateTableFromJSONString(json);
			
			if(parseErrors.Num() != 0)
			{
				for (FString Error : parseErrors)
				{
					UE_LOG(LogTemp, Error, TEXT("[WIREMOD API] There was an error when trying to parse file %s: %s"), *File, *Error)
					ErrorList.Append("[" + File + "]: " + Error + "\n");
				}
				continue;
			} else successParse++;


			FString modRef = File
			.Replace(*ForceFileUsePrefix, *FString())
			.Replace(*FString(".json"), *FString(""));
			
			AddList(modRef, table, File.StartsWith(ForceFileUsePrefix) || ForceOverwrite);
		}
	
		UE_LOG(LogTemp, Warning, TEXT("[WIREMOD API] Finished parsing lists. Parsed without errors %d out of %d lists"), successParse, Files.Num())
		return ErrorList.Len() > 0 ? ErrorList : "Successfully parsed all lists!";
	}
	
	
	
	UFUNCTION(BlueprintCallable)
	void AddLists(TMap<FString, UDataTable*> lists, UDataTable* factoryGameList, bool override)
	{
		Data.FactoryGameList = factoryGameList;
		for (TTuple<FString, UDataTable*> List : lists)
		{
			if(Data.ConnectionLists.Contains(List.Key) && !override) continue;
			Data.ConnectionLists.Add(List);
		}
	}

	UFUNCTION(BlueprintCallable)
	void AddList(FString modReference, UDataTable* list, bool allowOverwrite = false)
	{
		if(Data.ConnectionLists.Contains(modReference) && !allowOverwrite) return;

		UE_LOG(LogTemp, Warning, TEXT("[WIREMOD API] New connections list was added at runtime. {Mod reference:%s | Entries count:%d}"), *modReference, list->GetRowNames().Num())
		Data.ConnectionLists.Add(modReference, list);
	}

	UFUNCTION(BlueprintPure)
	UDataTable* GetListOrDefault(FString modReference)
	{
		if(Data.ConnectionLists.Contains(modReference)) return *Data.ConnectionLists.Find(modReference);
		return Data.FactoryGameList;
	}

	UFUNCTION(BlueprintPure, DisplayName="Is Registered", Category="Wiremod API")
	bool IsBuildableRegistered(AFGBuildable* buildable)
	{
		if(!IsValid(buildable)) return false;
		
		auto list = GetListOrDefault(Ref(buildable->GetClass()));
		auto className = UWiremodUtils::GetClassName(buildable->GetClass());
		
		return list->GetRowNames().Contains(className);
	}


	UFUNCTION(BlueprintCallable)
	FBuildingConnections FindConnections(AFGBuildable* Buildable)
	{
		if(!IsValid(Buildable)) return FBuildingConnections();
		
		auto modReference = Ref(Buildable->GetClass());
		
		auto list = GetListOrDefault(modReference);
		auto className = UWiremodUtils::GetClassName(Buildable->GetClass());

		if(list->GetRowNames().Contains(className))
		{
			auto row = list->FindRow<FBuildingConnections>(className, "");
			
			if(row->RedirectTo.ToString() == "None") return *row;

			return FindRedirectedList(row->RedirectTo.ToString());
		}

		return FBuildingConnections();
	}

	FBuildingConnections FindRedirectedList(FString redirector)
	{
		//Format the redirector
		FString listReference;
		FString objectReference;
		redirector.Split("__", &listReference, &objectReference);


		//Get the list from formatted redirector and check if it contains the row we're looking for
		auto list = GetListOrDefault(listReference);
		if(!list->GetRowNames().Contains(FName(objectReference))) return FBuildingConnections();

		//If the found row also has a redirector, recursively search for the source.
		auto row = list->FindRow<FBuildingConnections>(FName(objectReference), "");
		if(row->RedirectTo.ToString() == "None") return *row;

		return FindRedirectedList(row->RedirectTo.ToString());
	}

	
	
};
