#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QA_AgentDeliverableTracker.generated.h"

UENUM(BlueprintType)
enum class EQA_DeliverableType : uint8
{
    CppFile         UMETA(DisplayName = "C++ File"),
    HeaderFile      UMETA(DisplayName = "Header File"),
    UE5Command      UMETA(DisplayName = "UE5 Command"),
    Blueprint       UMETA(DisplayName = "Blueprint"),
    Asset           UMETA(DisplayName = "Asset"),
    Report          UMETA(DisplayName = "Report"),
    None            UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EQA_AgentProductivity : uint8
{
    Excellent       UMETA(DisplayName = "Excellent"),
    Good            UMETA(DisplayName = "Good"),
    Average         UMETA(DisplayName = "Average"),
    Poor            UMETA(DisplayName = "Poor"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_AgentDeliverable
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    EQA_DeliverableType DeliverableType;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    bool bIsConcreteImplementation;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    FDateTime CreationTime;

    FQA_AgentDeliverable()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        DeliverableType = EQA_DeliverableType::None;
        FileName = TEXT("");
        Description = TEXT("");
        bIsConcreteImplementation = false;
        CreationTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FQA_AgentPerformanceScore
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 CppFilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 ConcreteDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 ReportsOnly;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    EQA_AgentProductivity ProductivityRating;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float ProductivityScore;

    FQA_AgentPerformanceScore()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CppFilesCreated = 0;
        UE5CommandsExecuted = 0;
        ConcreteDeliverables = 0;
        ReportsOnly = 0;
        ProductivityRating = EQA_AgentProductivity::Average;
        ProductivityScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_AgentDeliverableTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_AgentDeliverableTracker();

    UFUNCTION(BlueprintCallable, Category = "QA Tracking", CallInEditor)
    void TrackAgentDeliverables();

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    void AddDeliverable(int32 AgentNumber, const FString& AgentName, EQA_DeliverableType Type, 
        const FString& FileName, const FString& Description, bool bIsConcrete);

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    FQA_AgentPerformanceScore CalculateAgentScore(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    TArray<FQA_AgentPerformanceScore> GenerateProductivityScorecard();

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    void FlagUnproductiveAgents();

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    TArray<FQA_AgentDeliverable> GetDeliverablesByAgent(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    void GenerateDeliverableReport();

    UFUNCTION(BlueprintCallable, Category = "QA Tracking")
    void ValidateMinPlayableMapContent();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    TArray<FQA_AgentDeliverable> TrackedDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "QA Tracking")
    TArray<FQA_AgentPerformanceScore> AgentScores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MinCppFilesPerAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MinUE5CommandsPerAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ExcellentScoreThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float GoodScoreThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float AverageScoreThreshold;

private:
    void AnalyzeAgentOutput(int32 AgentNumber, const FString& AgentName);
    EQA_AgentProductivity DetermineProductivityRating(float Score);
    float CalculateProductivityScore(const FQA_AgentPerformanceScore& Score);
    void LogAgentPerformance(const FQA_AgentPerformanceScore& Score);
};