#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// Crowd & Traffic Simulation — Agent #13
// Prehistoric herbivore herd simulation with predator avoidance
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting"),
    Scattered   UMETA(DisplayName = "Scattered")
};

UENUM(BlueprintType)
enum class ECrowd_AgentSpecies : uint8
{
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Gallimimus      UMETA(DisplayName = "Gallimimus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Panic       UMETA(DisplayName = "Panic")
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DensityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsDangerZone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThreatRadius = 500.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentSpecies Species = ECrowd_AgentSpecies::Gallimimus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetWaypoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdState State = ECrowd_HerdState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdGroupID = 0;
};

USTRUCT(BlueprintType)
struct FCrowd_PredatorThreat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThreatRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThreatIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString PredatorLabel = TEXT("Unknown");
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
    int32 MaxAgents = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationTickRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PredatorDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeResponseDelay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdCohesionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius = 80.0f;

    // ---- Runtime State ----

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdAgent> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_WaypointData> MigrationWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_PredatorThreat> ActiveThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 CurrentAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_HerdState GlobalHerdState = ECrowd_HerdState::Grazing;

    // ---- Blueprint-callable API ----

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterPredatorThreat(FVector ThreatLocation, float Radius, float Intensity, const FString& Label);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearPredatorThreat(const FString& Label);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void AddMigrationWaypoint(FVector Location, float DensityWeight, bool bIsDangerZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 SpawnHerdGroup(ECrowd_AgentSpecies Species, FVector SpawnCenter, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerMassFlee(FVector ThreatOrigin);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    float GetDensityAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool IsLocationSafe(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetAgentCountInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    ECrowd_ThreatLevel GetThreatLevelAtLocation(FVector Location) const;

private:
    void TickSimulation(float DeltaTime);
    void UpdateAgentState(FCrowd_HerdAgent& Agent, float DeltaTime);
    void ApplyFlockingBehavior(FCrowd_HerdAgent& Agent);
    void SelectNextWaypoint(FCrowd_HerdAgent& Agent);
    FVector ComputeFleeDirection(const FCrowd_HerdAgent& Agent) const;
    FVector ComputeSeparationForce(const FCrowd_HerdAgent& Agent) const;
    FVector ComputeCohesionForce(const FCrowd_HerdAgent& Agent) const;
    float GetSpeciesMoveSpeed(ECrowd_AgentSpecies Species) const;
    float GetSpeciesFleeSpeed(ECrowd_AgentSpecies Species) const;

    float SimTickAccumulator = 0.0f;
    int32 NextAgentID = 0;
    int32 NextGroupID = 0;
};
