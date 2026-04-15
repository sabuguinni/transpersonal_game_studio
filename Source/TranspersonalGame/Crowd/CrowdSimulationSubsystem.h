#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "CrowdSimulationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowdEventDelegate, int32, CrowdID, ECrowd_BehaviorState, NewState);

/**
 * Crowd Simulation Subsystem for managing large-scale prehistoric tribal populations
 * Supports up to 50,000 simultaneous agents using Mass Entity framework
 * Handles tribal settlements, gathering groups, hunting parties, and migration patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 SpawnCrowdGroup(FVector Location, ECrowd_GroupType GroupType, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowdGroup(int32 CrowdID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehavior(int32 CrowdID, ECrowd_BehaviorState NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void MoveCrowdToLocation(int32 CrowdID, FVector TargetLocation);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalCrowdDensity(float DensityMultiplier);

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<int32> GetCrowdGroupsInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    ECrowd_BehaviorState GetCrowdBehavior(int32 CrowdID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Crowd Events")
    FCrowdEventDelegate OnCrowdBehaviorChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crowd Events")
    FCrowdEventDelegate OnCrowdGroupSpawned;

    UPROPERTY(BlueprintAssignable, Category = "Crowd Events")
    FCrowdEventDelegate OnCrowdGroupDespawned;

protected:
    // Crowd Group Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TMap<int32, FCrowd_GroupData> ActiveCrowdGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxConcurrentAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float HighDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float MediumDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LowDetailRadius;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMassEntityIntegration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

private:
    // Internal Management
    int32 NextCrowdID;
    FTimerHandle LODUpdateTimer;
    
    void UpdateLODSystem();
    void InitializeMassEntity();
    void CleanupCrowdGroups();
    
    // Mass Entity Integration
    class UMassEntitySubsystem* MassEntitySubsystem;
    TMap<int32, uint32> CrowdToMassEntityMap;
};