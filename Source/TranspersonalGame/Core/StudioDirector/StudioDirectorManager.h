#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "StudioDirectorManager.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector      UMETA(DisplayName = "Studio Director"),
    EngineArchitect     UMETA(DisplayName = "Engine Architect"),
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance"),
    WorldGenerator      UMETA(DisplayName = "World Generator"),
    Environment         UMETA(DisplayName = "Environment Artist"),
    Architecture        UMETA(DisplayName = "Architecture"),
    Lighting            UMETA(DisplayName = "Lighting"),
    Character           UMETA(DisplayName = "Character Artist"),
    Animation           UMETA(DisplayName = "Animation"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd Simulation"),
    QuestDesigner       UMETA(DisplayName = "Quest Designer"),
    Narrative           UMETA(DisplayName = "Narrative"),
    Audio               UMETA(DisplayName = "Audio"),
    VFX                 UMETA(DisplayName = "VFX"),
    QA                  UMETA(DisplayName = "QA"),
    Integration         UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentType AgentType = EDir_AgentType::StudioDirector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float ProgressPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString LastOutput = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Outputs;

    FDir_AgentInfo()
    {
        LastUpdateTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CycleID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CycleName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentInfo> AgentStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CreativeVision = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsComplete = false;

    FDir_ProductionCycle()
    {
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
    }
};

/**
 * Studio Director Manager - Coordinates the 19-agent production chain
 * Ensures Miguel's creative vision is preserved through every step
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Chain Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CreativeVision, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentID, const TArray<FString>& Outputs);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentID, const FString& BlockReason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerNextAgent();

    // Quality Control
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateCreativeVision(const FString& AgentOutput, const FString& OriginalVision) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RejectAgentWork(int32 AgentID, const FString& RejectionReason);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionCycle GetCurrentCycle() const { return CurrentCycle; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgentStates() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SaveCycleToFile(const FString& FilePath) const;

    // Agent Chain Configuration
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetCurrentActiveAgent() const { return CurrentActiveAgentID; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionCycle CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentInfo> AgentChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    int32 CurrentActiveAgentID = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_ProductionCycle> CycleHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FString MiguelCreativeVision = TEXT("");

private:
    void SetupAgentDependencies();
    bool CheckAgentDependencies(int32 AgentID) const;
    void LogAgentActivity(int32 AgentID, const FString& Activity);
    FString GetAgentName(EDir_AgentType AgentType) const;
};