#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Dir_StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Blocked
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Architecture,
    CoreSystems,
    WorldGeneration,
    Environment,
    Characters,
    Animation,
    AI,
    Audio,
    VFX,
    QA,
    Integration
};

USTRUCT(BlueprintType)
struct FDir_AgentCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    FDir_AgentCoordination()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Phase = EDir_ProductionPhase::Architecture;
        CompletionPercentage = 0.0f;
        CurrentTask = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float Priority;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        bIsCompleted = false;
        Priority = 1.0f;
    }
};

/**
 * Studio Director - Coordinates the 19-agent production pipeline
 * Translates creative vision into technical tasks and manages dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core coordination system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentCoordination> AgentPipeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    int32 CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    float CycleDuration;

    // Milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bWalkAroundMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bTerrainMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bDinosaurMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bAtmosphereMilestone;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetReadyAgents();

    // Production milestone management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckMilestoneCompletion();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateWalkAroundPrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMap();

    // Asset pipeline coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AreAssetCriteriaReady();

    // Emergency protocols
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleAgentFailure(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ActivateEmergencyMode();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

private:
    float LastCycleTime;
    bool bEmergencyMode;
    int32 FailedAgentCount;
};