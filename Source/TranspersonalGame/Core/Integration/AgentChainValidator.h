// Copyright Transpersonal Game Studio. All Rights Reserved.
// AgentChainValidator.h - Validates the 19-agent production chain

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "AgentChainValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAgentChain, Log, All);

/**
 * Agent chain position and status
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentChainPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    FString AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    FString AgentRole;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    TArray<FString> Outputs;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    bool bHasCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    bool bIsBlocked = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    FDateTime LastActivityTime;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain")
    float CompletionPercentage = 0.0f;

    FAgentChainPosition()
    {
        AgentNumber = TEXT("");
        AgentName = TEXT("");
        AgentRole = TEXT("");
        bIsActive = false;
        bHasCompleted = false;
        bIsBlocked = false;
        LastActivityTime = FDateTime::Now();
        CompletionPercentage = 0.0f;
    }
};

/**
 * Chain validation result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChainValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    bool bIsValidChain = false;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    bool bAllAgentsRegistered = false;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    bool bProperSequence = false;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    TArray<FString> MissingAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    TArray<FString> OutOfSequenceAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    TArray<FString> BlockedAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Validation")
    float ChainCompletionPercentage = 0.0f;

    FChainValidationResult()
    {
        bIsValidChain = false;
        bAllAgentsRegistered = false;
        bProperSequence = false;
        ChainCompletionPercentage = 0.0f;
    }
};

/**
 * Agent handoff information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentHandoff
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    FString FromAgent;

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    FString ToAgent;

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    TArray<FString> HandoffData;

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    FDateTime HandoffTime;

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    bool bHandoffCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    bool bHandoffValidated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Handoff")
    FString HandoffNotes;

    FAgentHandoff()
    {
        FromAgent = TEXT("");
        ToAgent = TEXT("");
        HandoffTime = FDateTime::Now();
        bHandoffCompleted = false;
        bHandoffValidated = false;
        HandoffNotes = TEXT("");
    }
};

/**
 * Agent Chain Validator
 * 
 * This subsystem validates and monitors the 19-agent production chain
 * ensuring proper sequence, handoffs, and completion tracking.
 * Enforces the chain rules and prevents out-of-sequence execution.
 */
UCLASS()
class TRANSPERSONALGAME_API UAgentChainValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Chain Management
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    FChainValidationResult ValidateAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    TArray<FAgentChainPosition> GetAgentChainStatus() const { return AgentChain; }

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    FAgentChainPosition GetAgentPosition(const FString& AgentNumber) const;

    // Agent Registration and Activation
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool RegisterAgent(const FString& AgentNumber, const FString& AgentName, const FString& AgentRole);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool ActivateAgent(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool DeactivateAgent(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool CanActivateAgent(const FString& AgentNumber);

    // Sequence Validation
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool IsAgentInSequence(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    FString GetNextAgentInSequence();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    FString GetPreviousAgentInSequence(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool ValidateAgentSequence();

    // Handoff Management
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool InitiateHandoff(const FString& FromAgent, const FString& ToAgent, const TArray<FString>& HandoffData);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool CompleteHandoff(const FString& FromAgent, const FString& ToAgent);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool ValidateHandoff(const FString& FromAgent, const FString& ToAgent);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    TArray<FAgentHandoff> GetActiveHandoffs() const { return ActiveHandoffs; }

    // Progress Tracking
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool UpdateAgentProgress(const FString& AgentNumber, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool MarkAgentCompleted(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    float GetOverallChainProgress();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    TArray<FString> GetCompletedAgents();

    // Blocking and Dependencies
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool BlockAgent(const FString& AgentNumber, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool UnblockAgent(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool CheckAgentDependencies(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    TArray<FString> GetBlockedAgents();

    // Chain Rules Enforcement
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool EnforceChainRules();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool IsStudioDirectorApprovalRequired(const FString& AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool ValidateQABlockingPower();

    // Reporting and Analytics
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    void GenerateChainReport();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    void GenerateAgentPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    void LogChainStatus();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentActivated, FString, AgentNumber);
    UPROPERTY(BlueprintAssignable, Category = "Agent Chain")
    FOnAgentActivated OnAgentActivated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentCompleted, FString, AgentNumber);
    UPROPERTY(BlueprintAssignable, Category = "Agent Chain")
    FOnAgentCompleted OnAgentCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentBlocked, FString, AgentNumber, FString, Reason);
    UPROPERTY(BlueprintAssignable, Category = "Agent Chain")
    FOnAgentBlocked OnAgentBlocked;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHandoffInitiated, FString, FromAgent, FString, ToAgent);
    UPROPERTY(BlueprintAssignable, Category = "Agent Chain")
    FOnHandoffInitiated OnHandoffInitiated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHandoffCompleted, FString, FromAgent, FString, ToAgent);
    UPROPERTY(BlueprintAssignable, Category = "Agent Chain")
    FOnHandoffCompleted OnHandoffCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChainCompleted);
    UPROPERTY(BlueprintAssignable, Category = "Agent Chain")
    FOnChainCompleted OnChainCompleted;

protected:
    // Internal chain management
    void InitializeDefaultAgentChain();
    int32 GetAgentIndex(const FString& AgentNumber);
    bool ValidateAgentTransition(const FString& FromAgent, const FString& ToAgent);
    void UpdateChainStatus();

    // Rule enforcement
    bool CheckSequenceRules(const FString& AgentNumber);
    bool CheckDependencyRules(const FString& AgentNumber);
    bool CheckBlockingRules(const FString& AgentNumber);

private:
    // Agent chain data
    UPROPERTY()
    TArray<FAgentChainPosition> AgentChain;

    UPROPERTY()
    TMap<FString, int32> AgentNumberToIndex;

    // Handoff tracking
    UPROPERTY()
    TArray<FAgentHandoff> ActiveHandoffs;

    UPROPERTY()
    TArray<FAgentHandoff> CompletedHandoffs;

    // Chain state
    UPROPERTY()
    bool bChainInitialized = false;

    UPROPERTY()
    FString CurrentActiveAgent;

    UPROPERTY()
    int32 ChainPosition = 0;

    // Performance tracking
    UPROPERTY()
    FDateTime ChainStartTime;

    UPROPERTY()
    TMap<FString, FDateTime> AgentStartTimes;

    UPROPERTY()
    TMap<FString, float> AgentCompletionTimes;

    // The 19-agent chain definition
    static const TArray<FString> DefaultAgentChain;
    static const TMap<FString, FString> AgentRoles;
    static const TMap<FString, TArray<FString>> AgentDependencies;
};