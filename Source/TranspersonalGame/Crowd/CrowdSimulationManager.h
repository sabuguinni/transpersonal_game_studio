#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// Crowd & Traffic Simulation — Agent #13
// Manages up to 50,000 crowd agents using Mass AI patterns
// Herbivore herds, migration routes, stampede reactions
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Grazing       UMETA(DisplayName = "Grazing"),
    Wandering     UMETA(DisplayName = "Wandering"),
    Migrating     UMETA(DisplayName = "Migrating"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Stampeding    UMETA(DisplayName = "Stampeding"),
    Resting       UMETA(DisplayName = "Resting"),
    Drinking      UMETA(DisplayName = "Drinking"),
    Dead          UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Hadrosaur     UMETA(DisplayName = "Hadrosaur"),
    Triceratops   UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Gallimimus    UMETA(DisplayName = "Gallimimus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    LOD0_FullSim    UMETA(DisplayName = "LOD0 Full Simulation"),
    LOD1_Simplified UMETA(DisplayName = "LOD1 Simplified"),
    LOD2_Impostor   UMETA(DisplayName = "LOD2 Impostor"),
    LOD3_Culled     UMETA(DisplayName = "LOD3 Culled")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Hadrosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 HerdID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsHerdLeader = false;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float ThreatRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float EventTimestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float WaypointAcceptanceRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bIsLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    ECrowd_HerdSpecies AssignedSpecies = ECrowd_HerdSpecies::Hadrosaur;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Hadrosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_MigrationRoute MigrationRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float LastThreatTime = -9999.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationTickRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD0_Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD1_Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD2_Radius = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableStampede = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableMigration = true;

    // ---- Runtime State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdData> ActiveHerds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    bool bStampedeActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    FCrowd_StampedeEvent CurrentStampede;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void TriggerStampede(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void RegisterHerd(FCrowd_HerdData HerdData);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void UpdateHerdState(int32 HerdID, ECrowd_AgentState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    FCrowd_HerdData GetHerdData(int32 HerdID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    ECrowd_LODLevel GetLODForDistance(float DistanceFromPlayer) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void AddMigrationWaypoint(int32 HerdID, FVector Waypoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void ReceiveCombatThreatEvent(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void DebugDrawHerds();

private:
    float SimAccumulator = 0.0f;

    void TickSimulation(float DeltaTime);
    void UpdateHerdMigration(FCrowd_HerdData& Herd, float DeltaTime);
    void UpdateStampede(float DeltaTime);
    FVector CalculateFleeDirection(FVector AgentPos, FVector ThreatPos) const;
    int32 FindHerdIndex(int32 HerdID) const;
};
