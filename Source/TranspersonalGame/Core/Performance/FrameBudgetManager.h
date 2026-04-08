// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "HAL/PlatformFilemanager.h"
#include "FrameBudgetManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFrameBudget, Log, All);

/**
 * @brief Frame Budget Manager for Transpersonal Game Studio
 * 
 * Enforces strict performance targets:
 * - PC: 60fps (16.67ms frame budget)
 * - Console: 30fps (33.33ms frame budget)
 * 
 * Budget Allocation:
 * - Physics: 18% of frame time (3ms PC / 6ms Console)
 * - Rendering: 48% of frame time (8ms PC / 16ms Console)
 * - Game Logic: 24% of frame time (4ms PC / 8ms Console)
 * - Overhead: 10% of frame time (1.67ms PC / 3.33ms Console)
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS()
class TRANSPERSONALGAME_API UFrameBudgetManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the frame budget manager instance */
    UFUNCTION(BlueprintPure, Category = "Frame Budget")
    static UFrameBudgetManager* Get(const UObject* WorldContext);

    /** Initialize frame budget system */
    UFUNCTION(BlueprintCallable, Category = "Frame Budget")
    void InitializeFrameBudget();

    /** Set performance target (PC 60fps or Console 30fps) */
    UFUNCTION(BlueprintCallable, Category = "Frame Budget")
    void SetPerformanceTarget(EPerformanceTarget Target);

    /** Get current frame budget allocation */
    UFUNCTION(BlueprintPure, Category = "Frame Budget")
    FFrameBudget GetCurrentFrameBudget() const;

    /** Check if system is within budget */
    UFUNCTION(BlueprintPure, Category = "Frame Budget")
    bool IsWithinBudget(EBudgetCategory Category) const;

    /** Get budget usage percentage for category */
    UFUNCTION(BlueprintPure, Category = "Frame Budget")
    float GetBudgetUsage(EBudgetCategory Category) const;

    /** Force budget enforcement (emergency scaling) */
    UFUNCTION(BlueprintCallable, Category = "Frame Budget")
    void EnforceBudget();

    /** Enable/disable budget monitoring */
    UFUNCTION(BlueprintCallable, Category = "Frame Budget")
    void SetBudgetMonitoring(bool bEnabled);

    /** Get performance bottleneck analysis */
    UFUNCTION(BlueprintCallable, Category = "Frame Budget")
    FBudgetAnalysis AnalyzeBudgetUsage();

