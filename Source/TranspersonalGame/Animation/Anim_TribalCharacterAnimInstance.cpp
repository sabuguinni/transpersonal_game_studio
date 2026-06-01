#include "Anim_TribalCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_TribalCharacterAnimInstance::UAnim_TribalCharacterAnimInstance()
{
    // Initialize default values
    MovementSpeedThreshold = 10.0f;
    RunSpeedThreshold = 300.0f;
    AnimationBlendSpeed = 5.0f;
    bEnableFootIK = true;
    FootIKInterpSpeed = 15.0f;
    FootIKTraceDistance = 50.0f;
    
    // Initialize animation data
    AnimData = FAnim_TribalAnimationData();
    
    // Initialize references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Initialize animation assets to nullptr
    IdleAnimation = nullptr;
    WalkRunBlendSpace = nullptr;
    JumpStartAnimation = nullptr;
    JumpLoopAnimation = nullptr;
    JumpEndAnimation = nullptr;
    SpearThrowMontage = nullptr;
    CraftingMontage = nullptr;
    GatheringMontage = nullptr;
    FearReactionMontage = nullptr;
    
    // Initialize internal state
    LastUpdateTime = 0.0f;
    bWasInAir = false;
    PreviousMovementState = EAnim_TribalMovementState::Idle;
    PreviousActionState = EAnim_TribalActionState::None;
}

void UAnim_TribalCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        // Get movement component
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterAnimInstance: Failed to get owner character"));
    }
}

void UAnim_TribalCharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    LastUpdateTime += DeltaTimeX;
    
    // Update all animation data
    UpdateMovementData();
    UpdateMovementState();
    UpdateActionState();
    UpdateSurvivalData();
    UpdateToolData();
    
    // Store previous states
    PreviousMovementState = AnimData.MovementState;
    PreviousActionState = AnimData.ActionState;
    bWasInAir = AnimData.bIsInAir;
}

void UAnim_TribalCharacterAnimInstance::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    AnimData.Speed = Velocity.Size();
    
    // Calculate direction relative to character forward
    AnimData.Direction = CalculateDirection();
    
    // Check if character is in air
    AnimData.bIsInAir = MovementComponent->IsFalling();
    
    // Check if character is crouching
    AnimData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_TribalCharacterAnimInstance::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Determine movement state based on current conditions
    if (AnimData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            AnimData.MovementState = EAnim_TribalMovementState::Jumping;
        }
        else
        {
            AnimData.MovementState = EAnim_TribalMovementState::Falling;
        }
    }
    else if (AnimData.bIsCrouching)
    {
        AnimData.MovementState = EAnim_TribalMovementState::Crouching;
    }
    else if (IsRunning())
    {
        AnimData.MovementState = EAnim_TribalMovementState::Running;
    }
    else if (IsMoving())
    {
        AnimData.MovementState = EAnim_TribalMovementState::Walking;
    }
    else
    {
        AnimData.MovementState = EAnim_TribalMovementState::Idle;
    }
}

void UAnim_TribalCharacterAnimInstance::UpdateActionState()
{
    // Action state is primarily controlled externally through function calls
    // This function handles automatic state transitions and timeouts
    
    // Example: Auto-return to None state after certain actions
    if (AnimData.ActionState == EAnim_TribalActionState::Throwing)
    {
        // Check if throw animation has completed
        if (!IsAnyMontagePlaying())
        {
            AnimData.ActionState = EAnim_TribalActionState::None;
        }
    }
}

void UAnim_TribalCharacterAnimInstance::UpdateSurvivalData()
{
    // Update fear level based on game state
    // This would typically be driven by external systems
    
    // Clamp fear level
    AnimData.FearLevel = FMath::Clamp(AnimData.FearLevel, 0.0f, 1.0f);
    
    // Clamp energy level
    AnimData.EnergyLevel = FMath::Clamp(AnimData.EnergyLevel, 0.0f, 1.0f);
}

void UAnim_TribalCharacterAnimInstance::UpdateToolData()
{
    // Tool data is primarily set externally
    // This function can handle automatic detection if needed
    
    // Example: Detect tools in character's hands
    // This would require integration with inventory/equipment system
}

void UAnim_TribalCharacterAnimInstance::SetMovementState(EAnim_TribalMovementState NewState)
{
    if (AnimData.MovementState != NewState)
    {
        PreviousMovementState = AnimData.MovementState;
        AnimData.MovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Movement state changed to %d"), 
               (int32)NewState);
    }
}

