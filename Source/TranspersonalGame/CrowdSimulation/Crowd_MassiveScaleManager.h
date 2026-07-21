#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveScaleManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveScaleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    int32 MaxEntitiesPerChunk = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    float ChunkSize = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    int32 MaxActiveChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    float EntityCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    int32 LODLevels = 4;

    FCrowd_MassiveScaleConfig()
    {
        MaxEntitiesPerChunk = 1000;
        ChunkSize = 2000.0f;
        MaxActiveChunks = 25;
        EntityCullingDistance = 5000.0f;
        LODLevels = 4;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_ChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    FVector ChunkCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    int32 ActiveEntities = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    float LastUpdateTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Forest;

    FCrowd_ChunkData()
    {
        ChunkCenter = FVector::ZeroVector;
        ActiveEntities = 0;
        bIsActive = false;
        LastUpdateTime = 0.0f;
        BiomeType = ECrowd_BiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassiveScaleManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassiveScaleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* ManagementSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    FCrowd_MassiveScaleConfig ScaleConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    TArray<FCrowd_ChunkData> ActiveChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Scale")
    int32 TotalManagedEntities = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bDynamicLODEnabled = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void InitializeMassiveScale();

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void UpdateChunkSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void SpawnEntitiesInChunk(const FVector& ChunkCenter, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void CullDistantEntities();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustLODBasedOnPerformance();

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    int32 GetTotalActiveEntities() const { return TotalManagedEntities; }

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void SetMaxEntitiesPerChunk(int32 NewMax);

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void EnableEmergencyPerformanceMode();

private:
    void UpdatePerformanceMetrics();
    void OptimizeChunkDistribution();
    FVector GetPlayerLocation() const;
    bool ShouldActivateChunk(const FVector& ChunkCenter) const;
    void DeactivateDistantChunks();
};