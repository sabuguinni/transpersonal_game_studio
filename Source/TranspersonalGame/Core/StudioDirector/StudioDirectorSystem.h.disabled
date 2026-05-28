#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Completed UMETA(DisplayName = "Completed"),
    Error UMETA(DisplayName = "Error"),
    Blocked UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector UMETA(DisplayName = "Studio Director"),
    EngineArchitect UMETA(DisplayName = "Engine Architect"),
    CoreSystems UMETA(DisplayName = "Core Systems"),
    Performance UMETA(DisplayName = "Performance Optimizer"),
    WorldGenerator UMETA(DisplayName = "World Generator"),
    EnvironmentArtist UMETA(DisplayName = "Environment Artist"),
    Architecture UMETA(DisplayName = "Architecture & Interior"),
    Lighting UMETA(DisplayName = "Lighting & Atmosphere"),
    CharacterArtist UMETA(DisplayName = "Character Artist"),
    Animation UMETA(DisplayName = "Animation"),
    NPCBehavior UMETA(DisplayName = "NPC Behavior"),
    CombatAI UMETA(DisplayName = "Combat & Enemy AI"),
    CrowdSimulation UMETA(DisplayName = "Crowd & Traffic"),
    QuestDesigner UMETA(DisplayName = "Quest & Mission Designer"),
    Narrative UMETA(DisplayName = "Narrative & Dialogue"),
    Audio UMETA(DisplayName = "Audio"),
    VFX UMETA(DisplayName = "VFX"),
    QA UMETA(DisplayName = "QA & Testing"),
    Integration UMETA(DisplayName = "Integration & Build")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentID = 0;
        AgentType = EDir_AgentType::StudioDirector;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("Waiting for tasks");
        ProgressPercentage = 0.0f;
        LastOutput = TEXT("");
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float LevelCompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentBuildStatus;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        CharacterActors = 0;
        LevelCompletionPercentage = 0.0f;
        CurrentBuildStatus = TEXT("In Development");
    }
};

/**
 * Studio Director System - Coordinates the 19-agent development pipeline
 * Tracks agent status, manages production metrics, and ensures milestone completion
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgents() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestone1Complete() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetMilestone1Progress() const;

    // Task Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DispatchTaskToAgent(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentInfo> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bMilestone1Completed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID;

private:
    void SetupAgentDependencies();
    void ValidateAgentChain();
    int32 CountActorsByType(const FString& ActorType) const;
};

#include "StudioDirectorSystem.generated.h"