#include "Anim_CharacterAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimController::UAnim_CharacterAnimController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize animation properties
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsRunning = false;
    bIsInCombat = false;
    
    // Initialize state tracking
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::None;
    CurrentEmotionalState = EAnim_EmotionalState::Neutral;
    
    BlendTimeRemaining = 0.0f;
    CurrentAnimationTime = 0.0f;
}

void UAnim_CharacterAnimController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAnimationReferences();
}

void UAnim_CharacterAnimController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        // Update movement properties
        MovementSpeed = MovementComponent->Velocity.Size();
        bIsInAir = MovementComponent->IsFalling();
        bIsCrouching = OwnerCharacter->bIsCrouched;
        bIsRunning = MovementSpeed > 300.0f; // Running threshold
        
        // Update animation states
        UpdateMovementState(DeltaTime);
        UpdateCombatState(DeltaTime);
        BlendAnimations(DeltaTime);
        
        CurrentAnimationTime += DeltaTime;
    }
}

void UAnim_CharacterAnimController::InitializeAnimationReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (CharacterMesh)
        {
            AnimInstance = CharacterMesh->GetAnimInstance();
        }
    }
}

void UAnim_CharacterAnimController::UpdateMovementAnimations(float Speed, bool bInAir, bool bCrouching)
{
    MovementSpeed = Speed;
    bIsInAir = bInAir;
    bIsCrouching = bCrouching;
    
    // Determine movement state based on parameters
    if (bIsInAir)
    {
        CurrentMovementState = EAnim_MovementState::Jumping;
    }
    else if (bIsCrouching)
    {
        CurrentMovementState = EAnim_MovementState::Crouching;
    }
    else if (Speed > 300.0f)
    {
        CurrentMovementState = EAnim_MovementState::Running;
    }
    else if (Speed > 50.0f)
    {
        CurrentMovementState = EAnim_MovementState::Walking;
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_CharacterAnimController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing animation montage: %s"), *Montage->GetName());
    }
}

void UAnim_CharacterAnimController::StopMontage(UAnimMontage* Montage)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        UE_LOG(LogTemp, Log, TEXT("Stopping animation montage: %s"), *Montage->GetName());
    }
}

void UAnim_CharacterAnimController::SetAnimationBlendWeight(const FString& AnimationName, float Weight)
{
    if (AnimInstance)
    {
        // This would typically interact with blend spaces or animation blueprints
        UE_LOG(LogTemp, Log, TEXT("Setting animation blend weight for %s: %f"), *AnimationName, Weight);
    }
}

