#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArchitectureRuleViolation, FString, SystemName, FString, ViolationDetails);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPerformanceThresholdExceeded, FString, SystemName, float, CurrentValue, float, ThresholdValue);

/**
 * Core Engine Architecture Manager
 * Enforces architectural rules and validates system interactions
 * This is the technical backbone that ensures all other systems follow the established patterns
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

    // Core Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    bool ValidateSystemArchitecture(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void RegisterSystem(const FString& SystemName, EEng_SystemType SystemType, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void UnregisterSystem(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void SetPerformanceThreshold(const FString& SystemName, float ThresholdMS);

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    float GetSystemPerformance(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void LogPerformanceMetrics();

    // Module Dependency Tracking
    UFUNCTION(BlueprintCallable, Category = "Dependencies", CallInEditor)
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Dependencies", CallInEditor)
    void AddModuleDependency(const FString& ModuleName, const FString& DependsOn);

    // Rule Engine
    UFUNCTION(BlueprintCallable, Category = "Rules", CallInEditor)
    void AddArchitectureRule(const FString& RuleName, const FString& RuleDescription);

    UFUNCTION(BlueprintCallable, Category = "Rules", CallInEditor)
    bool CheckRule(const FString& RuleName, const FString& SystemName);

    // System Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Health", CallInEditor)
    EEng_SystemHealth GetSystemHealth(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Health", CallInEditor)
    void UpdateSystemHealth(const FString& SystemName, EEng_SystemHealth NewHealth);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnArchitectureRuleViolation OnArchitectureRuleViolation;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableRealTimeValidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationIntervalSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bLogViolationsToFile = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxConcurrentSystems = 50;

protected:
    // Internal system tracking
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY()
    TMap<FString, float> PerformanceThresholds;

    UPROPERTY()
    TMap<FString, float> CurrentPerformanceMetrics;

    UPROPERTY()
    TMap<FString, TArray<FString>> ModuleDependencies;

    UPROPERTY()
    TMap<FString, FString> ArchitectureRules;

    UPROPERTY()
    TMap<FString, EEng_SystemHealth> SystemHealthStatus;

    // Internal validation methods
    bool ValidateSystemType(const FString& SystemName, EEng_SystemType ExpectedType);
    bool ValidateSystemPriority(const FString& SystemName, int32 Priority);
    void LogArchitectureViolation(const FString& SystemName, const FString& Details);
    void CheckPerformanceThresholds();

    // Timer handles
    FTimerHandle ValidationTimerHandle;
    FTimerHandle PerformanceTimerHandle;

private:
    // Architecture constants
    static constexpr float DEFAULT_PERFORMANCE_THRESHOLD = 16.67f; // 60 FPS target
    static constexpr int32 MAX_VALIDATION_ERRORS = 100;
    
    int32 ValidationErrorCount = 0;
    bool bArchitectureValid = true;
};