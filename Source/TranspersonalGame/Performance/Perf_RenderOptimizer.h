#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_RenderOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableOcclusionCulling = true;

    FPerf_LODSettings()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 2500.0f;
        LOD2Distance = 5000.0f;
        CullDistance = 10000.0f;
        bEnableDistanceCulling = true;
        bEnableOcclusionCulling = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 LOD0Actors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 LOD1Actors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 LOD2Actors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageDrawDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float RenderThreadTime = 0.0f;

    FPerf_RenderStats()
    {
        VisibleActors = 0;
        CulledActors = 0;
        LOD0Actors = 0;
        LOD1Actors = 0;
        LOD2Actors = 0;
        AverageDrawDistance = 0.0f;
        RenderThreadTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RenderOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RenderOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD and culling settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Optimization")
    FPerf_LODSettings LODSettings;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoAdjustLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableRenderOptimization = true;

    // Render statistics
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FPerf_RenderStats RenderStats;

    // Optimization methods
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void OptimizeRenderingForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetLODForActor(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void CullActorByDistance(AActor* Actor, bool bShouldCull);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void UpdateRenderStats();

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void AutoAdjustLODSettings(float CurrentFrameRate);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    float GetDistanceToPlayer(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    int32 GetOptimalLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Render Optimization", CallInEditor = true)
    void ApplyOptimizationToAllActors();

private:
    // Internal optimization methods
    void OptimizeStaticMesh(UStaticMeshComponent* MeshComp, float Distance);
    void OptimizeSkeletalMesh(USkeletalMeshComponent* MeshComp, float Distance);
    void UpdateActorVisibility(AActor* Actor, float Distance);
    
    // Performance tracking
    float LastFrameRate = 60.0f;
    float FrameRateHistory[10] = {60.0f};
    int32 FrameHistoryIndex = 0;
    
    // Optimization state
    bool bOptimizationActive = false;
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 1.0f;
};