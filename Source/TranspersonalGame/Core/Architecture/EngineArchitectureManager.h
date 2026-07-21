#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

/**
 * Engine Architecture Manager - Central authority for all architectural decisions
 * Defines and enforces the technical architecture across all game systems
 * Ensures consistency, performance, and maintainability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceCompliance();

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetCoreSystem(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceBudget(const FString& SystemName, float BudgetMS);

    // Architecture Standards Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateThreadSafety();

    // World Partition Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool SetupWorldPartition(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateWorldPartitionSetup(UWorld* World);

    // LOD System Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ConfigureLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateLODConfiguration();

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void RunArchitecturalDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void GenerateSystemReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogArchitectureStatus();

protected:
    // Core System Registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    // Performance Budgets (in milliseconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TMap<FString, float> SystemPerformanceBudgets;

    // Current Performance Metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<FString, float> CurrentPerformanceMetrics;

    // Architecture Validation Results
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bArchitectureValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bDependenciesValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bPerformanceCompliant;

    // World Partition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bUseWorldPartition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float WorldPartitionCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float WorldPartitionLoadingRange;

    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODDistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel;

private:
    // Internal validation methods
    bool ValidateSystemIntegrity();
    bool ValidateMemoryFootprint();
    bool ValidateRenderingPipeline();
    bool ValidatePhysicsIntegration();
    
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceBudgets();
    
    // System lifecycle
    void InitializeCoreSystems();
    void ShutdownCoreSystems();
    
    // Architecture enforcement
    void EnforceArchitecturalStandards();
    void ValidateCodeCompliance();
};