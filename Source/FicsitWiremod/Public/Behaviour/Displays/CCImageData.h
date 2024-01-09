﻿// 

#pragma once

#include "CoreMinimal.h"
#include "FGGameState.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Object.h"
#include "CCImageData.generated.h"

USTRUCT(BlueprintType)
struct FPixelScreenRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FLinearColor> RowData;

	FPixelScreenRow(){}
	FPixelScreenRow(int Width, FLinearColor Color)
	{
		for(int i = 0; i < Width; i++) RowData.Add(Color);
	}


	bool operator==(const FPixelScreenRow& Other) const
	{
		return RowData == Other.RowData;
	}
};


USTRUCT(BlueprintType)
struct FPixelScreenData
{
	GENERATED_BODY()


	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FPixelScreenRow> Data;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FLinearColor BackgroundColor = FLinearColor::Transparent;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int Width = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int Height = 0;

	FPixelScreenData(){}
	FPixelScreenData(int Width, int Height, FLinearColor BackgroundColor)
	{
		for(int i = 0; i < Height; i++)
		{
			Data.Add(FPixelScreenRow(Width, BackgroundColor));
		}

		this->BackgroundColor = BackgroundColor;
		this->Width = Width;
		this->Height = Height;
	}

	int GetSizeInBytes() const
	{
		return Height * Width * sizeof(FLinearColor);
	}

	double GetSizeInMegabytes() const
	{
		return GetSizeInBytes() / 1000000.0f;
	}

	bool IsValid() const
	{
		const int Area = Width*Height;
		return Area && sqrt(Area) < 75;
	}

	TArray<TArray<FLinearColor>> MakeGrid() const
	{
		TArray<TArray<FLinearColor>> Out;
		
		for(int i = 0; i < Data.Num(); i++) Out.Add(Data[i].RowData);
		
		return Out;
	}

	bool operator ==(const FPixelScreenData& Other) const
	{
		if(Width != Other.Width || Height != Other.Height) return false;
		return Data == Other.Data;
	}

	UTexture2D* MakeTexture() const
	{
		auto GridData = MakeGrid();
		auto Texture = UTexture2D::CreateTransient(
		Width, 
		Height
		);

		Texture->CreateResource();
		// Lock the texture so it can be modified
		FColor* FormatedImageData = static_cast<FColor*>( Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

		// Create base mip.
		uint8* DestPtr = NULL;
		for( int32 y=0; y<Height; y++ )
		{
			for( int32 x=0; x<Width; x++ )
			{
				FormatedImageData[y * Width + x] = GridData[y][x].ToFColor(false);
			}
		}

		// Unlock the texture
		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

		//Configure texture for pixel art style rendering
		Texture->MipLoadOptions = ETextureMipLoadOptions::OnlyFirstMip;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->CompressionSettings = TC_Alpha;
		Texture->Filter = TF_Nearest;
		
		Texture->UpdateResource();
		
		return Texture;
	}
};


UCLASS()
class UPixelScreenBlueprintFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:


	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="GET"))
	static FPixelScreenRow GetRow(const FPixelScreenData& Data, int Row)
	{
		if(Data.Data.IsValidIndex(Row))
			return Data.Data[Row];

		GetTypeHash(&Data);

		return FPixelScreenRow();
	}

	UFUNCTION(BlueprintPure)
	static double GetSizeInMegabytes(const FPixelScreenData& Data){ return Data.GetSizeInMegabytes(); }

	UFUNCTION(BlueprintPure)
	static int GetSizeInBytes(const FPixelScreenData& Data){ return Data.GetSizeInBytes(); }

	UFUNCTION(BlueprintCallable)
	static FPixelScreenData MakePixelScreenFromData(int Width, int Height, FLinearColor BackgroundColor)
	{
		return FPixelScreenData(Width, Height, BackgroundColor);
	}

	UFUNCTION(BlueprintCallable)
	static FPixelScreenData MakePixelScreenFromFile(const FString& FileName)
	{
		auto OutputData = FPixelScreenData();
		auto Texture = FImageUtils::ImportFileAsTexture2D(FileName);
		if(Texture)
		{
			const int SizeX = Texture->GetSizeX();
			const int SizeY = Texture->GetSizeY();
			OutputData.Width = SizeX;
			OutputData.Height = SizeY;
			
			OutputData.Data.SetNum(SizeY);
			for(auto& Row : OutputData.Data) Row.RowData.SetNum(SizeX);
			
			const FColor* FormatedImageData = static_cast<const FColor*>( Texture->GetPlatformData()->Mips[0].BulkData.LockReadOnly());

			for(int32 X = 0; X < SizeX; X++)
			{
				for (int32 Y = 0; Y < SizeY; Y++)
				{
					OutputData.Data[Y].RowData[X] = FLinearColor(FormatedImageData[Y * SizeX + X]);
				}
			}

			Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		}

		return OutputData;
	}

	UFUNCTION(BlueprintCallable)
	static UTexture2D* ToTexture(const FPixelScreenData& Data)
	{
		return Data.MakeTexture();
	}

	UFUNCTION(BlueprintCallable)
	static bool IsTextureReadyForExport(UTexture* Texture)
	{
		if (!Texture || !Texture->GetResource() || !Texture->GetResource()->TextureRHI)
		{
			return false;
		}

		return true;
	}

	UFUNCTION(BlueprintPure)
	static bool IsValidImage(const FPixelScreenData& Data) { return Data.IsValid(); }

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="=="))
	static bool Equals(const FPixelScreenData& Data1, const FPixelScreenData& Data2) { return Data1 == Data2; }

	UFUNCTION(BlueprintCallable)
	static void SetTextureParam_Safe(UMaterialInstanceDynamic* Material, FName ParameterName, UTexture* Value)
	{
		if(!Material || !Value) return;

		//How the fuck is this even possible is a mystery to me, but it keeps crashing on this line in some scenarios, so i'm checking it as well.
		if(!Value->IsA(UTexture::StaticClass())) return;
		
		Material->SetTextureParameterValue(ParameterName, Value);
	}

	UFUNCTION(BlueprintPure)
	static FString GetPixelArtFolderPath(UObject* WorldContext)
	{
		auto const GameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(WorldContext));
		if(!GameState) return "ERROR: PIXEL ART MANAGER IS NULL";
		auto const SessionName = GameState->GetSessionName();
		
		return FPaths::ProjectSavedDir() + "Circuitry/" + SessionName + "/PixelArt/";
	}
	
};
