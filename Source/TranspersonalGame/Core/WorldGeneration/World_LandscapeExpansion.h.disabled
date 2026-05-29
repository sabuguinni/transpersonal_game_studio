#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Components/ActorComponent.h"
#include "World_LandscapeExpansion.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentSizeQuads = 63;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerSection = 31;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float ScaleX = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float ScaleY = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float ScaleZ = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountX = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountY = 32;

    FWorld_LandscapeSettings()
    {
        ComponentSizeQuads = 63;
        SectionsPerComponent = 2;
        QuadsPerSection = 31;
        ScaleX = 100.0f;
        ScaleY = 100.0f;
        ScaleZ = 100.0f;
        ComponentCountX = 32;
        ComponentCountY = 32;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeHeightData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float NoiseScale = 0.001f;

    FWorld_BiomeHeightData()
    {
        BiomeName = TEXT("Default");
        Center = FVector2D::ZeroVector;
        Bounds = FVector2D(10000.0f, 10000.0f);
        BaseHeight = 0.0f;
        HeightVariation = 100.0f;
        NoiseScale = 0.001f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_LandscapeExpansion : public AActor
{
    GENERATED_BODY()

public:
    AWorld_LandscapeExpansion();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings")
    FWorld_LandscapeSettings LandscapeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TArray<FWorld_BiomeHeightData> BiomeHeightData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UFUNCTION(BlueprintCallable, Category = "Landscape", CallInEditor)
    void GenerateExpandedLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape", CallInEditor)
    void BackupCurrentLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape", CallInEditor)
    void RestoreBackupLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void SetupBiomeHeightProfiles();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    float CalculateHeightAtPosition(const FVector2D& Position) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    TArray<uint16> GenerateHeightmapData() const;

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    bool ValidateLandscapeSettings() const;

private:
    UPROPERTY()
    class ALandscape* CurrentLandscape;

    UPROPERTY()
    TArray<uint16> BackupHeightData;

    bool bHasBackup = false;

    float PerlinNoise(float X, float Y, float Scale) const;
    float BiomeBlendWeight(const FVector2D& Position, const FWorld_BiomeHeightData& Biome) const;
    FVector2D WorldToLandscapeCoords(const FVector& WorldPosition) const;
    FVector LandscapeToWorldCoords(const FVector2D& LandscapePosition) const;
};