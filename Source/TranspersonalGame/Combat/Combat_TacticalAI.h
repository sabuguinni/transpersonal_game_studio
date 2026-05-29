#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Hunting UMETA(DisplayName = "Hunting"),
    Stalking UMETA(DisplayName = "Stalking"),
    Attacking UMETA(DisplayName = "Attacking"),
    Retreating UMETA(DisplayName = "Retreating"),
    Circling UMETA(DisplayName = "Circling")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Predator UMETA(DisplayName = "Predator"),
    Herbivore UMETA(DisplayName = "Herbivore"),
    Scavenger UMETA(DisplayName = "Scavenger"),
    PackHunter UMETA(DisplayName = "Pack Hunter")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanPackHunt = false;

    FCombat_TacticalData()
    {
        DetectionRange = 2000.0f;
        AttackRange = 500.0f;
        RetreatHealthThreshold = 0.3f;
        AggressionLevel = 0.7f;
        bCanPackHunt = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float DistanceToTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float MaxHealth;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsPlayerInRange(float Range);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttackPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreatPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCirclingPattern();

private:
    void UpdateTargetDistance();
    void EvaluateHealthStatus();
    bool ShouldRetreat();
    bool ShouldAttack();
    void HandleIdleState();
    void HandleHuntingState();
    void HandleStalkingState();
    void HandleAttackingState();
    void HandleRetreatingState();
    void HandleCirclingState();

    float LastStateChangeTime;
    float StateChangeDelay;
    FVector LastKnownPlayerPosition;
    bool bHasLineOfSight;
};