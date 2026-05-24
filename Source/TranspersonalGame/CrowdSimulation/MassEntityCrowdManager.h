#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "MassEntityCrowdManager.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

UENUM(BlueprintType)
enum class ECrowdBehaviorType : uint8
{
    Gathering       UMETA(DisplayName = "Gathering"),
    Commerce        UMETA(DisplayName = "Commerce"),
    Residential     UMETA(DisplayName = "Residential"),
    Work            UMETA(DisplayName = "Work"),
    Ceremony        UMETA(DisplayName = "Ceremony"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Flee            UMETA(DisplayName = "Flee")
};

UENUM(BlueprintType)
enum class ECrowdDensityLevel : uint8
{
    Sparse          UMETA(DisplayName = "Sparse (1-10 agents)"),
    Light           UMETA(DisplayName = "Light (10-50 agents)"),
    Medium          UMETA(DisplayName = "Medium (50-200 agents)"),
    Dense           UMETA(DisplayName = "Dense (200-1000 agents)"),
    Massive         UMETA(DisplayName = "Massive (1000+ agents)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSpawnConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    ECrowdBehaviorType BehaviorType = ECrowdBehaviorType::Gathering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    ECrowdDensityLevel DensityLevel = ECrowdDensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxAgents = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    bool bUseNavMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance2 = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance3 = 5000.0f;

    FCrowdSpawnConfiguration()
    {
        BehaviorType = ECrowdBehaviorType::Gathering;
        DensityLevel = ECrowdDensityLevel::Medium;
        MaxAgents = 200;
        SpawnRadius = 500.0f;
        SpawnCenter = FVector::ZeroVector;
        bUseNavMesh = true;
        MovementSpeed = 150.0f;
        LODDistance1 = 1000.0f;
        LODDistance2 = 2500.0f;
        LODDistance3 = 5000.0f;
    }
};

/**
 * Mass Entity Crowd Manager - Handles large-scale crowd simulation using UE5's Mass Entity framework
 * Supports up to 50,000 simultaneous agents with LOD-based optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AMassEntityCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    AMassEntityCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Crowd Management Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowd(const FCrowdSpawnConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void DespawnCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior(ECrowdBehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDensity(ECrowdDensityLevel NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void PauseCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void ResumeCrowdSimulation();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODDistances(float LOD1, float LOD2, float LOD3);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetPlayerLocation(const FVector& PlayerPos);

    // Event Response
    UFUNCTION(BlueprintCallable, Category = "Event Response")
    void TriggerCrowdReaction(const FVector& EventLocation, float EventRadius, ECrowdBehaviorType ReactionType);

    UFUNCTION(BlueprintCallable, Category = "Event Response")
    void SetCrowdFleeTarget(const FVector& FleeFromLocation);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetVisibleAgentCount() const;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    FCrowdSpawnConfiguration DefaultConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TArray<FCrowdSpawnConfiguration> SpawnConfigurations;

    // Mass Entity References
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bCrowdActive;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    int32 CurrentAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    ECrowdBehaviorType CurrentBehavior;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    FVector PlayerLocation;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleAgentCount;

    // Mass Entity Handles
    TArray<FMassEntityHandle> SpawnedEntities;

private:
    // Internal Methods
    void InitializeMassEntitySystems();
    void CreateCrowdEntities(const FCrowdSpawnConfiguration& Config);
    void UpdateAgentLOD();
    void ProcessCrowdBehavior(float DeltaTime);
    void CleanupCrowdEntities();
    
    // Behavior Implementation
    void ImplementGatheringBehavior();
    void ImplementCommerceBehavior();
    void ImplementResidentialBehavior();
    void ImplementWorkBehavior();
    void ImplementCeremonyBehavior();
    void ImplementPatrolBehavior();
    void ImplementFleeBehavior();

    // Performance Optimization
    void OptimizeForPerformance();
    void UpdatePerformanceMetrics();
    
    // Configuration Validation
    bool ValidateSpawnConfiguration(const FCrowdSpawnConfiguration& Config) const;
    
    // Timer Handles
    FTimerHandle LODUpdateTimer;
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle PerformanceUpdateTimer;
};