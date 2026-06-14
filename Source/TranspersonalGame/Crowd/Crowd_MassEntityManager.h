#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High     UMETA(DisplayName = "High Detail"),
    Medium   UMETA(DisplayName = "Medium Detail"), 
    Low      UMETA(DisplayName = "Low Detail"),
    Culled   UMETA(DisplayName = "Culled")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorMode : uint8
{
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Following   UMETA(DisplayName = "Following")
};

USTRUCT(BlueprintType)
struct FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_BehaviorMode BehaviorMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EntityID;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        LODLevel = ECrowd_LODLevel::High;
        BehaviorMode = ECrowd_BehaviorMode::Wandering;
        DistanceToPlayer = 0.0f;
        EntityID = -1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mass Entity System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float UpdateFrequency;

    // LOD System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float HighLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float MediumLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LowLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float CullDistance;

    // Pathfinding
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<ATargetPoint*> WaypointNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float WaypointRadius;

    // Behavior System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float GatherDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 EntitiesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceThrottling;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InfluenceRadius;

    // Crowd Data
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_EntityData> CrowdEntities;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    ATargetPoint* GetNearestWaypoint(const FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetRandomWaypointPosition();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TriggerFleeResponse(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TriggerGatherResponse(const FVector& GatherLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPerformanceMetric() const;

private:
    // Internal tracking
    float LastUpdateTime;
    int32 CurrentUpdateIndex;
    APawn* PlayerPawn;

    // Internal methods
    void UpdateEntityLOD(FCrowd_EntityData& Entity);
    void UpdateEntityBehavior(FCrowd_EntityData& Entity, float DeltaTime);
    FVector CalculateFleeDirection(const FVector& EntityPos, const FVector& ThreatPos);
    FVector CalculateWanderDirection(const FCrowd_EntityData& Entity);
    void OptimizePerformance();
};