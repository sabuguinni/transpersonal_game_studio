#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator
 * Manages cross-agent communication and production pipeline state
 * Tracks milestone progress and agent deliverables
 */
USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    int32 CycleNumber;

    UPROPERTY(BlueprintReadOnly)
    bool bHasDeliverables;

    UPROPERTY(BlueprintReadOnly)
    FString LastDeliverable;

    UPROPERTY(BlueprintReadOnly)
    float CompletionPercentage;

    FDir_AgentStatus()
    {
        AgentName = TEXT("Unknown");
        CycleNumber = 0;
        bHasDeliverables = false;
        LastDeliverable = TEXT("");
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString MilestoneName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsComplete;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Requirements;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> CompletedRequirements;

    UPROPERTY(BlueprintReadOnly)
    float ProgressPercentage;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("Unknown");
        bIsComplete = false;
        ProgressPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Production State Management
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_MilestoneStatus> Milestones;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bPlayablePrototypeReady;

    // Visual Marker
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MarkerMesh;

    // Production Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, int32 Cycle, bool HasDeliverables, const FString& LastDeliverable);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CheckMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RefreshProductionState();

private:
    void InitializeMilestones();
    void UpdateMilestoneProgress();
    bool CheckRequirement(const FString& Requirement);
};