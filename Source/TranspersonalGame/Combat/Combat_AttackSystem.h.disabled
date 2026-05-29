#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "Combat_AttackSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bRequiresLineOfSight = true;

    FCombat_AttackData()
    {
        Damage = 25.0f;
        Range = 200.0f;
        Cooldown = 2.0f;
        WindupTime = 0.5f;
        AttackType = ECombat_AttackType::Bite;
        bRequiresLineOfSight = true;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AttackSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AttackSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Attack Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    TArray<FCombat_AttackData> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AggroRange = 600.0f;

    // Attack State
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bCanAttack = true;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    int32 CurrentAttackIndex = 0;

    // Attack Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool TryAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(const FCombat_AttackData& AttackData, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartAttackCooldown(float CooldownDuration);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* FindNearestTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasLineOfSight(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_AttackData GetBestAttackForTarget(AActor* Target) const;

    // Combat Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackStarted(const FCombat_AttackData& AttackData, AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackHit(const FCombat_AttackData& AttackData, AActor* Target, float DamageDealt);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackMissed(const FCombat_AttackData& AttackData, AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnTargetAcquired(AActor* NewTarget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnTargetLost(AActor* LostTarget);

private:
    FTimerHandle AttackCooldownTimer;
    FTimerHandle AttackWindupTimer;

    void OnAttackCooldownComplete();
    void OnAttackWindupComplete();
    void ProcessAttackHit(const FCombat_AttackData& AttackData, AActor* Target);
    bool IsValidTarget(AActor* Target) const;
    void UpdateTargetTracking(float DeltaTime);
};