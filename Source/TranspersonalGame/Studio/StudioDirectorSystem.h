#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector      UMETA(DisplayName = "Studio Director"),
    EngineArchitect     UMETA(DisplayName = "Engine Architect"),
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance"),
    WorldGeneration     UMETA(DisplayName = "World Generation"),
    EnvironmentArt      UMETA(DisplayName = "Environment Art"),
    Architecture        UMETA(DisplayName = "Architecture"),
    Lighting            UMETA(DisplayName = "Lighting"),
    CharacterArt        UMETA(DisplayName = "Character Art"),
    Animation           UMETA(DisplayName = "Animation"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd Simulation"),
    QuestDesign         UMETA(DisplayName = "Quest Design"),
    Narrative           UMETA(DisplayName = "Narrative"),
    Audio               UMETA(DisplayName = "Audio"),
    VFX                 UMETA(DisplayName = "VFX"),
    QA                  UMETA(DisplayName = "QA"),
    Integration         UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    EDir_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    float TaskProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    float CycleTime;

    FDir_AgentTaskInfo()
    {
        AgentID = 0;
        AgentType = EDir_AgentType::StudioDirector;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("Waiting for assignment");
        TaskProgress = 0.0f;
        LastOutput = TEXT("");
        CycleTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float AverageCycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 UE5CommandsExecuted;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        ActiveAgents = 0;
        AverageCycleTime = 0.0f;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorSystem : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CommandCenterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTaskInfo> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    bool bAutoCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    float CoordinationUpdateInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    FDir_AgentTaskInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentTaskInfo> GetAllAgentInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void StartCoordinationCycle();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void StopCoordinationCycle();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool IsCoordinationActive() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogAgentStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ResetAllAgents();

private:
    void CoordinationTick();
    void ValidateAgentChain();
    void HandleAgentBlocking(int32 BlockedAgentID);
    
    FTimerHandle CoordinationTimerHandle;
    float LastCoordinationUpdate;
};