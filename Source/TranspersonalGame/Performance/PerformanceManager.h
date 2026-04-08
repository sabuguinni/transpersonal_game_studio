#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "PerformanceTargets.h"
#include "PerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Frame Metrics")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Metrics")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Metrics")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Metrics")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Metrics")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Metrics")
    int32 CurrentDrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Metrics")
    int32 CurrentTriangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Metrics")
    int32 CurrentInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Metrics")
    float UsedTextureMemory = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Metrics")
    float UsedMeshMemory = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Metrics")
    float UsedAudioMemory = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    bool bIsWithinBudget = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    TArray<FString> PerformanceWarnings;

    FPerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        CurrentFrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        CurrentDrawCalls = 0;
        CurrentTriangles = 0;
        CurrentInstances = 0;
        UsedTextureMemory = 0.0f;
        UsedMeshMemory = 0.0f;
        UsedAudioMemory = 0.0f;
        bIsWithinBudget = true;
    }
};

/**
 * Central performance management system for Transpersonal Game Studio
 * Ensures 60fps on PC high-end and 30fps on console
 * Monitors and enforces performance budgets across all systems
 * 
 * This is the legacy performance manager - use DynamicPerformanceManager for new features
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceManager : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceManager();

    // Core performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void Initialize(EPerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceBudget GetCurrentBudget() const { return CurrentBudget; }

    // Dynamic quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustQualityForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicResolutionScale(float Scale);

    // Monitoring and debugging
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceCapture(const FString& CaptureName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceCapture();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    // Console commands
    UFUNCTION(Exec)
    void ShowPerformanceStats();

    UFUNCTION(Exec)
    void SetPerformanceTarget(int32 TargetFPS);

    UFUNCTION(Exec)
    void TogglePerformanceHUD();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceTarget CurrentTarget = EPerformanceTarget::PC_HighEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerformanceBudget CurrentBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicQualityAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bShowPerformanceHUD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 0.1f; // Update every 100ms

private:
    // Internal performance tracking
    TArray<float> FrameTimeHistory;
    TArray<float> GPUTimeHistory;
    int32 HistorySize = 60; // 60 samples for 1-second average

    FDateTime LastPerformanceUpdate;
    bool bIsCapturing = false;
    FString CurrentCaptureName;

    // Performance adjustment methods
    void ApplyPerformanceBudget();
    void UpdateScalabilitySettings();
    void CheckPerformanceThresholds();
    void CollectRenderingMetrics();
    void CollectMemoryMetrics();
    
    // Quality adjustment helpers
    void AdjustViewDistance(float Multiplier);
    void AdjustShadowQuality(int32 Quality);
    void AdjustTextureQuality(int32 Quality);
    void AdjustEffectsQuality(int32 Quality);
    void AdjustPostProcessQuality(int32 Quality);
};