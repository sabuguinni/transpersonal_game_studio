#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_ZoneType : uint8
{
    WateringHole    UMETA(DisplayName = "Watering Hole"),
    FeedingGround   UMETA(DisplayName = "Feeding Ground"),
    ShelterArea     UMETA(DisplayName = "Shelter Area"),
    RaptorTerritory UMETA(DisplayName = "Raptor Territory"),
    MigrationPath   UMETA(DisplayName = "Migration Path")
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    RaptorPack      UMETA(DisplayName = "Raptor Pack"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    ScavengerGroup  UMETA(DisplayName = "Scavenger Group"),
    SolitaryPredator UMETA(DisplayName = "Solitary Predator")
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WaitTimeSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlertRadius;

    FCrowd_WaypointData()
        : Location(FVector::ZeroVector)
        , WaitTimeSeconds(2.0f)
        , AlertRadius(500.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxAgentCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DensityWeight;

    FCrowd_ZoneConfig()
        : Center(FVector::ZeroVector)
        , Radius(800.0f)
        , ZoneType(ECrowd_ZoneType::WateringHole)
        , MaxAgentCapacity(50)
        , DensityWeight(1.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_AgentGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FCrowd_WaypointData> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsAlerted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection;

    FCrowd_AgentGroup()
        : AgentType(ECrowd_AgentType::HerbivoreHerd)
        , GroupSize(8)
        , MovementSpeed(300.0f)
        , bIsAlerted(false)
        , FleeDirection(FVector::ZeroVector)
    {}
};

/**
 * CrowdSimulationManager — manages prehistoric creature herds and pack behaviors.
 * Coordinates up to 50,000 agents using Mass AI waypoint-driven simulation.
 * Handles raptor pack tactics, herbivore herd migration, and zone-based density.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Crowd|Zones")
    void RegisterCrowdZone(const FCrowd_ZoneConfig& ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zones")
    FCrowd_ZoneConfig GetNearestZone(FVector WorldLocation, ECrowd_ZoneType ZoneType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zones")
    int32 GetZoneCurrentOccupancy(const FCrowd_ZoneConfig& Zone) const;

    // Agent group management
    UFUNCTION(BlueprintCallable, Category = "Crowd|Agents")
    void RegisterAgentGroup(const FCrowd_AgentGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Agents")
    void AlertNearbyGroups(FVector AlertOrigin, float AlertRadius, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Agents")
    void TriggerHerdFlee(FVector ThreatLocation, float FleeRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Agents")
    void TriggerPackHunt(FVector PreyLocation, ECrowd_AgentType PredatorType);

    // Waypoint navigation
    UFUNCTION(BlueprintCallable, Category = "Crowd|Navigation")
    FVector GetNextWaypoint(const FCrowd_AgentGroup& Group, int32 CurrentWaypointIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Navigation")
    bool IsWaypointReached(FVector AgentLocation, FVector WaypointLocation, float Tolerance = 100.0f) const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void UpdateAgentLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    int32 GetActiveLODLevel(float DistanceToPlayer) const;

    // Stats
    UFUNCTION(BlueprintPure, Category = "Crowd|Stats")
    int32 GetTotalActiveAgents() const { return TotalActiveAgents; }

    UFUNCTION(BlueprintPure, Category = "Crowd|Stats")
    int32 GetRegisteredZoneCount() const { return RegisteredZones.Num(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<FCrowd_ZoneConfig> RegisteredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<FCrowd_AgentGroup> RegisteredGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxTotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_HighDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_MediumDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_LowDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrationCycleHours;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime", meta = (AllowPrivateAccess = "true"))
    int32 TotalActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime", meta = (AllowPrivateAccess = "true"))
    float CurrentMigrationProgress;

private:
    void UpdateMigrationPaths(float DeltaTime);
    void ProcessAlertPropagation();
    void BalanceZoneDensity();
};
