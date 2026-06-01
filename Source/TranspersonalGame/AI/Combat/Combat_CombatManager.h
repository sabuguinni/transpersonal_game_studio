#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "Combat_CombatManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    FCombat_CombatData()
    {
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        AttackCooldown = 2.0f;
        bIsInCombat = false;
        CurrentTarget = nullptr;
    }
};

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Searching   UMETA(DisplayName = "Searching"),
    Engaging    UMETA(DisplayName = "Engaging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Dead        UMETA(DisplayName = "Dead")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_CombatManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_CombatManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_CombatData CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_CombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatRadius = 2000.0f;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* FindNearestEnemy();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatState();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_CombatState NewState);

private:
    float LastAttackTime;
    FTimerHandle CombatUpdateTimer;

    void UpdateCombatLogic();
    bool IsValidTarget(AActor* Target) const;
    void HandleIdleState();
    void HandleSearchingState();
    void HandleEngagingState();
    void HandleAttackingState();
    void HandleRetreatingState();
};