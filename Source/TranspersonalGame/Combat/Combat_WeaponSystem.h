#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Combat_WeaponSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_WeaponStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EWeaponType WeaponType = EWeaponType::Spear;

    FCombat_WeaponStats()
    {
        BaseDamage = 10.0f;
        Range = 200.0f;
        AttackSpeed = 1.0f;
        Durability = 100.0f;
        WeaponType = EWeaponType::Spear;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_WeaponStats WeaponStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsEquipped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentDurability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UStaticMeshComponent* WeaponMesh;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EquipWeapon();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnequipWeapon();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDurabilityDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsWeaponBroken() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RepairWeapon(float RepairAmount);

    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombat_WeaponStats GetWeaponStats() const { return WeaponStats; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetDurabilityPercentage() const;
};