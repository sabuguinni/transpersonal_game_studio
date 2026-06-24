#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric herd/pack/cluster crowd simulation
// Supports up to 50,000 agents via Mass AI integration
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_GroupType : uint8
{
    HerbivoreHerd       UMETA(DisplayName = "Herbivore Herd"),
    PredatorPack        UMETA(DisplayName = "Predator Pack"),
    DefensiveCluster    UMETA(DisplayName = "Defensive Cluster"),
    SolitaryWanderer    UMETA(DisplayName = "Solitary Wanderer"),
    MigrationColumn     UMETA(DisplayName = "Migration Column"),
};

USTRUCT(BlueprintType)
struct FCrowd_GroupConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupType GroupID = ECrowd_GroupType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString GroupLabel = TEXT("DefaultGroup");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxGroupSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CohesionStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeThreshold = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsPreySpecies = true;

    // Runtime — not exposed to editor
    TArray<AActor*> AgentActors;
    FVector LastGroupCentroid = FVector::ZeroVector;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Debug")
    bool bCrowdDebugDraw;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdGroup(const FCrowd_GroupConfig& GroupConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgentToGroup(AActor* Agent, const FString& GroupLabel);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetTotalAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_GroupConfig GetGroupConfig(const FString& GroupLabel) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    TMap<FString, FCrowd_GroupConfig> CrowdGroups;

    void InitializeCrowdGroups();
    void UpdateCrowdBehavior(float DeltaTime);
    void UpdateGroupBehavior(FCrowd_GroupConfig& Group, float DeltaTime);
};
