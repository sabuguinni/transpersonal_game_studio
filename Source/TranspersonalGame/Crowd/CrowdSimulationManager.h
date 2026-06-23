#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class UNavigationSystemV1;

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdID;

    FCrowd_HerdAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , CurrentBehavior(ECrowd_HerdBehavior::Grazing)
        , Speed(200.f)
        , HerdID(0)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString PathName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float PathRadius;

    FCrowd_MigrationPath()
        : PathName(TEXT("DefaultPath"))
        , PathRadius(300.f)
    {}
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
    // Max simultaneous crowd agents (performance cap)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    // Current active agent count
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount;

    // Herd separation radius (boids algorithm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float SeparationRadius;

    // Herd cohesion radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float CohesionRadius;

    // Herd alignment radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float AlignmentRadius;

    // Migration paths defined in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FCrowd_MigrationPath> MigrationPaths;

    // All active herd agents
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdAgent> HerdAgents;

    // LOD distance — agents beyond this use simplified simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODSimplifiedDistance;

    // LOD distance — agents beyond this are culled from simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODCullDistance;

    // Panic radius — distance from threat that triggers flee behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Behavior")
    float PanicRadius;

    // Is simulation currently running
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

    // Spawn a new herd at location with given behavior
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnHerd(FVector SpawnLocation, int32 HerdSize, ECrowd_HerdBehavior InitialBehavior, int32 HerdID);

    // Trigger panic in all agents within radius of threat location
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerPanic(FVector ThreatLocation, float Radius);

    // Set behavior for all agents in a specific herd
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdBehavior(int32 HerdID, ECrowd_HerdBehavior NewBehavior);

    // Get agent count for a specific herd
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    int32 GetHerdAgentCount(int32 HerdID) const;

    // Start/stop simulation
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetSimulationActive(bool bActive);

    // Add a migration path
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void AddMigrationPath(const FCrowd_MigrationPath& Path);

    // Clear all agents
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllAgents();

private:
    // Update boids simulation for all agents
    void UpdateBoidsSimulation(float DeltaTime);

    // Update migration for agents on a path
    void UpdateMigration(FCrowd_HerdAgent& Agent, float DeltaTime);

    // Apply LOD — skip expensive updates for distant agents
    bool ShouldUpdateAgentFull(const FCrowd_HerdAgent& Agent) const;

    // Cached player location for LOD calculations
    FVector CachedPlayerLocation;

    // Timer for LOD player location update
    float LODUpdateTimer;
};
