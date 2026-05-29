#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/Engine.h"
#include "Perf_RagdollOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RagdollQuality : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct FPerf_RagdollPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RagdollMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceWarning = false;
};

/**
 * Advanced ragdoll performance optimization system
 * Manages ragdoll LOD, culling, and performance monitoring
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RagdollOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RagdollOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Performance")
    EPerf_RagdollQuality RagdollQuality = EPerf_RagdollQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Performance", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxActiveRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Performance", meta = (ClampMin = "100", ClampMax = "5000"))
    float RagdollCullDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Performance", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float RagdollLifetime = 5.0f;

    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 2000.0f;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_RagdollPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float PerformanceUpdateInterval = 1.0f;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxPhysicsTimeMs = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxRagdollMemoryMB = 100.0f;

public:
    // Ragdoll Management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void EnableRagdoll(USkeletalMeshComponent* SkeletalMesh, bool bApplyLOD = true);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void DisableRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void OptimizeAllRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void CullDistantRagdolls();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateRagdollLOD(USkeletalMeshComponent* SkeletalMesh, float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    int32 CalculateLODLevel(float Distance) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    // Quality Settings
    UFUNCTION(BlueprintCallable, Category = "Quality")
    void SetRagdollQuality(EPerf_RagdollQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Quality")
    void ApplyQualitySettings();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<USkeletalMeshComponent*> GetActiveRagdolls() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void CleanupExpiredRagdolls();

private:
    // Internal tracking
    UPROPERTY()
    TArray<USkeletalMeshComponent*> ActiveRagdolls;

    UPROPERTY()
    TMap<USkeletalMeshComponent*, float> RagdollActivationTimes;

    // Performance tracking
    float LastPerformanceUpdate = 0.0f;
    float AccumulatedPhysicsTime = 0.0f;
    int32 PhysicsFrameCount = 0;

    // Internal methods
    void RemoveInvalidRagdolls();
    void ApplyLODToRagdoll(USkeletalMeshComponent* SkeletalMesh, int32 LODLevel);
    float CalculateRagdollMemoryUsage() const;
    void LogPerformanceWarning(const FString& Warning) const;
};