#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIntegration, Log, All);

UENUM(BlueprintType)
enum class EIntegrationStatus : uint8
{
    Pending,
    InProgress,
    Success,
    Failed,
    Blocked
};

UENUM(BlueprintType)
enum class EAgentType : uint8
{
    EngineArchitect = 2,
    CoreSystems = 3,
    Performance = 4,
    WorldGenerator = 5,
    Environment = 6,
    Architecture = 7,
    Lighting = 8,
    Character = 9,
    Animation = 10,
    NPCBehavior = 11,
    Combat = 12,
    Crowd = 13,
    Quest = 14,
    Narrative = 15,
    Audio = 16,
    VFX = 17,
    QA = 18
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentOutput
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EAgentType AgentType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString OutputPath;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EIntegrationStatus Status;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime LastUpdated;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Priority;

    FAgentOutput()
    {
        AgentType = EAgentType::EngineArchitect;
        Status = EIntegrationStatus::Pending;
        LastUpdated = FDateTime::Now();
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FIntegrationCycle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString CycleID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FAgentOutput> AgentOutputs;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EIntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime StartTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime EndTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> BuildLogs;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bQAApproved;

    FIntegrationCycle()
    {
        OverallStatus = EIntegrationStatus::Pending;
        StartTime = FDateTime::Now();
        bQAApproved = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntegrationStatusChanged, const FString&, CycleID, EIntegrationStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAgentValidationComplete, EAgentType, AgentType, bool, bSuccess, const TArray<FString>&, Errors);

/**
 * Integration Manager - Coordinates outputs from all 18 agents in the production chain
 * Ensures compatibility, validates dependencies, and manages build integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool StartIntegrationCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAgentOutput(EAgentType AgentType, const FString& OutputPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EIntegrationStatus GetCycleStatus(const FString& CycleID) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetValidationErrors(const FString& CycleID) const;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TriggerBuild(const FString& Configuration = TEXT("Development"));

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool PackageProject(const FString& Configuration, const FString& Platform = TEXT("Win64"));

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool RollbackToPreviousBuild();

    // QA Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ProcessQAResults(const FString& QAReportPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsQAApproved(const FString& CycleID) const;

    // Agent Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<EAgentType> GetAgentDependencies(EAgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CanAgentProceed(EAgentType AgentType, const FString& CycleID) const;

    // Build History
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetBuildHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool MaintainBuildHistory(int32 MaxBuilds = 10);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Integration")
    FOnIntegrationStatusChanged OnIntegrationStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Integration")
    FOnAgentValidationComplete OnAgentValidationComplete;

protected:
    // Current integration cycles
    UPROPERTY()
    TMap<FString, FIntegrationCycle> ActiveCycles;

    // Agent dependency map
    UPROPERTY()
    TMap<EAgentType, TArray<EAgentType>> AgentDependencies;

    // Build history
    UPROPERTY()
    TArray<FString> BuildHistory;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxConcurrentCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationTimeoutSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoTriggerBuilds;

private:
    void InitializeAgentDependencies();
    bool ValidateArchitectureFiles(const FString& Path) const;
    bool ValidateCoreSystemFiles(const FString& Path) const;
    bool ValidateQAReport(const FString& Path) const;
    bool ValidateAssetIntegrity(const FString& Path) const;
    void UpdateCycleStatus(const FString& CycleID, EIntegrationStatus NewStatus);
    void LogIntegrationEvent(const FString& Message, bool bIsError = false) const;
};