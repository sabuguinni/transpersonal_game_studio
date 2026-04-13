#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/SharedTypes.h"
#include "EngineArchitecture.generated.h"

// Forward declarations
class UEng_SystemManager;
class UEng_PerformanceMonitor;

/**
 * Main Architecture System - The brain of the engine architecture
 * Manages all subsystems, enforces architectural rules, and monitors performance
 * This is the central authority that all other systems report to
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core architecture functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_SystemState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_SystemInfo> GetAllSystemsInfo() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceTier GetCurrentPerformanceTier() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTier(EEng_PerformanceTier NewTier);

    // World configuration
    UFUNCTION(BlueprintCallable, Category = "World")
    void ConfigureWorld(const FEng_WorldConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "World")
    FEng_WorldConfiguration GetWorldConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "World")
    bool ShouldUseWorldPartition() const;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    TArray<FString> GetArchitectureWarnings() const;

    // System dependencies
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanSystemStart(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddSystemDependency(const FString& SystemName, const FString& DependsOn);

protected:
    // Core system references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UEng_SystemManager> SystemManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UEng_PerformanceMonitor> PerformanceMonitor;

    // Architecture configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_WorldConfiguration WorldConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EEng_PerformanceTier TargetPerformanceTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableArchitectureValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float SystemUpdateFrequency;

private:
    // System registry
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // System dependencies
    UPROPERTY()
    TMap<FString, TArray<FString>> SystemDependencies;

    // Architecture state
    bool bIsInitialized;
    float LastValidationTime;
    TArray<FString> CachedWarnings;

    // Internal functions
    void UpdateSystemStates();
    void ValidateSystemDependencies();
    void CheckPerformanceThresholds();
    void LogArchitectureState() const;
};

/**
 * System Manager - Handles registration and lifecycle of all game systems
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemManager : public UObject
{
    GENERATED_BODY()

public:
    UEng_SystemManager();

    // System lifecycle
    UFUNCTION(BlueprintCallable, Category = "System Manager")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Manager")
    bool ShutdownSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Manager")
    void UpdateSystems(float DeltaTime);

    // System queries
    UFUNCTION(BlueprintCallable, Category = "System Manager")
    bool IsSystemActive(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Manager")
    TArray<FString> GetActiveSystemNames() const;

    UFUNCTION(BlueprintCallable, Category = "System Manager")
    int32 GetSystemCount() const;

protected:
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> ManagedSystems;

    UPROPERTY()
    TArray<FString> InitializationOrder;

private:
    void SortSystemsByPriority();
    bool ValidateSystemInitialization(const FString& SystemName) const;
};

/**
 * Performance Monitor - Tracks and reports system performance
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UObject
{
    GENERATED_BODY()

public:
    UEng_PerformanceMonitor();

    // Performance tracking
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetLatestMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceTier RecommendPerformanceTier() const;

    // Performance history
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<float> GetFrameTimeHistory(int32 SampleCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

protected:
    UPROPERTY()
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    EEng_PerformanceTier CurrentTier;

private:
    static constexpr int32 MAX_HISTORY_SAMPLES = 300; // 5 seconds at 60 FPS
    
    void CollectFrameTimeMetrics();
    void CollectMemoryMetrics();
    void CollectRenderMetrics();
    EEng_PerformanceTier CalculateOptimalTier() const;
};