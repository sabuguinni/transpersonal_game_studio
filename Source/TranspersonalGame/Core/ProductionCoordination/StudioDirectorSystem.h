#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Central coordination hub for 18-agent development pipeline
 * Tracks agent deliverables, manages production milestones, and coordinates workflow
 * This is the command center for the Transpersonal Game Studio development process
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"), 
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Blocked         UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Architecture    UMETA(DisplayName = "Architecture Phase"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Characters      UMETA(DisplayName = "Characters & Animation"),
    Gameplay        UMETA(DisplayName = "Gameplay Systems"),
    Polish          UMETA(DisplayName = "Polish & QA"),
    Release         UMETA(DisplayName = "Release Ready")
};

USTRUCT(BlueprintType)
struct FDir_AgentMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 FilesCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    float LastCycleTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FString LastDeliverable = "";

    FDir_AgentMetrics()
    {
        AgentID = 0;
        AgentName = "";
        Status = EDir_AgentStatus::Idle;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        LastCycleTime = 0.0f;
        LastDeliverable = "";
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionPhase Phase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description = "";

    FDir_ProductionMilestone()
    {
        MilestoneName = "";
        Phase = EDir_ProductionPhase::PreProduction;
        bCompleted = false;
        RequiredAgents.Empty();
        Description = "";
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
    virtual void Tick(float DeltaTime) override;

    // Visual Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* HubMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Production Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentMetrics> AgentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float TotalProductionTime;

public:
    // Agent Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& Name, EDir_ProductionPhase Phase, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetCurrentPhaseStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentMetrics> GetActiveAgents();

    // Quality Control
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDeliverables();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResetProductionPipeline();

private:
    void UpdateStatusDisplay();
    void InitializeDefaultMilestones();
    void CheckAgentDependencies();
    
    // Internal tracking
    float LastUpdateTime;
    bool bProductionActive;
    FString LastEmergencyReason;
};