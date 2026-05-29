#include "Anim_CombatController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_CombatController::UAnim_CombatController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentCombatState = EAnim_CombatState::Idle;
    CurrentWeaponType = EAnim_WeaponType::None;
    ComboWindowTime = 1.5f;
    BlockHoldTime = 3.0f;
    DodgeCooldown = 1.0f;
    
    LastAttackTime = 0.0f;
    LastDodgeTime = 0.0f;
    CurrentComboCount = 0;
    bIsInComboWindow = false;
    
    OwnerMesh = nullptr;
    OwnerAnimInstance = nullptr;
}

void UAnim_CombatController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get references to owner components
    if (AActor* Owner = GetOwner())
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (OwnerMesh)
        {
            OwnerAnimInstance = OwnerMesh->GetAnimInstance();
        }
    }
    
    // Initialize default animations for each weapon type
    InitializeDefaultAnimations();
}

void UAnim_CombatController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update combo window timing
    UpdateComboWindow(DeltaTime);
    
    // Auto-exit combat state if idle for too long
    if (CurrentCombatState != EAnim_CombatState::Idle)
    {
        float TimeSinceLastAction = GetWorld()->GetTimeSeconds() - LastAttackTime;
        if (TimeSinceLastAction > 3.0f)
        {
            ResetCombatState();
        }
    }
}

bool UAnim_CombatController::TryStartAttack()
{
    if (!CanAttack())
    {
        return false;
    }
    
    FAnim_CombatAnimData* AnimData = GetCurrentWeaponAnimData();
    if (!AnimData || !AnimData->AttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("No attack animation for current weapon type"));
        return false;
    }
    
    if (OwnerAnimInstance)
    {
        // Play attack montage
        float PlayRate = AnimData->AttackSpeed;
        OwnerAnimInstance->Montage_Play(AnimData->AttackMontage, PlayRate);
        
        // Bind montage end event
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UAnim_CombatController::OnAttackMontageEnded);
        OwnerAnimInstance->Montage_SetEndDelegate(EndDelegate, AnimData->AttackMontage);
        
        // Update combat state
        SetCombatState(EAnim_CombatState::Attacking);
        LastAttackTime = GetWorld()->GetTimeSeconds();
        
        // Handle combo system
        if (bIsInComboWindow)
        {
            CurrentComboCount++;
        }
        else
        {
            CurrentComboCount = 1;
        }
        
        bIsInComboWindow = true;
        
        return true;
    }
    
    return false;
}

bool UAnim_CombatController::TryStartBlock()
{
    if (!CanBlock())
    {
        return false;
    }
    
    FAnim_CombatAnimData* AnimData = GetCurrentWeaponAnimData();
    if (!AnimData || !AnimData->BlockMontage)
    {
        return false;
    }
    
    if (OwnerAnimInstance)
    {
        OwnerAnimInstance->Montage_Play(AnimData->BlockMontage);
        
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UAnim_CombatController::OnBlockMontageEnded);
        OwnerAnimInstance->Montage_SetEndDelegate(EndDelegate, AnimData->BlockMontage);
        
        SetCombatState(EAnim_CombatState::Blocking);
        return true;
    }
    
    return false;
}

bool UAnim_CombatController::TryStartDodge(FVector DodgeDirection)
{
    if (!CanDodge())
    {
        return false;
    }
    
    FAnim_CombatAnimData* AnimData = GetCurrentWeaponAnimData();
    if (!AnimData || !AnimData->DodgeMontage)
    {
        return false;
    }
    
    if (OwnerAnimInstance)
    {
        OwnerAnimInstance->Montage_Play(AnimData->DodgeMontage);
        
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UAnim_CombatController::OnDodgeMontageEnded);
        OwnerAnimInstance->Montage_SetEndDelegate(EndDelegate, AnimData->DodgeMontage);
        
        SetCombatState(EAnim_CombatState::Dodging);
        LastDodgeTime = GetWorld()->GetTimeSeconds();
        
        // Apply dodge movement impulse if owner is a character
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            FVector DodgeImpulse = DodgeDirection.GetSafeNormal() * 800.0f;
            OwnerCharacter->LaunchCharacter(DodgeImpulse, false, false);
        }
        
        return true;
    }
    
    return false;
}

void UAnim_CombatController::StopBlock()
{
    if (CurrentCombatState == EAnim_CombatState::Blocking && OwnerAnimInstance)
    {
        FAnim_CombatAnimData* AnimData = GetCurrentWeaponAnimData();
        if (AnimData && AnimData->BlockMontage)
        {
            OwnerAnimInstance->Montage_Stop(0.2f, AnimData->BlockMontage);
        }
        SetCombatState(EAnim_CombatState::Idle);
    }
}

void UAnim_CombatController::TriggerHitReaction(float Damage, FVector HitDirection)
{
    if (CurrentCombatState == EAnim_CombatState::Dead)
    {
        return;
    }
    
    FAnim_CombatAnimData* AnimData = GetCurrentWeaponAnimData();
    if (!AnimData || !AnimData->HitReactionMontage)
    {
        return;
    }
    
    if (OwnerAnimInstance)
    {
        // Stop current montage if playing
        if (OwnerAnimInstance->IsAnyMontagePlaying())
        {
            OwnerAnimInstance->StopAllMontages(0.1f);
        }
        
        // Play hit reaction
        OwnerAnimInstance->Montage_Play(AnimData->HitReactionMontage);
        
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UAnim_CombatController::OnHitReactionMontageEnded);
        OwnerAnimInstance->Montage_SetEndDelegate(EndDelegate, AnimData->HitReactionMontage);
        
        SetCombatState(EAnim_CombatState::Stunned);
        
        // Apply knockback
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            FVector KnockbackForce = CalculateKnockbackForce(HitDirection) * (Damage / 100.0f);
            OwnerCharacter->LaunchCharacter(KnockbackForce, false, false);
        }
    }
}

