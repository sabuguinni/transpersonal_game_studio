#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureManager.generated.h"

/**
 * Engine Architecture Manager - Central authority for technical architecture decisions
 * Enforces coding standards, module dependencies, and system integration rules
 * This is the technical backbone that ensures all agent outputs follow architectural principles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateClassNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegration();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterModuleSystem(const FString& ModuleName, const FString& SystemClass);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetRegisteredModules() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetPerformanceBudget(const FString& SystemName, float MaxFrameTimeMs);

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateAgentOutput(int32 AgentID, const FString& OutputType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetAgentConstraints(int32 AgentID, const TArray<FString>& AllowedSystems);

protected:
    // Module Registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FString> RegisteredModules;

    // Performance Budgets
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, float> PerformanceBudgets;

    // Agent Constraints
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<int32, TArray<FString>> AgentSystemConstraints;

    // Architecture Rules
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> MandatoryIncludePaths;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> ForbiddenDependencies;

    // System Health
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, float> SystemHealthMetrics;

private:
    // Internal validation methods
    bool ValidateIncludePaths();
    bool ValidateCircularDependencies();
    bool ValidateMemoryUsage();
    
    // Performance tracking
    void UpdateSystemMetrics();
    void CheckPerformanceBudgets();
    
    // Agent coordination
    void InitializeAgentConstraints();
    void ValidateAgentCompliance();
};