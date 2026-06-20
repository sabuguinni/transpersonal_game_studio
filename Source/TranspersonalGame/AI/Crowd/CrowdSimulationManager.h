#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// Enums — Crowd_ prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Predator    UMETA(DisplayName = "Predator"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Resting     UMETA(DisplayName = "Resting"),
};

UENUM(BlueprintType)
enum class ECrowd_DinoSpecies : uint8
{
    Raptor          UMETA(DisplayName = "Raptor"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
};

// ============================================================
// Structs — Crowd_ prefix
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_HerdZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FName ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_DinoSpecies PreferredSpecies = ECrowd_DinoSpecies::Brachiosaurus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxOccupants = 8;
};

USTRUCT(BlueprintType)
struct FCrowd_PatrolNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_DinoSpecies OwnerSpecies = ECrowd_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 NodeIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WaitTime = 2.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Herbivore;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_AgentBehavior CurrentBehavior = ECrowd_AgentBehavior::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float BehaviorTimer = 0.0f;
};

// ============================================================
// CrowdSimulationManager — WorldSubsystem
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Herd zone management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerdZone(const FCrowd_HerdZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_HerdZone GetNearestHerdZone(const FVector& WorldLocation) const;

    // Patrol node management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterPatrolNode(const FCrowd_PatrolNode& Node);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_PatrolNode> GetPatrolRouteForSpecies(ECrowd_DinoSpecies Species) const;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount(ECrowd_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnHerdAtZone(const FCrowd_HerdZone& Zone, int32 Count);

    // Behavior triggers
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    // Tick update (called from GameMode or subsystem tick)
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdTick(float DeltaTime);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerbivoreAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxPredatorAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PredatorDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bCrowdSystemActive;

private:
    UPROPERTY()
    TArray<FCrowd_HerdZone> HerdZones;

    UPROPERTY()
    TArray<FCrowd_PatrolNode> PatrolNodes;

    UPROPERTY()
    TArray<FCrowd_AgentState> ActiveAgents;

    void UpdateAgentBehavior(FCrowd_AgentState& Agent, float DeltaTime);
};
