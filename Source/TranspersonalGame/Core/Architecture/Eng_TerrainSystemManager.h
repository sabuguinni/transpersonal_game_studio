#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"
#include "SharedTypes.h"
#include "Eng_TerrainSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 512.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 TerrainSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bEnableErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ErosionStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ErosionIterations = 10;

    FEng_TerrainConfiguration()
    {
        HeightScale = 512.0f;
        NoiseScale = 0.001f;
        TerrainSize = 2048;
        bEnableErosion = true;
        ErosionStrength = 0.5f;
        ErosionIterations = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float MinHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float MaxHeight = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float BlendRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    bool bIsActive = true;

    FEng_TerrainLayer()
    {
        LayerName = TEXT("DefaultLayer");
        MinHeight = 0.0f;
        MaxHeight = 1000.0f;
        BlendRange = 100.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TerrainSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TerrainSystemManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void InitializeTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void GenerateTerrain(const FEng_TerrainConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void ApplyErosion(float Strength, int32 Iterations);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void AddTerrainLayer(const FEng_TerrainLayer& Layer);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void RemoveTerrainLayer(const FString& LayerName);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    FVector GetSurfaceNormalAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    bool IsLocationWalkable(const FVector& WorldLocation, float MaxSlope = 45.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    TArray<FVector> GenerateSpawnPoints(int32 NumPoints, float MinDistance = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void UpdateTerrainLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System", CallInEditor)
    void ValidateTerrainSystem();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_TerrainConfiguration DefaultConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEng_TerrainLayer> TerrainLayers;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    class ALandscapeProxy* CurrentLandscape;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsInitialized = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLODLevel = 4;

private:
    void CreateDefaultLayers();
    void CacheTerrainData();
    float CalculateNoiseValue(float X, float Y, float Scale) const;
    void ApplyHeightmapModifications(const TArray<float>& HeightData);
};