#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TRexBehaviorController.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol     UMETA(DisplayName = "Patrol"),
    Alert      UMETA(DisplayName = "Alert"),
    Chase      UMETA(DisplayName = "Chase"),
    Attack     UMETA(DisplayName = "Attack"),
    Rest       UMETA(DisplayName = "Rest"),
};

USTRUCT(BlueprintType)
struct FNPC_TRexPatrolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float ChaseDetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float AttackCooldown = 2.5f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void SetBehaviorState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ChaseTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ExecuteAttack();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    FNPC_TRexPatrolData PatrolData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    FVector PatrolCenter;

private:
    ENPC_TRexState CurrentState;
    AActor* CurrentTarget;
    float TimeSinceLastAttack;
    FVector CurrentPatrolDestination;

    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void ScanForPlayer();
    FVector GetRandomPatrolPoint() const;
};
