#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

// Forward declarations
class UPerformanceProfiler;
class USystemValidationManager;
class UModuleDependencyTracker;

/**
 * Core Engine Architecture Manager
 * Establishes and enforces the technical foundation for the entire project
 * Manages system dependencies, performance constraints, and architectural rules
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

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforcePerformanceConstraints();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void GenerateArchitectureReport();

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetCoreSystem(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EEng_PerformanceTarget Target);

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FEng_ValidationResult> ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemIntegration(const FString& SystemA, const FString& SystemB);

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Modules")
    TArray<FString> GetLoadedModules();

    UFUNCTION(BlueprintCallable, Category = "Modules")
    bool IsModuleLoaded(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Modules")
    void RefreshModuleDependencies();

protected:
    // Core subsystem references
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TObjectPtr<UPerformanceProfiler> PerformanceProfiler;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TObjectPtr<USystemValidationManager> ValidationManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TObjectPtr<UModuleDependencyTracker> DependencyTracker;

    // Registered core systems
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    // Architecture configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EEng_PerformanceTarget CurrentPerformanceTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnforceStrictValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoValidateOnSystemChange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationInterval;

    // Performance constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryUsageMB;

    // System state tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bArchitectureInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bValidationPassed;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastValidationTime;

private:
    // Internal validation methods
    bool ValidateSystemDependencies();
    bool ValidatePerformanceTargets();
    bool ValidateMemoryConstraints();
    
    // Internal management
    void InitializeSubsystems();
    void CleanupSubsystems();
    void UpdatePerformanceMetrics();
    
    // Validation timer
    FTimerHandle ValidationTimerHandle;
    void PerformScheduledValidation();
};