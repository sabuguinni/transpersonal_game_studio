#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxEntities = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float SpawnRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance0 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance1 = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance2 = 3000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Zone")
    float ZoneRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Zone")
    ECrowdBehaviorType BehaviorType = ECrowdBehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Zone")
    float DensityMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PanicResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float PanicDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float EvacuationSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    FVector EvacuationTarget;
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
    class UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    FCrowd_EntityConfig EntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TArray<FCrowd_BehaviorZone> BehaviorZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TArray<FCrowd_PanicResponse> PanicResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TArray<FVector> WaypointNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    bool bEnablePanicSystem = true;

private:
    UPROPERTY()
    TArray<class AActor*> SpawnedEntities;

    UPROPERTY()
    TArray<class AActor*> WaypointActors;

    UPROPERTY()
    TArray<class AActor*> ZoneActors;

    float LastUpdateTime = 0.0f;
    bool bSystemInitialized = false;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerPanicResponse(const FVector& TriggerLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void CreateWaypointNetwork();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void CreateBehaviorZones();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management", CallInEditor)
    void ValidateCrowdSystem();

    UFUNCTION(BlueprintPure, Category = "Crowd Management")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Management")
    float GetCrowdDensity(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintPure, Category = "Crowd Management")
    ECrowdBehaviorType GetZoneBehaviorType(const FVector& Location) const;
};