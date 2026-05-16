#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Crowd_FlockingComponent.generated.h"

UENUM(BlueprintType)
enum class ECrowd_FlockBehavior : uint8
{
    Wandering UMETA(DisplayName = "Wandering"),
    Flocking UMETA(DisplayName = "Flocking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Feeding UMETA(DisplayName = "Feeding"),
    Resting UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 100.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_FlockingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_FlockingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FCrowd_FlockingParams FlockingParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowd_FlockBehavior CurrentBehavior = ECrowd_FlockBehavior::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorChangeInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float PlayerDetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float PredatorDetectionRadius = 800.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<AActor*> NearbyFlockmates;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* DetectedThreat;

    // Flocking behavior functions
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateSeparation();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateAlignment();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateCohesion();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateFleeFromThreat();

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void UpdateNearbyFlockmates();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehavior(ECrowd_FlockBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool DetectThreats();

private:
    float LastBehaviorChange;
    FVector TargetDirection;
    
    FVector Seek(FVector Target);
    FVector Wander();
    void ApplyForce(FVector Force, float DeltaTime);
    FVector LimitVector(FVector Vector, float MaxLength);
};