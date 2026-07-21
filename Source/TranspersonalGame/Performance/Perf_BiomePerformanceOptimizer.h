#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "Perf_BiomePerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    int32 MaxDinosaursPerBiome = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    float BiomeUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    float MaxDrawDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    int32 MaxActiveActorsPerBiome = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    bool bEnableBiomeCulling = true;

    FPerf_BiomePerformanceSettings()
    {
        MaxDinosaursPerBiome = 20;
        BiomeUpdateInterval = 2.0f;
        MaxDrawDistance = 15000.0f;
        MaxActiveActorsPerBiome = 100;
        bEnableBiomeCulling = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomeStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome Stats")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Stats")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Stats")
    int32 ActiveDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Stats")
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Stats")
    bool bIsPlayerInBiome = false;

    FPerf_BiomeStats()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        ActiveDinosaurs = 0;
        TotalActors = 0;
        FrameTimeMs = 0.0f;
        bIsPlayerInBiome = false;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_BiomePerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_BiomePerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    FPerf_BiomePerformanceSettings PerformanceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    TArray<FPerf_BiomeStats> BiomeStats;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float CurrentFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float TargetFPS = 60.0f;

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void UpdateBiomeStats();

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    FPerf_BiomeStats GetBiomeStatsForLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void SetBiomeActorLOD(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void CullDistantBiomeActors();

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void BalanceDinosaurDistribution();

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    EBiomeType GetBiomeTypeForLocation(const FVector& Location);

private:
    float LastUpdateTime = 0.0f;
    TArray<AActor*> CachedBiomeActors;
    
    void InitializeBiomeStats();
    void UpdatePerformanceMetrics();
    bool ShouldCullActor(AActor* Actor, const FVector& PlayerLocation);
    void OptimizeBiomeActors(EBiomeType BiomeType, const FVector& BiomeCenter);
};