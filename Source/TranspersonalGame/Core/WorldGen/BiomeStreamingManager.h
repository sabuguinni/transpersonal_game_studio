#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LevelStreamingVolume.h"
#include "../SharedTypes.h"
#include "BiomeStreamingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<AActor*> BiomeActors;

    FWorld_BiomeZoneData()
    {
        BiomeName = TEXT("DefaultBiome");
        BiomeType = EWorld_BiomeType::Forest;
        Center = FVector::ZeroVector;
        StreamingRadius = 5000.0f;
        VegetationDensity = 100;
        LODDistance = 2000.0f;
        bHasWaterBodies = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VegetationLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling;

    FWorld_VegetationLODSettings()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 2000.0f;
        LOD2Distance = 4000.0f;
        CullDistance = 8000.0f;
        bEnableDistanceCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === BIOME ZONE MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bEnableBiomeStreaming;

    // === LOD SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    FWorld_VegetationLODSettings VegetationLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    bool bEnableVegetationLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LODUpdateFrequency;

    // === STREAMING VOLUMES ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Streaming")
    TArray<ULevelStreamingVolume*> StreamingVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bAutoCreateStreamingVolumes;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationActorsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceMonitoring;

private:
    // === INTERNAL STATE ===
    float LastBiomeUpdateTime;
    float LastLODUpdateTime;
    int32 CurrentProcessingIndex;
    TArray<AActor*> CachedVegetationActors;
    APawn* CachedPlayerPawn;

public:
    // === BIOME MANAGEMENT FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateBiomeZone(const FString& BiomeName, EWorld_BiomeType BiomeType, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeStreaming();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeZoneData GetNearestBiomeZone(FVector Location) const;

    // === VEGETATION LOD FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "LOD System")
    void UpdateVegetationLOD();

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    void SetVegetationLODLevel(AActor* VegetationActor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    void CacheVegetationActors();

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    int32 CalculateLODLevel(AActor* Actor, float Distance) const;

    // === STREAMING VOLUME FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void CreateStreamingVolume(FVector Center, FVector Extent);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreamingVolumes();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    bool IsLocationInStreamingRange(FVector Location) const;

    // === PERFORMANCE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetStreamingPerformanceMS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveVegetationCount() const;

    // === UTILITY FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void RegenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SaveBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LoadBiomeConfiguration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomeZones();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateBiomeSystem();
};