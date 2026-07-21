#include "Anim_CombatAnimationSet.h"

UAnim_CombatAnimationSet::UAnim_CombatAnimationSet()
{
    DodgeRollMontage = nullptr;
    HitReactionMontage = nullptr;
    DeathMontage = nullptr;
}

FAnim_WeaponAnimSet UAnim_CombatAnimationSet::GetWeaponAnimSet(EWeaponType WeaponType) const
{
    switch (WeaponType)
    {
        case EWeaponType::Unarmed:
            return UnarmedAnimSet;
        case EWeaponType::Spear:
            return SpearAnimSet;
        case EWeaponType::Club:
            return ClubAnimSet;
        case EWeaponType::Bow:
            return BowAnimSet;
        case EWeaponType::Stone:
            return StoneAnimSet;
        default:
            return UnarmedAnimSet;
    }
}

UAnimMontage* UAnim_CombatAnimationSet::GetAttackMontage(EWeaponType WeaponType) const
{
    FAnim_WeaponAnimSet AnimSet = GetWeaponAnimSet(WeaponType);
    return AnimSet.AttackMontage;
}

UAnimMontage* UAnim_CombatAnimationSet::GetBlockMontage(EWeaponType WeaponType) const
{
    FAnim_WeaponAnimSet AnimSet = GetWeaponAnimSet(WeaponType);
    return AnimSet.BlockMontage;
}