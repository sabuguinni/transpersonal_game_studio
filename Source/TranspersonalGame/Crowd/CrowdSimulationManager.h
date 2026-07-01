#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampede    UMETA(DisplayName = "Stampede"),
    Resting     UMETA(DisplayName = "Resting"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ECrowd_AgentLOD : uint8
{
    Full    UMETA(DisplayName = "Full — Full physics + animation"),
    Medium  UMETA(DisplayName = "Medium — Simplified movement"),
    Low     UMETA(DisplayName = "Low — Position-only update"),
    Culled  UMETA(DisplayName = "Culled — No update, position preserved")
};

UENUM(BlueprintType)
enum class ECrowd_StampedeType : uint8
{
    PanicFlee           UMETA(DisplayName = "Panic Flee"),
    DirectionalRush     UMETA(DisplayName = "Directional Rush"),
    Scatter             UMETA(DisplayName = "Scatter"),
    CircleStampede      UMETA(DisplayName = "Circle Stampede")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FName SpeciesName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 CurrentAgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdBehavior CurrentBehavior = ECrowd_HerdBehavior::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_AgentLOD CurrentLOD = ECrowd_AgentLOD::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsActive = true;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrowd_OnHerdSpawned, const FCrowd_HerdData&, HerdData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnStampedeTriggered, FVector, Origin, ECrowd_StampedeType, Type);

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Crowd Simulation Manager"))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Simulation Control ──────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SpawnHerd(const FCrowd_HerdData& HerdData);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void DisbandHerd(int32 HerdID);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector Origin, ECrowd_StampedeType Type, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Player")
    void SetPlayerLocation(FVector PlayerLoc);

    UFUNCTION(BlueprintPure, Category = "Crowd|Query")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Query")
    TArray<FCrowd_HerdData> GetHerdsInRadius(FVector Center, float Radius) const;

    // ── Events ──────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnHerdSpawned OnHerdSpawned;

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnStampedeTriggered OnStampedeTriggered;

    // ── Configuration ───────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 GlobalFlowFieldResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateBudgetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceNear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceMid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationWeight;

    // ── Runtime State ───────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bStampedeActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdData> ActiveHerds;

private:
    FVector PlayerLocation;
    FVector StampedeOrigin;
    ECrowd_StampedeType CurrentStampedeType;
    TArray<FVector> FlowFieldGrid;
    FTimerHandle SimulationTickHandle;

    void TickSimulation();
    void TickHerdMigration(FCrowd_HerdData& Herd, float DeltaTime);
    void TickHerdStampede(FCrowd_HerdData& Herd, float DeltaTime);
    void TickHerdGrazing(FCrowd_HerdData& Herd, float DeltaTime);
    void TickHerdFleeing(FCrowd_HerdData& Herd, float DeltaTime);
    void UpdateLODForAllHerds();
    void InitializeFlowField();
    void RegisterHerdSpawnZones();
    FVector ComputeFlockingForce(const FCrowd_HerdData& Herd) const;
};
