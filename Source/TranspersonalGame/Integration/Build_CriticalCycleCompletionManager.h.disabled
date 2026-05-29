#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_CriticalCycleCompletionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleCompletionStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    int32 AgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    int32 CompletedAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    bool bAllSystemsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    bool bQAApproved;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    float CycleExecutionTime;

    FBuild_CycleCompletionStatus()
    {
        CycleID = TEXT("");
        AgentCount = 19;
        CompletedAgents = 0;
        bAllSystemsValidated = false;
        bQAApproved = false;
        bIntegrationComplete = false;
        CycleExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCompilationSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bRuntimeStable;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bPerformanceAcceptable;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationErrors;

    FBuild_SystemIntegrationReport()
    {
        SystemName = TEXT("");
        bCompilationSuccess = false;
        bRuntimeStable = false;
        bPerformanceAcceptable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_CriticalCycleCompletionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CriticalCycleCompletionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cycle completion management
    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void InitializeCycleCompletion(const FString& InCycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void RegisterAgentCompletion(int32 AgentNumber, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void ValidateSystemIntegration(const FString& SystemName, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void SetQAApproval(bool bApproved, const TArray<FString>& Issues);

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void CompleteCycleIntegration();

    // Status queries
    UFUNCTION(BlueprintPure, Category = "Cycle Status")
    FBuild_CycleCompletionStatus GetCycleStatus() const { return CurrentCycleStatus; }

    UFUNCTION(BlueprintPure, Category = "Cycle Status")
    bool IsCycleComplete() const;

    UFUNCTION(BlueprintPure, Category = "Cycle Status")
    float GetCycleProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Status")
    TArray<FBuild_SystemIntegrationReport> GetSystemReports() const { return SystemReports; }

    // Critical validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAllCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateRuntimeStability();

    // Emergency procedures
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyCompletion(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ResetCycleState();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FBuild_CycleCompletionStatus CurrentCycleStatus;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FBuild_SystemIntegrationReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> CompletedAgents;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bEmergencyMode;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FDateTime CycleStartTime;

private:
    void UpdateCycleProgress();
    void LogCycleCompletion();
    bool ValidateSystemDependencies();
    void GenerateCompletionReport();
};