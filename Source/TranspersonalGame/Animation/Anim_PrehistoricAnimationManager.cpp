#include "Anim_PrehistoricAnimationManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PrehistoricAnimationManager::UAnim_PrehistoricAnimationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    LastUpdateTime = 0.0f;
    LastVelocity = FVector::ZeroVector;
    bWasInAir = false;
    LandingTimer = 0.0f;
    ActionTimer = 0.0f;
    
    // Initialize pointers
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
    MovementComponent = nullptr;
    
    // Initialize animation assets to null
    LocomotionBlendSpace = nullptr;
    IdleBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    CrouchMontage = nullptr;
    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    AttackMontage = nullptr;
    DeathMontage = nullptr;
}

void UAnim_PrehistoricAnimationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCharacterReferences();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    if (IsValidForAnimation())
    {
        UE_LOG(LogTemp, Log, TEXT("Prehistoric Animation Manager initialized for character: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Prehistoric Animation Manager failed to initialize properly"));
    }
}

void UAnim_PrehistoricAnimationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsValidForAnimation())
    {
        return;
    }
    
    UpdateAnimationData(DeltaTime);
    HandleStateTransitions();
}

void UAnim_PrehistoricAnimationManager::UpdateAnimationData(float DeltaTime)
{
    if (!IsValidForAnimation())
    {
        return;
    }
    
    UpdateMovementData(DeltaTime);
    UpdateActionData(DeltaTime);
    UpdateSurvivalStats();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_PrehistoricAnimationManager::UpdateMovementData(float DeltaTime)
{
    // Get current velocity and speed
    FVector CurrentVelocity = MovementComponent->Velocity;
    AnimationData.Speed = CurrentVelocity.Size();
    
    // Calculate direction relative to character forward
    AnimationData.Direction = CalculateDirection();
    
    // Update air state
    bWasInAir = AnimationData.bIsInAir;
    AnimationData.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouching state
    AnimationData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine movement state
    AnimationData.MovementState = DetermineMovementState();
    
    // Handle landing detection
    if (bWasInAir && !AnimationData.bIsInAir)
    {
        StartLanding();
    }
    
    // Update landing timer
    if (LandingTimer > 0.0f)
    {
        LandingTimer -= DeltaTime;
        if (LandingTimer <= 0.0f)
        {
            LandingTimer = 0.0f;
        }
    }
    
    LastVelocity = CurrentVelocity;
}

void UAnim_PrehistoricAnimationManager::UpdateActionData(float DeltaTime)
{
    // Update action timer
    if (ActionTimer > 0.0f)
    {
        ActionTimer -= DeltaTime;
        if (ActionTimer <= 0.0f)
        {
            ActionTimer = 0.0f;
            if (AnimationData.ActionState != EAnim_PrehistoricActionState::Dead)
            {
                AnimationData.ActionState = EAnim_PrehistoricActionState::None;
            }
        }
    }
}

void UAnim_PrehistoricAnimationManager::UpdateSurvivalStats()
{
    // This would typically get data from a survival component
    // For now, we'll use placeholder logic
    
    // Health percentage (would come from health component)
    AnimationData.HealthPercent = FMath::Clamp(AnimationData.HealthPercent, 0.0f, 1.0f);
    
    // Stamina percentage (would come from stamina system)
    if (AnimationData.Speed > 400.0f) // Running depletes stamina
    {
        AnimationData.StaminaPercent = FMath::Max(0.0f, AnimationData.StaminaPercent - 0.1f * GetWorld()->GetDeltaSeconds());
    }
    else if (AnimationData.MovementState == EAnim_PrehistoricMovementState::Idle)
    {
        AnimationData.StaminaPercent = FMath::Min(1.0f, AnimationData.StaminaPercent + 0.2f * GetWorld()->GetDeltaSeconds());
    }
    
    // Fear level (would be influenced by nearby threats)
    AnimationData.FearLevel = FMath::Clamp(AnimationData.FearLevel, 0.0f, 1.0f);
}

void UAnim_PrehistoricAnimationManager::HandleStateTransitions()
{
    // Handle automatic state transitions based on conditions
    
    // If health is zero, transition to death
    if (AnimationData.HealthPercent <= 0.0f && AnimationData.ActionState != EAnim_PrehistoricActionState::Dead)
    {
        StartDeath();
    }
    
    // If stamina is very low, prevent sprinting
    if (AnimationData.StaminaPercent < 0.1f && AnimationData.MovementState == EAnim_PrehistoricMovementState::Sprinting)
    {
        AnimationData.MovementState = EAnim_PrehistoricMovementState::Running;
    }
}

float UAnim_PrehistoricAnimationManager::CalculateDirection() const
{
    if (!IsValidForAnimation() || AnimationData.Speed < 1.0f)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = MovementComponent->Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection);
    
    float Angle = FMath::Acos(DotProduct) * (180.0f / PI);
    if (CrossProduct.Z < 0.0f)
    {
        Angle = -Angle;
    }
    
    return Angle;
}

