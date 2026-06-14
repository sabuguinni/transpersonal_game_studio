#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "SharedTypes.h"
#include "Perf_MemoryManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float PhysicalMemoryUsedMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float VirtualMemoryUsedMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float AvailablePhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float MeshMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float AudioMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float AnimationMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    int32 ActiveComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float MemoryPressureLevel;

    FPerf_MemoryStats()
    {
        PhysicalMemoryUsedMB = 0.0f;
        VirtualMemoryUsedMB = 0.0f;
        AvailablePhysicalMB = 0.0f;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        AudioMemoryMB = 0.0f;
        AnimationMemoryMB = 0.0f;
        ActiveActorCount = 0;
        ActiveComponentCount = 0;
        MemoryPressureLevel = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_MemoryPriority : uint8
{
    Critical = 0    UMETA(DisplayName = "Critical"),
    High = 1        UMETA(DisplayName = "High"),
    Medium = 2      UMETA(DisplayName = "Medium"),
    Low = 3         UMETA(DisplayName = "Low"),
    Disposable = 4  UMETA(DisplayName = "Disposable")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MaxTextureMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MaxMeshMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MaxAudioMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MaxAnimationMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 MaxActiveComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MemoryWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    float MemoryCriticalThreshold;

    FPerf_MemoryBudget()
    {
        MaxTextureMemoryMB = 2048.0f;
        MaxMeshMemoryMB = 1024.0f;
        MaxAudioMemoryMB = 512.0f;
        MaxAnimationMemoryMB = 256.0f;
        MaxActiveActors = 8000;
        MaxActiveComponents = 50000;
        MemoryWarningThreshold = 0.75f;
        MemoryCriticalThreshold = 0.9f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Memory monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    float GetMemoryPressureLevel();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    bool IsMemoryUnderPressure();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    bool IsMemoryCritical();

    // Memory optimization functions
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void TriggerGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeTextureMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeMeshMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeAudioMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ClearUnusedAssets();

    // Actor management functions
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void CullDistantActors(float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void CullActorsByPriority(EPerf_MemoryPriority MinPriority);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeActorComponents();

    // Budget management
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void SetMemoryBudget(const FPerf_MemoryBudget& NewBudget);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    FPerf_MemoryBudget GetMemoryBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    bool IsWithinMemoryBudget();

    // Streaming optimization
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeWorldPartitionStreaming();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void PreloadCriticalAssets();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void UnloadDistantAssets(float UnloadDistance);

    // Monitoring and reporting
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void LogMemoryReport();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    TArray<FString> GetMemoryHotspots();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void StopMemoryProfiling();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    FPerf_MemoryBudget MemoryBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    bool bAutoOptimizeMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    bool bLogMemoryWarnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float GCTriggerThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Memory State")
    FPerf_MemoryStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Memory State")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Memory State")
    bool bIsProfiling;

private:
    float LastMonitorTime;
    float LastGCTime;
    TArray<FPerf_MemoryStats> MemoryHistory;

    void UpdateMemoryStats();
    void CheckMemoryThresholds();
    void AutoOptimizeIfNeeded();
    float CalculateMemoryPressure();
    void LogMemoryWarning(const FString& Warning);
};