#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "ProductionPipelineV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProductionPipelineV43, Log, All);

UENUM(BlueprintType)
enum class EPipelineState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Complete    UMETA(DisplayName = "Complete"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class ECore_AgentStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Active      UMETA(DisplayName = "Active"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Error       UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FProductionStandard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Requirement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Validator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMandatory = true;

    FProductionStandard()
    {
        Category = TEXT("");
        Requirement = TEXT("");
        Validator = TEXT("");
        bMandatory = true;
    }
};

USTRUCT(BlueprintType)
struct FAgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECore_AgentStatus Status = ECore_AgentStatus::Pending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTimespan Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Output;

    FAgentStatus()
    {
        AgentID = 0;
        Status = ECore_AgentStatus::Pending;
        StartTime = FDateTime::MinValue();
        EndTime = FDateTime::MinValue();
        Duration = FTimespan::Zero();
        Output = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FCore_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTimespan TotalDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompletedAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTimespan TotalAgentTime;

    void Reset()
    {
        CycleID = TEXT("");
        StartTime = FDateTime::MinValue();
        EndTime = FDateTime::MinValue();
        TotalDuration = FTimespan::Zero();
        CompletedAgents = 0;
        TotalAgentTime = FTimespan::Zero();
    }
};

/**
 * Production Pipeline V43 - Manages the 19-agent production chain
 * Handles dependencies, standards validation, and production metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionPipelineV43 : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UProductionPipelineV43();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    // Pipeline State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    EPipelineState PipelineState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    FDateTime CycleStartTime;

    // Production Standards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    TArray<FProductionStandard> ProductionStandards;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TMap<int32, FAgentStatus> AgentStatus;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TMap<int32, TArray<int32>> AgentDependencies;

    // Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FCore_ProductionMetrics ProductionMetrics;

public:
    // Pipeline Management
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool StopProductionCycle();

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool ActivateAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool CompleteAgent(int32 AgentID, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool ValidateAgentDependencies(int32 AgentID);

    // Standards Validation
    UFUNCTION(BlueprintCallable, Category = "Standards")
    bool ValidateProductionStandards();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GetPipelineReport() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    TArray<int32> GetReadyAgents() const;

    // Initialization
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeProductionStandards();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeAgentDependencies();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Production Pipeline")
    EPipelineState GetPipelineState() const { return PipelineState; }

    UFUNCTION(BlueprintPure, Category = "Production Pipeline")
    FString GetCurrentCycleID() const { return CurrentCycleID; }

    UFUNCTION(BlueprintPure, Category = "Production Pipeline")
    int32 GetCompletedAgentCount() const { return ProductionMetrics.CompletedAgents; }

    UFUNCTION(BlueprintPure, Category = "Agent Management")
    ECore_AgentStatus GetAgentStatus(int32 AgentID) const 
    { 
        return AgentStatus.Contains(AgentID) ? AgentStatus[AgentID].Status : ECore_AgentStatus::Error; 
    }
};