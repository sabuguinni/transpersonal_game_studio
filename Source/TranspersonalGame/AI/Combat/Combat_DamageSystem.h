#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float KnockbackForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsCriticalHit = false;

    FCombat_DamageInfo()
    {
        DamageAmount = 0.0f;
        DamageType = EDamageType::Physical;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
        DamageSource = nullptr;
        KnockbackForce = 0.0f;
        bIsCriticalHit = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_HealthStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float HealthRegenRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float DamageReduction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bIsInvulnerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float LastDamageTime = 0.0f;

    FCombat_HealthStats()
    {
        MaxHealth = 100.0f;
        CurrentHealth = 100.0f;
        HealthRegenRate = 1.0f;
        DamageReduction = 0.0f;
        bIsInvulnerable = false;
        LastDamageTime = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, const FCombat_DamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealthPercent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Health and damage functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetInvulnerable(bool bInvulnerable);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ResetHealth();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Kill();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealth() const { return HealthStats.CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetMaxHealth() const { return HealthStats.MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const { return HealthStats.CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInvulnerable() const { return HealthStats.bIsInvulnerable; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAtFullHealth() const;

    // Damage calculation
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldTakeDamage(const FCombat_DamageInfo& DamageInfo) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnHealthChanged OnHealthChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    FCombat_HealthStats HealthStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float InvulnerabilityDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    bool bAutoRegen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float RegenDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    bool bShowDamageNumbers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    bool bScreenShakeOnDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    float ScreenShakeIntensity = 1.0f;

private:
    void HandleDeath();
    void ProcessHealthRegen(float DeltaTime);
    void ShowDamageEffect(const FCombat_DamageInfo& DamageInfo);
    void TriggerScreenShake(float DamageAmount);
    bool IsRecentlyDamaged() const;
};