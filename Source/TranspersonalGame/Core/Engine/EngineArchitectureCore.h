#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureCore.generated.h"

/**
 * Core Engine Architecture Manager
 * Defines and enforces the fundamental architectural rules for the entire game
 * This is the central authority for all technical decisions and system interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemIntegration(const FString& SystemName, const FString& TargetSystem);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CanSystemCommunicate(const FString& SourceSystem, const FString& TargetSystem);

    // Performance Architecture
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EEng_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceBudget(const FString& SystemName, float CPUBudgetMS, float MemoryBudgetMB);

    // World Partition Rules
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ShouldUseWorldPartition(float WorldSizeKM) const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    int32 GetRecommendedGridSize(float WorldSizeKM) const;

    // Module Dependencies
    UFUNCTION(BlueprintCallable, Category = "Module Architecture")
    TArray<FString> GetModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Architecture")
    bool ValidateModuleDependency(const FString& ModuleName, const FString& DependencyName);

protected:
    // System Registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, EEng_SystemPriority> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TArray<FString>> SystemDependencies;

    // Performance Targets
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EEng_PerformanceTarget CurrentPerformanceTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetMemoryBudgetMB;

    // Architecture Rules
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bEnforceStrictDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bValidatePerformanceBudgets;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float WorldPartitionThresholdKM;

private:
    void InitializeArchitectureRules();
    void SetupPerformanceTargets();
    void RegisterCoreModules();
    bool ValidateSystemPriority(EEng_SystemPriority Priority) const;
};

/**
 * World-specific architecture manager
 * Handles per-world architectural decisions and optimizations
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorldArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorldArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void PostInitialize() override;

    // World-specific architecture
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void OptimizeWorldForTarget(EEng_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SetupLODChain(AActor* Actor, const TArray<float>& LODDistances);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ShouldUseCulling(const FString& ActorType) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, bool> CullingRules;

    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<float> DefaultLODDistances;

private:
    void SetupCullingRules();
    void InitializeLODSystem();
};