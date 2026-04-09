#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "PerformanceTargets.h"
#include "MemoryProfiler.generated.h"

/**
 * Memory Profiler for the Transpersonal Game Studio
 * Tracks memory usage for massive dinosaur ecosystems
 * Ensures memory stays within performance budgets
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemorySnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 TotalUsedMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 TotalAvailableMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 TextureMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 MeshMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 AudioMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 AIMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int64 PhysicsMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 ActiveAIAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 PhysicsBodies = 0;

    FMemorySnapshot()
    {
        // Default constructor
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemoryCategory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Category")
    FString CategoryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Category")
    int64 CurrentUsage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Category")
    int64 PeakUsage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Category")
    int64 BudgetLimit = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Category")
    float UsagePercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Category")
    bool bExceedsBudget = false;

    FMemoryCategory()
    {
        CategoryName = TEXT("Unknown");
    }

    FMemoryCategory(const FString& InName, int64 InBudget)
        : CategoryName(InName)
        , BudgetLimit(InBudget)
    {
    }

    void UpdateUsage(int64 NewUsage)
    {
        CurrentUsage = NewUsage;
        PeakUsage = FMath::Max(PeakUsage, CurrentUsage);
        
        if (BudgetLimit > 0)
        {
            UsagePercentage = (float)CurrentUsage / (float)BudgetLimit * 100.0f;
            bExceedsBudget = CurrentUsage > BudgetLimit;
        }
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryWarning, const FString&, WarningMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryBudgetExceeded, const FString&, CategoryName);

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMemoryProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UMemoryProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core memory profiling
    UFUNCTION(BlueprintCallable, Category = "Memory Profiling")
    FMemorySnapshot TakeMemorySnapshot();

    UFUNCTION(BlueprintCallable, Category = "Memory Profiling")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Memory Profiling")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Memory Profiling")
    void ClearMemoryHistory();

    // Memory analysis
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    TArray<FMemoryCategory> GetMemoryCategories() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    float GetMemoryUsagePercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    int64 GetTotalMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    int64 GetAvailableMemory() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    bool IsMemoryBudgetExceeded() const;

    // Dinosaur-specific memory tracking
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    int64 GetDinosaurMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    int64 GetAIMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    void TrackDinosaurSpawned(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    void TrackDinosaurDestroyed(AActor* DinosaurActor);

    // Memory optimization
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void TrimMemoryPools();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Memory Reporting")
    FString GenerateMemoryReport() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Reporting")
    void LogMemoryStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Reporting")
    void ExportMemoryData(const FString& FilePath) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Memory Events")
    FOnMemoryWarning OnMemoryWarning;

    UPROPERTY(BlueprintAssignable, Category = "Memory Events")
    FOnMemoryBudgetExceeded OnMemoryBudgetExceeded;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Configuration")
    float ProfilingInterval = 1.0f;  // Sample every second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Configuration")
    int32 MaxHistorySnapshots = 300;  // Keep 5 minutes of data at 1Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Configuration")
    float MemoryWarningThreshold = 0.85f;  // Warn at 85% usage

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Configuration")
    float MemoryCriticalThreshold = 0.95f;  // Critical at 95% usage

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Configuration")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Configuration")
    bool bLogMemoryWarnings = true;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    EPerformanceTarget TargetPlatform = EPerformanceTarget::PC_HighEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    FPerformanceBudget MemoryBudget;

private:
    // Internal state
    bool bIsProfilingActive = false;
    float LastProfilingTime = 0.0f;
    TArray<FMemorySnapshot> MemoryHistory;
    
    // Memory categories
    TMap<FString, FMemoryCategory> MemoryCategories;
    
    // Dinosaur tracking
    TSet<TWeakObjectPtr<AActor>> TrackedDinosaurs;
    int64 EstimatedDinosaurMemory = 0;
    
    // Helper functions
    void InitializeMemoryCategories();
    void UpdateMemoryCategories();
    int64 GetCategoryMemoryUsage(const FString& CategoryName) const;
    void CheckMemoryThresholds(const FMemorySnapshot& Snapshot);
    void TriggerMemoryOptimization();
    
    // Platform-specific memory queries
    int64 GetTextureMemoryUsage() const;
    int64 GetMeshMemoryUsage() const;
    int64 GetAudioMemoryUsageInternal() const;
    int64 GetPhysicsMemoryUsage() const;
    
    // Optimization helpers
    void OptimizeTextureMemory();
    void OptimizeMeshMemory();
    void OptimizeAIMemory();
    void OptimizePhysicsMemory();
};

/**
 * Global Memory Profiler Manager
 * Singleton that manages memory profiling across the entire game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AMemoryProfilerManager : public AActor
{
    GENERATED_BODY()

public:
    AMemoryProfilerManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Memory Manager")
    static AMemoryProfilerManager* GetInstance(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Memory Manager")
    void StartGlobalMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Memory Manager")
    void StopGlobalMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Memory Manager")
    FMemorySnapshot GetCurrentMemorySnapshot() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Manager")
    void SetPerformanceTarget(EPerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Memory Manager")
    void EmergencyMemoryCleanup();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UMemoryProfiler* MemoryProfiler;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoStartProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float EmergencyCleanupInterval = 30.0f;  // Check every 30 seconds

private:
    static TWeakObjectPtr<AMemoryProfilerManager> Instance;
    float LastEmergencyCheckTime = 0.0f;

    UFUNCTION()
    void OnMemoryWarningReceived(const FString& WarningMessage);

    UFUNCTION()
    void OnMemoryBudgetExceededReceived(const FString& CategoryName);
};