#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Perf_DinosaurPerformanceOptimizer.generated.h"

/**
 * Performance optimization system specifically designed for dinosaur AI and rendering
 * Manages LOD, culling, and behavior complexity based on distance and performance metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurOptimizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Optimization")
    float DistanceFromPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Optimization")
    int32 CurrentLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Optimization")
    bool bAIBehaviorActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Optimization")
    bool bPhysicsSimulationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Optimization")
    float PerformanceImpactScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Optimization")
    ECreatureSize CreatureSize;

    FPerf_DinosaurOptimizationData()
    {
        DistanceFromPlayer = 0.0f;
        CurrentLODLevel = 0;
        bAIBehaviorActive = true;
        bPhysicsSimulationActive = true;
        PerformanceImpactScore = 1.0f;
        CreatureSize = ECreatureSize::Medium;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxVisibleDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float AIUpdateFrequencyMultiplier;

    FPerf_DinosaurLODSettings()
    {
        HighDetailDistance = 1500.0f;
        MediumDetailDistance = 3000.0f;
        LowDetailDistance = 5000.0f;
        CullingDistance = 8000.0f;
        MaxVisibleDinosaurs = 50;
        AIUpdateFrequencyMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_DinosaurPerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_DinosaurPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core optimization components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* OptimizationRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualizationMesh;

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_DinosaurLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance", meta = (AllowPrivateAccess = "true"))
    float OptimizationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance", meta = (AllowPrivateAccess = "true"))
    bool bEnableAggressiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_DinosaurOptimizationData> TrackedDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data", meta = (AllowPrivateAccess = "true"))
    int32 ActiveDinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data", meta = (AllowPrivateAccess = "true"))
    float TotalPerformanceImpact;

public:
    // Main optimization functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void OptimizeDinosaurPerformance();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void UpdateDinosaurLOD(AActor* DinosaurActor, float DistanceFromPlayer);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void SetDinosaurAIComplexity(AActor* DinosaurActor, float ComplexityLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void CullDistantDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void AnalyzeDinosaurPerformanceImpact();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetCurrentPerformanceScore() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    int32 GetOptimalDinosaurCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void LogPerformanceMetrics();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetLODSettings(const FPerf_DinosaurLODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void EnableAggressiveOptimization(bool bEnable);

    UFUNCTION(CallInEditor, Category = "Debug")
    void TestDinosaurOptimization();

private:
    // Internal optimization logic
    void UpdateOptimizationData();
    void ApplyLODOptimizations();
    void ManageAIComplexity();
    void HandlePerformanceCriticalSituation();
    
    // Timer handles
    FTimerHandle OptimizationTimerHandle;
    FTimerHandle PerformanceAnalysisTimerHandle;

    // Performance tracking
    float LastFrameTime;
    float AverageFrameTime;
    int32 FrameTimesSampled;
    TArray<float> RecentFrameTimes;
};