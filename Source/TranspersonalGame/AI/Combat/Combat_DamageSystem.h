#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
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
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsCriticalHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float KnockbackForce = 0.0f;

    FCombat_DamageInfo()
    {
        DamageAmount = 0.0f;
        DamageType = EDamageType::Physical;
        ImpactLocation = FVector::ZeroVector;
        ImpactDirection = FVector::ZeroVector;
        DamageSource = nullptr;
        bIsCriticalHit = false;
        KnockbackForce = 0.0f;
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
    float HealthRegenRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float DamageResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bIsInvulnerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float LastDamageTime = 0.0f;

    FCombat_HealthStats()
    {
        MaxHealth = 100.0f;
        CurrentHealth = 100.0f;
        HealthRegenRate = 0.0f;
        DamageResistance = 0.0f;
        bIsInvulnerable = false;
        LastDamageTime = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, const FCombat_DamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealthPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Health and damage properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
    FCombat_HealthStats HealthStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    TMap<EDamageType, float> DamageTypeResistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
    bool bShowDamageNumbers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
    float DamageNumberDisplayTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
    FLinearColor CriticalHitColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
    FLinearColor NormalHitColor = FLinearColor::White;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDeath OnDeath;

    // Core damage functions
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    bool ApplyDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat|Health")
    void HealDamage(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Health")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Combat|Health")
    void SetInvulnerable(bool bNewInvulnerable);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat|Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Health")
    bool IsAtFullHealth() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Damage")
    float CalculateActualDamage(const FCombat_DamageInfo& DamageInfo) const;

protected:
    // Internal functions
    void ProcessHealthRegeneration(float DeltaTime);
    void TriggerDeathSequence();
    void ShowDamageNumber(const FCombat_DamageInfo& DamageInfo, float ActualDamage);
    float GetDamageTypeResistance(EDamageType DamageType) const;

private:
    bool bIsDead = false;
    float LastRegenTime = 0.0f;
    TArray<AActor*> DamageNumberActors;
};