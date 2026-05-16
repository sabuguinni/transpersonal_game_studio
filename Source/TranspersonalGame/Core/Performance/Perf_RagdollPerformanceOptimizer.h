#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "Perf_RagdollPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RagdollLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxRagdolls = 10;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceWarning = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_RagdollPerformanceOptimizer : public UObject
{
    GENERATED_BODY()

public:
    UPerf_RagdollPerformanceOptimizer();

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRagdollPerformance(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRagdollLODLevel(AActor* RagdollActor, EPerf_RagdollLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_RagdollMetrics GetRagdollMetrics(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LimitActiveRagdolls(UWorld* World, int32 MaxCount = 10);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void DisableDistantRagdolls(UWorld* World, float MaxDistance = 5000.0f);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsRagdollPerformanceCritical(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableRagdollCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunRagdollPerformanceTest();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_RagdollMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThresholdMs = 16.67f; // 60 FPS target

private:
    TArray<TWeakObjectPtr<AActor>> TrackedRagdolls;
    
    void UpdateRagdollMetrics(UWorld* World);
    void ApplyLODOptimizations(AActor* RagdollActor, float Distance);
    bool ShouldCullRagdoll(AActor* RagdollActor, const FVector& ViewLocation);
};