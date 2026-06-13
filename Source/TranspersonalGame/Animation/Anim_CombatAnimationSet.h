#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "../Shared/SharedTypes.h"
#include "Anim_CombatAnimationSet.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_WeaponAnimSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations")
    class UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations")
    class UAnimMontage* ParryMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations")
    class UAnimSequence* IdleSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations")
    class UAnimSequence* WalkSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Animations")
    class UAnimSequence* RunSequence;

    FAnim_WeaponAnimSet()
    {
        AttackMontage = nullptr;
        BlockMontage = nullptr;
        ParryMontage = nullptr;
        IdleSequence = nullptr;
        WalkSequence = nullptr;
        RunSequence = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CombatState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EWeaponType CurrentWeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatAlpha;

    FAnim_CombatState()
    {
        CurrentWeaponType = EWeaponType::Unarmed;
        bIsInCombat = false;
        bIsAttacking = false;
        bIsBlocking = false;
        CombatAlpha = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CombatAnimationSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UAnim_CombatAnimationSet();

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    FAnim_WeaponAnimSet GetWeaponAnimSet(EWeaponType WeaponType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    class UAnimMontage* GetAttackMontage(EWeaponType WeaponType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    class UAnimMontage* GetBlockMontage(EWeaponType WeaponType) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed Combat")
    FAnim_WeaponAnimSet UnarmedAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spear Combat")
    FAnim_WeaponAnimSet SpearAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Club Combat")
    FAnim_WeaponAnimSet ClubAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow Combat")
    FAnim_WeaponAnimSet BowAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Combat")
    FAnim_WeaponAnimSet StoneAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Combat")
    class UAnimMontage* DodgeRollMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Combat")
    class UAnimMontage* HitReactionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Combat")
    class UAnimMontage* DeathMontage;
};