#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "Crowd_BehaviorComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxForce;

    FCrowd_BehaviorParams()
    {
        SeparationRadius = 100.0f;
        AlignmentRadius = 200.0f;
        CohesionRadius = 150.0f;
        SeparationWeight = 2.0f;
        AlignmentWeight = 1.0f;
        CohesionWeight = 1.0f;
        MaxSpeed = 200.0f;
        MaxForce = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Flocking")
    FVector SeparationForce;

    UPROPERTY(BlueprintReadWrite, Category = "Flocking")
    FVector AlignmentForce;

    UPROPERTY(BlueprintReadWrite, Category = "Flocking")
    FVector CohesionForce;

    UPROPERTY(BlueprintReadWrite, Category = "Flocking")
    FVector AvoidanceForce;

    UPROPERTY(BlueprintReadWrite, Category = "Flocking")
    int32 NeighborCount;

    FCrowd_FlockingData()
    {
        SeparationForce = FVector::ZeroVector;
        AlignmentForce = FVector::ZeroVector;
        CohesionForce = FVector::ZeroVector;
        AvoidanceForce = FVector::ZeroVector;
        NeighborCount = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_BehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    FCrowd_BehaviorParams BehaviorParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    ECrowd_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float MinStateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float MaxStateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Behavior")
    FCrowd_FlockingData FlockingData;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Behavior")
    FVector CurrentVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Behavior")
    FVector TargetLocation;

private:
    TArray<AActor*> NearbyAgents;
    float LastNeighborUpdate;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    FVector CalculateFlockingForce();

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    FVector CalculateSeparation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    FVector CalculateAlignment();

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    FVector CalculateCohesion();

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    FVector CalculateAvoidance();

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void UpdateNearbyAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetBehaviorState(ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetTargetLocation(FVector NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    FVector GetSteeringForce(FVector TargetVelocity);

    UFUNCTION(BlueprintPure, Category = "Crowd Behavior")
    ECrowd_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Crowd Behavior")
    FVector GetCurrentVelocity() const { return CurrentVelocity; }
};