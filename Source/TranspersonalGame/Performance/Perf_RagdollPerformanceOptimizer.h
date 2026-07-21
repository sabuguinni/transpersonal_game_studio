#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Perf_RagdollPerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollPerformanceSettings
{
    GENERATED_BODY()

    // Maximum number of active ragdolls for 60fps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveRagdolls = 8;

    // Distance-based LOD thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowDetailDistance = 8000.0f;

    // Performance optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RagdollLifetime = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxBonesPerRagdoll = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateRate = 60.0f;

    FPerf_RagdollPerformanceSettings()
    {
        MaxActiveRagdolls = 8;
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 3000.0f;
        LowDetailDistance = 8000.0f;
        RagdollLifetime = 15.0f;
        MaxBonesPerRagdoll = 32;
        PhysicsUpdateRate = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollInstanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TWeakObjectPtr<USkeletalMeshComponent> RagdollComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CreationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveBoneCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_OptimizationLevel CurrentLOD;

    FPerf_RagdollInstanceData()
    {
        CreationTime = 0.0f;
        LastUpdateTime = 0.0f;
        ActiveBoneCount = 0;
        CurrentLOD = EPerf_OptimizationLevel::High;
    }
};

/**
 * Performance optimizer for ragdoll physics systems
 * Manages ragdoll LOD, culling, and performance budgets for 60fps target
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RagdollPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RagdollPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_RagdollPerformanceSettings PerformanceSettings;

    // Active ragdoll tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    TArray<FPerf_RagdollInstanceData> ActiveRagdolls;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    int32 TotalActiveRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float PhysicsSimulationTime;

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterRagdoll(USkeletalMeshComponent* RagdollComponent);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterRagdoll(USkeletalMeshComponent* RagdollComponent);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRagdollLOD(USkeletalMeshComponent* RagdollComponent, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceBudget(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullExpiredRagdolls();

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetRagdollPerformanceCost() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldCreateNewRagdoll() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_OptimizationLevel GetOptimalLODForDistance(float Distance) const;

    // Emergency performance controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EmergencyPerformanceCleanup();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(EPerf_OptimizationLevel NewMode);

private:
    // Internal optimization functions
    void UpdateRagdollLOD(FPerf_RagdollInstanceData& RagdollData, float DeltaTime);
    void CalculatePhysicsPerformance();
    void OptimizeRagdollBones(USkeletalMeshComponent* RagdollComponent, int32 TargetBoneCount);
    void UpdatePhysicsSettings(USkeletalMeshComponent* RagdollComponent, EPerf_OptimizationLevel LOD);

    // Performance tracking
    float LastPerformanceCheck;
    float AccumulatedFrameTime;
    int32 FrameCount;
    
    // Emergency performance state
    bool bEmergencyMode;
    float EmergencyModeStartTime;
};