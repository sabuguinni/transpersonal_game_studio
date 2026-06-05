#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CriticalMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ArmorPenetration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDamageType DamageType = EDamageType::Physical;

    FCombat_DamageData()
    {
        BaseDamage = 25.0f;
        CriticalMultiplier = 2.0f;
        ArmorPenetration = 0.0f;
        DamageType = EDamageType::Physical;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_HitResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float FinalDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bWasCritical = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bWasBlocked = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector HitLocation = FVector::ZeroVector;

    FCombat_HitResult()
    {
        FinalDamage = 0.0f;
        bWasCritical = false;
        bWasBlocked = false;
        HitLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defense")
    float BaseArmor = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defense")
    float BlockChance = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defense")
    float CriticalResistance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
    float HealthRegenRate = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Status")
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Status")
    bool bIsInvulnerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Status")
    float InvulnerabilityDuration = 1.0f;

    FTimerHandle InvulnerabilityTimer;
    FTimerHandle HealthRegenTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_HitResult ApplyDamage(const FCombat_DamageData& DamageData, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetInvulnerable(bool bNewInvulnerable);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Kill();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Revive(float ReviveHealth = 50.0f);

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsLowHealth() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanTakeDamage() const;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, float, Damage, bool, bWasCritical, AActor*, DamageSource);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KilledActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRevive, AActor*, RevivedActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnRevive OnRevive;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnHealthChanged OnHealthChanged;

private:
    void StartHealthRegen();
    void StopHealthRegen();
    void RegenHealth();
    void EndInvulnerability();
    float CalculateDamageReduction(float IncomingDamage, float ArmorPenetration) const;
    bool RollCritical(float CritChance) const;
    bool RollBlock() const;
};