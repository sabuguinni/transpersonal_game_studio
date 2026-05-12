#include "Anim_EnhancedCharacterController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_EnhancedCharacterController::UAnim_EnhancedCharacterController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    
    // Initialize enhanced features
    bFootIKEnabled = true;
    bLookAtEnabled = false;
    LookAtTarget = FVector::ZeroVector;
    LookAtAlpha = 0.0f;
    
    // Initialize emotional state
    EmotionalHappiness = 0.5f;
    EmotionalFear = 0.0f;
    EmotionalAnger = 0.0f;
    EmotionalCalm = 0.8f;
    
    // Performance settings
    UpdateFrequency = 30.0f; // 30 FPS for animation updates
    LastUpdateTime = 0.0f;
}

void UAnim_EnhancedCharacterController::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_EnhancedCharacterController::InitializeComponent()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp)
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Enhanced Character Animation Controller initialized for: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Enhanced Character Animation Controller: Owner is not a Character!"));
    }
}

void UAnim_EnhancedCharacterController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Performance optimization - limit update frequency
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < (1.0f / UpdateFrequency))
    {
        return;
    }
    LastUpdateTime = CurrentTime;
    
    // Update core animation data
    UpdateMovementData();
    UpdateMovementState();
    
    // Update enhanced features
    if (bLookAtEnabled)
    {
        UpdateLookAt(DeltaTime);
    }
}

void UAnim_EnhancedCharacterController::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    MovementData.bIsMoving = MovementData.Speed > 10.0f;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Calculate direction relative to character forward
    if (MovementData.bIsMoving && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Calculate aim direction (for future use with weapon systems)
    if (OwnerCharacter)
    {
        FRotator ControlRotation = OwnerCharacter->GetControlRotation();
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        
        FRotator DeltaRotation = ControlRotation - ActorRotation;
        DeltaRotation.Normalize();
        
        MovementData.AimYaw = DeltaRotation.Yaw;
        MovementData.AimPitch = DeltaRotation.Pitch;
    }
}

void UAnim_EnhancedCharacterController::UpdateMovementState()
{
    EAnim_MovementState NewState = CalculateMovementState();
    
    if (NewState != CurrentMovementState)
    {
        CurrentMovementState = NewState;
        
        // Log state changes for debugging
        UE_LOG(LogTemp, Log, TEXT("Movement State Changed to: %d"), (int32)CurrentMovementState);
    }
}

EAnim_MovementState UAnim_EnhancedCharacterController::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if in air
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (MovementComponent->IsCrouching())
    {
        if (MovementData.bIsMoving)
        {
            return EAnim_MovementState::Crawling;
        }
        else
        {
            return EAnim_MovementState::Crouching;
        }
    }
    
    // Check movement speed for ground states
    if (MovementData.bIsMoving)
    {
        float MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
        float SpeedRatio = MovementData.Speed / MaxWalkSpeed;
        
        if (SpeedRatio > 1.5f) // Sprinting
        {
            return EAnim_MovementState::Sprinting;
        }
        else if (SpeedRatio > 0.8f) // Running
        {
            return EAnim_MovementState::Running;
        }
        else // Walking
        {
            return EAnim_MovementState::Walking;
        }
    }
    
    return EAnim_MovementState::Idle;
}

void UAnim_EnhancedCharacterController::UpdateActionState(EAnim_ActionState NewState)
{
    if (NewState != CurrentActionState)
    {
        CurrentActionState = NewState;
        
        // Play appropriate montage if available
        if (ActionMontages.Contains(NewState))
        {
            UAnimMontage* Montage = ActionMontages[NewState];
            if (Montage)
            {
                PlayActionMontage(Montage);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Action State Changed to: %d"), (int32)CurrentActionState);
    }
}

void UAnim_EnhancedCharacterController::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }
    
    AnimInstance->Montage_Play(Montage, PlayRate);
    UE_LOG(LogTemp, Log, TEXT("Playing Action Montage: %s"), *Montage->GetName());
}

