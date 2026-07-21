#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousTerrainProps.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TerrainPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    FVector BaseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    float ScaleVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    float RotationVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    int32 MaxInstancesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    float SpawnProbability;

    FEnvArt_TerrainPropData()
    {
        PropName = TEXT("DefaultProp");
        BaseScale = FVector(1.0f, 1.0f, 1.0f);
        ScaleVariation = 0.3f;
        RotationVariation = 360.0f;
        MaxInstancesPerBiome = 50;
        SpawnProbability = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FallenLogData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    float LogLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    float LogDiameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    float DecayLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    bool bHasMoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    bool bHasFerns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    FVector SpawnLocation;

    FEnvArt_FallenLogData()
    {
        LogLength = 800.0f;
        LogDiameter = 120.0f;
        DecayLevel = 0.5f;
        bHasMoss = true;
        bHasFerns = false;
        SpawnLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_WeatheredRockData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathered Rocks")
    float RockSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathered Rocks")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathered Rocks")
    bool bHasLichens;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathered Rocks")
    bool bHasCracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathered Rocks")
    FLinearColor RockColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathered Rocks")
    FVector SpawnLocation;

    FEnvArt_WeatheredRockData()
    {
        RockSize = 200.0f;
        WeatheringLevel = 0.6f;
        bHasLichens = true;
        bHasCracks = true;
        RockColor = FLinearColor(0.4f, 0.35f, 0.3f, 1.0f);
        SpawnLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousTerrainProps : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousTerrainProps();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Terrain prop configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    TArray<FEnvArt_TerrainPropData> PropConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    TArray<FEnvArt_FallenLogData> FallenLogConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Props")
    TArray<FEnvArt_WeatheredRockData> WeatheredRockConfigurations;

    // Biome distribution settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float MinDistanceBetweenProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float MaxDistanceBetweenProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    int32 MaxPropsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    bool bUseRandomSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    int32 RandomSeed;

    // Spawned prop tracking
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedFallenLogs;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedWeatheredRocks;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedGenericProps;

public:
    // Terrain prop management
    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void SpawnFallenLogsInBiome(EBiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void SpawnWeatheredRocksInBiome(EBiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void SpawnGenericPropsInBiome(EBiomeType BiomeType, const FString& PropType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void ClearAllPropsInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void RegeneratePropsForBiome(EBiomeType BiomeType);

    // Prop configuration
    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void SetPropConfiguration(const FString& PropName, const FEnvArt_TerrainPropData& Configuration);

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    FEnvArt_TerrainPropData GetPropConfiguration(const FString& PropName) const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    bool IsValidPropLocation(const FVector& Location, float MinDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    int32 GetTotalPropsInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Props")
    void OptimizePropDistribution();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckPropPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ReducePropDensity(float ReductionFactor);

private:
    // Internal prop spawning
    AActor* SpawnFallenLogActor(const FEnvArt_FallenLogData& LogData);
    AActor* SpawnWeatheredRockActor(const FEnvArt_WeatheredRockData& RockData);
    AActor* SpawnGenericPropActor(const FEnvArt_TerrainPropData& PropData, const FVector& Location);

    // Validation and cleanup
    void ValidatePropCounts();
    void CleanupInvalidProps();
    FVector GetBiomeBounds(EBiomeType BiomeType) const;
};