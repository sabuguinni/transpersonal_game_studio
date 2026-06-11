#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Crowd_MassEntitySubsystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorType : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Gather      UMETA(DisplayName = "Gather Resources"),
    Social      UMETA(DisplayName = "Social Interaction"),
    Guard       UMETA(DisplayName = "Guard Position"),
    Travel      UMETA(DisplayName = "Travel Between Points"),
    Flee        UMETA(DisplayName = "Flee from Danger")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    ECrowd_BehaviorType BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    int32 GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    bool bIsActive;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        BehaviorType = ECrowd_BehaviorType::Idle;
        LODLevel = ECrowd_LODLevel::High;
        Speed = 100.0f;
        GroupID = 0;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    ECrowd_BehaviorType DefaultBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float EntityDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<FVector> WaypointPath;

    FCrowd_SimulationZone()
    {
        Center = FVector::ZeroVector;
        Radius = 500.0f;
        DefaultBehavior = ECrowd_BehaviorType::Idle;
        MaxEntities = 50;
        EntityDensity = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Tick interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

    // Crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 SpawnCrowdEntities(const FCrowd_SimulationZone& Zone, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowdEntities(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateEntityBehavior(int32 EntityID, ECrowd_BehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetEntityDestination(int32 EntityID, const FVector& Destination);

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    int32 CreateSimulationZone(const FCrowd_SimulationZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void RemoveSimulationZone(int32 ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void UpdateZoneParameters(int32 ZoneID, const FCrowd_SimulationZone& NewZoneData);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODLevels(const FVector& ViewerPosition);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODDistances(float HighDetailDistance, float MediumDetailDistance, float LowDetailDistance);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSimulationFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxEntityCount(int32 MaxCount);

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawSimulationZones();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SimulationTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime;

private:
    TArray<FCrowd_EntityData> CrowdEntities;
    TArray<FCrowd_SimulationZone> SimulationZones;
    
    int32 NextEntityID;
    int32 NextZoneID;
    float LastTickTime;
    float AccumulatedFrameTime;
    int32 FrameTimeCounter;
    
    bool bIsSimulationActive;
    
    // Internal methods
    void UpdateEntityMovement(FCrowd_EntityData& Entity, float DeltaTime);
    void UpdateEntityBehaviors(float DeltaTime);
    void ProcessCollisionAvoidance();
    void OptimizePerformance();
    ECrowd_LODLevel CalculateLODLevel(const FVector& EntityPosition, const FVector& ViewerPosition) const;
    FVector CalculateFlockingForce(const FCrowd_EntityData& Entity) const;
    FVector CalculateAvoidanceForce(const FCrowd_EntityData& Entity) const;
    FVector CalculateSeekForce(const FCrowd_EntityData& Entity, const FVector& Target) const;
};