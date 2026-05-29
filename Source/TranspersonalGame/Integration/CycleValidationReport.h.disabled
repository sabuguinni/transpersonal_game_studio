#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../SharedTypes.h"
#include "CycleValidationReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_AgentOutput
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 CycleNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCompilationSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> CreatedFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Summary;

    FBuild_AgentOutput()
    {
        AgentName = TEXT("");
        CycleNumber = 0;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        bCompilationSuccess = false;
        Summary = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime CycleStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime CycleEndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_AgentOutput> AgentOutputs;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bOverallSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalFilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalUE5Commands;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString PlayabilityStatus;

    FBuild_CycleReport()
    {
        CycleID = TEXT("");
        CycleStartTime = FDateTime::Now();
        CycleEndTime = FDateTime::Now();
        bOverallSuccess = false;
        TotalFilesCreated = 0;
        TotalUE5Commands = 0;
        PlayabilityStatus = TEXT("Unknown");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCycleValidationReport : public UDataAsset
{
    GENERATED_BODY()

public:
    UCycleValidationReport();

    // Current cycle data
    UPROPERTY(BlueprintReadOnly, Category = "Cycle Validation")
    FBuild_CycleReport CurrentCycle;

    // Historical data (last 5 cycles)
    UPROPERTY(BlueprintReadOnly, Category = "Cycle Validation")
    TArray<FBuild_CycleReport> RecentCycles;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    void AddAgentOutput(const FBuild_AgentOutput& AgentOutput);

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    void FinalizeCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    bool ValidateCycleProgress();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    TArray<FString> GetCriticalIssues();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    float GetPlayabilityScore();

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    FBuild_AgentOutput GetAgentOutput(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    TArray<FString> GetAgentsWithErrors();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    bool IsMinimumViablePrototype();

protected:
    // Internal validation helpers
    void ValidateAgentOutput(FBuild_AgentOutput& Output);
    void CheckCompilationStatus();
    void AssessPlayability();
    void UpdateHistoricalData();
};