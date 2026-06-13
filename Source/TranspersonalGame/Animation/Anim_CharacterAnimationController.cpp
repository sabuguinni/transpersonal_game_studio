#include "Anim_CharacterAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize state
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    PreviousMovementState = EAnim_MovementState::Idle;
    
    // Initialize animation parameters
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsMoving = false;
    
    // Initialize settings
    MovementSpeedThreshold = 10.0f;
    RunSpeedThreshold = 300.0f;
    AnimationBlendTime = 0.25f;
    
    // Initialize component references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Controller initialized for character: %s"), *OwnerCharacter->GetName());
    }
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateAnimationParameters();
        UpdateMovementAnimation();
    }
}

void UAnim_CharacterAnimationController::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }
}

void UAnim_CharacterAnimationController::UpdateAnimationParameters()
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Update movement speed
    FVector Velocity = MovementComponent->Velocity;
    MovementSpeed = Velocity.Size();
    bIsMoving = MovementSpeed > MovementSpeedThreshold;
    
    // Update movement direction
    if (bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        MovementDirection = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            MovementDirection = -MovementDirection;
        }
    }
    else
    {
        MovementDirection = 0.0f;
    }
    
    // Update air state
    bIsInAir = MovementComponent->IsFalling();
    
    // Update crouch state
    bIsCrouching = MovementComponent->IsCrouching();
}

EAnim_MovementState UAnim_CharacterAnimationController::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if in air
    if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check movement speed
    if (bIsMoving)
    {
        if (MovementSpeed >= RunSpeedThreshold)
        {
            return EAnim_MovementState::Running;
        }
        else
        {
            return EAnim_MovementState::Walking;
        }
    }
    
    return EAnim_MovementState::Idle;
}

void UAnim_CharacterAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        HandleStateTransition(NewState);
    }
}

void UAnim_CharacterAnimationController::SetActionState(EAnim_ActionState NewState)
{
    if (CurrentActionState != NewState)
    {
        CurrentActionState = NewState;
        
        // Handle action state changes
        switch (NewState)
        {
            case EAnim_ActionState::Gathering:
                PlayGatherAnimation();
                break;
            case EAnim_ActionState::Crafting:
                PlayCraftAnimation();
                break;
            case EAnim_ActionState::Attacking:
                PlayAttackAnimation();
                break;
            case EAnim_ActionState::Defending:
                PlayDefendAnimation();
                break;
            default:
                break;
        }
    }
}

void UAnim_CharacterAnimationController::UpdateMovementAnimation()
{
    EAnim_MovementState NewState = CalculateMovementState();
    SetMovementState(NewState);
}

void UAnim_CharacterAnimationController::HandleStateTransition(EAnim_MovementState NewState)
{
    PlayStateAnimation(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Animation state changed from %d to %d"), 
           static_cast<int32>(PreviousMovementState), 
           static_cast<int32>(NewState));
}

void UAnim_CharacterAnimationController::PlayStateAnimation(EAnim_MovementState State)
{
    if (!AnimInstance)
    {
        return;
    }
    
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (State)
    {
        case EAnim_MovementState::Idle:
            MontageToPlay = AnimationSet.IdleMontage;
            break;
        case EAnim_MovementState::Walking:
            MontageToPlay = AnimationSet.WalkMontage;
            break;
        case EAnim_MovementState::Running:
            MontageToPlay = AnimationSet.RunMontage;
            break;
        case EAnim_MovementState::Jumping:
            MontageToPlay = AnimationSet.JumpMontage;
            break;
        case EAnim_MovementState::Falling:
            MontageToPlay = AnimationSet.FallMontage;
            break;
        case EAnim_MovementState::Crouching:
            MontageToPlay = AnimationSet.CrouchMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay)
    {
        PlayAnimationMontage(MontageToPlay);
    }
}

void UAnim_CharacterAnimationController::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }
    
    AnimInstance->Montage_Play(Montage, PlayRate);
    UE_LOG(LogTemp, Log, TEXT("Playing animation montage: %s"), *Montage->GetName());
}

void UAnim_CharacterAnimationController::StopAnimationMontage(UAnimMontage* Montage)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }
    
    AnimInstance->Montage_Stop(AnimationBlendTime, Montage);
}

void UAnim_CharacterAnimationController::StopAllMontages()
{
    if (!AnimInstance)
    {
        return;
    }
    
    AnimInstance->StopAllMontages(AnimationBlendTime);
}

void UAnim_CharacterAnimationController::PlayJumpAnimation()
{
    if (AnimationSet.JumpMontage)
    {
        PlayAnimationMontage(AnimationSet.JumpMontage);
    }
}

void UAnim_CharacterAnimationController::PlayLandAnimation()
{
    // Transition back to appropriate ground state
    UpdateMovementAnimation();
}

void UAnim_CharacterAnimationController::PlayGatherAnimation()
{
    // Play gathering animation - could be a specific montage for resource collection
    UE_LOG(LogTemp, Log, TEXT("Playing gather animation"));
}

void UAnim_CharacterAnimationController::PlayCraftAnimation()
{
    // Play crafting animation - could be a specific montage for item creation
    UE_LOG(LogTemp, Log, TEXT("Playing craft animation"));
}

void UAnim_CharacterAnimationController::PlayAttackAnimation()
{
    // Play attack animation - could be weapon-specific montages
    UE_LOG(LogTemp, Log, TEXT("Playing attack animation"));
}

void UAnim_CharacterAnimationController::PlayDefendAnimation()
{
    // Play defend animation - could be blocking or dodging montages
    UE_LOG(LogTemp, Log, TEXT("Playing defend animation"));
}

float UAnim_CharacterAnimationController::GetMovementSpeed() const
{
    return MovementSpeed;
}

float UAnim_CharacterAnimationController::GetMovementDirection() const
{
    return MovementDirection;
}

bool UAnim_CharacterAnimationController::IsInAir() const
{
    return bIsInAir;
}

bool UAnim_CharacterAnimationController::IsCrouching() const
{
    return bIsCrouching;
}