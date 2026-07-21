#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QA_AgentPerformanceScorecard.generated.h"

UENUM(BlueprintType)
enum class EQA_AgentStatus : uint8
{
    Excellent,
    Good,
    Warning,
    Critical,
    Blocked
};

USTRUCT(BlueprintType)
struct FQA_AgentMetrics : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 CppFilesCreated;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 HeaderFilesCreated;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 ActorsSpawned;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    bool bBuildsOnExistingContent;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    bool bProducedConcreteDeliverables;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_AgentStatus Status;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Notes;

    FQA_AgentMetrics()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CppFilesCreated = 0;
        HeaderFilesCreated = 0;
        UE5CommandsExecuted = 0;
        ActorsSpawned = 0;
        bBuildsOnExistingContent = false;
        bProducedConcreteDeliverables = false;
        Status = EQA_AgentStatus::Warning;
        Notes = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_AgentPerformanceScorecard : public UObject
{
    GENERATED_BODY()

public:
    UQA_AgentPerformanceScorecard();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void InitializeScorecard();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void UpdateAgentMetrics(int32 AgentNumber, const FQA_AgentMetrics& Metrics);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_AgentMetrics GetAgentMetrics(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_AgentMetrics> GetAllMetrics() const { return AgentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalCppFiles() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalUE5Commands() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetDeliverableSuccessRate() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<int32> GetCriticalAgents() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_AgentMetrics> AgentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalAgents;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float OverallScore;

private:
    void CalculateOverallScore();
    EQA_AgentStatus DetermineAgentStatus(const FQA_AgentMetrics& Metrics);
};