#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Perf_MemoryManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float VirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MeshMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AudioMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveComponentCount;

    FPerf_MemoryStats()
    {
        PhysicalMemoryMB = 0.0f;
        VirtualMemoryMB = 0.0f;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        AudioMemoryMB = 0.0f;
        ActiveActorCount = 0;
        ActiveComponentCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Memory monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMemoryProfilingActive() const { return bIsProfilingActive; }

    // Memory optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextureMemory();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMeshMemory();

    // Memory limits
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMemoryBudget(float PhysicalBudgetMB, float TextureBudgetMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinMemoryBudget();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicalMemoryBudgetMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryBudgetMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FPerf_MemoryStats LastMemoryStats;

    // Profiling timer
    FTimerHandle ProfilingTimerHandle;

    void UpdateMemoryStats();
    void CheckMemoryBudget();
    float GetPhysicalMemoryUsageMB();
    float GetTextureMemoryUsageMB();
    float GetMeshMemoryUsageMB();
    float GetAudioMemoryUsageMB();
};