protected:
    /** Performance targets */
    UENUM(BlueprintType)
    enum class EPerformanceTarget : uint8
    {
        Console_30fps   UMETA(DisplayName = "Console 30fps"),
        PC_60fps        UMETA(DisplayName = "PC 60fps"),
        PC_120fps       UMETA(DisplayName = "PC 120fps (Future)")
    };

    /** Budget categories */
    UENUM(BlueprintType)
    enum class EBudgetCategory : uint8
    {
        Physics         UMETA(DisplayName = "Physics Simulation"),
        Rendering       UMETA(DisplayName = "Rendering Pipeline"),
        GameLogic       UMETA(DisplayName = "Game Logic"),
        Audio           UMETA(DisplayName = "Audio Processing"),
        AI              UMETA(DisplayName = "AI Systems"),
        Animation       UMETA(DisplayName = "Animation"),
        Networking      UMETA(DisplayName = "Networking"),
        Overhead        UMETA(DisplayName = "Engine Overhead")
    };

    /** Frame budget structure */
    USTRUCT(BlueprintType)
    struct FFrameBudget
    {
        GENERATED_BODY()

        /** Total frame time budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float TotalFrameTimeMS = 16.67f;

        /** Physics budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float PhysicsBudgetMS = 3.0f;

        /** Rendering budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float RenderingBudgetMS = 8.0f;

        /** Game logic budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float GameLogicBudgetMS = 4.0f;

        /** Audio budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float AudioBudgetMS = 1.0f;

        /** AI budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float AIBudgetMS = 1.5f;

        /** Animation budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float AnimationBudgetMS = 1.0f;

        /** Networking budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float NetworkingBudgetMS = 0.5f;

        /** Engine overhead budget in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float OverheadBudgetMS = 1.67f;
    };

    /** Budget analysis structure */
    USTRUCT(BlueprintType)
    struct FBudgetAnalysis
    {
        GENERATED_BODY()

        /** Categories over budget */
        UPROPERTY(BlueprintReadOnly)
        TArray<EBudgetCategory> CategoriesOverBudget;

        /** Worst offender category */
        UPROPERTY(BlueprintReadOnly)
        EBudgetCategory WorstOffender = EBudgetCategory::Physics;

        /** Budget overage percentage */
        UPROPERTY(BlueprintReadOnly)
        float BudgetOveragePercent = 0.0f;

        /** Recommended actions */
        UPROPERTY(BlueprintReadOnly)
        TArray<FString> RecommendedActions;

        /** Emergency scaling required */
        UPROPERTY(BlueprintReadOnly)
        bool bEmergencyScalingRequired = false;
    };

    /** Current performance target */
    UPROPERTY(BlueprintReadOnly, Category = "Frame Budget")
    EPerformanceTarget CurrentTarget = EPerformanceTarget::PC_60fps;

    /** Current frame budget */
    UPROPERTY(BlueprintReadOnly, Category = "Frame Budget")
    FFrameBudget CurrentBudget;

    /** Budget monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frame Budget")
    bool bBudgetMonitoringEnabled = true;

    /** Emergency scaling threshold (percentage over budget) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frame Budget", meta = (ClampMin = "10.0", ClampMax = "50.0"))
    float EmergencyScalingThreshold = 20.0f;

    /** Budget enforcement aggressiveness (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frame Budget", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BudgetEnforcementStrength = 0.8f;

private:
    /** Initialize platform-specific budgets */
    void InitializePlatformBudgets();
    
    /** Update budget monitoring */
    void UpdateBudgetMonitoring();
    
    /** Calculate budget usage for category */
    float CalculateCategoryUsage(EBudgetCategory Category) const;
    
    /** Apply emergency scaling */
    void ApplyEmergencyScaling();
    
    /** Get current platform performance target */
    EPerformanceTarget DetectPlatformTarget() const;
    
    /** Log budget violations */
    void LogBudgetViolations(const FBudgetAnalysis& Analysis);

    /** Budget monitoring timer */
    FTimerHandle BudgetMonitoringTimer;
    
    /** Performance tracking */
    TMap<EBudgetCategory, TArray<float>> BudgetUsageHistory;
    
    /** Last frame times for each category */
    TMap<EBudgetCategory, float> LastCategoryTimes;
};

/**
 * @brief Budget Enforcer Component
 * 
 * Attaches to critical systems to enforce budget limits
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBudgetEnforcerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBudgetEnforcerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Set the budget category this component monitors */
    UFUNCTION(BlueprintCallable, Category = "Budget Enforcement")
    void SetBudgetCategory(UFrameBudgetManager::EBudgetCategory Category);

    /** Check if this component is within budget */
    UFUNCTION(BlueprintPure, Category = "Budget Enforcement")
    bool IsWithinBudget() const;

    /** Get current budget usage percentage */
    UFUNCTION(BlueprintPure, Category = "Budget Enforcement")
    float GetBudgetUsagePercent() const;

    /** Force budget compliance (reduce quality/complexity) */
    UFUNCTION(BlueprintCallable, Category = "Budget Enforcement")
    void ForceBudgetCompliance();

protected:
    /** Budget category to monitor */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget Enforcement")
    UFrameBudgetManager::EBudgetCategory MonitoredCategory = UFrameBudgetManager::EBudgetCategory::Physics;

    /** Auto-enforce budget when exceeded */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget Enforcement")
    bool bAutoEnforceBudget = true;

    /** Budget violation threshold before enforcement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget Enforcement", meta = (ClampMin = "5.0", ClampMax = "50.0"))
    float ViolationThreshold = 15.0f;

    /** Quality reduction steps when over budget */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget Enforcement")
    TArray<float> QualityReductionSteps = {0.9f, 0.75f, 0.5f, 0.25f};

private:
    /** Current quality level index */
    int32 CurrentQualityIndex = 0;
    
    /** Frame budget manager reference */
    UPROPERTY()
    TObjectPtr<UFrameBudgetManager> FrameBudgetManager;
    
    /** Track timing for this component */
    double LastTickStartTime = 0.0;
    double LastTickEndTime = 0.0;
    
    /** Apply quality reduction */
    void ApplyQualityReduction(int32 QualityIndex);
    
    /** Restore quality when budget allows */
    void RestoreQuality();
    
    /** Register with budget manager */
    void RegisterWithBudgetManager();
    
    /** Unregister from budget manager */
    void UnregisterFromBudgetManager();
};