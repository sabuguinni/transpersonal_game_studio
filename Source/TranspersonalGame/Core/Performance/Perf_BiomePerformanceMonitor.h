#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_BiomePerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveBiomeActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float BiomePhysicsOverhead = 0.0f;

    FPerf_BiomePerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        PhysicsTime = 0.0f;
        RenderTime = 0.0f;
        ActiveBiomeActors = 0;
        MemoryUsageMB = 0.0f;
        BiomePhysicsOverhead = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Optimal     UMETA(DisplayName = "Optimal"),
    Good        UMETA(DisplayName = "Good"),
    Acceptable  UMETA(DisplayName = "Acceptable"),
    Poor        UMETA(DisplayName = "Poor"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Performance monitoring system specifically for biome physics integration
 * Tracks performance impact of Core_BiomePhysicsAdapter and related systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_BiomePerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_BiomePerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MonitorMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_BiomePerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FPSWarningThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTimeWarningThreshold = 16.0f; // milliseconds

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

private:
    float TimeSinceLastUpdate = 0.0f;
    TArray<float> FPSHistory;
    TArray<float> PhysicsTimeHistory;
    int32 MaxHistorySize = 60; // 1 minute at 1 sample per second

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_BiomePerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const { return CurrentPerformanceLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<float> GetFPSHistory() const { return FPSHistory; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceHistory();

    // Biome-specific performance functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorBiomePhysicsImpact();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 CountActiveBiomeActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateBiomePhysicsOverhead();

protected:
    void UpdatePerformanceLevel();
    void AddToHistory(TArray<float>& History, float Value);
    void LogPerformanceWarning(const FString& Warning);
};