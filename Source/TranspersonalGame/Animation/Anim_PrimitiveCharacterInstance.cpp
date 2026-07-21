#include "Anim_PrimitiveCharacterInstance.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_PrimitiveCharacterInstance::UAnim_PrimitiveCharacterInstance()
{
    // Initialize animation variables
    Speed = 0.0f;
    bIsInAir = false;
    bIsMoving = false;
    Direction = 0.0f;
    LeanAngle = 0.0f;
    
    // Initialize survival states
    bIsCrouching = false;
    bIsExhausted = false;
    bIsInjured = false;
    FearLevel = 0.0f;
    
    // Initialize IK settings
    bEnableFootIK = true;
    FootTraceDistance = 50.0f;
    FootOffsetInterpSpeed = 15.0f;
    PelvisOffsetInterpSpeed = 10.0f;
    
    // Initialize IK offsets
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    PelvisOffset = FVector::ZeroVector;
    LeftFootRotation = FRotator::ZeroRotator;
    RightFootRotation = FRotator::ZeroRotator;
    
    // Initialize targets
    TargetLeftFootOffset = FVector::ZeroVector;
    TargetRightFootOffset = FVector::ZeroVector;
    TargetPelvisOffset = FVector::ZeroVector;
    TargetLeftFootRotation = FRotator::ZeroRotator;
    TargetRightFootRotation = FRotator::ZeroRotator;
    
    // Initialize cached references
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Initialize montages and blend spaces
    IdleMontage = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    CrouchMontage = nullptr;
    LocomotionBlendSpace = nullptr;
}

void UAnim_PrimitiveCharacterInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache references
    OwningCharacter = Cast<ATranspersonalCharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Instance could not find TranspersonalCharacter owner"));
    }
}

void UAnim_PrimitiveCharacterInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation variables
    UpdateLocomotionVariables();
    UpdateSurvivalStates();
    
    // Update IK if enabled
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_PrimitiveCharacterInstance::UpdateLocomotionVariables()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector Velocity = MovementComponent->Velocity;
    
    // Calculate speed
    Speed = Velocity.Size2D();
    
    // Check if moving
    bIsMoving = Speed > 3.0f;
    
    // Check if in air
    bIsInAir = MovementComponent->IsFalling();
    
    // Calculate direction relative to character rotation
    if (bIsMoving)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Calculate lean angle for turns
    if (bIsMoving && !PreviousVelocity.IsZero())
    {
        FVector VelocityDelta = Velocity - PreviousVelocity;
        float TurnRate = VelocityDelta.Size2D();
        LeanAngle = FMath::Clamp(TurnRate * 0.1f, -45.0f, 45.0f);
    }
    else
    {
        LeanAngle = FMath::FInterpTo(LeanAngle, 0.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
    }
    
    // Store velocity for next frame
    PreviousVelocity = Velocity;
}

void UAnim_PrimitiveCharacterInstance::UpdateSurvivalStates()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Update crouching state
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Get survival stats from character
    // Note: These would come from the character's survival component
    // For now, we'll use placeholder logic
    
    // Check exhaustion based on stamina (placeholder)
    // bIsExhausted = (character stamina < 20%)
    bIsExhausted = false; // Placeholder
    
    // Check injury state (placeholder)
    // bIsInjured = (character health < 50%)
    bIsInjured = false; // Placeholder
    
    // Calculate fear level based on nearby threats (placeholder)
    // FearLevel would be calculated based on nearby dinosaurs, darkness, etc.
    FearLevel = 0.0f; // Placeholder
}

void UAnim_PrimitiveCharacterInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwningCharacter || bIsInAir)
    {
        // Reset IK when in air
        LeftFootOffset = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        PelvisOffset = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        return;
    }
    
    // Perform foot traces
    FVector LeftFootHit = PerformFootTrace(TEXT("foot_l"), FootTraceDistance);
    FVector RightFootHit = PerformFootTrace(TEXT("foot_r"), FootTraceDistance);
    
    // Calculate target offsets
    float LeftOffset = LeftFootHit.Z;
    float RightOffset = RightFootHit.Z;
    
    // Calculate pelvis offset (average of both feet, but keep character level)
    float PelvisTargetZ = FMath::Min(LeftOffset, RightOffset);
    
    // Set target offsets
    TargetLeftFootOffset = FVector(0, 0, LeftOffset - PelvisTargetZ);
    TargetRightFootOffset = FVector(0, 0, RightOffset - PelvisTargetZ);
    TargetPelvisOffset = FVector(0, 0, PelvisTargetZ);
    
    // Calculate foot rotations based on surface normals
    // This would use the hit normal from the trace
    TargetLeftFootRotation = FRotator::ZeroRotator; // Placeholder
    TargetRightFootRotation = FRotator::ZeroRotator; // Placeholder
    
    // Interpolate to targets
    LeftFootOffset = FMath::VInterpTo(LeftFootOffset, TargetLeftFootOffset, DeltaTime, FootOffsetInterpSpeed);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, TargetRightFootOffset, DeltaTime, FootOffsetInterpSpeed);
    PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, PelvisOffsetInterpSpeed);
    
    LeftFootRotation = FMath::RInterpTo(LeftFootRotation, TargetLeftFootRotation, DeltaTime, FootOffsetInterpSpeed);
    RightFootRotation = FMath::RInterpTo(RightFootRotation, TargetRightFootRotation, DeltaTime, FootOffsetInterpSpeed);
}

FVector UAnim_PrimitiveCharacterInstance::PerformFootTrace(FName SocketName, float TraceDistance)
{
    if (!OwningCharacter)
    {
        return FVector::ZeroVector;
    }
    
    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }
    
    // Get socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    
    // Perform line trace downward
    FVector TraceStart = SocketLocation;
    FVector TraceEnd = TraceStart - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Return the offset needed to place foot on ground
        float GroundDistance = (TraceStart - HitResult.Location).Z;
        return FVector(0, 0, TraceDistance - GroundDistance);
    }
    
    // No ground found, return zero offset
    return FVector::ZeroVector;
}

FRotator UAnim_PrimitiveCharacterInstance::CalculateFootRotation(FVector SurfaceNormal)
{
    // Calculate foot rotation to match surface normal
    FVector UpVector = FVector::UpVector;
    FVector ForwardVector = FVector::CrossProduct(UpVector, SurfaceNormal).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector);
    
    return FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
}