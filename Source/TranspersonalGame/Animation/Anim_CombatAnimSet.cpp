#include "Anim_CombatAnimSet.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"

UAnim_CombatAnimSet::UAnim_CombatAnimSet()
{
    // Initialize all montage pointers to null
    // These will be set in the Blueprint/Editor
    UnarmedAttack1 = nullptr;
    UnarmedAttack2 = nullptr;
    UnarmedAttack3 = nullptr;
    UnarmedBlock = nullptr;
    UnarmedDodgeLeft = nullptr;
    UnarmedDodgeRight = nullptr;
    UnarmedDodgeBack = nullptr;

    StoneAxeAttack1 = nullptr;
    StoneAxeAttack2 = nullptr;
    StoneAxeHeavyAttack = nullptr;
    StoneKnifeStab = nullptr;
    StoneKnifeSlash = nullptr;
    StoneHammerSmash = nullptr;

    SpearThrust = nullptr;
    SpearSweep = nullptr;
    SpearThrow = nullptr;
    SpearBlock = nullptr;
    SpearRetrieve = nullptr;

    BowDraw = nullptr;
    BowAim = nullptr;
    BowRelease = nullptr;
    BowReload = nullptr;

    ShieldBlock = nullptr;
    ShieldBash = nullptr;
    RollDodge = nullptr;
    CrouchBlock = nullptr;

    HitReactionLight = nullptr;
    HitReactionMedium = nullptr;
    HitReactionHeavy = nullptr;
    KnockdownFront = nullptr;
    KnockdownBack = nullptr;
    GetUpFront = nullptr;
    GetUpBack = nullptr;
    Death = nullptr;
}

UAnimMontage* UAnim_CombatAnimSet::GetUnarmedAttackMontage(int32 AttackIndex) const
{
    switch (AttackIndex)
    {
        case 0:
            return UnarmedAttack1;
        case 1:
            return UnarmedAttack2;
        case 2:
            return UnarmedAttack3;
        default:
            return UnarmedAttack1; // Default to first attack
    }
}

UAnimMontage* UAnim_CombatAnimSet::GetWeaponAttackMontage(EAnim_WeaponType WeaponType, int32 AttackIndex) const
{
    switch (WeaponType)
    {
        case EAnim_WeaponType::Unarmed:
            return GetUnarmedAttackMontage(AttackIndex);
            
        case EAnim_WeaponType::StoneAxe:
            switch (AttackIndex)
            {
                case 0: return StoneAxeAttack1;
                case 1: return StoneAxeAttack2;
                case 2: return StoneAxeHeavyAttack;
                default: return StoneAxeAttack1;
            }
            
        case EAnim_WeaponType::StoneKnife:
            switch (AttackIndex)
            {
                case 0: return StoneKnifeStab;
                case 1: return StoneKnifeSlash;
                default: return StoneKnifeStab;
            }
            
        case EAnim_WeaponType::StoneHammer:
            return StoneHammerSmash;
            
        case EAnim_WeaponType::Spear:
            switch (AttackIndex)
            {
                case 0: return SpearThrust;
                case 1: return SpearSweep;
                case 2: return SpearThrow;
                default: return SpearThrust;
            }
            
        case EAnim_WeaponType::Bow:
            switch (AttackIndex)
            {
                case 0: return BowDraw;
                case 1: return BowAim;
                case 2: return BowRelease;
                default: return BowDraw;
            }
            
        default:
            return GetUnarmedAttackMontage(AttackIndex);
    }
}

UAnimMontage* UAnim_CombatAnimSet::GetDefenseMontage(EAnim_DefenseType DefenseType) const
{
    switch (DefenseType)
    {
        case EAnim_DefenseType::Block:
            return UnarmedBlock;
        case EAnim_DefenseType::ShieldBlock:
            return ShieldBlock;
        case EAnim_DefenseType::ShieldBash:
            return ShieldBash;
        case EAnim_DefenseType::DodgeLeft:
            return UnarmedDodgeLeft;
        case EAnim_DefenseType::DodgeRight:
            return UnarmedDodgeRight;
        case EAnim_DefenseType::DodgeBack:
            return UnarmedDodgeBack;
        case EAnim_DefenseType::Roll:
            return RollDodge;
        case EAnim_DefenseType::Crouch:
            return CrouchBlock;
        default:
            return UnarmedBlock;
    }
}

