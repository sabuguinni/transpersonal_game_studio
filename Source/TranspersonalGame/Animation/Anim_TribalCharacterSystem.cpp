#include "Anim_TribalCharacterSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

// UAnim_TribalCharacterController Implementation
UAnim_TribalCharacterController::UAnim_TribalCharacterController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize animation data
    AnimationData = FAnim_TribalAnimationData();

    // Initialize montage references
    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    HuntingMontage = nullptr;
    CommunicationMontage = nullptr;

    // Initialize blend space references
    LocomotionBlendSpace = nullptr;
    CombatBlendSpace = nullptr;

    // Initialize state management
    StateTransitionTimer = 0.0f;
    StateTransitionDuration = 0.5f;
    CurrentBlendValue = 0.0f;
    TargetBlendValue = 0.0f;
    BlendTransitionSpeed = 2.0f;

    // Initialize action montage tracking
    bIsPlayingActionMontage = false;
    CurrentActionMontage = nullptr;

    // Initialize references
    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
}

void UAnim_TribalCharacterController::BeginPlay()
{
    Super::BeginPlay();

    // Get owner character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        // Get animation instance reference
        if (OwnerCharacter->GetMesh())
        {
            AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        }
    }

    // Load default animation assets if not set
    if (!GatheringMontage)
    {
        GatheringMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/TranspersonalGame/Animation/Montages/AM_TribalGathering"));
    }
    if (!CraftingMontage)
    {
        CraftingMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/TranspersonalGame/Animation/Montages/AM_TribalCrafting"));
    }
    if (!HuntingMontage)
    {
        HuntingMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/TranspersonalGame/Animation/Montages/AM_TribalHunting"));
    }
    if (!CommunicationMontage)
    {
        CommunicationMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/TranspersonalGame/Animation/Montages/AM_TribalCommunication"));
    }

    // Load blend spaces
    if (!LocomotionBlendSpace)
    {
        LocomotionBlendSpace = LoadObject<UBlendSpace1D>(nullptr, TEXT("/Game/TranspersonalGame/Animation/BlendSpaces/BS_TribalLocomotion"));
    }
    if (!CombatBlendSpace)
    {
        CombatBlendSpace = LoadObject<UBlendSpace1D>(nullptr, TEXT("/Game/TranspersonalGame/Animation/BlendSpaces/BS_TribalCombat"));
    }
}

void UAnim_TribalCharacterController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateAnimationState(DeltaTime);
}

void UAnim_TribalCharacterController::SetTribalState(EAnim_TribalState NewState)
{
    if (AnimationData.CurrentState != NewState)
    {
        HandleStateTransition(NewState);
        AnimationData.CurrentState = NewState;
    }
}

void UAnim_TribalCharacterController::PlayGatheringAnimation()
{
    if (GatheringMontage && AnimInstance && !bIsPlayingActionMontage)
    {
        AnimInstance->Montage_Play(GatheringMontage);
        bIsPlayingActionMontage = true;
        CurrentActionMontage = GatheringMontage;
        SetTribalState(EAnim_TribalState::Gathering);
    }
}

void UAnim_TribalCharacterController::PlayCraftingAnimation()
{
    if (CraftingMontage && AnimInstance && !bIsPlayingActionMontage)
    {
        AnimInstance->Montage_Play(CraftingMontage);
        bIsPlayingActionMontage = true;
        CurrentActionMontage = CraftingMontage;
        SetTribalState(EAnim_TribalState::Crafting);
    }
}

void UAnim_TribalCharacterController::PlayHuntingAnimation()
{
    if (HuntingMontage && AnimInstance && !bIsPlayingActionMontage)
    {
        AnimInstance->Montage_Play(HuntingMontage);
        bIsPlayingActionMontage = true;
        CurrentActionMontage = HuntingMontage;
        SetTribalState(EAnim_TribalState::Hunting);
    }
}

void UAnim_TribalCharacterController::PlayCommunicationAnimation()
{
    if (CommunicationMontage && AnimInstance && !bIsPlayingActionMontage)
    {
        AnimInstance->Montage_Play(CommunicationMontage);
        bIsPlayingActionMontage = true;
        CurrentActionMontage = CommunicationMontage;
        SetTribalState(EAnim_TribalState::Communicating);
    }
}

void UAnim_TribalCharacterController::UpdateMovementSpeed(float Speed)
{
    AnimationData.MovementSpeed = Speed;

    // Update target blend value based on speed
    if (Speed > 0.1f)
    {
        if (Speed > 300.0f) // Running threshold
        {
            TargetBlendValue = 1.0f;
            if (AnimationData.CurrentState == EAnim_TribalState::Idle || AnimationData.CurrentState == EAnim_TribalState::Walking)
            {
                SetTribalState(EAnim_TribalState::Running);
            }
        }
        else // Walking
        {
            TargetBlendValue = Speed / 300.0f;
            if (AnimationData.CurrentState == EAnim_TribalState::Idle)
            {
                SetTribalState(EAnim_TribalState::Walking);
            }
        }
    }
    else
    {
        TargetBlendValue = 0.0f;
        if (AnimationData.CurrentState == EAnim_TribalState::Walking || AnimationData.CurrentState == EAnim_TribalState::Running)
        {
            SetTribalState(EAnim_TribalState::Idle);
        }
    }
}

void UAnim_TribalCharacterController::SetCarryingObject(bool bCarrying)
{
    AnimationData.bIsCarryingObject = bCarrying;
}

void UAnim_TribalCharacterController::SetInjuredState(bool bInjured)
{
    AnimationData.bIsInjured = bInjured;
    if (bInjured)
    {
        SetTribalState(EAnim_TribalState::Injured);
    }
}

