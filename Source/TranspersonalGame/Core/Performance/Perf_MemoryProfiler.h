#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Perf_MemoryProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakUsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailablePhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MeshMemoryMB;

    FPerf_MemoryStats()
    {
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        PeakUsedPhysicalMemoryMB = 0.0f;
        AvailablePhysicalMemoryMB = 0.0f;
        ActiveActorCount = 0;
        ActiveComponentCount = 0;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_MemoryWarningLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_MemoryProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPerf_MemoryProfiler();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    EPerf_MemoryWarningLevel GetMemoryWarningLevel();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    TArray<FString> GetMemoryHogs();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|Memory")
    void ProfileCurrentLevel();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FPerf_MemoryStats LastRecordedStats;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryWarningThresholdMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryCriticalThresholdMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FPerf_MemoryStats> MemoryHistory;

private:
    void UpdateMemoryStats();
    void CheckMemoryThresholds();
    void LogMemoryWarning(EPerf_MemoryWarningLevel WarningLevel);
    void CleanupUnusedAssets();
    void OptimizeTextureMemory();
    void OptimizeMeshMemory();
    void AnalyzeActorMemoryUsage();
    float GetTextureMemoryUsage();
    float GetMeshMemoryUsage();
    int32 CountActiveActors();
    int32 CountActiveComponents();
};