UAnimMontage* UAnim_CombatAnimSet::GetReactionMontage(EAnim_HitReactionType ReactionType) const
{
    switch (ReactionType)
    {
        case EAnim_HitReactionType::Light:
            return HitReactionLight;
        case EAnim_HitReactionType::Medium:
            return HitReactionMedium;
        case EAnim_HitReactionType::Heavy:
            return HitReactionHeavy;
        case EAnim_HitReactionType::KnockdownFront:
            return KnockdownFront;
        case EAnim_HitReactionType::KnockdownBack:
            return KnockdownBack;
        case EAnim_HitReactionType::GetUpFront:
            return GetUpFront;
        case EAnim_HitReactionType::GetUpBack:
            return GetUpBack;
        case EAnim_HitReactionType::Death:
            return Death;
        default:
            return HitReactionLight;
    }
}

TArray<UAnimMontage*> UAnim_CombatAnimSet::GetAllMontagesForWeapon(EAnim_WeaponType WeaponType) const
{
    TArray<UAnimMontage*> Montages;
    
    switch (WeaponType)
    {
        case EAnim_WeaponType::Unarmed:
            Montages.Add(UnarmedAttack1);
            Montages.Add(UnarmedAttack2);
            Montages.Add(UnarmedAttack3);
            Montages.Add(UnarmedBlock);
            break;
            
        case EAnim_WeaponType::StoneAxe:
            Montages.Add(StoneAxeAttack1);
            Montages.Add(StoneAxeAttack2);
            Montages.Add(StoneAxeHeavyAttack);
            break;
            
        case EAnim_WeaponType::StoneKnife:
            Montages.Add(StoneKnifeStab);
            Montages.Add(StoneKnifeSlash);
            break;
            
        case EAnim_WeaponType::StoneHammer:
            Montages.Add(StoneHammerSmash);
            break;
            
        case EAnim_WeaponType::Spear:
            Montages.Add(SpearThrust);
            Montages.Add(SpearSweep);
            Montages.Add(SpearThrow);
            Montages.Add(SpearBlock);
            Montages.Add(SpearRetrieve);
            break;
            
        case EAnim_WeaponType::Bow:
            Montages.Add(BowDraw);
            Montages.Add(BowAim);
            Montages.Add(BowRelease);
            Montages.Add(BowReload);
            break;
    }
    
    // Remove null entries
    Montages.RemoveAll([](UAnimMontage* Montage) { return Montage == nullptr; });
    
    return Montages;
}

bool UAnim_CombatAnimSet::HasValidMontages() const
{
    // Check if we have at least basic unarmed combat animations
    bool HasBasicUnarmed = (UnarmedAttack1 != nullptr) && (UnarmedBlock != nullptr);
    
    // Check if we have at least one weapon type with animations
    bool HasWeaponAnimations = (StoneAxeAttack1 != nullptr) || 
                              (StoneKnifeStab != nullptr) || 
                              (SpearThrust != nullptr) || 
                              (BowDraw != nullptr);
    
    // Check if we have basic reaction animations
    bool HasReactions = (HitReactionLight != nullptr) && (Death != nullptr);
    
    return HasBasicUnarmed && HasWeaponAnimations && HasReactions;
}

void UAnim_CombatAnimSet::ValidateAnimationAssets()
{
    // This function can be called in editor to validate all animation assets
    TArray<UAnimMontage**> AllMontages = {
        &UnarmedAttack1, &UnarmedAttack2, &UnarmedAttack3, &UnarmedBlock,
        &UnarmedDodgeLeft, &UnarmedDodgeRight, &UnarmedDodgeBack,
        &StoneAxeAttack1, &StoneAxeAttack2, &StoneAxeHeavyAttack,
        &StoneKnifeStab, &StoneKnifeSlash, &StoneHammerSmash,
        &SpearThrust, &SpearSweep, &SpearThrow, &SpearBlock, &SpearRetrieve,
        &BowDraw, &BowAim, &BowRelease, &BowReload,
        &ShieldBlock, &ShieldBash, &RollDodge, &CrouchBlock,
        &HitReactionLight, &HitReactionMedium, &HitReactionHeavy,
        &KnockdownFront, &KnockdownBack, &GetUpFront, &GetUpBack, &Death
    };
    
    int32 ValidCount = 0;
    int32 TotalCount = AllMontages.Num();
    
    for (UAnimMontage** MontagePtr : AllMontages)
    {
        if (*MontagePtr != nullptr)
        {
            ValidCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Animation Set Validation: %d/%d montages assigned"), ValidCount, TotalCount);
    
    if (ValidCount < TotalCount * 0.5f)
    {
        UE_LOG(LogTemp, Error, TEXT("Less than 50% of combat animations are assigned! Combat system may not function properly."));
    }
}