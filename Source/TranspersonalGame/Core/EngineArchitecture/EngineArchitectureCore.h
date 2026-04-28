#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureCore.generated.h"

/**
 * Core Engine Architecture Manager
 * Defines and enforces architectural rules across all game systems
 * Manages module dependencies, performance constraints, and system integration
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

    // Architecture validation and enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforcePerformanceConstraints();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystemModule(const FString& ModuleName);

    // Module dependency management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetLoadedModules() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetMemoryUsageMB() const;

    // System health checks
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void RunArchitecturalHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemHealthy() const;

protected:
    // Registered system modules
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, EEng_SystemPriority> RegisteredModules;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsageMB;

    // Architecture state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValid;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bPerformanceWithinLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FDateTime LastHealthCheckTime;

private:
    // Internal validation methods
    bool ValidateWorldPartitionSetup();
    bool ValidateLODConfiguration();
    bool ValidateMemoryPools();
    void LogArchitectureStatus();
};

/**
 * World-specific architecture manager
 * Handles per-world architectural concerns like streaming, LOD, and culling
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineWorldArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngineWorldArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World architecture management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ConfigureWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SetupLODDistances();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ConfigureCullingSettings();

    // Streaming management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void EnableStreamingForLargeWorlds();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldStreamingActive() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "World Architecture")
    bool bWorldPartitionEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "World Architecture")
    bool bStreamingConfigured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    TArray<float> LODDistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    float CullingDistance;
};