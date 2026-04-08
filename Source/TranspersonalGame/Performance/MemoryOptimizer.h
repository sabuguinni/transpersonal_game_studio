#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "MemoryOptimizer.generated.h"

USTRUCT(BlueprintType)
struct FMemoryUsageReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float TotalPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float UsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float AvailablePhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float GameMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    float TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float MeshMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float AudioMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    float ConsciousnessSystemMemoryMB;

    UPROPERTY(BlueprintReadOnly)
    int32 LoadedActorCount;

    UPROPERTY(BlueprintReadOnly)
    int32 LoadedComponentCount;

    FMemoryUsageReport()
    {
        TotalPhysicalMemoryMB = 0.0f;
        UsedPhysicalMemoryMB = 0.0f;
        AvailablePhysicalMemoryMB = 0.0f;
        GameMemoryUsageMB = 0.0f;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        AudioMemoryMB = 0.0f;
        ConsciousnessSystemMemoryMB = 0.0f;
        LoadedActorCount = 0;
        LoadedComponentCount = 0;
    }
};

UENUM(BlueprintType)
enum class EMemoryOptimizationStrategy : uint8
{
    Conservative    UMETA(DisplayName = "Conservative - Minimal Impact"),
    Balanced        UMETA(DisplayName = "Balanced - Moderate Optimization"),
    Aggressive      UMETA(DisplayName = "Aggressive - Maximum Memory Savings"),
    Emergency       UMETA(DisplayName = "Emergency - Critical Memory Recovery")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryWarning, float, MemoryUsagePercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryOptimized, float, MemoryFreedMB);

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMemoryOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UMemoryOptimizer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory Monitoring
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    FMemoryUsageReport GetCurrentMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    float GetMemoryUsagePercent() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    bool IsMemoryUsageCritical() const;

    // Optimization Control
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void SetOptimizationStrategy(EMemoryOptimizationStrategy Strategy);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void ForceMemoryOptimization();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void SetMemoryThresholds(float WarningPercent, float CriticalPercent);

    // Consciousness-Specific Optimization
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizeConsciousnessMemory(float PlayerConsciousnessRadius);

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void UnloadDistantConsciousnessData();

    // Asset Management
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizeTextureMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizeMeshMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizeAudioMemory();

    // Garbage Collection
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void TriggerGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void ScheduleIncrementalGC();

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnMemoryWarning OnMemoryWarning;

    UPROPERTY(BlueprintAssignable)
    FOnMemoryOptimized OnMemoryOptimized;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory Thresholds")
    float MemoryWarningThresholdPercent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory Thresholds")
    float MemoryCriticalThresholdPercent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory Thresholds")
    float ConsciousnessMemoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings")
    EMemoryOptimizationStrategy CurrentStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings")
    bool bAutoOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings")
    float OptimizationCheckInterval;

    // Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Memory Tracking")
    FMemoryUsageReport LastMemoryReport;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Tracking")
    bool bMemoryWarningActive;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Tracking")
    float LastOptimizationTime;

private:
    float LastMemoryCheck;
    float LastGCTime;
    TArray<FMemoryUsageReport> MemoryHistory;
    
    // Optimization methods
    void UpdateMemoryUsage();
    void CheckMemoryThresholds();
    void ApplyOptimizationStrategy();
    
    // Specific optimization functions
    void OptimizeConservative();
    void OptimizeBalanced();
    void OptimizeAggressive();
    void OptimizeEmergency();
    
    // Asset optimization
    void UnloadUnusedTextures();
    void ReduceTextureLOD();
    void UnloadUnusedMeshes();
    void UnloadUnusedAudio();
    void OptimizeParticleSystems();
    
    // Consciousness system optimization
    void OptimizeConsciousnessEntities(float Radius);
    void UnloadDistantConsciousnessStates();
    void ReduceConsciousnessComplexity();
    
    // Actor and component management
    void UnloadDistantActors(float Distance);
    void OptimizeComponentMemory();
    void CacheImportantObjects();
    
    // Memory tracking utilities
    float CalculateGameMemoryUsage() const;
    float CalculateTextureMemoryUsage() const;
    float CalculateMeshMemoryUsage() const;
    float CalculateAudioMemoryUsage() const;
    float CalculateConsciousnessMemoryUsage() const;
    
    // Logging and reporting
    void LogMemoryOptimization(const FString& OptimizationType, float MemoryFreed);
};