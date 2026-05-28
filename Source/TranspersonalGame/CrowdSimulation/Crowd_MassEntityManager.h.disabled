#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingTypes.h"
#include "MassEntityTemplate.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntityManager.generated.h"

// Mass Entity configuration for crowd simulation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassEntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntities = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance = 2000.0f;

    FCrowd_MassEntityConfig()
    {
        MaxEntities = 10000;
        SpawnRadius = 5000.0f;
        MovementSpeed = 300.0f;
        LODDistance = 2000.0f;
    }
};

// Mass Entity crowd behavior states
UENUM(BlueprintType)
enum class ECrowd_MassBehaviorState : uint8
{
    Idle,
    Wandering,
    Fleeing,
    Gathering,
    Following,
    Panicking
};

/**
 * Mass Entity Manager for large-scale crowd simulation
 * Handles 10,000+ entities using UE5 Mass Entity framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mass Entity configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FCrowd_MassEntityConfig EntityConfig;

    // Current behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    ECrowd_MassBehaviorState CurrentBehaviorState;

    // Entity template for crowd members
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    class UMassEntityTemplate* CrowdEntityTemplate;

    // Spawner subsystem reference
    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    class UMassSpawnerSubsystem* SpawnerSubsystem;

    // Entity subsystem reference
    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    class UMassEntitySubsystem* EntitySubsystem;

    // Core Mass Entity functions
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnCrowdEntities(int32 EntityCount, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateCrowdBehavior(ECrowd_MassBehaviorState NewBehaviorState);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetCrowdTarget(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void TriggerCrowdPanic(FVector PanicSource, float PanicRadius);

    // Performance and LOD management
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    float GetCurrentPerformanceMetric() const;

protected:
    // Internal Mass Entity management
    void CreateEntityTemplate();
    void RegisterMassProcessors();
    void UpdateEntityBehaviors(float DeltaTime);
    void HandleEntityCulling();

    // Performance tracking
    float LastPerformanceCheck;
    int32 ActiveEntityCount;
    float AverageFrameTime;

    // Mass Entity handles
    TArray<FMassEntityHandle> SpawnedEntities;
    
    // Behavior targets
    FVector CrowdTargetLocation;
    bool bHasCrowdTarget;
    
    // Panic system
    FVector PanicSourceLocation;
    float PanicRadius;
    bool bInPanicMode;
    float PanicStartTime;
};