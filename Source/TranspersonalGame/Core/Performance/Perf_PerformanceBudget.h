#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Perf_PerformanceBudget.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BudgetCategory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    FString CategoryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float CurrentFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxActiveObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    int32 CurrentActiveObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    bool bIsOverBudget;

    FPerf_BudgetCategory()
    {
        CategoryName = TEXT("Unknown");
        MaxFrameTimeMS = 16.67f; // 60fps default
        CurrentFrameTimeMS = 0.0f;
        MaxActiveObjects = 100;
        CurrentActiveObjects = 0;
        bIsOverBudget = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float TotalFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float PhysicsTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float RenderingTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float AITimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float AudioTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    int32 PhysicsActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float MemoryUsageMB;

    FPerf_SystemMetrics()
    {
        TotalFrameTimeMS = 0.0f;
        PhysicsTimeMS = 0.0f;
        RenderingTimeMS = 0.0f;
        AITimeMS = 0.0f;
        AudioTimeMS = 0.0f;
        TotalActorCount = 0;
        PhysicsActorCount = 0;
        DinosaurCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceBudget : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Budget Management
    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    void InitializeBudgets();

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    void UpdateBudgetCategory(const FString& CategoryName, float FrameTimeMS, int32 ActiveObjects);

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    bool IsCategoryOverBudget(const FString& CategoryName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    FPerf_BudgetCategory GetBudgetCategory(const FString& CategoryName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    TArray<FString> GetOverBudgetCategories() const;

    // System Metrics
    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void UpdateSystemMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    FPerf_SystemMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    float GetTargetFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void SetTargetFrameRate(float NewFrameRate);

    // Performance Enforcement
    UFUNCTION(BlueprintCallable, Category = "Performance Enforcement")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance Enforcement")
    void CullDistantActors(float CullDistance = 10000.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance Enforcement")
    void OptimizePhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Performance Enforcement")
    void ReduceDinosaurCount(int32 MaxDinosaurs = 150);

    // Adaptive Quality
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void AdjustQualityBasedOnPerformance();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetLODDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetShadowQuality(int32 Quality);

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Debug")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Debug")
    void EnablePerformanceHUD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance Debug", CallInEditor = true)
    void TestPerformanceBudgets();

private:
    UPROPERTY()
    TMap<FString, FPerf_BudgetCategory> BudgetCategories;

    UPROPERTY()
    FPerf_SystemMetrics CurrentMetrics;

    UPROPERTY()
    float TargetFrameRate;

    UPROPERTY()
    bool bPerformanceHUDEnabled;

    UPROPERTY()
    float LastMetricsUpdateTime;

    // Internal helper functions
    void InitializeDefaultBudgets();
    void UpdatePhysicsMetrics();
    void UpdateRenderingMetrics();
    void UpdateAIMetrics();
    void UpdateMemoryMetrics();
    int32 CountActorsOfType(UClass* ActorClass) const;
    void DestroyExcessActors(const TArray<AActor*>& Actors, int32 MaxCount);
};