void UAnim_TribalCharacterController::SetEmotionalState(float EmotionLevel)
{
    AnimationData.EmotionalState = FMath::Clamp(EmotionLevel, 0.0f, 1.0f);
}

EAnim_TribalState UAnim_TribalCharacterController::GetCurrentTribalState() const
{
    return AnimationData.CurrentState;
}

bool UAnim_TribalCharacterController::IsPlayingActionMontage() const
{
    return bIsPlayingActionMontage;
}

void UAnim_TribalCharacterController::UpdateAnimationState(float DeltaTime)
{
    // Update state transition timer
    if (StateTransitionTimer > 0.0f)
    {
        StateTransitionTimer -= DeltaTime;
    }

    // Update movement blending
    ApplyMovementBlending();

    // Check if action montage is still playing
    if (bIsPlayingActionMontage && AnimInstance && CurrentActionMontage)
    {
        if (!AnimInstance->Montage_IsPlaying(CurrentActionMontage))
        {
            bIsPlayingActionMontage = false;
            CurrentActionMontage = nullptr;
            
            // Return to appropriate idle/movement state
            if (AnimationData.MovementSpeed > 0.1f)
            {
                SetTribalState(AnimationData.MovementSpeed > 300.0f ? EAnim_TribalState::Running : EAnim_TribalState::Walking);
            }
            else
            {
                SetTribalState(EAnim_TribalState::Idle);
            }
        }
    }

    // Update character movement speed if we have a character reference
    if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
    {
        FVector Velocity = OwnerCharacter->GetCharacterMovement()->Velocity;
        float CurrentSpeed = Velocity.Size();
        UpdateMovementSpeed(CurrentSpeed);
    }
}

void UAnim_TribalCharacterController::HandleStateTransition(EAnim_TribalState NewState)
{
    StateTransitionTimer = StateTransitionDuration;
    
    // Handle specific state transitions
    switch (NewState)
    {
        case EAnim_TribalState::Combat:
            // Switch to combat blend space
            break;
        case EAnim_TribalState::Injured:
            // Apply injury modifiers
            BlendTransitionSpeed = 1.0f; // Slower transitions when injured
            break;
        default:
            BlendTransitionSpeed = 2.0f; // Normal transition speed
            break;
    }
}

void UAnim_TribalCharacterController::ApplyMovementBlending()
{
    // Smooth blend value transition
    CurrentBlendValue = FMath::FInterpTo(CurrentBlendValue, TargetBlendValue, GetWorld()->GetDeltaSeconds(), BlendTransitionSpeed);
}

// UAnim_TribalAnimInstance Implementation
UAnim_TribalAnimInstance::UAnim_TribalAnimInstance()
{
    // Initialize animation properties
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;

    // Initialize tribal states
    bIsGathering = false;
    bIsCrafting = false;
    bIsHunting = false;
    bIsCommunicating = false;
    bIsCarryingLoad = false;
    EmotionalIntensity = 0.5f;

    // Initialize references
    Character = nullptr;
    TribalController = nullptr;

    // Initialize cached values
    Velocity = FVector::ZeroVector;
    CharacterRotation = FRotator::ZeroRotator;
    LastFrameRotation = FRotator::ZeroRotator;
}

void UAnim_TribalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character reference
    Character = Cast<ACharacter>(GetOwningActor());
    if (Character)
    {
        // Get tribal controller component
        TribalController = Character->FindComponentByClass<UAnim_TribalCharacterController>();
    }
}

void UAnim_TribalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (Character)
    {
        UpdateMovementValues();
        UpdateTribalStates();
        UpdateEmotionalBlending();
    }
}

void UAnim_TribalAnimInstance::UpdateMovementValues()
{
    if (!Character) return;

    // Get movement component
    UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
    if (!MovementComponent) return;

    // Update velocity and speed
    Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();

    // Calculate direction relative to character rotation
    CharacterRotation = Character->GetActorRotation();
    if (Speed > 0.1f)
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal();
        Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if direction is left or right
        FVector RightVector = Character->GetActorRightVector();
        if (FVector::DotProduct(RightVector, VelocityDirection) < 0.0f)
        {
            Direction *= -1.0f;
        }
    }
    else
    {
        Direction = 0.0f;
    }

    // Check if character is in air
    bIsInAir = MovementComponent->IsFalling();

    // Check if character is accelerating
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    bIsAccelerating = Acceleration.SizeSquared() > 0.1f;

    // Store rotation for next frame
    LastFrameRotation = CharacterRotation;
}

void UAnim_TribalAnimInstance::UpdateTribalStates()
{
    if (!TribalController) return;

    // Get current tribal data
    TribalData = TribalController->AnimationData;

    // Update tribal state booleans
    bIsGathering = (TribalData.CurrentState == EAnim_TribalState::Gathering);
    bIsCrafting = (TribalData.CurrentState == EAnim_TribalState::Crafting);
    bIsHunting = (TribalData.CurrentState == EAnim_TribalState::Hunting);
    bIsCommunicating = (TribalData.CurrentState == EAnim_TribalState::Communicating);
    bIsCarryingLoad = TribalData.bIsCarryingObject;
}

void UAnim_TribalAnimInstance::UpdateEmotionalBlending()
{
    if (!TribalController) return;

    // Update emotional intensity for animation blending
    EmotionalIntensity = TribalData.EmotionalState;

    // Apply emotional state to movement speed modifiers
    if (EmotionalIntensity < 0.3f) // Fearful state
    {
        // Character moves more cautiously, slower transitions
    }
    else if (EmotionalIntensity > 0.7f) // Confident state
    {
        // Character moves more boldly, faster transitions
    }
}