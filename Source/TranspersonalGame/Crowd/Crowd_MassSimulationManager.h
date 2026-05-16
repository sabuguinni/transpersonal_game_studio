#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

class UMassEntitySubsystem;
class UCrowd_FlockingComponent;

// Mass Entity archetype for crowd simulation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    FMassEntityTemplate EntityTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    ECrowd_BehaviorState DefaultBehaviorState = ECrowd_BehaviorState::Wandering;

    FCrowd_MassArchetype()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        DefaultBehaviorState = ECrowd_BehaviorState::Wandering;
    }
};

// Mass crowd spawn configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    int32 EntityCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    ECrowd_BehaviorState InitialBehavior = ECrowd_BehaviorState::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float MovementSpeed = 200.0f;

    FCrowd_SpawnConfig()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRadius = 2000.0f;
        EntityCount = 500;
        InitialBehavior = ECrowd_BehaviorState::Wandering;
        MovementSpeed = 200.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnPanicTriggered, FVector, ThreatLocation, float, ThreatRadius);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrowd_OnCrowdStateChanged, ECrowd_BehaviorState, NewState);

/**
 * Mass Entity-based crowd simulation manager for large-scale NPC behavior
 * Handles up to 50,000 simultaneous entities using UE5 Mass Entity framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassSimulationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass simulation management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool SpawnCrowdAtLocation(const FCrowd_SpawnConfig& SpawnConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowds();

    // Combat integration for panic responses
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPanicResponse(FVector ThreatLocation, float ThreatRadius, float PanicDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalCrowdBehavior(ECrowd_BehaviorState NewBehavior);

    // LOD and performance management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdLODDistance(float NearDistance, float FarDistance);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxActiveEntities(int32 MaxEntities);

    // Query and information
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Simulation")
    bool IsSimulationActive() const { return bSimulationActive; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Simulation")
    ECrowd_BehaviorState GetCurrentGlobalBehavior() const { return CurrentGlobalBehavior; }

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Crowd Events")
    FCrowd_OnPanicTriggered OnPanicTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Crowd Events")
    FCrowd_OnCrowdStateChanged OnCrowdStateChanged;

protected:
    // Mass Entity system references
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Crowd archetypes and configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Configuration")
    TArray<FCrowd_MassArchetype> CrowdArchetypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Configuration")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Configuration")
    float LODNearDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Configuration")
    float LODFarDistance = 5000.0f;

    // Current simulation state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation State")
    bool bSimulationActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation State")
    int32 CurrentActiveEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation State")
    ECrowd_BehaviorState CurrentGlobalBehavior = ECrowd_BehaviorState::Wandering;

    // Panic response system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float PanicSpeedMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float PanicFleeDistance = 3000.0f;

    UPROPERTY()
    FTimerHandle PanicTimerHandle;

private:
    // Internal management
    void SetupDefaultArchetypes();
    void CleanupMassEntities();
    void OnPanicTimerExpired();
    
    // Mass Entity tracking
    TArray<FMassEntityHandle> SpawnedEntities;
    TMap<FMassEntityHandle, FCrowd_SpawnConfig> EntityConfigurations;
};