void UAnim_CombatController::SetCurrentWeapon(EAnim_WeaponType NewWeaponType)
{
    if (CurrentWeaponType != NewWeaponType)
    {
        CurrentWeaponType = NewWeaponType;
        ResetCombatState();
        
        UE_LOG(LogTemp, Log, TEXT("Weapon changed to: %d"), (int32)NewWeaponType);
    }
}

void UAnim_CombatController::RegisterWeaponAnimations(EAnim_WeaponType WeaponType, const FAnim_CombatAnimData& AnimData)
{
    WeaponAnimations.Add(WeaponType, AnimData);
    UE_LOG(LogTemp, Log, TEXT("Registered animations for weapon type: %d"), (int32)WeaponType);
}

bool UAnim_CombatController::CanAttack() const
{
    return CurrentCombatState == EAnim_CombatState::Idle || 
           (CurrentCombatState == EAnim_CombatState::Attacking && bIsInComboWindow);
}

bool UAnim_CombatController::CanBlock() const
{
    return CurrentCombatState == EAnim_CombatState::Idle;
}

bool UAnim_CombatController::CanDodge() const
{
    if (CurrentCombatState == EAnim_CombatState::Dodging || CurrentCombatState == EAnim_CombatState::Dead)
    {
        return false;
    }
    
    float TimeSinceLastDodge = GetWorld()->GetTimeSeconds() - LastDodgeTime;
    return TimeSinceLastDodge >= DodgeCooldown;
}

bool UAnim_CombatController::IsInCombat() const
{
    return CurrentCombatState != EAnim_CombatState::Idle;
}

void UAnim_CombatController::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        SetCombatState(EAnim_CombatState::Idle);
        
        // Start recovery timer
        if (GetWorld())
        {
            FAnim_CombatAnimData* AnimData = GetCurrentWeaponAnimData();
            float RecoveryTime = AnimData ? AnimData->RecoveryTime : 0.5f;
            
            FTimerHandle RecoveryTimer;
            GetWorld()->GetTimerManager().SetTimer(RecoveryTimer, [this]()
            {
                // Recovery complete - can start new actions
            }, RecoveryTime, false);
        }
    }
}

void UAnim_CombatController::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    SetCombatState(EAnim_CombatState::Idle);
}

void UAnim_CombatController::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    SetCombatState(EAnim_CombatState::Idle);
}

void UAnim_CombatController::OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    SetCombatState(EAnim_CombatState::Idle);
}

void UAnim_CombatController::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        EAnim_CombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_CombatController::UpdateComboWindow(float DeltaTime)
{
    if (bIsInComboWindow)
    {
        float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
        if (TimeSinceLastAttack > ComboWindowTime)
        {
            bIsInComboWindow = false;
            CurrentComboCount = 0;
        }
    }
}

void UAnim_CombatController::InitializeDefaultAnimations()
{
    // Initialize empty animation data for each weapon type
    // These will be populated by the animation blueprint or game data
    for (int32 i = 0; i < (int32)EAnim_WeaponType::Bow + 1; ++i)
    {
        EAnim_WeaponType WeaponType = (EAnim_WeaponType)i;
        if (!WeaponAnimations.Contains(WeaponType))
        {
            WeaponAnimations.Add(WeaponType, FAnim_CombatAnimData());
        }
    }
}

FAnim_CombatAnimData* UAnim_CombatController::GetCurrentWeaponAnimData()
{
    return WeaponAnimations.Find(CurrentWeaponType);
}

void UAnim_CombatController::ProcessAttackCombo()
{
    // Handle combo logic based on current combo count
    if (CurrentComboCount >= 3)
    {
        // Max combo reached - add special effects or damage multiplier
        CurrentComboCount = 0;
        bIsInComboWindow = false;
    }
}

void UAnim_CombatController::ResetCombatState()
{
    SetCombatState(EAnim_CombatState::Idle);
    CurrentComboCount = 0;
    bIsInComboWindow = false;
    
    // Stop any playing montages
    if (OwnerAnimInstance && OwnerAnimInstance->IsAnyMontagePlaying())
    {
        OwnerAnimInstance->StopAllMontages(0.2f);
    }
}

float UAnim_CombatController::CalculateAttackDamage() const
{
    float BaseDamage = 25.0f;
    float ComboMultiplier = 1.0f + (CurrentComboCount * 0.2f);
    
    // Weapon type damage modifiers
    switch (CurrentWeaponType)
    {
        case EAnim_WeaponType::Spear:
            BaseDamage *= 1.5f;
            break;
        case EAnim_WeaponType::Club:
            BaseDamage *= 1.3f;
            break;
        case EAnim_WeaponType::Stone:
            BaseDamage *= 0.8f;
            break;
        case EAnim_WeaponType::Bow:
            BaseDamage *= 1.2f;
            break;
        default:
            BaseDamage *= 0.5f; // Unarmed
            break;
    }
    
    return BaseDamage * ComboMultiplier;
}

FVector UAnim_CombatController::CalculateKnockbackForce(FVector HitDirection) const
{
    FVector NormalizedDirection = HitDirection.GetSafeNormal();
    float BaseForce = 300.0f;
    
    // Add upward component for more dramatic effect
    NormalizedDirection.Z = FMath::Max(NormalizedDirection.Z, 0.3f);
    
    return NormalizedDirection * BaseForce;
}

void UAnim_CombatController::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    // Handle montage blending out
}

void UAnim_CombatController::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // Handle montage ended
}