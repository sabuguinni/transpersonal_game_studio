#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "NPC_PatrolBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_PatrolState : uint8
{
    Idle,
    MovingToPoint,
    Investigating,
    Returning,
    Alerted
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLookAround;

    FNPC_PatrolPoint()
    {
        Location = FVector::ZeroVector;
        WaitTime = 3.0f;
        bLookAround = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PatrolBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PatrolBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Patrol Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bRandomPatrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bReverseDirection;

    // Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TArray<TSubclassOf<AActor>> TargetClasses;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPC_PatrolState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentPatrolIndex;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* DetectedTarget;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StopPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetPatrolRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void AddPatrolPoint(FVector Location, float WaitTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void GenerateRandomPatrolPoints(int32 NumPoints);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* ScanForTargets();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnPatrolPointReached(int32 PointIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnReturnedHome();

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void MoveToNextPatrolPoint();
    void HandleDetectedTarget();
    void ReturnToPatrol();
    
    FVector GetRandomPointInRadius(FVector Center, float Radius);
    bool IsAtLocation(FVector TargetLocation, float Tolerance = 150.0f);
    
    FTimerHandle PatrolTimerHandle;
    float CurrentWaitTime;
    bool bIsWaiting;
    bool bPatrolActive;
    
    // Movement tracking
    FVector LastPosition;
    float StuckTimer;
    float MaxStuckTime;
};