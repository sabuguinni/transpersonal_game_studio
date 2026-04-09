#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "../VFXTypes.h"
#include "Engine/World.h"
#include "VFXPoolManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVFXPool, Log, All);

/**
 * VFX Pool Manager
 * 
 * Manages object pooling for Niagara VFX systems to optimize performance.
 * Pools are organized by VFX category and priority for efficient reuse.
 * 
 * Philosophy: Memory is finite, creativity is infinite. Pool wisely.
 */

USTRUCT(BlueprintType)
struct FVFXPoolEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    class UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    bool bIsInUse = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    float LastUsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    EVFXCategory Category = EVFXCategory::EnvironmentalAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    EVFXPriority Priority = EVFXPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool Entry")
    int32 PoolID = -1;

    FVFXPoolEntry()
    {
        NiagaraComponent = nullptr;
        bIsInUse = false;
        LastUsedTime = 0.0f;
        Category = EVFXCategory::EnvironmentalAmbient;
        Priority = EVFXPriority::Medium;
        CurrentLocation = FVector::ZeroVector;
        PoolID = -1;
    }
};

USTRUCT(BlueprintType)
struct FVFXPool
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystemAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    TArray<FVFXPoolEntry> PoolEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    EVFXCategory Category = EVFXCategory::EnvironmentalAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    EVFXPriority Priority = EVFXPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    int32 MaxPoolSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    int32 InitialPoolSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    bool bCanGrow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    bool bAutoRecycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    float RecycleTimeout = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    FString PoolName;

    FVFXPool()
    {
        Category = EVFXCategory::EnvironmentalAmbient;
        Priority = EVFXPriority::Medium;
        MaxPoolSize = 10;
        InitialPoolSize = 5;
        bCanGrow = true;
        bAutoRecycle = true;
        RecycleTimeout = 30.0f;
        PoolName = TEXT("DefaultPool");
    }
};

