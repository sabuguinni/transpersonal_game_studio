#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomePerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomePerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetTriangleCount = 500000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome = 1000;

    FWorld_BiomePerformanceSettings()
    {
        TargetTriangleCount = 500000;
        MaxDrawDistance = 15000.0f;
        LODDistanceMultiplier = 1.0f;
        bEnableDistanceCulling = true;
        MaxActorsPerBiome = 1000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CurrentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CurrentTriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bPerformanceTarget = true;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float LastUpdateTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomePerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomePerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_BiomePerformanceSettings PerformanceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FWorld_BiomeStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float UpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float BiomeRadius = 25000.0f;

private:
    float LastStatsUpdate = 0.0f;
    TArray<AActor*> ManagedActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomeActors();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FWorld_BiomeStats GetBiomeStats() const { return CurrentStats; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceTargetMet() const { return CurrentStats.bPerformanceTarget; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBiomeType(EBiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODSettings();

protected:
    void CalculateStats();
    void ApplyPerformanceOptimizations();
    int32 CountTrianglesInBiome();
    void UpdateVisualization();
};