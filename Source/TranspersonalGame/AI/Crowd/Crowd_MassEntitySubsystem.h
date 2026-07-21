#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassEntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance1 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance2 = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance3 = 3000.0f;

    FCrowd_MassEntityConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 2000.0f;
        LODDistance1 = 500.0f;
        LODDistance2 = 1500.0f;
        LODDistance3 = 3000.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void SpawnCrowdEntities(const FVector& Location, int32 Count = 100);

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void DespawnAllCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void UpdateCrowdLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void SetCrowdEntityConfig(const FCrowd_MassEntityConfig& NewConfig);

protected:
    UPROPERTY()
    FCrowd_MassEntityConfig CrowdConfig;

    UPROPERTY()
    TArray<FMassEntityHandle> ActiveCrowdEntities;

    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

private:
    void InitializeMassEntity();
    void CreateCrowdArchetype();
    void SpawnEntityAtLocation(const FVector& Location);
    void UpdateEntityLOD(const FMassEntityHandle& EntityHandle, float DistanceToPlayer);

    FMassArchetypeHandle CrowdArchetype;
    bool bIsInitialized = false;
};