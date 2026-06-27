#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdBehaviorZone.generated.h"

UENUM(BlueprintType)
enum class ECrowd_ZoneType : uint8
{
    WateringHole    UMETA(DisplayName = "Watering Hole"),
    NestingGround   UMETA(DisplayName = "Nesting Ground"),
    MigrationPath   UMETA(DisplayName = "Migration Path"),
    FeedingArea     UMETA(DisplayName = "Feeding Area"),
    FleeZone        UMETA(DisplayName = "Flee Zone"),
    TerritoryBorder UMETA(DisplayName = "Territory Border")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Nesting     UMETA(DisplayName = "Nesting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alert       UMETA(DisplayName = "Alert"),
    Migrating   UMETA(DisplayName = "Migrating")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString SpeciesTag;

    FCrowd_AgentData()
        : Location(FVector::ZeroVector)
        , State(ECrowd_AgentState::Idle)
        , FearLevel(0.0f)
        , HungerLevel(0.5f)
        , ThirstLevel(0.5f)
        , AgentID(-1)
        , SpeciesTag(TEXT("Unknown"))
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxAgentCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsActive;

    FCrowd_ZoneConfig()
        : ZoneType(ECrowd_ZoneType::FeedingArea)
        , ZoneRadius(500.0f)
        , MaxAgentCapacity(20)
        , DangerLevel(0.0f)
        , bIsActive(true)
    {}
};

/**
 * ACrowd_BehaviorZone — defines an area where crowd agents exhibit specific behaviors.
 * Placed in the world to create ecological hotspots: watering holes, nesting grounds,
 * migration corridors, feeding areas. Agents within the zone radius adopt the zone behavior.
 */
UCLASS(ClassGroup = "Crowd", meta = (DisplayName = "Crowd Behavior Zone"))
class TRANSPERSONALGAME_API ACrowd_BehaviorZone : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_BehaviorZone();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Zone configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    FCrowd_ZoneConfig ZoneConfig;

    /** Agents currently inside this zone */
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Zone")
    TArray<FCrowd_AgentData> ActiveAgents;

    /** Trigger danger response — all agents in zone enter flee state */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void TriggerDangerResponse(float DangerIntensity);

    /** Add an agent to this zone */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void RegisterAgent(const FCrowd_AgentData& AgentData);

    /** Remove an agent from this zone */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void UnregisterAgent(int32 AgentID);

    /** Get current occupancy ratio (0.0 - 1.0) */
    UFUNCTION(BlueprintPure, Category = "Crowd|Zone")
    float GetOccupancyRatio() const;

    /** Check if zone is at capacity */
    UFUNCTION(BlueprintPure, Category = "Crowd|Zone")
    bool IsAtCapacity() const;

    /** Get zone center in world space */
    UFUNCTION(BlueprintPure, Category = "Crowd|Zone")
    FVector GetZoneCenter() const;

private:
    /** Sphere component for zone boundary visualization */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Zone", meta = (AllowPrivateAccess = "true"))
    class USphereComponent* ZoneBoundary;

    /** Time accumulator for periodic behavior updates */
    float BehaviorUpdateTimer;

    /** How often to update agent behaviors (seconds) */
    static constexpr float BehaviorUpdateInterval = 2.0f;

    /** Update all agents in zone based on zone type */
    void UpdateAgentBehaviors();

    /** Apply watering hole behavior to agents */
    void ApplyWateringHoleBehavior();

    /** Apply nesting ground behavior to agents */
    void ApplyNestingBehavior();

    /** Apply flee behavior to all agents */
    void ApplyFleeBehavior(float Intensity);
};
