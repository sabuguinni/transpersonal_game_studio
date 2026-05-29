#include "Anim_CharacterMovementStates.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_CharacterMovementStates::UAnim_CharacterMovementStates()
{
    // Initialize default values
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    IdleMontage = nullptr;
    WalkMontage = nullptr;
    RunMontage = nullptr;
    JumpMontage = nullptr;
    LocomotionBlendSpace = nullptr;
    
    // State transition thresholds
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    IdleSpeedThreshold = 10.0f;
    JumpVelocityThreshold = 100.0f;
    
    // Ground detection settings
    GroundTraceDistance = 150.0f;
    bEnableGroundAdaptation = true;
    
    // Internal state
    PreviousState = EAnim_MovementState::Idle;
    StateChangeTime = 0.0f;
    bIsTransitioning = false;
}

void UAnim_CharacterMovementStates::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get character reference in animation instance"));
    }
    
    // Initialize movement data
    MovementData.CurrentState = EAnim_MovementState::Idle;
}

void UAnim_CharacterMovementStates::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data
    UpdateMovementData();
    
    // Update movement state
    UpdateMovementState();
    
    // Update blend spaces
    UpdateLocomotionBlendSpace();
    
    // Perform ground trace if enabled
    if (bEnableGroundAdaptation)
    {
        PerformGroundTrace();
    }
}

void UAnim_CharacterMovementStates::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and speed
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    
    // Calculate movement direction relative to character rotation
    if (MovementData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Check if moving backwards
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    MovementData.bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
}

void UAnim_CharacterMovementStates::UpdateMovementState()
{
    EAnim_MovementState NewState = MovementData.CurrentState;
    
    // Determine new state based on movement data
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > JumpVelocityThreshold)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.Speed <= IdleSpeedThreshold)
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed <= WalkSpeedThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else if (MovementData.Speed > RunSpeedThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else
    {
        NewState = EAnim_MovementState::Walking;
    }
    
    // Check if state should change
    if (ShouldTransitionState(NewState))
    {
        SetMovementState(NewState);
    }
}

EAnim_MovementState UAnim_CharacterMovementStates::GetCurrentMovementState() const
{
    return MovementData.CurrentState;
}

void UAnim_CharacterMovementStates::SetMovementState(EAnim_MovementState NewState)
{
    if (MovementData.CurrentState != NewState)
    {
        EAnim_MovementState OldState = MovementData.CurrentState;
        PreviousState = OldState;
        MovementData.CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        bIsTransitioning = true;
        
        OnStateChanged(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), (int32)OldState, (int32)NewState);
    }
}

void UAnim_CharacterMovementStates::PlayIdleMontage()
{
    if (IdleMontage && GetOwningComponent())
    {
        Montage_Play(IdleMontage);
    }
}

void UAnim_CharacterMovementStates::PlayWalkMontage()
{
    if (WalkMontage && GetOwningComponent())
    {
        Montage_Play(WalkMontage);
    }
}

void UAnim_CharacterMovementStates::PlayRunMontage()
{
    if (RunMontage && GetOwningComponent())
    {
        Montage_Play(RunMontage);
    }
}

void UAnim_CharacterMovementStates::PlayJumpMontage()
{
    if (JumpMontage && GetOwningComponent())
    {
        Montage_Play(JumpMontage);
    }
}

void UAnim_CharacterMovementStates::StopAllMontages()
{
    if (GetOwningComponent())
    {
        Montage_StopGroupByName(0.2f, FName("DefaultGroup"));
    }
}

void UAnim_CharacterMovementStates::UpdateLocomotionBlendSpace()
{
    // This would typically update blend space parameters
    // Implementation depends on specific blend space setup
    if (LocomotionBlendSpace)
    {
        // Example: Update blend space with speed and direction
        // This would be handled in the Animation Blueprint
    }
}

void UAnim_CharacterMovementStates::PerformGroundTrace()
{
    if (!OwnerCharacter || !GetWorld())
    {
        return;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, GroundTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        MovementData.GroundDistance = HitResult.Distance;
    }
    else
    {
        MovementData.GroundDistance = GroundTraceDistance;
    }
    
    // Debug visualization
    #if WITH_EDITOR
    if (GEngine && GEngine->bIsEditor)
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, 
                     bHit ? FColor::Green : FColor::Red, false, 0.1f);
    }
    #endif
}

bool UAnim_CharacterMovementStates::ShouldTransitionState(EAnim_MovementState NewState)
{
    // Add hysteresis to prevent rapid state changes
    float TimeSinceLastChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    const float MinTransitionTime = 0.1f;
    
    if (TimeSinceLastChange < MinTransitionTime && bIsTransitioning)
    {
        return false;
    }
    
    // Allow transition
    bIsTransitioning = false;
    return true;
}

void UAnim_CharacterMovementStates::OnStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState)
{
    // Handle state-specific logic when transitioning
    switch (NewState)
    {
        case EAnim_MovementState::Idle:
            PlayIdleMontage();
            break;
            
        case EAnim_MovementState::Walking:
            PlayWalkMontage();
            break;
            
        case EAnim_MovementState::Running:
            PlayRunMontage();
            break;
            
        case EAnim_MovementState::Jumping:
            PlayJumpMontage();
            break;
            
        case EAnim_MovementState::Falling:
            // Handle falling animation
            break;
            
        case EAnim_MovementState::Crouching:
            // Handle crouching animation
            break;
            
        default:
            break;
    }
}