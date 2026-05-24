#include "Anim_CharacterAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UAnim_CharacterAnimController::UAnim_CharacterAnimController()
{
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsAccelerating = false;
    
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    bEnableFootIK = true;
    
    SpeedSmoothingRate = 10.0f;
    DirectionSmoothingRate = 15.0f;
    
    LastSpeed = 0.0f;
    LastDirection = 0.0f;
    StateTransitionTimer = 0.0f;
    bWasInAir = false;
}

void UAnim_CharacterAnimController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
    
    // Initialize action montages map
    ActionMontages.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Animation Controller initialized for character: %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("None"));
}

void UAnim_CharacterAnimController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update basic movement variables
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    
    // Smooth speed changes
    Speed = FMath::FInterpTo(LastSpeed, CurrentSpeed, DeltaTimeX, SpeedSmoothingRate);
    LastSpeed = Speed;
    
    // Calculate direction relative to character forward
    if (Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        float TargetDirection = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
        Direction = FMath::FInterpTo(LastDirection, TargetDirection, DeltaTimeX, DirectionSmoothingRate);
        LastDirection = Direction;
    }
    
    // Update state flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Handle landing detection
    if (bWasInAir && !bIsInAir && LandMontage)
    {
        PlayActionMontage(LandMontage, 1.0f);
    }
    bWasInAir = bIsInAir;
    
    // Update movement state
    UpdateMovementState();
    
    // Update foot IK if enabled
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTimeX);
    }
    
    // Update state transition timer
    StateTransitionTimer += DeltaTimeX;
}

void UAnim_CharacterAnimController::UpdateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewState = CurrentMovementState;
    
    if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (Speed < 1.0f)
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (Speed < 150.0f)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else if (Speed < 400.0f)
    {
        NewState = EAnim_MovementState::Running;
    }
    else
    {
        NewState = EAnim_MovementState::Sprinting;
    }
    
    if (NewState != CurrentMovementState)
    {
        CurrentMovementState = NewState;
        StateTransitionTimer = 0.0f;
        
        // Handle special state transitions
        if (CurrentMovementState == EAnim_MovementState::Jumping && JumpMontage)
        {
            PlayActionMontage(JumpMontage, 1.0f);
        }
    }
}

void UAnim_CharacterAnimController::SetActionState(EAnim_ActionState NewState)
{
    if (CurrentActionState != NewState)
    {
        CurrentActionState = NewState;
        
        // Play corresponding montage if available
        if (ActionMontages.Contains(NewState))
        {
            UAnimMontage* Montage = ActionMontages[NewState];
            if (Montage)
            {
                PlayActionMontage(Montage, 1.0f);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Action state changed to: %d"), (int32)NewState);
    }
}

void UAnim_CharacterAnimController::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (Montage && GetOwningComponent())
    {
        GetOwningComponent()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s"), *Montage->GetName());
    }
}

void UAnim_CharacterAnimController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Reset IK when in air
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaTime, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaTime, FootIKInterpSpeed);
        FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, 0.0f, DeltaTime, FootIKInterpSpeed);
        return;
    }
    
    // Perform foot traces
    FVector LeftFootHit = PerformFootTrace(TEXT("foot_l"), FootIKTraceDistance);
    FVector RightFootHit = PerformFootTrace(TEXT("foot_r"), FootIKTraceDistance);
    
    // Calculate foot positions and rotations
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    
    // Left foot IK
    if (LeftFootHit != FVector::ZeroVector)
    {
        FootIKData.LeftFootLocation = LeftFootHit;
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 1.0f, DeltaTime, FootIKInterpSpeed);
        
        // Calculate foot rotation based on ground normal
        FVector GroundNormal = FVector::UpVector; // Simplified - would need actual surface normal
        FootIKData.LeftFootRotation = UKismetMathLibrary::MakeRotFromZX(GroundNormal, OwnerCharacter->GetActorForwardVector());
    }
    else
    {
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaTime, FootIKInterpSpeed);
    }
    
    // Right foot IK
    if (RightFootHit != FVector::ZeroVector)
    {
        FootIKData.RightFootLocation = RightFootHit;
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 1.0f, DeltaTime, FootIKInterpSpeed);
        
        FVector GroundNormal = FVector::UpVector;
        FootIKData.RightFootRotation = UKismetMathLibrary::MakeRotFromZX(GroundNormal, OwnerCharacter->GetActorForwardVector());
    }
    else
    {
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaTime, FootIKInterpSpeed);
    }
    
    // Calculate hip offset to keep character grounded
    float LeftOffset = (LeftFootHit != FVector::ZeroVector) ? (LeftFootHit.Z - CharacterLocation.Z) : 0.0f;
    float RightOffset = (RightFootHit != FVector::ZeroVector) ? (RightFootHit.Z - CharacterLocation.Z) : 0.0f;
    float TargetHipOffset = FMath::Min(LeftOffset, RightOffset);
    
    FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, TargetHipOffset, DeltaTime, FootIKInterpSpeed);
}

FVector UAnim_CharacterAnimController::PerformFootTrace(FName SocketName, float TraceDistance)
{
    if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    if (SocketLocation == FVector::ZeroVector)
    {
        // Fallback to approximate foot positions if sockets don't exist
        FVector CharacterLocation = OwnerCharacter->GetActorLocation();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        if (SocketName == TEXT("foot_l"))
        {
            SocketLocation = CharacterLocation - (RightVector * 20.0f);
        }
        else
        {
            SocketLocation = CharacterLocation + (RightVector * 20.0f);
        }
    }
    
    FVector TraceStart = SocketLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = SocketLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    return FVector::ZeroVector;
}