#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class UCrowd_AgentComponent;
class ACrowd_SpawnPoint;
class ACrowd_Waypoint;

UENUM(BlueprintType)
enum class ECrowd_BehaviorType : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"), 
    Following   UMETA(DisplayName = "Following"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Working     UMETA(DisplayName = "Working")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Impostor    UMETA(DisplayName = "Impostor"),
    Hidden      UMETA(DisplayName = "Hidden")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_BehaviorType BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 TargetWaypointIndex;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        BehaviorType = ECrowd_BehaviorType::Idle;
        LODLevel = ECrowd_LODLevel::High;
        Speed = 150.0f;
        TargetWaypointIndex = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    ECrowd_BehaviorType DefaultBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSubclassOf<APawn> AgentClass;

    FCrowd_SpawnSettings()
    {
        MaxAgents = 50;
        SpawnRadius = 500.0f;
        DefaultBehavior = ECrowd_BehaviorType::Wandering;
        AgentClass = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_SpawnSettings SpawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceHigh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceMedium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceLow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceImpostor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<ACrowd_SpawnPoint*> SpawnPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<ACrowd_Waypoint*> Waypoints;

    // Core functionality
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdAgents(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void RegisterSpawnPoint(ACrowd_SpawnPoint* SpawnPoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void RegisterWaypoint(ACrowd_Waypoint* Waypoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    ECrowd_LODLevel CalculateLODLevel(const FVector& AgentPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    FVector GetNearestWaypoint(const FVector& Position) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawCrowdInfo();

private:
    float LastUpdateTime;
    APawn* PlayerPawn;

    void UpdateAgentLOD(FCrowd_AgentData& Agent);
    void UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    FVector CalculateSteeringForce(const FCrowd_AgentData& Agent);
    void FindNearbyAgents(const FCrowd_AgentData& Agent, TArray<FCrowd_AgentData*>& NearbyAgents);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    ACrowdSimulationManager* GetCrowdManager();

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void RegisterCrowdManager(ACrowdSimulationManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    int32 GetTotalCrowdAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdPerformance();

private:
    UPROPERTY()
    ACrowdSimulationManager* CrowdManager;

    UPROPERTY()
    TArray<ACrowdSimulationManager*> AllCrowdManagers;
};

#include "CrowdSimulationManager.generated.h"