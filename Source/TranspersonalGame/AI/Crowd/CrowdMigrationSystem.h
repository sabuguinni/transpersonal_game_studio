#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMigrationSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_MigrationState : uint8
{
    Foraging        UMETA(DisplayName = "Foraging"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Resting         UMETA(DisplayName = "Resting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Drinking        UMETA(DisplayName = "Drinking"),
};

UENUM(BlueprintType)
enum class ECrowd_FormationType : uint8
{
    VFormation      UMETA(DisplayName = "V Formation"),
    LineAbreast     UMETA(DisplayName = "Line Abreast"),
    Scattered       UMETA(DisplayName = "Scattered"),
    Circular        UMETA(DisplayName = "Circular"),
    Column          UMETA(DisplayName = "Column"),
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_MigrationState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 AgentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsLeader;

    FCrowd_HerdAgent()
        : CurrentLocation(FVector::ZeroVector)
        , TargetLocation(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_MigrationState::Foraging)
        , MoveSpeed(300.0f)
        , LODDistance(1500.0f)
        , AgentIndex(0)
        , bIsLeader(false)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Route")
    float WaypointRadius;

    FCrowd_MigrationRoute()
        : CurrentWaypointIndex(0)
        , bIsLooping(true)
        , WaypointRadius(200.0f)
    {}
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdMigrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdMigrationSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Herd configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FormationSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatDetectionRadius;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD_NearDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD_MidDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD_FarDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 LOD_NearTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 LOD_MidTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 LOD_FarTickRate;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdAgent> HerdAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FCrowd_MigrationRoute ActiveRoute;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_MigrationState CurrentHerdState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bThreatDetected;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector ThreatLocation;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void InitializeHerd(int32 Count, ECrowd_FormationType Formation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void SetMigrationRoute(const TArray<FVector>& Waypoints, bool bLoop);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void TriggerFleeResponse(FVector ThreatPos, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void TriggerStampede(FVector Direction, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void SetHerdState(ECrowd_MigrationState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    int32 GetAgentLODLevel(int32 AgentIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    float GetDistanceToPlayer(int32 AgentIndex) const;

    UFUNCTION(BlueprintPure, Category = "Crowd|State")
    bool IsHerdFleeing() const { return CurrentHerdState == ECrowd_MigrationState::Fleeing; }

    UFUNCTION(BlueprintPure, Category = "Crowd|State")
    int32 GetActiveAgentCount() const { return HerdAgents.Num(); }

private:
    void UpdateAgentPositions(float DeltaTime);
    void UpdateFormation();
    void UpdateLOD();
    FVector ComputeFormationOffset(int32 AgentIndex) const;
    FVector GetLeaderPosition() const;
    bool CheckWaypointReached() const;
    void AdvanceWaypoint();

    UPROPERTY()
    AActor* CachedPlayerActor;

    float LODUpdateTimer;
    float FormationUpdateTimer;
    int32 FrameCounter;
};
