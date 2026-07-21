#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMassAgentController.generated.h"

// ============================================================
// Crowd Mass Agent Controller — Agent #13 Crowd Simulation
// Manages up to 50,000 simultaneous crowd agents using
// lightweight tick-based simulation (Mass AI compatible)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentBehavior : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Sheltering      UMETA(DisplayName = "Sheltering"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_AgentSpecies : uint8
{
    HumanTribe      UMETA(DisplayName = "Human Tribe Member"),
    HadrosaurHerd   UMETA(DisplayName = "Hadrosaur Herd"),
    CeratopsHerd    UMETA(DisplayName = "Ceratops Herd"),
    OrnithopodFlock UMETA(DisplayName = "Ornithopod Flock"),
    PterosaurSwarm  UMETA(DisplayName = "Pterosaur Swarm"),
    SmallMammal     UMETA(DisplayName = "Small Mammal")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full            UMETA(DisplayName = "Full Simulation"),
    Reduced         UMETA(DisplayName = "Reduced Tick Rate"),
    Minimal         UMETA(DisplayName = "Position Only"),
    Culled          UMETA(DisplayName = "Culled — No Tick")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    int32 AgentID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentBehavior Behavior = ECrowd_AgentBehavior::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentSpecies Species = ECrowd_AgentSpecies::HumanTribe;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_LODLevel LOD = ECrowd_LODLevel::Full;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    float PanicLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    FVector FleeTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsVisible = true;
};

USTRUCT(BlueprintType)
struct FCrowd_ThreatSource
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Threat")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Threat")
    float Radius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Threat")
    float Intensity = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Threat")
    float ExpiresAt = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Threat")
    FString SourceTag = TEXT("Unknown");
};

USTRUCT(BlueprintType)
struct FCrowd_FlockingParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationRadius = 120.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentRadius = 350.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationWeight = 1.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionWeight = 0.8f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float MaxSpeed = 400.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float FleeSpeedMultiplier = 2.2f;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Crowd")
class TRANSPERSONALGAME_API ACrowdMassAgentController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdMassAgentController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 AgentsPerSpecies = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FullLODRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float ReducedLODRadius = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_FlockingParams FlockingParams;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_ThreatSource> ActiveThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 TotalAgentsSimulated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    float GlobalPanicLevel = 0.0f;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void SpawnAgentGroup(ECrowd_AgentSpecies Species, int32 Count, FVector CenterLocation, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void RegisterThreat(FVector Location, float Radius, float Intensity, FString Tag);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void ClearAllThreats();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    float GetPanicLevelAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetAgentCountInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    TArray<FCrowd_AgentData> GetFleeingAgentsNear(FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Debug")
    void DrawDebugAgents(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Debug")
    FString GetSimulationStats() const;

private:
    void TickAgents(float DeltaTime);
    void UpdateLOD(FCrowd_AgentData& Agent, const FVector& PlayerLocation);
    void UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void ApplyFlocking(FCrowd_AgentData& Agent, float DeltaTime);
    void PropagateFleeSignal(FCrowd_AgentData& Agent);
    void CleanExpiredThreats(float CurrentTime);
    FVector ComputeFleeDirection(const FVector& AgentPos) const;
    ECrowd_LODLevel ComputeLODForDistance(float Distance) const;

    bool bDebugDraw = false;
    float SimTime = 0.0f;
    int32 NextAgentID = 0;
    FVector LastPlayerLocation = FVector::ZeroVector;
};
