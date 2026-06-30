// CrowdMassSpawner.h
// Agent #13 — Crowd & Traffic Simulation
// Mass Entity spawner for prehistoric herd simulation — up to 200 agents with LOD and flee response

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMassSpawner.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_SpawnPattern : uint8
{
    Scattered       UMETA(DisplayName = "Scattered"),
    HerdFormation   UMETA(DisplayName = "Herd Formation"),
    MigrationLine   UMETA(DisplayName = "Migration Line"),
    WaterGathering  UMETA(DisplayName = "Water Gathering"),
    ForestEdge      UMETA(DisplayName = "Forest Edge")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting"),
    Alert       UMETA(DisplayName = "Alert")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float FullDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    bool bUseBillboardBeyondLow = true;
};

USTRUCT(BlueprintType)
struct FCrowd_SpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    ECrowd_SpawnPattern SpawnPattern = ECrowd_SpawnPattern::Scattered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    int32 AgentCount = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    float MinSpacing = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    FVector MigrationDestination = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    FName SpeciesTag = NAME_None;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    int32 AgentID = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Grazing;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    int32 LODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FName SpeciesTag = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    bool bIsHerdLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FVector MigrationTarget = FVector::ZeroVector;
};

// ============================================================
// ACTOR CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Crowd Mass Spawner"))
class TRANSPERSONALGAME_API ACrowd_MassSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bAutoSpawnOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bDebugDrawEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    FCrowd_LODSettings DefaultLODSettings;

    // ---- Runtime State ----

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_AgentState> ActiveAgents;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnHerd(const FCrowd_SpawnRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius, float PanicIntensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentState> GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void DrawDebugAgentStates();

private:
    int32 NextAgentID = 0;

    TArray<FVector> GenerateSpawnPositions(const FCrowd_SpawnRequest& Request, int32 Count);
    FVector SnapToGround(const FVector& WorldLocation);
    void UpdateMigrationPaths(float DeltaTime);
    void UpdateLODForAllAgents();
    FVector ComputeSeparationForce(const FCrowd_AgentState& Agent);
    void SpawnDefaultHerds();
};
