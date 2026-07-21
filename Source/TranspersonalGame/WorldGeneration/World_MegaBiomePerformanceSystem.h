#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_MegaBiomePerformanceSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_MegaBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceWeight;

    FWorld_MegaBiomeData()
    {
        BiomeName = TEXT("DefaultBiome");
        CenterLocation = FVector::ZeroVector;
        Radius = 5000.0f;
        LODDistance = 3000.0f;
        MaxActorCount = 100;
        PerformanceWeight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VegetationZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FString ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_DensityLevel Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    FWorld_VegetationZoneData()
    {
        ZoneType = TEXT("DefaultVegetation");
        Position = FVector::ZeroVector;
        Density = EWorld_DensityLevel::Medium;
        LODLevels = 3;
        CullingDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterFeatureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWorld_WaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseSimplifiedShader;

    FWorld_WaterFeatureData()
    {
        FeatureName = TEXT("DefaultWater");
        WaterType = EWorld_WaterType::Lake;
        CenterLocation = FVector::ZeroVector;
        Size = 1000.0f;
        Depth = 100.0f;
        bUseSimplifiedShader = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveBiomes;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StreamedVegetationZones;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceTargetMet;

    FWorld_PerformanceMetrics()
    {
        TotalActorCount = 0;
        CurrentFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        ActiveBiomes = 0;
        StreamedVegetationZones = 0;
        bPerformanceTargetMet = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MegaBiomePerformanceSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MegaBiomePerformanceSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CORE COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SystemMeshComponent;

    // === MEGA-BIOME MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mega-Biomes")
    TArray<FWorld_MegaBiomeData> MegaBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FWorld_VegetationZoneData> VegetationZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FWorld_WaterFeatureData> WaterFeatures;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization;

    // === RUNTIME METRICS ===
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FWorld_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedBiomeActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedVegetationActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedWaterActors;

public:
    // === MEGA-BIOME FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Mega-Biomes")
    void InitializeMegaBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Mega-Biomes")
    void CreateMegaBiome(const FWorld_MegaBiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Mega-Biomes")
    void UpdateBiomePerformance(const FString& BiomeName, float PerformanceWeight);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void CreateVegetationZone(const FWorld_VegetationZoneData& VegetationData);

    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateWaterFeature(const FWorld_WaterFeatureData& WaterData);

    // === PERFORMANCE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(float NewTargetFPS, int32 NewMaxActors, float NewMaxMemoryMB);

    // === BIOME STREAMING ===
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void EnableBiomeStreaming(const FString& BiomeName, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateBiomeStreamingDistance(const FString& BiomeName, float NewDistance);

    // === TRANSITION MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Transitions")
    void CreateBiomeTransition(const FString& FromBiome, const FString& ToBiome, const FVector& TransitionCenter);

    UFUNCTION(BlueprintCallable, Category = "Transitions")
    void UpdateTransitionPerformance();

    // === RESOURCE OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void OptimizeResourceDistribution();

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void CreateResourceNode(const FVector& Location, const FString& ResourceType, EWorld_ResourceYield Yield);

    // === DEBUGGING & VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetSystemStatusReport() const;

protected:
    // === INTERNAL HELPERS ===
    void InitializeDefaultBiomes();
    void InitializeDefaultVegetation();
    void InitializeDefaultWaterFeatures();
    void CalculatePerformanceMetrics();
    void ApplyPerformanceOptimizations();
    AActor* SpawnBiomeActor(const FWorld_MegaBiomeData& BiomeData);
    AActor* SpawnVegetationActor(const FWorld_VegetationZoneData& VegetationData);
    AActor* SpawnWaterActor(const FWorld_WaterFeatureData& WaterData);

private:
    float LastPerformanceUpdateTime;
    bool bSystemInitialized;
    int32 OptimizationCycleCount;
};