#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Eng_PerformanceArchitect.generated.h"

/**
 * Engine Architect - Performance Architecture Manager
 * CRITICAL SYSTEM: Defines and enforces performance standards across all game systems
 * Ensures 60fps PC / 30fps console targets are maintained
 * Manages LOD systems, culling, and optimization strategies
 */

UENUM(BlueprintType)
enum class EEng_PerformanceTarget : uint8
{
    PC_60FPS        UMETA(DisplayName = "PC 60 FPS"),
    Console_30FPS   UMETA(DisplayName = "Console 30 FPS"),
    Mobile_30FPS    UMETA(DisplayName = "Mobile 30 FPS"),
    VR_90FPS        UMETA(DisplayName = "VR 90 FPS")
};

UENUM(BlueprintType)
enum class EEng_PerformanceBudget : uint8
{
    Critical        UMETA(DisplayName = "Critical Systems"),
    High            UMETA(DisplayName = "High Priority"),
    Medium          UMETA(DisplayName = "Medium Priority"),
    Low             UMETA(DisplayName = "Low Priority"),
    Background      UMETA(DisplayName = "Background Tasks")
};

USTRUCT(BlueprintType)
struct FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FEng_PerformanceBudgetAllocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Performance Budget")
    EEng_PerformanceBudget BudgetCategory;

    UPROPERTY(BlueprintReadWrite, Category = "Performance Budget")
    float MaxFrameTimeMS;

    UPROPERTY(BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxDrawCalls;

    UPROPERTY(BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxTriangles;

    UPROPERTY(BlueprintReadWrite, Category = "Performance Budget")
    float MaxMemoryMB;

    FEng_PerformanceBudgetAllocation()
    {
        BudgetCategory = EEng_PerformanceBudget::Medium;
        MaxFrameTimeMS = 16.67f; // 60 FPS target
        MaxDrawCalls = 1000;
        MaxTriangles = 500000;
        MaxMemoryMB = 512.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PerformanceArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PerformanceArchitect();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool IsPerformanceTargetMet(EEng_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    void SetPerformanceTarget(EEng_PerformanceTarget Target);

    // Budget management
    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool AllocatePerformanceBudget(const FString& SystemName, EEng_PerformanceBudget BudgetLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool ValidatePerformanceBudgets();

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    FEng_PerformanceBudgetAllocation GetBudgetAllocation(EEng_PerformanceBudget BudgetLevel);

    // Optimization enforcement
    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool EnforceLODStandards();

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool EnforceCullingStandards();

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool ValidateRenderingBudgets();

    // Performance reporting
    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    FString GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Architecture")
    bool IsSystemWithinBudget(const FString& SystemName);

protected:
    // Performance targets and metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Architecture")
    EEng_PerformanceTarget CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Architecture")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Architecture")
    TMap<EEng_PerformanceBudget, FEng_PerformanceBudgetAllocation> BudgetAllocations;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Architecture")
    TMap<FString, EEng_PerformanceBudget> SystemBudgets;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Architecture")
    bool bPerformanceMonitoringEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Architecture")
    float PerformanceCheckInterval;

    // Internal methods
    void UpdatePerformanceMetrics();
    void InitializeBudgetAllocations();
    bool CheckFrameTimeTarget(float TargetMS);
    void LogPerformanceViolation(const FString& SystemName, const FString& Violation);
    
    // LOD and culling validation
    bool ValidateLODChain(UStaticMeshComponent* MeshComponent);
    bool ValidateCullingDistance(AActor* Actor);
    bool ValidateRenderBudget(const FString& SystemName);
};