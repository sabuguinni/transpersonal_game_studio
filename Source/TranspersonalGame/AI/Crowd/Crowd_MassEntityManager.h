#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassGameplayProcessor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 EntityCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float FlockingRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    ECrowd_BehaviorType BehaviorType = ECrowd_BehaviorType::Grazing;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxHighDetailEntities = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxMediumDetailEntities = 1000;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    TArray<FCrowd_HerdData> HerdConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    bool bEnableAvoidance = true;

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    TArray<FMassEntityHandle> ActiveEntities;
    float LastUpdateTime = 0.0f;
    int32 CurrentEntityCount = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void SpawnHerd(const FCrowd_HerdData& HerdData);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void SetHerdDestination(int32 HerdIndex, FVector NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    int32 GetActiveEntityCount() const { return CurrentEntityCount; }

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void ClearAllHerds();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestHerd();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugShowHerdStats();
};