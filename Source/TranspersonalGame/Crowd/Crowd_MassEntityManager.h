#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassAgentComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

class UMassEntitySubsystem;
class UMassSpawner;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float MinSpawnDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    bool bSpawnOnNavMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    TArray<TSoftObjectPtr<UStaticMesh>> CrowdMeshes;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseMovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxMovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float FleeDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bEnableFlocking = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxHighDetailEntities = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxMediumDetailEntities = 200;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Core Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(int32 EntityCount, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdTarget(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateEntityLOD(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FCrowd_LODSettings& NewLODSettings);

    // Crowd Behavior
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetMovementSettings(const FCrowd_MovementSettings& NewMovementSettings);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddCrowdWaypoint(FVector WaypointLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ClearCrowdWaypoints();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void TestSpawnCrowd();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_EntitySpawnSettings SpawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_MovementSettings MovementSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_LODSettings LODSettings;

    // Mass Entity System
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TArray<FMassEntityHandle> CrowdEntities;

    UPROPERTY()
    TArray<FVector> CrowdWaypoints;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bCrowdSystemInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 ActiveEntityCount;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentCrowdTarget;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bDebugVisualization;

private:
    // Internal Methods
    void InitializeMassEntitySystem();
    void CreateCrowdArchetype();
    void UpdateCrowdMovement(float DeltaTime);
    void ProcessFleeResponse(float DeltaTime);
    void DrawDebugVisualization();

    // Performance Tracking
    float LastLODUpdateTime;
    float LODUpdateInterval;
    int32 HighDetailEntityCount;
    int32 MediumDetailEntityCount;
    int32 LowDetailEntityCount;
};