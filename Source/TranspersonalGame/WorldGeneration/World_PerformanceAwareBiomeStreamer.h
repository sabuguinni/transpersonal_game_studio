#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerVolume.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_PerformanceAwareBiomeStreamer.generated.h"

// Forward declarations
class APerf_RealTimePerformanceController;
class UWorld_BiomeDataAsset;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeStreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    float UnloadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    int32 MaxActorsInZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    float PerformanceWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    bool bPerformanceOptimized;

    FWorld_BiomeStreamingZone()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneCenter = FVector::ZeroVector;
        StreamingRadius = 5000.0f;
        UnloadRadius = 7000.0f;
        BiomeType = EBiomeType::Forest;
        MaxActorsInZone = 100;
        PerformanceWeight = 1.0f;
        bIsActive = true;
        bPerformanceOptimized = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PerformanceStreamingMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveStreamingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LoadedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceOptimizationActive;

    FWorld_PerformanceStreamingMetrics()
    {
        CurrentFPS = 60.0f;
        TargetFPS = 60.0f;
        ActiveStreamingZones = 0;
        LoadedActors = 0;
        MemoryUsageMB = 0.0f;
        StreamingBudgetUsed = 0.0f;
        bPerformanceOptimizationActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PerformanceAwareBiomeStreamer : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PerformanceAwareBiomeStreamer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core streaming management
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void InitializeStreamingSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UpdateStreamingZones(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void OptimizeStreamingForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void RegisterStreamingZone(const FWorld_BiomeStreamingZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UnregisterStreamingZone(const FString& ZoneName);

    // Performance integration
    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void IntegrateWithPerformanceController();

    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void AdjustStreamingBasedOnPerformance(float CurrentFPS, float TargetFPS);

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void LoadBiomeAssets(EBiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UnloadBiomeAssets(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void TransitionBetweenBiomes(EBiomeType FromBiome, EBiomeType ToBiome);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FWorld_PerformanceStreamingMetrics GetStreamingMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<FWorld_BiomeStreamingZone> GetActiveStreamingZones() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsZoneWithinStreamingDistance(const FWorld_BiomeStreamingZone& Zone, const FVector& PlayerLocation) const;

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StreamingVisualizationMesh;

    // Streaming configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    TArray<FWorld_BiomeStreamingZone> StreamingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    float StreamingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    int32 MaxConcurrentStreams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    float MemoryBudgetMB;

    // Performance integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationAggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimizeForPerformance;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FWorld_PerformanceStreamingMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FString> ActiveZoneNames;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastPerformanceCheckTime;

private:
    // Internal references
    APerf_RealTimePerformanceController* PerformanceController;

    // Internal state
    bool bIsInitialized;
    bool bPerformanceIntegrationActive;
    TMap<FString, float> ZoneLoadTimes;
    TArray<AActor*> ManagedActors;

    // Helper functions
    void UpdateZoneActivation(const FVector& PlayerLocation);
    void ManageMemoryBudget();
    void OptimizeZonePerformance(FWorld_BiomeStreamingZone& Zone);
    float CalculateZonePerformanceImpact(const FWorld_BiomeStreamingZone& Zone) const;
    void CleanupInactiveZones();
};