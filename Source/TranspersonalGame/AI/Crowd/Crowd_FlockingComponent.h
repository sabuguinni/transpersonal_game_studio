#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Crowd_FlockingComponent.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_FlockingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 500.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_FlockingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_FlockingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FCrowd_FlockingData FlockingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    TArray<TObjectPtr<APawn>> FlockMembers;

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void AddFlockMember(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void RemoveFlockMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateFlockingForce();

protected:
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateSeparation();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateAlignment();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateCohesion();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    TArray<APawn*> GetNearbyFlockMembers(float Radius);

private:
    FVector CurrentVelocity;
    FVector TargetVelocity;
};