EAnim_PrehistoricMovementState UAnim_PrehistoricAnimationManager::DetermineMovementState() const
{
    if (!IsValidForAnimation())
    {
        return EAnim_PrehistoricMovementState::Idle;
    }
    
    // Check for air states first
    if (AnimationData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_PrehistoricMovementState::Jumping;
        }
        else
        {
            return EAnim_PrehistoricMovementState::Falling;
        }
    }
    
    // Check for landing
    if (LandingTimer > 0.0f)
    {
        return EAnim_PrehistoricMovementState::Landing;
    }
    
    // Check for crouching states
    if (AnimationData.bIsCrouching)
    {
        if (AnimationData.Speed > 1.0f)
        {
            return EAnim_PrehistoricMovementState::Crawling;
        }
        else
        {
            return EAnim_PrehistoricMovementState::Crouching;
        }
    }
    
    // Determine movement based on speed
    if (AnimationData.Speed < 1.0f)
    {
        return EAnim_PrehistoricMovementState::Idle;
    }
    else if (AnimationData.Speed < 200.0f)
    {
        return EAnim_PrehistoricMovementState::Walking;
    }
    else if (AnimationData.Speed < 400.0f)
    {
        return EAnim_PrehistoricMovementState::Running;
    }
    else
    {
        return EAnim_PrehistoricMovementState::Sprinting;
    }
}

void UAnim_PrehistoricAnimationManager::SetMovementState(EAnim_PrehistoricMovementState NewState)
{
    if (AnimationData.MovementState != NewState)
    {
        AnimationData.MovementState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Movement state changed to: %d"), (int32)NewState);
    }
}

void UAnim_PrehistoricAnimationManager::SetActionState(EAnim_PrehistoricActionState NewState)
{
    if (AnimationData.ActionState != NewState)
    {
        AnimationData.ActionState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Action state changed to: %d"), (int32)NewState);
    }
}

void UAnim_PrehistoricAnimationManager::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!IsValidForAnimation() || !Montage)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s"), *Montage->GetName());
    }
}

void UAnim_PrehistoricAnimationManager::StopMontage(UAnimMontage* Montage)
{
    if (!IsValidForAnimation() || !Montage)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
    }
}

bool UAnim_PrehistoricAnimationManager::IsMontagePlayingAny() const
{
    if (!IsValidForAnimation())
    {
        return false;
    }
    
    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (AnimInstance)
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
    
    return false;
}

void UAnim_PrehistoricAnimationManager::StartJump()
{
    if (JumpMontage)
    {
        PlayMontage(JumpMontage);
    }
    SetMovementState(EAnim_PrehistoricMovementState::Jumping);
}

void UAnim_PrehistoricAnimationManager::StartLanding()
{
    if (LandMontage)
    {
        PlayMontage(LandMontage);
    }
    LandingTimer = 0.5f; // Landing state duration
    SetMovementState(EAnim_PrehistoricMovementState::Landing);
}

void UAnim_PrehistoricAnimationManager::StartCrouch()
{
    if (CrouchMontage)
    {
        PlayMontage(CrouchMontage);
    }
    SetMovementState(EAnim_PrehistoricMovementState::Crouching);
}

void UAnim_PrehistoricAnimationManager::StopCrouch()
{
    if (CrouchMontage)
    {
        StopMontage(CrouchMontage);
    }
}

void UAnim_PrehistoricAnimationManager::StartGathering()
{
    if (GatheringMontage)
    {
        PlayMontage(GatheringMontage);
    }
    SetActionState(EAnim_PrehistoricActionState::Gathering);
    ActionTimer = 3.0f; // Gathering duration
}

void UAnim_PrehistoricAnimationManager::StartCrafting()
{
    if (CraftingMontage)
    {
        PlayMontage(CraftingMontage);
    }
    SetActionState(EAnim_PrehistoricActionState::Crafting);
    ActionTimer = 5.0f; // Crafting duration
}

void UAnim_PrehistoricAnimationManager::StartAttack()
{
    if (AttackMontage)
    {
        PlayMontage(AttackMontage);
    }
    SetActionState(EAnim_PrehistoricActionState::Fighting);
    ActionTimer = 2.0f; // Attack duration
}

void UAnim_PrehistoricAnimationManager::StartDeath()
{
    if (DeathMontage)
    {
        PlayMontage(DeathMontage);
    }
    SetActionState(EAnim_PrehistoricActionState::Dead);
    // Death state is permanent (no timer)
}

void UAnim_PrehistoricAnimationManager::StopCurrentAction()
{
    ActionTimer = 0.0f;
    if (AnimationData.ActionState != EAnim_PrehistoricActionState::Dead)
    {
        SetActionState(EAnim_PrehistoricActionState::None);
    }
}

void UAnim_PrehistoricAnimationManager::InitializeCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

bool UAnim_PrehistoricAnimationManager::IsValidForAnimation() const
{
    return OwnerCharacter && CharacterMesh && MovementComponent;
}