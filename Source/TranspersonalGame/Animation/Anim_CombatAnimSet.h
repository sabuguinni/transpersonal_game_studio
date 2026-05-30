#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Anim_CombatAnimSet.generated.h"

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Ready           UMETA(DisplayName = "Combat Ready"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_WeaponType : uint8
{
    Unarmed         UMETA(DisplayName = "Unarmed"),
    Spear           UMETA(DisplayName = "Spear"),
    Club            UMETA(DisplayName = "Club"),
    Bow             UMETA(DisplayName = "Bow"),
    Stone           UMETA(DisplayName = "Stone Tool"),
    Torch           UMETA(DisplayName = "Torch")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CombatMontages
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Montages")
    TArray<UAnimMontage*> LightAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Montages")
    TArray<UAnimMontage*> HeavyAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Montages")
    UAnimMontage* BlockStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Montages")
    UAnimMontage* BlockLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Montages")
    UAnimMontage* BlockEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Montages")
    UAnimMontage* DodgeLeft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Montages")
    UAnimMontage* DodgeRight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Montages")
    UAnimMontage* DodgeBack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction Montages")
    TArray<UAnimMontage*> HitReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction Montages")
    UAnimMontage* Death;

    FAnim_CombatMontages()
    {
        BlockStart = nullptr;
        BlockLoop = nullptr;
        BlockEnd = nullptr;
        DodgeLeft = nullptr;
        DodgeRight = nullptr;
        DodgeBack = nullptr;
        Death = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CombatAnimSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UAnim_CombatAnimSet();

    // Combat animation sets for different weapon types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animations")
    TMap<EAnim_WeaponType, FAnim_CombatMontages> WeaponAnimSets;

    // Shared animations (weapon-independent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shared Animations")
    UAnimSequence* CombatIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shared Animations")
    UAnimSequence* CombatWalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shared Animations")
    UAnimSequence* CombatRunAnimation;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AttackComboWindow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlockTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DodgeInvulnerabilityTime;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    FAnim_CombatMontages GetCombatMontages(EAnim_WeaponType WeaponType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    UAnimMontage* GetRandomAttackMontage(EAnim_WeaponType WeaponType, bool bHeavyAttack = false) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    UAnimMontage* GetRandomHitReaction(EAnim_WeaponType WeaponType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Animations")
    bool HasAnimationsForWeapon(EAnim_WeaponType WeaponType) const;
};