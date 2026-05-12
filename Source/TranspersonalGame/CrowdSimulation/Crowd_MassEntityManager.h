#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Crowd_MassEntityManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Wandering UMETA(DisplayName = "Wandering"),
    Following UMETA(DisplayName = "Following"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Gathering UMETA(DisplayName = "Gathering"),
    Evacuating UMETA(DisplayName = "Evacuating")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None UMETA(DisplayName = "None"),
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float PerceptionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    bool bIsLeader;

    FCrowd_EntityData()
    {
        EntityID = 0;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        BehaviorState = ECrowd_BehaviorState::Idle;
        MovementSpeed = 150.0f;
        PerceptionRadius = 500.0f;
        GroupID = 0;
        bIsLeader = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxEntitiesPerLOD;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 3000.0f;
        LowDetailDistance = 8000.0f;
        MaxEntitiesPerLOD = 50;
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
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    ECrowd_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FVector> EvacuationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FVector> GatheringPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    int32 ActiveEntityCount;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(int32 Count, FVector CenterLocation, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetThreatLevel(ECrowd_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerEvacuation(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void AddEvacuationPoint(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void AddGatheringPoint(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    FCrowd_EntityData GetEntityData(int32 EntityID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateEntityBehavior(int32 EntityID, ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    void UpdateLODSystem(FVector ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    int32 GetLODLevel(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCrowdPerformanceMetric();

private:
    float LastUpdateTime;
    int32 NextEntityID;
    
    void UpdateEntityMovement(FCrowd_EntityData& Entity, float DeltaTime);
    void ProcessGroupBehavior(int32 GroupID);
    void HandleThreatResponse(FCrowd_EntityData& Entity, FVector ThreatLocation);
    FVector CalculateFlockingForce(const FCrowd_EntityData& Entity);
    FVector FindNearestEvacuationPoint(FVector FromLocation);
    void CullDistantEntities(FVector ViewerLocation);
};