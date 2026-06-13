#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_TrafficFlowManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficFlow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    float FlowIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    TArray<AActor*> AssignedEntities;

    FCrowd_TrafficFlow()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        FlowIntensity = 1.0f;
        MaxEntities = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_DensityZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Zone")
    ECrowd_DensityLevel DensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Zone")
    int32 CurrentEntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Zone")
    int32 MaxEntityCount;

    FCrowd_DensityZone()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 500.0f;
        DensityLevel = ECrowd_DensityLevel::Medium;
        CurrentEntityCount = 0;
        MaxEntityCount = 20;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_TrafficFlowManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_TrafficFlowManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Management")
    TArray<FCrowd_TrafficFlow> TrafficFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Management")
    TArray<FCrowd_DensityZone> DensityZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Management")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Management")
    int32 MaxSimulatedEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Management")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Management")
    bool bEnableTrafficSimulation;

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void InitializeTrafficFlows();

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void CreateTrafficFlow(FVector StartPos, FVector EndPos, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void UpdateEntityMovement();

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void AssignEntitiesToFlows();

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void UpdateDensityZones();

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    int32 GetEntityCountInZone(const FCrowd_DensityZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    bool IsLocationInDensityZone(FVector Location, const FCrowd_DensityZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void OptimizeTrafficFlow();

    UFUNCTION(BlueprintCallable, Category = "Traffic Management", CallInEditor)
    void DebugDrawTrafficFlows();

private:
    UPROPERTY()
    TArray<AActor*> ManagedEntities;

    UPROPERTY()
    float LastUpdateTime;

    void FindAndRegisterCrowdEntities();
    void MoveEntityAlongFlow(AActor* Entity, const FCrowd_TrafficFlow& Flow, float DeltaTime);
    FVector CalculateFlowDirection(const FCrowd_TrafficFlow& Flow, FVector CurrentPos);
    void BalanceEntityDistribution();
};