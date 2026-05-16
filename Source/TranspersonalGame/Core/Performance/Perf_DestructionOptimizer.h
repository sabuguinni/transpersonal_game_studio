#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_DestructionOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_DestructionQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DestructionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageDestructionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDestructibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeImpact;

    FPerf_DestructionMetrics()
        : AverageDestructionTime(0.0f)
        , ActiveDestructibleActors(0)
        , MemoryUsageMB(0.0f)
        , FrameTimeImpact(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousDestructions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DestructionCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDestructionLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_DestructionQuality QualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DebrisLifetime;

    FPerf_DestructionSettings()
        : MaxSimultaneousDestructions(5)
        , DestructionCullingDistance(5000.0f)
        , bEnableDestructionLOD(true)
        , QualityLevel(EPerf_DestructionQuality::Medium)
        , DebrisLifetime(30.0f)
    {}
};

/**
 * Performance optimizer for destruction systems
 * Manages destruction quality, culling, and performance metrics
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_DestructionOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_DestructionOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_DestructionMetrics GetDestructionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeDestructionPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDestructionQuality(EPerf_DestructionQuality NewQuality);

    // Destruction management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanPerformDestruction(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RegisterDestructionEvent(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupOldDebris();

    // Performance testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunDestructionPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void AnalyzeDestructionBottlenecks();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_DestructionSettings DestructionSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_DestructionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceLogging;

private:
    // Internal tracking
    TArray<FVector> RecentDestructions;
    TArray<float> DestructionTimes;
    float LastPerformanceUpdate;
    int32 FramesSinceLastUpdate;

    // Performance optimization methods
    void UpdatePerformanceMetrics();
    void ApplyQualitySettings();
    void CullDistantDestructions();
    void OptimizeDebrisCount();
    float CalculateFrameTimeImpact() const;
    void LogPerformanceData() const;
};