void UAnim_TribalCharacterAnimInstance::SetActionState(EAnim_TribalActionState NewState)
{
    if (AnimData.ActionState != NewState)
    {
        PreviousActionState = AnimData.ActionState;
        AnimData.ActionState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Action state changed to %d"), 
               (int32)NewState);
    }
}

void UAnim_TribalCharacterAnimInstance::PlayActionMontage(EAnim_TribalActionState ActionType)
{
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (ActionType)
    {
        case EAnim_TribalActionState::Gathering:
            MontageToPlay = GatheringMontage;
            break;
        case EAnim_TribalActionState::Hunting:
            MontageToPlay = SpearThrowMontage;
            break;
        case EAnim_TribalActionState::Crafting:
            MontageToPlay = CraftingMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay)
    {
        Montage_Play(MontageToPlay);
        SetActionState(ActionType);
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Playing action montage for state %d"), 
               (int32)ActionType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterAnimInstance: No montage found for action state %d"), 
               (int32)ActionType);
    }
}

void UAnim_TribalCharacterAnimInstance::StopActionMontage()
{
    if (IsAnyMontagePlaying())
    {
        Montage_Stop(0.2f);
        SetActionState(EAnim_TribalActionState::None);
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Stopped action montage"));
    }
}

void UAnim_TribalCharacterAnimInstance::TriggerFearReaction(float FearIntensity)
{
    AnimData.FearLevel = FMath::Clamp(FearIntensity, 0.0f, 1.0f);
    
    if (FearReactionMontage && FearIntensity > 0.5f)
    {
        Montage_Play(FearReactionMontage);
        SetActionState(EAnim_TribalActionState::Fleeing);
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Triggered fear reaction with intensity %f"), 
               FearIntensity);
    }
}

void UAnim_TribalCharacterAnimInstance::UpdateEnergyLevel(float NewEnergyLevel)
{
    AnimData.EnergyLevel = FMath::Clamp(NewEnergyLevel, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("TribalCharacterAnimInstance: Energy level updated to %f"), 
           NewEnergyLevel);
}

void UAnim_TribalCharacterAnimInstance::SetHasSpear(bool bNewHasSpear)
{
    if (AnimData.bHasSpear != bNewHasSpear)
    {
        AnimData.bHasSpear = bNewHasSpear;
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Has spear set to %s"), 
               bNewHasSpear ? TEXT("true") : TEXT("false"));
    }
}

void UAnim_TribalCharacterAnimInstance::SetHasTool(bool bNewHasTool)
{
    if (AnimData.bHasTool != bNewHasTool)
    {
        AnimData.bHasTool = bNewHasTool;
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Has tool set to %s"), 
               bNewHasTool ? TEXT("true") : TEXT("false"));
    }
}

void UAnim_TribalCharacterAnimInstance::PlaySpearThrowAnimation()
{
    if (SpearThrowMontage)
    {
        Montage_Play(SpearThrowMontage);
        SetActionState(EAnim_TribalActionState::Hunting);
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Playing spear throw animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterAnimInstance: SpearThrowMontage not set"));
    }
}

void UAnim_TribalCharacterAnimInstance::PlayCraftingAnimation()
{
    if (CraftingMontage)
    {
        Montage_Play(CraftingMontage);
        SetActionState(EAnim_TribalActionState::Crafting);
        
        UE_LOG(LogTemp, Log, TEXT("TribalCharacterAnimInstance: Playing crafting animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterAnimInstance: CraftingMontage not set"));
    }
}

bool UAnim_TribalCharacterAnimInstance::IsMoving() const
{
    return AnimData.Speed > MovementSpeedThreshold;
}

bool UAnim_TribalCharacterAnimInstance::IsRunning() const
{
    return AnimData.Speed > RunSpeedThreshold;
}

bool UAnim_TribalCharacterAnimInstance::IsInAir() const
{
    return AnimData.bIsInAir;
}

float UAnim_TribalCharacterAnimInstance::CalculateDirection() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    if (Velocity.SizeSquared() < MovementSpeedThreshold * MovementSpeedThreshold)
    {
        return 0.0f;
    }
    
    // Normalize velocity to get direction
    FVector VelocityDirection = Velocity.GetSafeNormal();
    
    // Calculate angle between forward vector and velocity direction
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    // Convert to angle in degrees
    float Angle = FMath::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
    
    return Angle;
}