#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_BiomePerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRenderDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxFoliageInstances = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    FPerf_BiomePerformanceSettings()
    {
        MaxRenderDistance = 15000.0f;
        MaxFoliageInstances = 10000;
        LODDistanceMultiplier = 1.0f;
        bEnableOcclusionCulling = true;
        TargetFrameTime = 16.67f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_BiomePerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_BiomePerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_BiomePerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringRadius = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledActorCount = 0;

private:
    float LastUpdateTime = 0.0f;
    TArray<AActor*> ManagedActors;
    TArray<AActor*> CulledActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomeActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBiomeType(EBiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDistanceCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetManagedActorCount() const { return ManagedActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorForManagement(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActorFromManagement(AActor* Actor);

protected:
    void FindActorsInRadius();
    void ApplyBiomeSpecificOptimizations();
    void UpdateActorLOD(AActor* Actor, float Distance);
    bool ShouldCullActor(AActor* Actor, float Distance) const;
    void RestoreCulledActors();
    FPerf_BiomePerformanceSettings GetBiomeSpecificSettings() const;
};