#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    CharacterMovement   UMETA(DisplayName = "Character Movement"),
    DinosaurAI         UMETA(DisplayName = "Dinosaur AI"),
    SurvivalSystems    UMETA(DisplayName = "Survival Systems"),
    WorldGeneration    UMETA(DisplayName = "World Generation"),
    Performance        UMETA(DisplayName = "Performance")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float ProgressPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercent = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type = EDir_MilestoneType::CharacterMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Name = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FVector WorldLocation = FVector::ZeroVector;

    FDir_ProductionMilestone()
    {
        Type = EDir_MilestoneType::CharacterMovement;
        Name = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        RequiredAgents.Empty();
        WorldLocation = FVector::ZeroVector;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages 19-agent production pipeline and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === AGENT MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentInfo> Agents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === COORDINATION FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Task);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteMilestone(EDir_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestoneRequirements(EDir_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentInfo> GetAgentsByStatus(EDir_AgentStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

    // === MILESTONE VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateSurvivalSystems();

    // === DEBUG FUNCTIONS ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintAgentStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintMilestones();

private:
    void SetupAgentData();
    void SetupMilestoneData();
    void CheckForBlockedAgents();
    void ValidateProductionChain();

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    int32 CompletedMilestones = 0;
};

#include "Dir_ProductionCoordinator.generated.h"