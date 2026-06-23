#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// LOD levels for crowd agents — distance-based rendering quality
UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full    UMETA(DisplayName = "Full Detail"),
    Medium  UMETA(DisplayName = "Medium Detail"),
    Low     UMETA(DisplayName = "Low Detail"),
    Culled  UMETA(DisplayName = "Culled")
};

// Crowd agent type — determines behaviour tree and animation set
UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    HerdAnimal   UMETA(DisplayName = "Herd Animal"),
    TribeMember  UMETA(DisplayName = "Tribe Member"),
    RaptorPack   UMETA(DisplayName = "Raptor Pack Member"),
    Scavenger    UMETA(DisplayName = "Scavenger")
};

// Struct for a single crowd agent state
USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::HerdAnimal;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Full;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 CurrentWaypointIndex = 0;
};

// Struct for herd migration data
USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TArray<FVector> MigrationPath;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdSize = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MigrationSpeed = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bMigrationActive = false;
};

/**
 * UCrowdSimulationManager
 * World subsystem managing prehistoric crowd simulation:
 * - Dinosaur herd migration along waypoint corridors
 * - Primitive tribe member camp behaviour
 * - Raptor pack coordinated patrol and flanking
 * - Distance-based LOD for up to 50,000 agents
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Waypoint Registration ---

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerdWaypoint(const FVector& WaypointLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Tribe")
    void RegisterTribeMemberLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Raptor")
    void RegisterRaptorPatrolNode(const FVector& NodeLocation);

    // --- Simulation Control ---

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void StartHerdMigration();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void StopHerdMigration();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Tribe")
    void ActivateTribeCamp();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Raptor")
    void ActivateRaptorPack();

    // --- Query ---

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FVector GetNextHerdWaypoint() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FVector GetNextPatrolNode() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetActiveCrowdAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    ECrowd_LODLevel GetLODLevelForDistance(float DistanceFromPlayer) const;

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxTribeMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxRaptorPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdMigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TribeMemberWanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float RaptorPatrolSpeed;

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bHerdMigrationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bTribeCampActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bRaptorPackActive;

private:
    TArray<FVector> HerdWaypoints;
    TArray<FVector> TribeMemberLocations;
    TArray<FVector> RaptorPatrolNodes;

    int32 CurrentHerdWaypointIndex = 0;
    int32 CurrentPatrolNodeIndex = 0;

    void TickHerdMigration(float DeltaTime);
    void TickTribeCamp(float DeltaTime);
    void TickRaptorPack(float DeltaTime);
};
