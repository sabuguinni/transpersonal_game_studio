#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float MinSpawnDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float DespawnDistance = 5000.0f;

    FCrowd_SpawnConfiguration()
    {
        MaxEntities = 1000;
        SpawnRadius = 2000.0f;
        MinSpawnDistance = 100.0f;
        DespawnDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float HighLODDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float MediumLODDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float LowLODDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float CullDistance = 5000.0f;

    FCrowd_LODConfiguration()
    {
        HighLODDistance = 500.0f;
        MediumLODDistance = 1500.0f;
        LowLODDistance = 3000.0f;
        CullDistance = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Crowd management functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(const FVector& CenterLocation, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void ClearAllCrowdEntities();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_SpawnConfiguration SpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_LODConfiguration LODConfig;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 ActiveEntityCount;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentDensityMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsCrowdSystemActive;

private:
    void UpdateEntityLOD(int32 EntityIndex, float DistanceToViewer);
    void CullDistantEntities(const FVector& ViewerLocation);
    void SpawnEntityAtLocation(const FVector& Location);
    void DespawnEntity(int32 EntityIndex);

    TArray<FVector> EntityLocations;
    TArray<int32> EntityLODLevels;
    TArray<bool> EntityActiveStates;
};