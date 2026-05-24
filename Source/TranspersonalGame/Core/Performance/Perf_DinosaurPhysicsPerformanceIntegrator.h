#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Perf_DinosaurPhysicsPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurMovementTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurCollisionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurAnimationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurAITime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurRagdollTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    int32 ActiveDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    int32 DinosaurPhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurCPUUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    float DinosaurGPUUsage = 0.0f;

    FPerf_DinosaurPhysicsMetrics()
    {
        DinosaurMovementTime = 0.0f;
        DinosaurCollisionTime = 0.0f;
        DinosaurAnimationTime = 0.0f;
        DinosaurAITime = 0.0f;
        DinosaurRagdollTime = 0.0f;
        ActiveDinosaurs = 0;
        DinosaurPhysicsObjects = 0;
        DinosaurMemoryUsage = 0.0f;
        DinosaurCPUUsage = 0.0f;
        DinosaurGPUUsage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurMovementTime = 8.33f; // 120 FPS budget

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurCollisionTime = 5.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurAnimationTime = 6.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurAITime = 4.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurRagdollTime = 3.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    int32 MaxActiveDinosaurs = 50;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    int32 MaxDinosaurPhysicsObjects = 200;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurMemoryUsage = 512.0f; // MB

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurCPUUsage = 25.0f; // Percentage

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Performance Settings")
    float MaxDinosaurGPUUsage = 30.0f; // Percentage

    FPerf_DinosaurPerformanceSettings()
    {
        MaxDinosaurMovementTime = 8.33f;
        MaxDinosaurCollisionTime = 5.0f;
        MaxDinosaurAnimationTime = 6.0f;
        MaxDinosaurAITime = 4.0f;
        MaxDinosaurRagdollTime = 3.0f;
        MaxActiveDinosaurs = 50;
        MaxDinosaurPhysicsObjects = 200;
        MaxDinosaurMemoryUsage = 512.0f;
        MaxDinosaurCPUUsage = 25.0f;
        MaxDinosaurGPUUsage = 30.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_DinosaurPerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_DinosaurPhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_DinosaurPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    FPerf_DinosaurPhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void UpdatePerformanceSettings(const FPerf_DinosaurPerformanceSettings& NewSettings);

    // Dinosaur-Specific Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void OptimizeDinosaurMovement();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void OptimizeDinosaurCollision();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void OptimizeDinosaurAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void OptimizeDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void OptimizeDinosaurRagdoll();

    // Performance Level Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void SetDinosaurPerformanceLevel(EPerf_DinosaurPerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    EPerf_DinosaurPerformanceLevel GetCurrentPerformanceLevel() const;

    // Adaptive Quality Control
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void UpdateAdaptiveQuality();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void OptimizeDinosaurMemory();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void CleanupUnusedDinosaurResources();

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void AnalyzeDinosaurPerformance();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics Performance")
    void GeneratePerformanceReport();

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dinosaur Physics Performance")
    void RunDinosaurPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dinosaur Physics Performance")
    void ValidateDinosaurPhysicsIntegration();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    FPerf_DinosaurPhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Physics Performance")
    FPerf_DinosaurPerformanceSettings PerformanceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Physics Performance")
    EPerf_DinosaurPerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Physics Performance")
    bool bIsMonitoringActive;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Physics Performance")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Physics Performance")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Physics Performance")
    float AdaptiveQualityInterval;

private:
    FTimerHandle MonitoringTimerHandle;
    FTimerHandle AdaptiveQualityTimerHandle;
    
    double LastUpdateTime;
    double AccumulatedDeltaTime;
    
    // Internal performance tracking
    void UpdateDinosaurMetrics();
    void UpdateDinosaurMovementMetrics();
    void UpdateDinosaurCollisionMetrics();
    void UpdateDinosaurAnimationMetrics();
    void UpdateDinosaurAIMetrics();
    void UpdateDinosaurRagdollMetrics();
    void UpdateDinosaurMemoryMetrics();
    void UpdateDinosaurResourceMetrics();
    
    // Optimization helpers
    void ApplyPerformanceLevel(EPerf_DinosaurPerformanceLevel Level);
    void AdjustDinosaurQuality(float PerformanceFactor);
    void ScaleDinosaurComplexity(float ScaleFactor);
    void OptimizeDinosaurLOD();
    void OptimizeDinosaurCulling();
    
    // Adaptive quality helpers
    float CalculatePerformanceScore() const;
    bool ShouldReduceQuality() const;
    bool ShouldIncreaseQuality() const;
    void AdaptiveQualityUpdate();
};