#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdHerdBehavior.generated.h"

// ============================================================
// Crowd & Traffic Simulation — Agent #13
// CrowdHerdBehavior.h — Herd behavior component for dinosaur crowds
// Implements flocking, migration, panic response, and LOD scaling
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Regrouping      UMETA(DisplayName = "Regrouping"),
    Resting         UMETA(DisplayName = "Resting"),
    Drinking        UMETA(DisplayName = "Drinking")
};

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    AlphaLeader     UMETA(DisplayName = "Alpha Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Follower        UMETA(DisplayName = "Follower"),
    Juvenile        UMETA(DisplayName = "Juvenile"),
    Elder           UMETA(DisplayName = "Elder"),
    Sentinel        UMETA(DisplayName = "Sentinel")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full            UMETA(DisplayName = "Full Simulation"),
    Medium          UMETA(DisplayName = "Medium LOD"),
    Low             UMETA(DisplayName = "Low LOD"),
    Dormant         UMETA(DisplayName = "Dormant")
};

USTRUCT(BlueprintType)
struct FCrowd_FlockingParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentWeight = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float LeaderFollowWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float PanicSpeedMultiplier = 2.5f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    AActor* AgentActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_HerdRole Role = ECrowd_HerdRole::Follower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float PanicLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float Age = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::Full;
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
    bool bLoopRoute = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float SeasonalTriggerTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FString RouteName = TEXT("DefaultMigration");
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCrowdHerdBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdBehavior();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Herd State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|State")
    ECrowd_HerdState CurrentHerdState = ECrowd_HerdState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|State")
    int32 HerdSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|State")
    float HerdPanicThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|State")
    float GroupCohesionStrength = 1.0f;

    // ---- Agents ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agents")
    TArray<FCrowd_HerdAgent> HerdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agents")
    FCrowd_FlockingParams FlockingParams;

    // ---- Migration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FCrowd_MigrationRoute ActiveMigrationRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bIsMigrating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float MigrationSpeed = 400.0f;

    // ---- LOD ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODFullDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODMediumDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODLowDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::Full;

    // ---- Threat Response ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Threat")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Threat")
    float PanicPropagationRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Threat")
    float PanicDecayRate = 0.1f;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void TriggerPanic(FVector PanicOrigin, float PanicRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void StartMigration(const FCrowd_MigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void StopMigration();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void AddHerdAgent(AActor* AgentActor, ECrowd_HerdRole Role);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void RemoveHerdAgent(AActor* AgentActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd|State")
    ECrowd_HerdState GetCurrentHerdState() const { return CurrentHerdState; }

    UFUNCTION(BlueprintCallable, Category = "Crowd|State")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void UpdateLODForDistance(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    FVector ComputeFlockingVelocity(const FCrowd_HerdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    FVector ComputeFleeVector(const FCrowd_HerdAgent& Agent, FVector ThreatLocation);

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void DebugDrawHerd();

private:
    float TickAccumulator = 0.0f;
    float TickInterval = 0.1f; // 10Hz update

    FVector CurrentThreatLocation = FVector::ZeroVector;
    float CurrentThreatLevel = 0.0f;
    bool bHasThreat = false;

    void UpdateHerdState(float DeltaTime);
    void UpdateAgentPositions(float DeltaTime);
    void PropagateHerdPanic(float DeltaTime);
    void UpdateMigrationProgress(float DeltaTime);
    void UpdateLODChain();
};
