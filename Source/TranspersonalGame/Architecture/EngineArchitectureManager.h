#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitecture, Log, All);

/**
 * Core engine architecture validation and management system
 * Ensures all game systems follow architectural standards and performance requirements
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceRequirements();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystemModule(const FString& ModuleName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const;

    // System health checks
    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckWorldPartitionHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckLODSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckCullingSystemHealth();

    // Architecture enforcement
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void GenerateArchitectureReport();

protected:
    // Core architecture settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
    int32 MaxActiveActors = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
    float MaxMemoryUsageMB = 8192.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
    bool bEnforceWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
    bool bEnforceLODChain = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
    bool bEnablePerformanceMonitoring = true;

    // Registered system modules
    UPROPERTY(BlueprintReadOnly, Category = "System Modules")
    TMap<FString, int32> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bSystemHealthy = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LastActorCount = 0;

private:
    // Internal validation methods
    bool ValidateWorldPartitionSetup();
    bool ValidateLODConfiguration();
    bool ValidateMemoryPools();
    bool ValidateRenderingPipeline();
    
    // Performance tracking
    void UpdatePerformanceMetrics();
    void LogPerformanceWarnings();
    
    // Architecture enforcement
    void EnforceNamingConventions();
    void EnforceModuleStructure();
    void EnforceComponentLimits();
    
    // Timer handles
    FTimerHandle PerformanceMonitorTimer;
    FTimerHandle ArchitectureValidationTimer;
};