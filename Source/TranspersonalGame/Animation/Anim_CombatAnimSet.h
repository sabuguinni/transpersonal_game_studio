#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "SharedTypes.h"
#include "Anim_CombatAnimSet.generated.h"

/**
 * Data asset containing all combat animations for primitive human characters
 * Organized by weapon type and combat action for prehistoric survival gameplay
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAnim_CombatAnimSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UAnim_CombatAnimSet();

    // Unarmed Combat Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedAttack1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedAttack2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedAttack3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedBlock;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedDodgeLeft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedDodgeRight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Combat")
    class UAnimMontage* UnarmedDodgeBack;

    // Stone Tool Combat Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stone Tools")
    class UAnimMontage* StoneAxeAttack1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stone Tools")
    class UAnimMontage* StoneAxeAttack2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stone Tools")
    class UAnimMontage* StoneAxeHeavyAttack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stone Tools")
    class UAnimMontage* StoneKnifeStab;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stone Tools")
    class UAnimMontage* StoneKnifeSlash;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stone Tools")
    class UAnimMontage* StoneHammerSmash;

    // Spear Combat Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spear Combat")
    class UAnimMontage* SpearThrust;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spear Combat")
    class UAnimMontage* SpearSweep;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spear Combat")
    class UAnimMontage* SpearThrow;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spear Combat")
    class UAnimMontage* SpearBlock;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spear Combat")
    class UAnimMontage* SpearRetrieve;

    // Bow Combat Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow Combat")
    class UAnimMontage* BowDraw;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow Combat")
    class UAnimMontage* BowAim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow Combat")
    class UAnimMontage* BowRelease;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow Combat")
    class UAnimMontage* BowReload;

    // Defensive Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
    class UAnimMontage* ShieldBlock;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
    class UAnimMontage* ShieldBash;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
    class UAnimMontage* RollDodge;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
    class UAnimMontage* CrouchBlock;

    // Reaction Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* HitReactionLight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* HitReactionMedium;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* HitReactionHeavy;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* KnockdownFront;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* KnockdownBack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* GetUpFront;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* GetUpBack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    class UAnimMontage* Death;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    UAnimMontage* GetUnarmedAttackMontage(int32 AttackIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    UAnimMontage* GetWeaponAttackMontage(EAnim_WeaponType WeaponType, int32 AttackIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    UAnimMontage* GetDefenseMontage(EAnim_DefenseType DefenseType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    UAnimMontage* GetReactionMontage(EAnim_HitReactionType ReactionType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    TArray<UAnimMontage*> GetAllMontagesForWeapon(EAnim_WeaponType WeaponType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    bool HasValidMontages() const;

private:
    void ValidateAnimationAssets();
};