USTRUCT(BlueprintType)
struct FVFXPoolManagerSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    int32 MaxTotalVFXInstances = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    int32 MaxPoolsPerCategory = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    float PoolCleanupInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    float UnusedPoolTimeout = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    bool bEnablePoolStatistics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    bool bEnableAutoCleanup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    bool bEnablePoolGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
    bool bPreloadCriticalPools = true;

    FVFXPoolManagerSettings()
    {
        MaxTotalVFXInstances = 200;
        MaxPoolsPerCategory = 5;
        PoolCleanupInterval = 10.0f;
        UnusedPoolTimeout = 60.0f;
        bEnablePoolStatistics = true;
        bEnableAutoCleanup = true;
        bEnablePoolGrowth = true;
        bPreloadCriticalPools = true;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXPoolManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXPoolManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Pool Management
    UFUNCTION(BlueprintCallable, Category = "VFX Pool Management")
    void InitializeVFXPoolManager(const FVFXPoolManagerSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Management")
    bool CreateVFXPool(const FVFXPoolData& PoolData);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Management")
    bool RemoveVFXPool(EVFXCategory Category, const FString& PoolName);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Management")
    void PreloadCriticalPools();

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Management")
    void CleanupUnusedPools();

    // VFX Instance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Instance Management")
    UNiagaraComponent* GetVFXFromPool(EVFXCategory Category, EVFXPriority Priority, const FString& PoolName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "VFX Instance Management")
    bool ReturnVFXToPool(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX Instance Management")
    UNiagaraComponent* SpawnVFXFromPool(const FVFXEffectData& EffectData, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX Instance Management")
    bool RecycleVFXInstance(UNiagaraComponent* VFXComponent);

    // Pool Queries
    UFUNCTION(BlueprintCallable, Category = "VFX Pool Queries")
    int32 GetPoolSize(EVFXCategory Category, const FString& PoolName);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Queries")
    int32 GetAvailableVFXCount(EVFXCategory Category, const FString& PoolName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Queries")
    int32 GetActiveVFXCount(EVFXCategory Category, const FString& PoolName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Queries")
    TArray<FString> GetPoolNames(EVFXCategory Category);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Queries")
    bool IsPoolAvailable(EVFXCategory Category, const FString& PoolName);

    // Pool Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX Pool Configuration")
    void SetPoolMaxSize(EVFXCategory Category, const FString& PoolName, int32 MaxSize);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Configuration")
    void SetPoolGrowthEnabled(EVFXCategory Category, const FString& PoolName, bool bCanGrow);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Configuration")
    void SetPoolAutoRecycle(EVFXCategory Category, const FString& PoolName, bool bAutoRecycle);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Configuration")
    void SetGlobalPoolSettings(const FVFXPoolManagerSettings& Settings);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void ForcePoolCleanup();

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void OptimizePoolSizes();

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetMaxTotalVFXInstances(int32 MaxInstances);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    int32 GetTotalActiveVFXCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    float GetPoolMemoryUsage() const;

    // Priority Management
    UFUNCTION(BlueprintCallable, Category = "VFX Priority")
    void SetVFXPriority(UNiagaraComponent* VFXComponent, EVFXPriority NewPriority);

    UFUNCTION(BlueprintCallable, Category = "VFX Priority")
    void CullLowPriorityVFX(int32 MaxToKeep);

    UFUNCTION(BlueprintCallable, Category = "VFX Priority")
    void ForceRecycleLowPriorityVFX();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "VFX LOD")
    void UpdateVFXLOD(UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX LOD")
    void UpdateAllVFXLOD(const FVFXLODSettings& LODSettings);

    UFUNCTION(BlueprintCallable, Category = "VFX LOD")
    EVFXLODLevel CalculateVFXLOD(FVector VFXLocation, FVector ViewerLocation) const;

    // Debug and Statistics
    UFUNCTION(BlueprintCallable, Category = "VFX Pool Debug")
    void PrintPoolStatistics();

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Debug")
    void DebugDrawPoolInfo(bool bDrawPoolLocations = true, bool bDrawPoolStats = true);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Debug")
    void DumpPoolConfiguration();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Pool Utility")
    void WarmUpPool(EVFXCategory Category, const FString& PoolName, int32 WarmUpCount);

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Utility")
    bool ValidatePoolIntegrity();

    UFUNCTION(BlueprintCallable, Category = "VFX Pool Utility")
    void ResetAllPools();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool Manager Settings")
    FVFXPoolManagerSettings PoolSettings;

    // Pool Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pool Storage")
    TMap<EVFXCategory, TArray<FVFXPool>> VFXPools;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pool Storage")
    TMap<UNiagaraComponent*, FVFXPoolEntry> ActiveVFXMap;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalActiveVFXCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalPooledVFXCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastCleanupTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastOptimizationTime = 0.0f;

    // System State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System State")
    bool bIsInitialized = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System State")
    bool bIsCleanupInProgress = false;

private:
    // Internal Pool Management
    FVFXPool* FindPool(EVFXCategory Category, const FString& PoolName);
    FVFXPool* FindBestPool(EVFXCategory Category, EVFXPriority Priority);
    
    bool CreatePoolEntry(FVFXPool& Pool, UNiagaraSystem* NiagaraSystem);
    void DestroyPoolEntry(FVFXPoolEntry& Entry);
    
    UNiagaraComponent* GetAvailableVFXFromPool(FVFXPool& Pool);
    bool ReturnVFXToSpecificPool(UNiagaraComponent* VFXComponent, FVFXPool& Pool);
    
    // Pool Maintenance
    void UpdatePoolMaintenance(float DeltaTime);
    void CleanupExpiredVFX();
    void OptimizePoolMemory();
    
    // Performance Monitoring
    void UpdatePerformanceMetrics();
    bool ShouldCullVFX() const;
    
    // Utility
    FString GeneratePoolKey(EVFXCategory Category, const FString& PoolName) const;
    int32 GetNextPoolID();
    
    // Static counters
    static int32 NextPoolEntryID;
};