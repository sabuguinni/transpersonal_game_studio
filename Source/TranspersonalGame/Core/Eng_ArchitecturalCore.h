#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalCore.generated.h"

class UEng_SystemsRegistry;
class UBiomeManager;
class UCore_PhysicsManager;

/**
 * Core Architectural System - Central hub for all engine architecture validation and coordination
 * Manages system dependencies, initialization order, and cross-agent compliance
 * Ensures all agents follow the established architectural patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture") 
    bool InitializeCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateAgentCompliance(const FString& AgentName, const FString& SystemName);

    // System Registration and Dependencies
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterCoreSystem(const FString& SystemName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies();

    // Performance and Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceRequirements();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetArchitecturalStatus();

    // Cross-Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CoordinateAgentSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemInitializationOrder();

protected:
    // Core system references
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UEng_SystemsRegistry* SystemsRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UBiomeManager* BiomeManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UCore_PhysicsManager* PhysicsManager;

    // Architecture validation state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bCoreSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, int32> SystemPriorities;

private:
    // Internal validation methods
    bool ValidatePhysicsArchitecture();
    bool ValidateWorldGenerationArchitecture();
    bool ValidateBiomeArchitecture();
    bool ValidateCharacterArchitecture();
    bool ValidateAIArchitecture();

    // System coordination
    void InitializeSystemsRegistry();
    void EstablishSystemDependencies();
    void ValidateAgentInterfaces();

    // Performance monitoring
    void MonitorSystemPerformance();
    void EnforceArchitecturalStandards();

    // Logging and diagnostics
    void LogArchitecturalStatus();
    void ReportComplianceViolations(const FString& AgentName, const FString& Violation);
};