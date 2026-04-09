// Copyright Transpersonal Game Studio. All Rights Reserved.
// SystemIntegrationManager.h - Manages integration between all agent systems

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TechnicalArchitecture.h"
#include "EngineArchitectureValidator.h"
#include "SystemIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSystemIntegration, Log, All);

/**
 * System registration information for agent systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentSystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Provides;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsCoreSystem = false;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 InitializationPriority = 0;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime = 0.0f;

    FAgentSystemInfo()
    {
        SystemName = TEXT("");
        AgentNumber = TEXT("");
        AgentName = TEXT("");
        bIsCoreSystem = false;
        bIsInitialized = false;
        InitializationPriority = 0;
        InitializationTime = 0.0f;
    }
};

/**
 * System dependency validation result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemDependencyResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    bool bAllDependenciesMet = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> MissingDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> CircularDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> InitializationOrder;

    FSystemDependencyResult()
    {
        bAllDependenciesMet = false;
    }
};

/**
 * Integration status for the entire system
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bArchitectureCompliant = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 InitializedSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> SystemErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalInitializationTime = 0.0f;

    FSystemIntegrationStatus()
    {
        bAllSystemsInitialized = false;
        bArchitectureCompliant = false;
        TotalSystems = 0;
        InitializedSystems = 0;
        FailedSystems = 0;
        TotalInitializationTime = 0.0f;
    }
};

/**
 * System Integration Manager
 * 
 * This subsystem manages the integration and coordination of all agent systems
 * in the Transpersonal Game Studio pipeline. It ensures proper initialization
 * order, dependency resolution, and architectural compliance.
 */
UCLASS()
class TRANSPERSONALGAME_API USystemIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool RegisterAgentSystem(const FAgentSystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool UnregisterAgentSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    TArray<FAgentSystemInfo> GetRegisteredSystems() const { return RegisteredSystems; }

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FAgentSystemInfo GetSystemInfo(const FString& SystemName) const;

    // System Initialization
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool InitializeSystemByName(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool InitializeSystemsByAgent(const FString& AgentNumber);

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FSystemDependencyResult ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    TArray<FString> GetInitializationOrder();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool CheckSystemDependency(const FString& SystemName, const FString& DependencyName);

    // Status and Monitoring
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FSystemIntegrationStatus GetIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool IsSystemInitialized(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool AreAllSystemsReady();

    // Architecture Validation Integration
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void GenerateSystemIntegrationReport();

    // Agent Chain Management
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateAgentChain();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    TArray<FString> GetAgentChainOrder();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemRegistered, FString, SystemName);
    UPROPERTY(BlueprintAssignable, Category = "System Integration")
    FOnSystemRegistered OnSystemRegistered;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemInitialized, FString, SystemName);
    UPROPERTY(BlueprintAssignable, Category = "System Integration")
    FOnSystemInitialized OnSystemInitialized;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemInitializationFailed, FString, SystemName);
    UPROPERTY(BlueprintAssignable, Category = "System Integration")
    FOnSystemInitializationFailed OnSystemInitializationFailed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSystemsReady);
    UPROPERTY(BlueprintAssignable, Category = "System Integration")
    FOnAllSystemsReady OnAllSystemsReady;

protected:
    // Internal system management
    bool InitializeSystemInternal(FAgentSystemInfo& SystemInfo);
    bool ValidateSystemRequirements(const FAgentSystemInfo& SystemInfo);
    void UpdateSystemStatus();

    // Dependency resolution
    TArray<FString> ResolveDependencyOrder();
    bool HasCircularDependencies(TArray<FString>& CircularDeps);
    bool CanInitializeSystem(const FAgentSystemInfo& SystemInfo);

    // Agent chain validation
    bool ValidateAgentSequence();
    void LogAgentChainStatus();

private:
    // System registry
    UPROPERTY()
    TArray<FAgentSystemInfo> RegisteredSystems;

    UPROPERTY()
    TMap<FString, int32> SystemNameToIndex;

    // Initialization state
    UPROPERTY()
    bool bIntegrationInitialized = false;

    UPROPERTY()
    bool bAllSystemsInitialized = false;

    UPROPERTY()
    TArray<FString> InitializationOrder;

    // Architecture validator reference
    UPROPERTY()
    UEngineArchitectureValidator* ArchitectureValidator = nullptr;

    // Agent chain definition (19 agents)
    static const TArray<FString> AgentChain;

    // Performance tracking
    UPROPERTY()
    float SystemInitializationStartTime = 0.0f;

    UPROPERTY()
    TMap<FString, float> SystemInitializationTimes;
};