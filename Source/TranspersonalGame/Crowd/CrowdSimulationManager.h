#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_ZoneType : uint8
{
    WateringHole    UMETA(DisplayName = "Watering Hole"),
    ForestEdge      UMETA(DisplayName = "Forest Edge"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    HuntingGround   UMETA(DisplayName = "Hunting Ground"),
    MigrationPath   UMETA(DisplayName = "Migration Path")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Drinking    UMETA(DisplayName = "Drinking")
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ZoneType ZoneType = ECrowd_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxAgentsInZone = 20;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    ECrowd_ZoneType ZoneType = ECrowd_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    int32 MaxAgents = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float AgentSpawnInterval = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    bool IsLocationInZone(const FVector& TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    FVector GetRandomLocationInZone() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Zone")
    int32 GetCurrentAgentCount() const { return CurrentAgentCount; }

private:
    UPROPERTY()
    int32 CurrentAgentCount = 0;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Zone Configuration ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zones")
    TArray<FCrowd_WaypointData> CrowdZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zones")
    int32 MaxTotalAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zones")
    float LOD_NearDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zones")
    float LOD_FarDistance = 8000.0f;

    // ── Herd Behaviour ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdCohesionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdSeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float PredatorFleeRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationTriggerInterval = 300.0f;

    // ── Runtime State ───────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    int32 ActiveAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastMigration = 0.0f;

    // ── Public API ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdZone(const FCrowd_WaypointData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerHerdFlee(FVector PredatorLocation, float FleeRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerMigration(FVector MigrationTarget);

    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetActiveAgentCount() const { return ActiveAgentCount; }

    UFUNCTION(BlueprintPure, Category = "Crowd")
    FCrowd_WaypointData GetNearestZone(const FVector& FromLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetLODDistances(float NearDist, float FarDist);

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void DebugDrawZones();

private:
    void UpdateAgentLOD(float DeltaTime);
    void UpdateHerdBehaviour(float DeltaTime);
    void CheckMigrationCycle(float DeltaTime);

    UPROPERTY()
    TArray<FCrowd_AgentData> AgentPool;
};