void UAnim_CharacterAnimController::PlayAttackAnimation(EAnim_AttackType AttackType)
{
    CurrentCombatState = EAnim_CombatState::Attacking;
    
    switch (AttackType)
    {
        case EAnim_AttackType::Light:
            if (AttackMontage)
            {
                PlayMontage(AttackMontage, 1.2f); // Faster for light attacks
            }
            break;
        case EAnim_AttackType::Heavy:
            if (AttackMontage)
            {
                PlayMontage(AttackMontage, 0.8f); // Slower for heavy attacks
            }
            break;
        case EAnim_AttackType::Ranged:
            // Would use different montage for ranged attacks
            if (AttackMontage)
            {
                PlayMontage(AttackMontage, 1.0f);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing attack animation: %d"), (int32)AttackType);
}

void UAnim_CharacterAnimController::PlayDefenseAnimation(bool bIsBlocking)
{
    if (bIsBlocking)
    {
        CurrentCombatState = EAnim_CombatState::Blocking;
        if (DefenseMontage)
        {
            PlayMontage(DefenseMontage);
        }
    }
    else
    {
        CurrentCombatState = EAnim_CombatState::None;
        if (DefenseMontage)
        {
            StopMontage(DefenseMontage);
        }
    }
}

void UAnim_CharacterAnimController::PlayDodgeAnimation(EAnim_DodgeDirection Direction)
{
    CurrentCombatState = EAnim_CombatState::Dodging;
    
    // Different dodge animations based on direction
    if (AttackMontage) // Using attack montage as placeholder
    {
        float PlayRate = 1.5f; // Dodges should be quick
        PlayMontage(AttackMontage, PlayRate);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing dodge animation: %d"), (int32)Direction);
}

void UAnim_CharacterAnimController::PlayCraftingAnimation(EAnim_CraftingType CraftingType)
{
    switch (CraftingType)
    {
        case EAnim_CraftingType::ToolMaking:
            if (CraftingMontage)
            {
                PlayMontage(CraftingMontage, 0.8f); // Slower, precise movements
            }
            break;
        case EAnim_CraftingType::WeaponCrafting:
            if (CraftingMontage)
            {
                PlayMontage(CraftingMontage, 1.0f);
            }
            break;
        case EAnim_CraftingType::ShelterBuilding:
            if (CraftingMontage)
            {
                PlayMontage(CraftingMontage, 0.9f);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing crafting animation: %d"), (int32)CraftingType);
}

void UAnim_CharacterAnimController::PlayGatheringAnimation(EAnim_ResourceType ResourceType)
{
    switch (ResourceType)
    {
        case EAnim_ResourceType::Stone:
            if (GatheringMontage)
            {
                PlayMontage(GatheringMontage, 1.1f); // Slightly faster for stone gathering
            }
            break;
        case EAnim_ResourceType::Wood:
            if (GatheringMontage)
            {
                PlayMontage(GatheringMontage, 0.9f); // Slower for wood chopping
            }
            break;
        case EAnim_ResourceType::Food:
            if (GatheringMontage)
            {
                PlayMontage(GatheringMontage, 1.2f); // Quick picking motions
            }
            break;
        case EAnim_ResourceType::Water:
            if (GatheringMontage)
            {
                PlayMontage(GatheringMontage, 1.0f);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing gathering animation: %d"), (int32)ResourceType);
}

void UAnim_CharacterAnimController::PlayEmotionalAnimation(EAnim_EmotionalState EmotionalState)
{
    CurrentEmotionalState = EmotionalState;
    
    switch (EmotionalState)
    {
        case EAnim_EmotionalState::Fear:
            // Fear animations would show trembling, defensive postures
            if (DefenseMontage)
            {
                PlayMontage(DefenseMontage, 0.7f);
            }
            break;
        case EAnim_EmotionalState::Aggressive:
            // Aggressive animations would show threatening postures
            if (AttackMontage)
            {
                PlayMontage(AttackMontage, 0.5f);
            }
            break;
        case EAnim_EmotionalState::Tired:
            // Tired animations would show slouched postures, slower movements
            if (IdleMontage)
            {
                PlayMontage(IdleMontage, 0.6f);
            }
            break;
        case EAnim_EmotionalState::Alert:
            // Alert animations would show upright, scanning postures
            if (IdleMontage)
            {
                PlayMontage(IdleMontage, 1.3f);
            }
            break;
        default:
            // Neutral state
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing emotional animation: %d"), (int32)EmotionalState);
}

void UAnim_CharacterAnimController::UpdateMovementState(float DeltaTime)
{
    // Smooth transitions between movement states
    EAnim_MovementState NewState = CurrentMovementState;
    
    if (bIsInAir)
    {
        NewState = EAnim_MovementState::Jumping;
    }
    else if (bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MovementSpeed > 300.0f)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (MovementSpeed > 50.0f)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }
    
    // Handle state transitions
    if (NewState != CurrentMovementState)
    {
        CurrentMovementState = NewState;
        BlendTimeRemaining = 0.3f; // 300ms blend time
        
        // Play appropriate montage for new state
        switch (CurrentMovementState)
        {
            case EAnim_MovementState::Idle:
                if (IdleMontage) PlayMontage(IdleMontage);
                break;
            case EAnim_MovementState::Walking:
                if (WalkMontage) PlayMontage(WalkMontage);
                break;
            case EAnim_MovementState::Running:
                if (RunMontage) PlayMontage(RunMontage);
                break;
            case EAnim_MovementState::Jumping:
                if (JumpMontage) PlayMontage(JumpMontage);
                break;
            default:
                break;
        }
    }
}

void UAnim_CharacterAnimController::UpdateCombatState(float DeltaTime)
{
    // Combat state management
    if (CurrentCombatState != EAnim_CombatState::None)
    {
        // Auto-return to none state after combat actions complete
        // This would typically be handled by animation notify events
    }
}

void UAnim_CharacterAnimController::BlendAnimations(float DeltaTime)
{
    if (BlendTimeRemaining > 0.0f)
    {
        BlendTimeRemaining -= DeltaTime;
        
        // Calculate blend weight
        float BlendWeight = FMath::Clamp(BlendTimeRemaining / 0.3f, 0.0f, 1.0f);
        
        // Apply blending logic here
        // This would typically involve setting blend space parameters
        // or animation blueprint variables
    }
}