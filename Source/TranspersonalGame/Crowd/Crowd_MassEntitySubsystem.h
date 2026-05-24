#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntitySubsystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float FlockingStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SeparationDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    FCrowd_MovementFragment() = default;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CuriosityLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HerdInstinct = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 BiomePreference = 0; // 0=Savana, 1=Forest, 2=Desert, 3=Swamp, 4=Mountain

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPredator = false;

    FCrowd_BehaviorFragment() = default;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_VisualFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    int32 LODLevel = 0;

    FCrowd_VisualFragment() = default;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass Entity crowd simulation methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(int32 EntityCount, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetBiomePreferences(int32 BiomeIndex, float AggressionMod, float FearMod);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowdEntities();

protected:
    // Mass Entity system references
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Crowd entity management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bEnableLODSystem = true;

    // Biome-specific crowd parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TArray<FVector> BiomeCenters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TArray<float> BiomeRadii;

    // Entity archetype storage
    TArray<FMassEntityHandle> CrowdEntities;
    FMassArchetypeHandle CrowdArchetype;

    // Internal methods
    void CreateCrowdArchetype();
    void ProcessCrowdMovement(float DeltaTime);
    void ProcessCrowdBehavior(float DeltaTime);
    void UpdateCrowdLOD();

    // Timer for periodic updates
    FTimerHandle CrowdUpdateTimer;
};