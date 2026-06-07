#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    bool bIsVisible;

    FCrowd_EntityData()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Speed = 150.0f;
        EntityID = -1;
        BehaviorState = ECrowd_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        DistanceToPlayer = 0.0f;
        bIsVisible = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassEntityChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    TArray<FCrowd_EntityData> Entities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    FVector ChunkCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    float ChunkRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    int32 ChunkID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    float LastUpdateTime;

    FCrowd_MassEntityChunk()
    {
        ChunkCenter = FVector::ZeroVector;
        ChunkRadius = 1000.0f;
        ChunkID = -1;
        bIsActive = false;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnMassEntities(int32 EntityCount, FVector CenterLocation, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateMassEntities(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void CullDistantEntities(FVector PlayerLocation, float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void ActivateChunk(int32 ChunkID);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void DeactivateChunk(int32 ChunkID);

    // Entity Queries
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    TArray<FCrowd_EntityData> GetEntitiesInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetTotalEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    float GetSystemPerformanceMetric() const;

    // Chunk Management
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void CreateEntityChunks(int32 ChunkCount, float ChunkSize);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateChunkActivation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    FCrowd_MassEntityChunk GetChunkByID(int32 ChunkID);

protected:
    // Mass Entity Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    TArray<FCrowd_MassEntityChunk> EntityChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    TArray<FCrowd_EntityData> ActiveEntities;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ChunkActivationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EntityCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableChunkSystem;

    // System State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    bool bIsInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    int32 TotalSpawnedEntities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    float AverageFrameTime;

private:
    // Internal Methods
    void UpdateEntityChunk(FCrowd_MassEntityChunk& Chunk, float DeltaTime);
    void ProcessEntityBehavior(FCrowd_EntityData& Entity, float DeltaTime);
    bool IsChunkInRange(const FCrowd_MassEntityChunk& Chunk, FVector PlayerLocation, float Range);
    void OptimizeEntityDistribution();
    void CalculatePerformanceMetrics(float DeltaTime);
};