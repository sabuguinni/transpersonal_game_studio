#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerformanceTarget : uint8
{
    PC_HighEnd_60FPS    UMETA(DisplayName = "PC High-End 60fps"),
    Console_30FPS       UMETA(DisplayName = "Console 30fps"),
    PC_MidRange_30FPS   UMETA(DisplayName = "PC Mid-Range 30fps"),
    Mobile_30FPS        UMETA(DisplayName = "Mobile 30fps")
};

UENUM(BlueprintType)
enum class EPerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Epic        UMETA(DisplayName = "Epic"),
    Custom      UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FPerformanceBudget
{
    GENERATED_BODY()

    // Frame time budgets in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float TargetFrameTime = 16.67f; // 60fps default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float GameThreadBudget = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float RenderThreadBudget = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float GPUBudget = 14.0f;

    // Draw call limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering Budget")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering Budget")
    int32 MaxTriangles = 2000000; // 2M triangles

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering Budget")
    int32 MaxInstances = 10000;

    // Memory budgets in MB
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float TextureMemoryBudget = 2048.0f; // 2GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MeshMemoryBudget = 1024.0f; // 1GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float AudioMemoryBudget = 512.0f; // 512MB

    FPerformanceBudget()
    {
        TargetFrameTime = 16.67f;
        GameThreadBudget = 8.0f;
        RenderThreadBudget = 12.0f;
        GPUBudget = 14.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 2000000;
        MaxInstances = 10000;
        TextureMemoryBudget = 2048.0f;
        MeshMemoryBudget = 1024.0f;
        AudioMemoryBudget = 512.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerformanceMetrics
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
};

/**
 * Central performance management system for Transpersonal Game Studio
 * Ensures 60fps on PC high-end and 30fps on console
 * Monitors and enforces performance budgets across all systems
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
    void SetPerformanceLevel(EPerformanceLevel Level);

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
    EPerformanceTarget CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceLevel CurrentLevel;

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