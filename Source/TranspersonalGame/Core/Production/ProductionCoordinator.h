#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    CoreSystems UMETA(DisplayName = "Core Systems"),
    ContentCreation UMETA(DisplayName = "Content Creation"),
    Polish UMETA(DisplayName = "Polish"),
    Testing UMETA(DisplayName = "Testing"),
    Release UMETA(DisplayName = "Release")
};

USTRUCT(BlueprintType)
struct FDir_AgentMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentMetrics()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        CycleTime = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TerrainActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCharacterMovementReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bBasicLightingComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bGameModeConfigured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime ProductionStartTime;

    FDir_ProductionMetrics()
    {
        CurrentPhase = EDir_ProductionPhase::CoreSystems;
        OverallProgress = 0.0f;
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        TerrainActorCount = 0;
        bCharacterMovementReady = false;
        bBasicLightingComplete = false;
        bGameModeConfigured = false;
        CurrentMilestone = TEXT("Milestone 1: Walk Around");
        ProductionStartTime = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Tracks all 19 agents, production metrics, and milestone progress
 * Provides real-time production dashboard for the development team
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentMetrics> AgentRegistry;

    // Production Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateMetrics;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FString> MilestoneList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    int32 CurrentMilestoneIndex;

public:
    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void RegisterAgent(int32 AgentID, const FString& AgentName, const FString& Specialization);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& CurrentTask);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentMetrics(int32 AgentID, int32 FilesCreated, int32 UE5Commands, float CycleTime);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    FDir_AgentMetrics GetAgentMetrics(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentMetrics> GetAllAgentMetrics() const;

    // Production Metrics Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void AdvanceToNextMilestone();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    FString GetCurrentMilestone() const;

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Level Analysis Functions
    UFUNCTION(BlueprintCallable, Category = "Level Analysis")
    void AnalyzeLevelState();

    UFUNCTION(BlueprintCallable, Category = "Level Analysis")
    int32 CountActorsByType(const FString& ActorType) const;

    UFUNCTION(BlueprintCallable, Category = "Level Analysis")
    bool ValidateMinimumViablePrototype() const;

    // Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void GenerateAgentTaskList();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void BroadcastProductionUpdate();

    UFUNCTION(BlueprintCallable, Category = "Coordination", CallInEditor = true)
    void RefreshProductionDashboard();

private:
    void InitializeAgentRegistry();
    void InitializeMilestones();
    void UpdateLevelMetrics();
    void CheckMilestoneCompletion();
    
    float LastUpdateTime;
};