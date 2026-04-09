#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "Engine/World.h"
#include "MassCrowdSubsystem.generated.h"

class UMassEntityConfigAsset;
class UZoneGraphSubsystem;
class AMassSpawner;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdAgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AgentRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdDensityZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetDensity = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentDensity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRate = 1.0f;
};

/**
 * Mass Entity-based crowd simulation subsystem for managing up to 50,000 agents
 * Handles spawning, LOD management, and behavioral coordination of crowd entities
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMassCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UMassCrowdSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(const TArray<FCrowdAgentData>& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowdAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensityZone(const FCrowdDensityZone& DensityZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalCrowdBehavior(const FString& BehaviorName);

    // Emergency Response
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerCrowdEmergencyResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdEvacuationTarget(const FVector& EvacuationPoint);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdLODDistance(float NearDistance, float MidDistance, float FarDistance);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableCrowdCulling(bool bEnable);

protected:
    // Mass Entity Management
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> EntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> SpawnerSubsystem;

    UPROPERTY()
    TObjectPtr<UZoneGraphSubsystem> ZoneGraphSubsystem;

    // Crowd Configuration
    UPROPERTY(EditDefaultsOnly, Category = "Crowd Config")
    TObjectPtr<UMassEntityConfigAsset> CrowdEntityConfig;

    UPROPERTY(EditDefaultsOnly, Category = "Crowd Config")
    int32 MaxCrowdAgents = 50000;

    UPROPERTY(EditDefaultsOnly, Category = "Crowd Config")
    float UpdateFrequency = 30.0f;

    // Density Management
    UPROPERTY()
    TArray<FCrowdDensityZone> DensityZones;

    UPROPERTY()
    int32 CurrentCrowdCount = 0;

    // Emergency State
    UPROPERTY()
    bool bEmergencyActive = false;

    UPROPERTY()
    FVector EmergencyThreatLocation = FVector::ZeroVector;

    UPROPERTY()
    float EmergencyThreatRadius = 0.0f;

    UPROPERTY()
    FVector EvacuationTarget = FVector::ZeroVector;

private:
    void InitializeMassEntity();
    void SetupCrowdProcessors();
    void UpdateDensityZones();
    
    FTimerHandle DensityUpdateTimer;
    FTimerHandle CrowdUpdateTimer;
};