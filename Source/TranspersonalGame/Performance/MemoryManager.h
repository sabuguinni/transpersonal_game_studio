/**
 * @file MemoryManager.h
 * @brief Advanced memory management for consciousness-based game systems
 * 
 * Handles memory pooling, streaming, and optimization for spiritual experiences
 * that may involve large amounts of consciousness field data.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Templates/SharedPointer.h"
#include "MemoryManager.generated.h"

USTRUCT(BlueprintType)
struct FMemoryPoolConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PoolName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BlockSize = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InitialBlocks = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxBlocks = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoGrow = true;
};

USTRUCT(BlueprintType)
struct FMemoryUsageStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float TotalAllocatedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float ConsciousnessDataMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float SpiritualFieldsMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float AudioMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TextureStreamingMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveMemoryPools = 0;

    UPROPERTY(BlueprintReadOnly)
    float MemoryFragmentation = 0.0f;
};

/**
 * Memory pool for efficient allocation of consciousness-related data
 */
class TRANSPERSONALGAME_API FConsciousnessMemoryPool
{
public:
    FConsciousnessMemoryPool(const FMemoryPoolConfig& Config);
    ~FConsciousnessMemoryPool();

    void* Allocate(size_t Size);
    void Deallocate(void* Ptr);
    void Reset();
    
    int32 GetUsedBlocks() const { return UsedBlocks; }
    int32 GetTotalBlocks() const { return TotalBlocks; }
    float GetFragmentation() const;

private:
    FMemoryPoolConfig PoolConfig;
    TArray<void*> FreeBlocks;
    TArray<void*> UsedBlockList;
    uint8* MemoryBase = nullptr;
    int32 TotalBlocks = 0;
    int32 UsedBlocks = 0;
    
    void GrowPool();
};

UCLASS()
class TRANSPERSONALGAME_API UMemoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Memory pool management
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    bool CreateMemoryPool(const FMemoryPoolConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void DestroyMemoryPool(const FString& PoolName);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ResetMemoryPool(const FString& PoolName);

    // Consciousness-specific memory allocation
    UFUNCTION(BlueprintCallable, Category = "Consciousness Memory")
    void* AllocateConsciousnessData(int32 Size);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Memory")
    void DeallocateConsciousnessData(void* Ptr);

    // Spiritual field memory management
    UFUNCTION(BlueprintCallable, Category = "Spiritual Fields")
    void PreallocateSpiritualFieldMemory(int32 SizeMB);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Fields")
    void OptimizeSpiritualFieldMemory();

    // Memory monitoring and stats
    UFUNCTION(BlueprintCallable, Category = "Memory Stats")
    FMemoryUsageStats GetMemoryUsageStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Stats")
    void LogDetailedMemoryStats() const;

    // Garbage collection optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetGCFrequency(float Seconds);

    // Streaming and loading optimization
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void PreloadConsciousnessAssets(const TArray<FString>& AssetPaths);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadUnusedConsciousnessAssets();

    // Memory pressure handling
    UFUNCTION(BlueprintCallable, Category = "Memory Pressure")
    void HandleMemoryPressure();

    UFUNCTION(BlueprintCallable, Category = "Memory Pressure")
    bool IsMemoryPressureHigh() const;

protected:
    // Memory pools
    UPROPERTY()
    TMap<FString, TSharedPtr<FConsciousnessMemoryPool>> MemoryPools;

    // Configuration
    UPROPERTY(EditAnywhere, Category = "Configuration")
    float GCFrequency = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    float MemoryPressureThreshold = 0.85f; // 85% of available memory

    UPROPERTY(EditAnywhere, Category = "Configuration")
    int32 MaxConsciousnessDataMB = 512;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    int32 MaxSpiritualFieldsMB = 256;

private:
    FTimerHandle GCTimerHandle;
    mutable FMemoryUsageStats CachedStats;
    mutable float LastStatsUpdate = 0.0f;

    void PerformScheduledGC();
    void UpdateMemoryStats() const;
    void InitializeDefaultPools();
};