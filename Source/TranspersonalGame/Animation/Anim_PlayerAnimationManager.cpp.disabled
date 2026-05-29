#include "Anim_PlayerAnimationManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PlayerAnimationManager::UAnim_PlayerAnimationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    PreviousState = EAnim_PlayerState::Idle;
    StateChangeTimer = 0.0f;
    MinStateChangeDuration = 0.1f;
    
    // Initialize pointers
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
    AnimInstance = nullptr;
    
    // Initialize animation assets
    LocomotionBlendSpace = nullptr;
    JumpMontage = nullptr;
    CraftingMontage = nullptr;
    GatheringMontage = nullptr;
    CombatMontage = nullptr;
}

void UAnim_PlayerAnimationManager::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponents();
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("PlayerAnimationManager initialized for character: %s"), *OwnerCharacter->GetName());
    }
}

void UAnim_PlayerAnimationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateChangeTimer += DeltaTime;
    
    UpdateAnimationData();
    UpdateBlendSpaceParameters();
}

void UAnim_PlayerAnimationManager::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (MeshComponent)
        {
            AnimInstance = MeshComponent->GetAnimInstance();
        }
    }
}

void UAnim_PlayerAnimationManager::UpdateAnimationData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update basic movement data
    FVector Velocity = OwnerCharacter->GetVelocity();
    AnimationData.Speed = Velocity.Size();
    
    // Calculate direction relative to character's forward vector
    if (AnimationData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        AnimationData.Direction = FMath::Atan2(
            FVector::CrossProduct(ForwardVector, VelocityNormalized).Z,
            FVector::DotProduct(ForwardVector, VelocityNormalized)
        ) * 180.0f / PI;
    }
    else
    {
        AnimationData.Direction = 0.0f;
    }
    
    // Update movement state
    AnimationData.bIsInAir = MovementComponent->IsFalling();
    AnimationData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate and update player state
    EAnim_PlayerState NewState = CalculatePlayerState();
    if (NewState != AnimationData.CurrentState && StateChangeTimer >= MinStateChangeDuration)
    {
        HandleStateTransition(NewState);
    }
}

EAnim_PlayerState UAnim_PlayerAnimationManager::CalculatePlayerState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_PlayerState::Idle;
    }
    
    // Check for air states first
    if (AnimationData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_PlayerState::Jumping;
        }
        else
        {
            return EAnim_PlayerState::Falling;
        }
    }
    
    // Check for crouching
    if (AnimationData.bIsCrouching)
    {
        return EAnim_PlayerState::Crouching;
    }
    
    // Check for movement states
    if (AnimationData.Speed > 0.1f)
    {
        if (AnimationData.Speed > 300.0f) // Running threshold
        {
            return EAnim_PlayerState::Running;
        }
        else
        {
            return EAnim_PlayerState::Walking;
        }
    }
    
    // Default to idle
    return EAnim_PlayerState::Idle;
}

void UAnim_PlayerAnimationManager::HandleStateTransition(EAnim_PlayerState NewState)
{
    EAnim_PlayerState OldState = AnimationData.CurrentState;
    AnimationData.CurrentState = NewState;
    PreviousState = OldState;
    StateChangeTimer = 0.0f;
    
    // Trigger Blueprint event
    OnStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Player state changed from %d to %d"), (int32)OldState, (int32)NewState);
}

void UAnim_PlayerAnimationManager::SetPlayerState(EAnim_PlayerState NewState)
{
    if (NewState != AnimationData.CurrentState)
    {
        HandleStateTransition(NewState);
    }
}

void UAnim_PlayerAnimationManager::SetMovementDirection(const FVector& MovementInput)
{
    AnimationData.MovementDirection = CalculateMovementDirection(MovementInput);
}

EAnim_MovementDirection UAnim_PlayerAnimationManager::CalculateMovementDirection(const FVector& MovementInput) const
{
    if (MovementInput.IsNearlyZero())
    {
        return EAnim_MovementDirection::Forward;
    }
    
    float ForwardInput = MovementInput.X;
    float RightInput = MovementInput.Y;
    
    // Determine primary direction
    if (FMath::Abs(ForwardInput) > FMath::Abs(RightInput))
    {
        if (ForwardInput > 0)
        {
            if (RightInput > 0.3f)
                return EAnim_MovementDirection::ForwardRight;
            else if (RightInput < -0.3f)
                return EAnim_MovementDirection::ForwardLeft;
            else
                return EAnim_MovementDirection::Forward;
        }
        else
        {
            if (RightInput > 0.3f)
                return EAnim_MovementDirection::BackwardRight;
            else if (RightInput < -0.3f)
                return EAnim_MovementDirection::BackwardLeft;
            else
                return EAnim_MovementDirection::Backward;
        }
    }
    else
    {
        if (RightInput > 0)
            return EAnim_MovementDirection::Right;
        else
            return EAnim_MovementDirection::Left;
    }
}

void UAnim_PlayerAnimationManager::UpdateSurvivalStats(float Health, float Stamina, float Fear)
{
    AnimationData.HealthPercentage = FMath::Clamp(Health, 0.0f, 1.0f);
    AnimationData.StaminaPercentage = FMath::Clamp(Stamina, 0.0f, 1.0f);
    AnimationData.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    
    // Update state based on survival stats
    if (AnimationData.HealthPercentage < 0.3f)
    {
        SetPlayerState(EAnim_PlayerState::Injured);
    }
    else if (AnimationData.StaminaPercentage < 0.1f)
    {
        SetPlayerState(EAnim_PlayerState::Exhausted);
    }
    else if (AnimationData.FearLevel > 0.7f)
    {
        SetPlayerState(EAnim_PlayerState::Afraid);
    }
}

void UAnim_PlayerAnimationManager::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !AnimInstance)
    {
        return;
    }
    
    if (!IsPlayingMontage(Montage))
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        OnMontageStarted(Montage);
        
        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s"), *Montage->GetName());
    }
}

void UAnim_PlayerAnimationManager::StopMontage(UAnimMontage* Montage)
{
    if (!Montage || !AnimInstance)
    {
        return;
    }
    
    if (IsPlayingMontage(Montage))
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        OnMontageEnded(Montage);
        
        UE_LOG(LogTemp, Log, TEXT("Stopped montage: %s"), *Montage->GetName());
    }
}

bool UAnim_PlayerAnimationManager::IsPlayingMontage(UAnimMontage* Montage) const
{
    if (!Montage || !AnimInstance)
    {
        return false;
    }
    
    return AnimInstance->Montage_IsPlaying(Montage);
}

void UAnim_PlayerAnimationManager::UpdateBlendSpaceParameters()
{
    if (!AnimInstance || !LocomotionBlendSpace)
    {
        return;
    }
    
    // Update blend space parameters based on current animation data
    // This would typically be done in the Animation Blueprint,
    // but we can set up the data here for Blueprint consumption
    
    // The actual blend space parameter setting would be done
    // in the Animation Blueprint using the AnimationData struct
}