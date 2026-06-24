#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Socializing UMETA(DisplayName = "Socializing"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Panicking   UMETA(DisplayName = "Panicking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Resting     UMETA(DisplayName = "Resting"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    TribalHunter    UMETA(DisplayName = "Tribal Hunter"),
    TribalGatherer  UMETA(DisplayName = "Tribal Gatherer"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalChild     UMETA(DisplayName = "Tribal Child"),
    TribalScout     UMETA(DisplayName = "Tribal Scout"),
    DinosaurHerbivore UMETA(DisplayName = "Dinosaur Herbivore"),
    DinosaurCarnivore UMETA(DisplayName = "Dinosaur Carnivore"),
    WildAnimal      UMETA(DisplayName = "Wild Animal"),
};

UENUM(BlueprintType)
enum class ECrowd_ThreatType : uint8
{
    None            UMETA(DisplayName = "None"),
    Predator        UMETA(DisplayName = "Predator"),
    PlayerPresence  UMETA(DisplayName = "Player Presence"),
    Fire            UMETA(DisplayName = "Fire"),
    Earthquake      UMETA(DisplayName = "Earthquake"),
    Stampede        UMETA(DisplayName = "Stampede"),
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FGuid AgentID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentType AgentType = ECrowd_AgentType::TribalHunter;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    float MaxSpeed = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    AActor* RepresentingActor = nullptr;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FGuid HerdID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FName HerdName;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FGuid> MemberIDs;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsMigrating = false;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, meta = (DisplayName = "Crowd Simulation Manager"))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent registration
    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void RegisterCrowdAgent(UPARAM(ref) FCrowd_AgentData& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void UnregisterCrowdAgent(const FGuid& AgentID);

    // Herd management
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerd(UPARAM(ref) FCrowd_HerdData& HerdData);

    // Threat system
    UFUNCTION(BlueprintCallable, Category = "Crowd|Threat")
    void BroadcastThreatAlert(const FVector& ThreatLocation, float ThreatRadius, ECrowd_ThreatType ThreatType);

    // Simulation tick
    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void TickSimulation(float DeltaTime);

    // Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd|Query")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FCrowd_AgentData* GetAgentData(const FGuid& AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void SetSimulationActive(bool bActive);

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingSeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingAlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float ThreatResponseRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 CurrentAgentCount;

private:
    void UpdateFlockingBehavior(float DeltaTime);

    TMap<FGuid, FCrowd_AgentData> RegisteredAgents;
    TMap<FGuid, FCrowd_HerdData> ActiveHerds;

    float SimulationTickRate;
    float AccumulatedDeltaTime;
};
