#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High    UMETA(DisplayName = "High Detail"),
    Medium  UMETA(DisplayName = "Medium Detail"), 
    Low     UMETA(DisplayName = "Low Detail"),
    Culled  UMETA(DisplayName = "Culled")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorMode : uint8
{
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Trading     UMETA(DisplayName = "Trading"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxCrowdMembers = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    ECrowd_BehaviorMode DefaultBehavior = ECrowd_BehaviorMode::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ActivityLevel = 1.0f;

    FCrowd_SimulationZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        MaxCrowdMembers = 50;
        DefaultBehavior = ECrowd_BehaviorMode::Wandering;
        ActivityLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<int32> ConnectedNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float TrafficWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    ECrowd_BehaviorMode PreferredBehavior = ECrowd_BehaviorMode::Wandering;

    FCrowd_PathfindingNode()
    {
        Location = FVector::ZeroVector;
        ConnectedNodes.Empty();
        TrafficWeight = 1.0f;
        PreferredBehavior = ECrowd_BehaviorMode::Wandering;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Mass Entity Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    int32 MaxSimulatedEntities = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float SimulationRadius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    bool bEnableMassEntitySystem = true;

    // LOD System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float HighLODDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float MediumLODDistance = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LowLODDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float CullDistance = 12000.0f;

    // Simulation Zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Zones")
    TArray<FCrowd_SimulationZone> SimulationZones;

    // Pathfinding Network
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathfindingNode> PathfindingNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathUpdateInterval = 2.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxUpdatesPerFrame = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThrottleThreshold = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceThrottling = true;

private:
    // Internal state
    float LastPathUpdateTime = 0.0f;
    int32 CurrentUpdateIndex = 0;
    TArray<AActor*> ManagedCrowdActors;
    TMap<AActor*, ECrowd_LODLevel> ActorLODLevels;
    TMap<AActor*, ECrowd_BehaviorMode> ActorBehaviorModes;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SpawnCrowdInZone(const FCrowd_SimulationZone& Zone, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    void UpdateLODLevels(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    ECrowd_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdatePathfindingNetwork();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<int32> FindPath(int32 StartNodeIndex, int32 EndNodeIndex);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetCrowdBehavior(AActor* CrowdActor, ECrowd_BehaviorMode NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateCrowdBehaviors(float DeltaTime);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldThrottleUpdates() const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetTotalManagedActors() const { return ManagedCrowdActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetActorsInLODLevel(ECrowd_LODLevel LODLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FCrowd_SimulationZone* FindZoneAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawSimulationInfo();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Editor")
    void SetupDefaultZones();

    UFUNCTION(CallInEditor, Category = "Editor")
    void GeneratePathfindingNetwork();

    UFUNCTION(CallInEditor, Category = "Editor")
    void TestCrowdSpawning();
};