void UAnim_EnhancedCharacterController::StopActionMontage(float BlendOutTime)
{
    if (!AnimInstance)
    {
        return;
    }
    
    AnimInstance->Montage_Stop(BlendOutTime);
    UE_LOG(LogTemp, Log, TEXT("Stopped Action Montage"));
}

void UAnim_EnhancedCharacterController::SetMovementBlendSpace(EAnim_MovementState State, UBlendSpace* BlendSpace)
{
    switch (State)
    {
        case EAnim_MovementState::Idle:
            BlendSpaces.IdleBlendSpace = BlendSpace;
            break;
        case EAnim_MovementState::Walking:
            BlendSpaces.WalkBlendSpace = BlendSpace;
            break;
        case EAnim_MovementState::Running:
            BlendSpaces.RunBlendSpace = BlendSpace;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set BlendSpace for Movement State: %d"), (int32)State);
}

void UAnim_EnhancedCharacterController::EnableFootIK(bool bEnable)
{
    bFootIKEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Foot IK %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAnim_EnhancedCharacterController::SetLookAtTarget(FVector TargetLocation)
{
    LookAtTarget = TargetLocation;
    bLookAtEnabled = true;
    LookAtAlpha = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Look At Target Set: %s"), *TargetLocation.ToString());
}

void UAnim_EnhancedCharacterController::ClearLookAtTarget()
{
    bLookAtEnabled = false;
    LookAtAlpha = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Look At Target Cleared"));
}

void UAnim_EnhancedCharacterController::UpdateLookAt(float DeltaTime)
{
    if (!bLookAtEnabled || !OwnerCharacter)
    {
        return;
    }
    
    // Smoothly interpolate look at alpha
    float TargetAlpha = 1.0f;
    LookAtAlpha = FMath::FInterpTo(LookAtAlpha, TargetAlpha, DeltaTime, 2.0f);
    
    // Calculate look at rotation
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    FVector LookDirection = (LookAtTarget - CharacterLocation).GetSafeNormal();
    
    if (!LookDirection.IsZero())
    {
        FRotator LookAtRotation = LookDirection.Rotation();
        FRotator CurrentRotation = OwnerCharacter->GetControlRotation();
        
        // Blend the rotations based on alpha
        FRotator BlendedRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, 3.0f * LookAtAlpha);
        
        // Apply rotation limits to prevent unnatural head movement
        float MaxYawOffset = 90.0f;
        float MaxPitchOffset = 45.0f;
        
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FRotator DeltaRotation = BlendedRotation - ActorRotation;
        DeltaRotation.Normalize();
        
        DeltaRotation.Yaw = FMath::Clamp(DeltaRotation.Yaw, -MaxYawOffset, MaxYawOffset);
        DeltaRotation.Pitch = FMath::Clamp(DeltaRotation.Pitch, -MaxPitchOffset, MaxPitchOffset);
        
        FRotator FinalRotation = ActorRotation + DeltaRotation;
        
        // This would be applied to head bone in animation blueprint
        // For now, just store the data for the animation system to use
        MovementData.AimYaw = DeltaRotation.Yaw;
        MovementData.AimPitch = DeltaRotation.Pitch;
    }
}

void UAnim_EnhancedCharacterController::SetEmotionalState(float Happiness, float Fear, float Anger, float Calm)
{
    EmotionalHappiness = FMath::Clamp(Happiness, 0.0f, 1.0f);
    EmotionalFear = FMath::Clamp(Fear, 0.0f, 1.0f);
    EmotionalAnger = FMath::Clamp(Anger, 0.0f, 1.0f);
    EmotionalCalm = FMath::Clamp(Calm, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Emotional State Updated - H:%.2f F:%.2f A:%.2f C:%.2f"), 
           EmotionalHappiness, EmotionalFear, EmotionalAnger, EmotionalCalm);
}