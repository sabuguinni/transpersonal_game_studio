#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol     UMETA(DisplayName = "Patrol"),
    Alerted    UMETA(DisplayName = "Alerted"),
    Chase      UMETA(DisplayName = "Chase"),
    Attack     UMETA(DisplayName = "Attack"),
    Roar       UMETA(DisplayName = "Roar"),
    Rest       UMETA(DisplayName = "Rest")
};

USTRUCT(BlueprintType)
struct FNPC_TRexMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    bool bPlayerEverSeen = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    float AlertLevel = 0.0f;
};

UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float ChaseSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AlertDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float RoarDuration = 3.0f;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState = ENPC_TRexState::Patrol;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    FNPC_TRexMemory Memory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    FVector CurrentPatrolTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    float RoarTimer = 0.0f;

    // --- Blueprintable Events ---
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC|TRex|Events")
    void OnTRexRoar();

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC|TRex|Events")
    void OnTRexAttack(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC|TRex|Events")
    void OnTRexStateChanged(ENPC_TRexState NewState);

    // --- Callable ---
    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    float GetAlertLevel() const { return Memory.AlertLevel; }

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ForceState(ENPC_TRexState NewState);

private:
    void UpdatePatrol(float DeltaTime);
    void UpdateAlerted(float DeltaTime);
    void UpdateChase(float DeltaTime, AActor* Player);
    void UpdateAttack(float DeltaTime, AActor* Player);
    void UpdateRoar(float DeltaTime);
    void SetState(ENPC_TRexState NewState);
    void PickNewPatrolTarget();
    AActor* FindPlayer() const;
    bool CanSeePlayer(AActor* Player) const;
    float DistanceToPlayer(AActor* Player) const;
};
