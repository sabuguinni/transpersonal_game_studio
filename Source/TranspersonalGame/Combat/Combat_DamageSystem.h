#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "../SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, float, DamageAmount, AActor*, DamageSource, FVector, HitLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);

USTRUCT(BlueprintType)
struct FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCanCauseKnockback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float KnockbackForce;

    FCombat_DamageInfo()
    {
        BaseDamage = 0.0f;
        DamageType = EDamageType::Physical;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
        DamageSource = nullptr;
        bCanCauseKnockback = false;
        KnockbackForce = 0.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Propriedades de saúde
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float HealthRegenerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float RegenerationDelay;

    // Resistências
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    TMap<EDamageType, float> DamageResistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float ArmorValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    bool bIsInvulnerable;

    // Estado
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeSinceLastDamage;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* LastDamageSource;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeath OnDeath;

public:
    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Damage")
    float TakeDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetCurrentHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsAtFullHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Resistance")
    void SetDamageResistance(EDamageType DamageType, float Resistance);

    UFUNCTION(BlueprintCallable, Category = "Resistance")
    float GetDamageResistance(EDamageType DamageType) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    void SetInvulnerable(bool bNewInvulnerable);

    UFUNCTION(BlueprintCallable, Category = "State")
    void Kill();

    UFUNCTION(BlueprintCallable, Category = "State")
    void Revive(float NewHealth = -1.0f);

protected:
    // Funções internas
    float CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const;
    void ApplyKnockback(const FCombat_DamageInfo& DamageInfo);
    void HandleDeath();
    void UpdateHealthRegeneration(float DeltaTime);

private:
    // Estado interno
    bool bCanRegenerate;
    float RegenerationTimer;
};