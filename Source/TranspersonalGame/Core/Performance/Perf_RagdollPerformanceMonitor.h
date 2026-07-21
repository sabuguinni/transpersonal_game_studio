#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"
#include "Perf_RagdollPerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Performance")
    float SimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Performance")
    int32 ActiveRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Performance")
    int32 PhysicsConstraints;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Performance")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Performance")
    float CPUTime;

    FPerf_RagdollMetrics()
        : SimulationTime(0.0f)
        , ActiveRagdolls(0)
        , PhysicsConstraints(0)
        , MemoryUsage(0.0f)
        , CPUTime(0.0f)
    {}
};

UENUM(BlueprintType)
enum class EPerf_RagdollQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_RagdollPerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_RagdollPerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    FPerf_RagdollMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void SetRagdollQuality(EPerf_RagdollQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void CullDistantRagdolls(float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void LimitActiveRagdolls(int32 MaxActive);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance Testing")
    void RunRagdollPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance Testing")
    void GeneratePerformanceReport();

protected:
    // Core monitoring properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_RagdollMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_RagdollQuality QualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveRagdolls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

private:
    // Internal monitoring data
    float MonitoringTimer;
    TArray<float> FrameTimeHistory;
    TArray<FPerf_RagdollMetrics> MetricsHistory;
    
    // Performance optimization methods
    void UpdateMetrics();
    void AnalyzePerformance();
    void ApplyOptimizations();
    void LogPerformanceData();
    
    // Ragdoll management
    void FindActiveRagdolls();
    void OptimizeRagdollConstraints();
    void AdjustRagdollLOD();
    void CleanupInactiveRagdolls();
};