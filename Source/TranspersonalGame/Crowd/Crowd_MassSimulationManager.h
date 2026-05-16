#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MassEntityTypes.h"
#include "MassEntityManager.h"
#include "MassProcessor.h"
#include "MassSpawnerTypes.h"
#include "MassEntityConfigAsset.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

// Forward declarations
class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FString ArchetypeName = "DefaultCrowd";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    ECrowdBehaviorType BehaviorType = ECrowdBehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    bool bCanTakeDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float MaxHealth = 100.0f;

    FCrowd_EntityArchetype()
    {
        ArchetypeName = "DefaultCrowd";
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        SpawnCenter = FVector::ZeroVector;
        BehaviorType = ECrowdBehaviorType::Wandering;
        MovementSpeed = 300.0f;
        bCanTakeDamage = true;
        MaxHealth = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CombatZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PanicRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 EntitiesInZone = 0;

    FCrowd_CombatZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 2000.0f;
        PanicRadius = 5000.0f;
        bIsActive = false;
        CombatStartTime = 0.0f;
        EntitiesInZone = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdArchetype(const FCrowd_EntityArchetype& Archetype);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalEntityCount() const;

    // Combat Integration
    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void RegisterCombatZone(const FCrowd_CombatZone& CombatZone);

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void TriggerCombatPanic(FVector CombatLocation, float PanicRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void ApplyMassDamage(FVector DamageCenter, float DamageRadius, float DamageAmount);

    // Crowd Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetGlobalBehaviorMode(ECrowdBehaviorType NewBehaviorType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void UpdateCrowdDensity(FVector Location, float Radius, float DensityMultiplier);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistance(float NearLOD, float MidLOD, float FarLOD);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode(bool bEnable);

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void SpawnTestCrowd();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ClearAllCrowds();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintCrowdStats();

protected:
    // Mass Entity System References
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FCrowd_EntityArchetype> CrowdArchetypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxTotalEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TickInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceOptimization = true;

    // Combat Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FCrowd_CombatZone> ActiveCombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatPanicDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageProcessingRadius = 1000.0f;

    // Performance Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentEntityCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceModeActive = false;

private:
    // Internal state
    float LastTickTime = 0.0f;
    bool bIsInitialized = false;
    
    // Helper functions
    void UpdateCombatZones(float DeltaTime);
    void ProcessCrowdDamage(float DeltaTime);
    void OptimizePerformance(float DeltaTime);
    bool IsLocationInCombatZone(const FVector& Location) const;
    void UpdateEntityLOD(float DeltaTime);
};