#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Combat_PlayerCombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDamaged, float, Damage, AActor*, DamageSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeath, AActor*, Killer);

UENUM(BlueprintType)
enum class ECombat_WeaponType : uint8
{
    None = 0,
    Stone,
    Spear,
    Club,
    Bow
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_WeaponData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    ECombat_WeaponType WeaponType = ECombat_WeaponType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float StaminaCost = 15.0f;

    FCombat_WeaponData()
    {
        WeaponType = ECombat_WeaponType::None;
        BaseDamage = 10.0f;
        AttackRange = 150.0f;
        AttackSpeed = 1.0f;
        StaminaCost = 15.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PlayerCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PlayerCombatComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartBlock();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopBlock();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Dodge();

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DealDamage(AActor* Target, float DamageAmount);

    // Weapon System
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EquipWeapon(ECombat_WeaponType NewWeapon);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnequipWeapon();

    // Status Checks
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAttacking() const { return bIsAttacking; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsBlocking() const { return bIsBlocking; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_WeaponType GetCurrentWeapon() const { return CurrentWeapon.WeaponType; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnPlayerDamaged OnPlayerDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnPlayerDeath OnPlayerDeath;

protected:
    // Health System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    // Combat State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsBlocking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsDodging = false;

    // Weapon System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FCombat_WeaponData CurrentWeapon;

    // Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackDuration = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DodgeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DodgeCooldown = 2.0f;

    // Internal Timers
    float AttackTimer = 0.0f;
    float DodgeTimer = 0.0f;
    float DodgeCooldownTimer = 0.0f;

    // Helper Functions
    void PerformAttack();
    void EndAttack();
    void EndDodge();
    bool HasEnoughStamina(float StaminaCost) const;
    void ConsumeStamina(float Amount);
    AActor* GetOwnerCharacter() const;
};

#include "Combat_PlayerCombatComponent.generated.h"