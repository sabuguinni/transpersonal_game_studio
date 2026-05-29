#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "NPC_TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Return      UMETA(DisplayName = "Return")
};

USTRUCT(BlueprintType)
struct FNPC_TRexPatrolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector PatrolCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed;

    FNPC_TRexPatrolData()
    {
        PatrolCenter = FVector::ZeroVector;
        PatrolRadius = 5000.0f;
        PatrolSpeed = 300.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TRexBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    ENPC_TRexState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    FNPC_TRexPatrolData PatrolData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float ChaseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Behavior")
    APawn* TargetPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Behavior")
    FVector CurrentPatrolTarget;

    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Behavior")
    float LastAttackTime;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetPatrolCenter(FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetPatrolRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void ExecuteAttack();

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateChaseBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateReturnBehavior(float DeltaTime);
    
    APawn* FindNearestPlayer();
    FVector GetRandomPatrolPoint();
    bool IsPlayerInRange(float Range);
    void MoveToLocation(FVector TargetLocation, float Speed);
};