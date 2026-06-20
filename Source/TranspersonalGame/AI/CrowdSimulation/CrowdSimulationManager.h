#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Guard       UMETA(DisplayName = "Guard")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Working     UMETA(DisplayName = "Working"),
    Socializing UMETA(DisplayName = "Socializing"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentRole Role;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float AlertLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed;

    FCrowd_AgentData()
        : HomeLocation(FVector::ZeroVector)
        , CurrentTarget(FVector::ZeroVector)
        , Role(ECrowd_AgentRole::Hunter)
        , State(ECrowd_AgentState::Idle)
        , AlertLevel(0.0f)
        , MoveSpeed(150.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FString WaypointName;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Radius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsDangerZone;

    FCrowd_WaypointData()
        : Location(FVector::ZeroVector)
        , WaypointName(TEXT("Waypoint"))
        , Radius(200.0f)
        , bIsDangerZone(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CrowdUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeRadius;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    float GlobalAlertLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    bool bTribeInPanic;

    // Waypoints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoints")
    TArray<FCrowd_WaypointData> Waypoints;

    // Agent registry
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agents")
    TArray<FCrowd_AgentData> AgentRegistry;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(FCrowd_AgentData AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerThreatAlert(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearThreatAlert();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_WaypointData GetNearestWaypoint(FVector FromLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetAgentCountByRole(ECrowd_AgentRole Role) const;

    UFUNCTION(BlueprintPure, Category = "Crowd")
    bool IsTribeInDanger() const;

    UFUNCTION(CallInEditor, Category = "Crowd")
    void DebugDrawCrowdState();

private:
    float TimeSinceLastUpdate;
    FVector LastKnownThreatLocation;
    bool bHasActiveThreat;
};
