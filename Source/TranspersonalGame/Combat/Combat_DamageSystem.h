#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
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
    AActor* DamageInstigator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ZeroVector;

    FCombat_DamageInfo()
    {
        DamageAmount = 0.0f;
        DamageType = EDamageType::Physical;
        DamageInstigator = nullptr;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, const FCombat_DamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
    bool bCanTakeDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
    float DamageReduction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
    TArray<EDamageType> DamageImmunities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
    float InvulnerabilityDuration = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
    bool bIsInvulnerable = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;

    FTimerHandle InvulnerabilityTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Damage")
    bool ApplyDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const { return !bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void SetInvulnerable(bool bNewInvulnerable);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void AddDamageImmunity(EDamageType DamageType);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void RemoveDamageImmunity(EDamageType DamageType);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeath OnDeath;

protected:
    UFUNCTION()
    void EndInvulnerability();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnHealthChanged(float NewHealth, float MaxHealth);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnActorDied();

    void Die();
    bool ShouldTakeDamage(const FCombat_DamageInfo& DamageInfo) const;
    float CalculateFinalDamage(float BaseDamage) const;
};