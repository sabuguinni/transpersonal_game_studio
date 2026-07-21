// CrowdMigrationController.h
// Agent #13 — Crowd & Traffic Simulation
// Manages large-scale herbivore migration herds using Mass AI patterns
// Prehistoric survival game — dinosaur migration crowd simulation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMigrationController.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_MigrationState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Resting         UMETA(DisplayName = "Resting"),
    Fleeing         UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECrowd_HerdFormation : uint8
{
    Column          UMETA(DisplayName = "Column"),       // Migration column
    VFormation      UMETA(DisplayName = "V-Formation"),  // Efficient travel
    Circle          UMETA(DisplayName = "Circle"),       // Defensive
    Scattered       UMETA(DisplayName = "Scattered"),    // Grazing
    Stampede        UMETA(DisplayName = "Stampede")      // Panic run
};

UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Gallimimus      UMETA(DisplayName = "Gallimimus"),
    Iguanodon       UMETA(DisplayName = "Iguanodon")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_MigrationAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector TargetPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float StaminaLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float HungerLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_MigrationState AgentState = ECrowd_MigrationState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 FormationIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    TWeakObjectPtr<AActor> ActorRef;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FString HerdID = TEXT("Herd_001");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Parasaurolophus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdFormation Formation = ECrowd_HerdFormation::Column;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_MigrationState HerdState = ECrowd_MigrationState::Migrating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector LeaderPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdCohesionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsStampeding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FCrowd_MigrationAgent> Agents;
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    FString RouteID = TEXT("Route_001");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    float SeasonalWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    bool bIsCircular = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    int32 CurrentWaypointIndex = 0;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowdMigrationController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdMigrationController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    ECrowd_HerdSpecies DefaultSpecies = ECrowd_HerdSpecies::Parasaurolophus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxSimultaneousHerds = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationTickRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentSeparationDistance = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionStrength = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentStrength = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationStrength = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PredatorDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float StampedeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float NormalMigrationSpeed = 280.0f;

    // ---- Runtime State ----

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdData> ActiveHerds;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 TotalActiveAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive = false;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void SpawnHerd(ECrowd_HerdSpecies Species, FVector SpawnLocation, int32 HerdSize, ECrowd_HerdFormation Formation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void TriggerStampede(FVector PanicSource, float PanicRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void SetMigrationRoute(const FString& HerdID, const FCrowd_MigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FCrowd_HerdData GetHerdData(const FString& HerdID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    bool IsPlayerInStampedeZone(FVector PlayerLocation, float& OutDangerLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Debug")
    void DrawDebugHerdBounds() const;

private:
    void TickHerdSimulation(float DeltaTime);
    void UpdateAgentFlocking(FCrowd_HerdData& Herd, float DeltaTime);
    FVector CalculateSeparation(const FCrowd_MigrationAgent& Agent, const TArray<FCrowd_MigrationAgent>& Neighbors) const;
    FVector CalculateCohesion(const FCrowd_MigrationAgent& Agent, const TArray<FCrowd_MigrationAgent>& Neighbors) const;
    FVector CalculateAlignment(const FCrowd_MigrationAgent& Agent, const TArray<FCrowd_MigrationAgent>& Neighbors) const;
    void UpdateFormationPositions(FCrowd_HerdData& Herd);
    void CheckPredatorProximity(FCrowd_HerdData& Herd);
    FVector GetFormationOffset(ECrowd_HerdFormation Formation, int32 Index, int32 TotalCount) const;

    float SimulationTimer = 0.0f;
    int32 NextHerdID = 0;
};
