#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Perf_PhysicsPerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsQuality : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveRagdolls;

    UPROPERTY(BlueprintReadOnly)
    int32 PhysicsConstraints;

    UPROPERTY(BlueprintReadOnly)
    float PhysicsMemoryUsage;

    FPerf_PhysicsPerformanceMetrics()
        : PhysicsFrameTime(0.0f)
        , ActiveRigidBodies(0)
        , ActiveRagdolls(0)
        , PhysicsConstraints(0)
        , PhysicsMemoryUsage(0.0f)
    {}
};

/**
 * Physics Performance Manager - Optimizes physics simulation for 60fps PC / 30fps console
 * Manages physics LOD, ragdoll culling, and physics tick rates
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // Quality settings
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsQuality(EPerf_PhysicsQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    EPerf_PhysicsQuality GetPhysicsQuality() const { return CurrentQuality; }

    // Ragdoll optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetMaxActiveRagdolls(int32 MaxRagdolls);

    // Physics LOD
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsLODDistance(float Distance);

    // Performance targets
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetTargetPhysicsFrameTime(float TargetTime);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PhysicsQuality CurrentQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetPhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveRagdolls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsOptimization;

private:
    void ApplyPhysicsQualitySettings();
    void CullDistantPhysicsObjects();
    void OptimizePhysicsTickRate();
    void MonitorPhysicsMemory();

    FTimerHandle PerformanceUpdateTimer;
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
};