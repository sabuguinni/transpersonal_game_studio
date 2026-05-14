#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

USTRUCT(BlueprintType)
struct FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsCriticalHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float CriticalMultiplier = 2.0f;

    FCombat_DamageInfo()
    {
        BaseDamage = 10.0f;
        DamageType = EDamageType::Physical;
        DamageSource = nullptr;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
        bIsCriticalHit = false;
        CriticalMultiplier = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_ResistanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float PhysicalResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float FireResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float PoisonResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float BleedingResistance = 0.0f;

    FCombat_ResistanceData()
    {
        PhysicalResistance = 0.0f;
        FireResistance = 0.0f;
        PoisonResistance = 0.0f;
        BleedingResistance = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, Damage, const FCombat_DamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageDealt, AActor*, Target, const FCombat_DamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Health Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float HealthRegenRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float HealthRegenDelay = 5.0f;

    // Resistance System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    FCombat_ResistanceData ResistanceData;

    // Status Effects
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsBleeding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsPoisoned = false;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsOnFire = false;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsDead = false;

    // Damage Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageDealt OnDamageDealt;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeath OnDeath;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Damage")
    float TakeDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void DealDamageToTarget(AActor* Target, const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const;

    // Status Effects
    UFUNCTION(BlueprintCallable, Category = "Status")
    void ApplyBleeding(float Duration, float DamagePerSecond);

    UFUNCTION(BlueprintCallable, Category = "Status")
    void ApplyPoison(float Duration, float DamagePerSecond);

    UFUNCTION(BlueprintCallable, Category = "Status")
    void ApplyBurning(float Duration, float DamagePerSecond);

    UFUNCTION(BlueprintCallable, Category = "Status")
    void ClearAllStatusEffects();

    // Resistance
    UFUNCTION(BlueprintPure, Category = "Resistance")
    float CalculateDamageReduction(EDamageType DamageType) const;

    UFUNCTION(BlueprintCallable, Category = "Resistance")
    void ModifyResistance(EDamageType DamageType, float ResistanceChange);

private:
    // Internal tracking
    float LastDamageTime = 0.0f;
    float BleedingTimer = 0.0f;
    float BleedingDPS = 0.0f;
    float PoisonTimer = 0.0f;
    float PoisonDPS = 0.0f;
    float BurningTimer = 0.0f;
    float BurningDPS = 0.0f;

    // Internal functions
    void ProcessStatusEffects(float DeltaTime);
    void ProcessHealthRegeneration(float DeltaTime);
    void Die();
    float CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const;
    bool ShouldBeCriticalHit(const FCombat_DamageInfo& DamageInfo) const;
};