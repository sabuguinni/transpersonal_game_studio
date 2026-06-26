#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "CrowdSimulationManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Crowd & Traffic Simulation — Agent #13
// Manages prehistoric herd behaviour, stampedes, and territorial zones
// Up to 50,000 agents via Mass AI LOD chain
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Scattered   UMETA(DisplayName = "Scattered")
};

UENUM(BlueprintType)
enum class ECrowd_AgentLOD : uint8
{
    Full        UMETA(DisplayName = "Full Simulation"),   // <200m  — full physics + anim
    Medium      UMETA(DisplayName = "Medium LOD"),        // 200-800m — simplified movement
    Low         UMETA(DisplayName = "Low LOD"),           // 800-2000m — position only
    Culled      UMETA(DisplayName = "Culled")             // >2000m — data only, no render
};

UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pachycephalo    UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentLOD LODLevel = ECrowd_AgentLOD::Culled;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Parasaurolophus;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.0f;   // 0-1, drives flee/stampede

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float HungerLevel = 0.5f; // 0-1, drives migration to food

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Parasaurolophus;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdState State = ECrowd_HerdState::Grazing;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CentreOfMass = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float GroupFearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StampedeRadius = 2000.0f; // unreal units — danger zone for player
};

USTRUCT(BlueprintType)
struct FCrowd_ThreatZone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Centre = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Radius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ThreatIntensity = 1.0f; // 0-1

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ExpiryTime = 30.0f; // seconds until zone fades
};

// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_SimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_SimulationManager();

    // ── Configuration ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentsFullLOD = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentsMediumLOD = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentsLowLOD = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickIntervalSeconds = 0.1f; // 10Hz crowd update

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float StampedeThreshold = 0.75f; // fear level that triggers stampede

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed = 1200.0f; // cm/s during stampede

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float GrazeSpeed = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrateSpeed = 350.0f;

    // ── Runtime State ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdGroup> ActiveHerds;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_ThreatZone> ThreatZones;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 TotalActiveAgents = 0;

    // ── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterThreatAtLocation(FVector Location, float Radius, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerStampede(int32 HerdID, FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnHerd(ECrowd_HerdSpecies Species, FVector CentreLocation, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_HerdGroup GetNearestHerd(FVector FromLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool IsLocationInStampedeZone(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetAgentCountByLOD(ECrowd_AgentLOD LODLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdMigrationTarget(int32 HerdID, FVector Target);

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void DebugDrawAllHerds();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    float AccumulatedTime = 0.0f;
    int32 NextHerdID = 1;
    int32 NextAgentID = 1;

    TArray<FCrowd_HerdAgent> AgentPool; // flat pool — indexed by AgentID

    void UpdateHerdStates(float DeltaTime);
    void UpdateThreatZones(float DeltaTime);
    void UpdateLODChain();
    void PropagateFleeSignal(int32 HerdID, float FearDelta);
    ECrowd_AgentLOD ComputeLOD(const FVector& AgentLoc, const FVector& PlayerLoc) const;
    FVector ComputeFlockingVelocity(const FCrowd_HerdAgent& Agent,
                                    const FCrowd_HerdGroup& Herd) const;
};
