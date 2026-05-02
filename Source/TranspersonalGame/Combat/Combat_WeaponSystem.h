#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Combat_WeaponSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_WeaponType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneSpear      UMETA(DisplayName = "Stone Spear"),
    WoodenClub      UMETA(DisplayName = "Wooden Club"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    BoneKnife       UMETA(DisplayName = "Bone Knife"),
    Sling           UMETA(DisplayName = "Sling"),
    Torch           UMETA(DisplayName = "Torch")
};

UENUM(BlueprintType)
enum class ECombat_WeaponState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Broken          UMETA(DisplayName = "Broken"),
    Repairing       UMETA(DisplayName = "Repairing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_WeaponStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float MaxDurability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float Weight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    bool bCanBlock = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    bool bIsRanged = false;

    FCombat_WeaponStats()
    {
        BaseDamage = 10.0f;
        AttackRange = 150.0f;
        AttackSpeed = 1.0f;
        Durability = 100.0f;
        MaxDurability = 100.0f;
        Weight = 2.0f;
        bCanBlock = false;
        bIsRanged = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_WeaponSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_WeaponSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Weapon Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    ECombat_WeaponType CurrentWeaponType = ECombat_WeaponType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    ECombat_WeaponState CurrentWeaponState = ECombat_WeaponState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FCombat_WeaponStats WeaponStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    class UStaticMeshComponent* WeaponMesh;

    // Weapon Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Assets")
    TMap<ECombat_WeaponType, UStaticMesh*> WeaponMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Assets")
    TMap<ECombat_WeaponType, USoundCue*> AttackSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Assets")
    TMap<ECombat_WeaponType, UParticleSystem*> HitEffects;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool EquipWeapon(ECombat_WeaponType WeaponType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool UnequipWeapon();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformAttack(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool StartBlocking();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool StopBlocking();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetCurrentDamage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDurabilityDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool RepairWeapon(float RepairAmount);

    // Weapon State
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsWeaponEquipped() const { return CurrentWeaponType != ECombat_WeaponType::None; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsWeaponBroken() const { return WeaponStats.Durability <= 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDurabilityPercentage() const;

protected:
    // Internal Functions
    void InitializeWeaponStats(ECombat_WeaponType WeaponType);
    void UpdateWeaponMesh();
    void PlayAttackSound();
    void SpawnHitEffect(FVector HitLocation);

    // Attack Timer
    UPROPERTY()
    float LastAttackTime = 0.0f;

    UPROPERTY()
    bool bIsAttacking = false;

    UPROPERTY()
    float AttackCooldown = 1.0f;
};