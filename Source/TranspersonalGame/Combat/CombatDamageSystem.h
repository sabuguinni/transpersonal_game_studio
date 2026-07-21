#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "CombatDamageSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_DamageType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Charge          UMETA(DisplayName = "Charge"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Fall            UMETA(DisplayName = "Fall")
};

USTRUCT(BlueprintType)
struct FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float ArmorPenetration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float CriticalChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float CriticalMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCanKnockdown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float KnockdownForce = 500.0f;

    FCombat_DamageInfo()
    {
        BaseDamage = 10.0f;
        DamageType = ECombat_DamageType::None;
        ArmorPenetration = 0.0f;
        CriticalChance = 0.1f;
        CriticalMultiplier = 2.0f;
        bCanKnockdown = false;
        KnockdownForce = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatDamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatDamageSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (AllowPrivateAccess = "true"))
    float ArmorValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (AllowPrivateAccess = "true"))
    float DamageReduction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (AllowPrivateAccess = "true"))
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (AllowPrivateAccess = "true"))
    bool bIsKnockedDown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (AllowPrivateAccess = "true"))
    float KnockdownRecoveryTime = 3.0f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float ApplyDamage(const FCombat_DamageInfo& DamageInfo, AActor* DamageSource = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyHealing(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ResetHealth();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void KillActor();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyKnockdown(float Duration, float Force = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RecoverFromKnockdown();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercentage() const { return MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const { return bIsAlive; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsKnockedDown() const { return bIsKnockedDown; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetArmorValue() const { return ArmorValue; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, float, DamageAmount, ECombat_DamageType, DamageType, AActor*, DamageSource);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealthPercentage);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKnockedDown);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecovered);

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnKnockedDown OnKnockedDown;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnRecovered OnRecovered;

private:
    FTimerHandle KnockdownRecoveryTimer;
    
    void HandleKnockdownRecovery();
    float CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo);
    bool ShouldCriticalHit(float CritChance);
};