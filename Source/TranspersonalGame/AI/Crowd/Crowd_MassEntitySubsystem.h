#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

// Forward declarations
class UCrowd_MassEntityManager;
class AActor;

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
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_AgentState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_LODLevel LODLevel;

    FCrowd_EntityData()
    {
        EntityID = -1;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        CurrentState = ECrowd_AgentState::Idle;
        LODLevel = ECrowd_LODLevel::High;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    ECrowd_DensityLevel DensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    bool bIsActive;

    FCrowd_SpawnZone()
    {
        ZoneName = TEXT("DefaultZone");
        Center = FVector::ZeroVector;
        Radius = 500.0f;
        MaxAgents = 100;
        DensityLevel = ECrowd_DensityLevel::Medium;
        bIsActive = true;
    }
};

/**
 * Mass Entity Subsystem for managing large-scale crowd simulation
 * Handles up to 50,000 simultaneous agents using UE5 Mass Entity framework
 */
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

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool SpawnCrowdAgents(const FCrowd_SpawnZone& SpawnZone, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowdAgents(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateAgentLOD(float DeltaTime);

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterSpawnZone(const FCrowd_SpawnZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UnregisterSpawnZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_SpawnZone GetSpawnZone(const FString& ZoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_SpawnZone> GetAllActiveZones() const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxCrowdAgents(int32 NewMaxAgents);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetCurrentAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OptimizePerformance();

    // Pathfinding Integration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdatePathfinding(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool FindPathForAgent(int32 EntityID, const FVector& TargetLocation);

    // Event Handling
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OnPlayerEnterZone(const FString& ZoneName, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OnPlayerExitZone(const FString& ZoneName, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OnDangerEvent(const FVector& DangerLocation, float DangerRadius);

protected:
    // Core system properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    float TickRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    bool bSystemInitialized;

    // Spawn zones
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Zones", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_SpawnZone> SpawnZones;

    // Active entities
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_EntityData> ActiveEntities;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    float LODDistanceNear;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    float LODUpdateFrequency;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 CurrentAgentCount;

private:
    // Internal management
    void UpdateEntityStates(float DeltaTime);
    void ProcessSpawnRequests();
    void ProcessDespawnRequests();
    void UpdateLODLevels();
    void HandlePerformanceOptimization();

    // Helper functions
    FCrowd_SpawnZone* FindSpawnZone(const FString& ZoneName);
    bool IsLocationInZone(const FVector& Location, const FCrowd_SpawnZone& Zone) const;
    ECrowd_LODLevel CalculateLODLevel(const FVector& AgentLocation, const FVector& ViewerLocation) const;

    // Timers and tracking
    FTimerHandle LODUpdateTimer;
    FTimerHandle PerformanceOptimizationTimer;
    
    TArray<FString> PendingSpawnZones;
    TArray<FString> PendingDespawnZones;
};