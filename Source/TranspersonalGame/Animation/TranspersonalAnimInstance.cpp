#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Initialize default values
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    MovementState = EAnim_MovementState::Idle;
    
    MotionMatchingVelocity = FVector::ZeroVector;
    DesiredFacingDirection = FVector::ForwardVector;
    PoseCost = 0.0f;
    SearchRadius = 100.0f;
    
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    PelvisIKOffset = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    
    ActionState = EAnim_ActionState::None;
    WeaponType = EAnim_WeaponType::Unarmed;
    bIsCrouching = false;
    StaminaPercentage = 1.0f;
    
    Character = nullptr;
    CharacterMovement = nullptr;
    
    // IK Settings
    FootIKTraceDistance = 50.0f;
    IKInterpSpeed = 10.0f;
    MinPelvisOffset = -20.0f;
    MaxPelvisOffset = 20.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache character references
    Character = Cast<ACharacter>(GetOwningActor());
    if (Character)
    {
        CharacterMovement = Character->GetCharacterMovement();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!Character || !CharacterMovement)
    {
        return;
    }
    
    // Update all animation data
    UpdateMovementVariables();
    UpdateMotionMatchingData();
    UpdateFootIK();
    UpdateCharacterState();
}

void UTranspersonalAnimInstance::UpdateMovementVariables()
{
    if (!Character || !CharacterMovement)
    {
        return;
    }
    
    // Calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to character facing
    if (Speed > 0.1f)
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector RightVector = Character->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Check if in air
    bIsInAir = CharacterMovement->IsFalling();
    
    // Check if accelerating
    FVector Acceleration = CharacterMovement->GetCurrentAcceleration();
    bIsAccelerating = Acceleration.SizeSquared() > 0.1f;
    
    // Determine movement state
    if (bIsInAir)
    {
        MovementState = EAnim_MovementState::InAir;
    }
    else if (Speed < 1.0f)
    {
        MovementState = EAnim_MovementState::Idle;
    }
    else if (Speed < 150.0f)
    {
        MovementState = EAnim_MovementState::Walking;
    }
    else if (Speed < 400.0f)
    {
        MovementState = EAnim_MovementState::Running;
    }
    else
    {
        MovementState = EAnim_MovementState::Sprinting;
    }
}

void UTranspersonalAnimInstance::UpdateMotionMatchingData()
{
    if (!Character || !CharacterMovement)
    {
        return;
    }
    
    // Update motion matching velocity (smoothed for better matching)
    FVector CurrentVelocity = CharacterMovement->Velocity;
    MotionMatchingVelocity = FMath::VInterpTo(MotionMatchingVelocity, CurrentVelocity, GetWorld()->GetDeltaSeconds(), 5.0f);
    
    // Update desired facing direction based on input
    FVector InputVector = CharacterMovement->GetLastInputVector();
    if (InputVector.SizeSquared() > 0.1f)
    {
        DesiredFacingDirection = InputVector.GetSafeNormal();
    }
    else
    {
        DesiredFacingDirection = Character->GetActorForwardVector();
    }
    
    // Calculate pose cost (simplified - in real implementation this would query the motion matching database)
    float VelocityDifference = (CurrentVelocity - MotionMatchingVelocity).Size();
    float DirectionDifference = FVector::Dist(DesiredFacingDirection, Character->GetActorForwardVector());
    PoseCost = VelocityDifference + DirectionDifference * 10.0f;
}

void UTranspersonalAnimInstance::UpdateFootIK()
{
    if (!Character)
    {
        return;
    }
    
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    
    // Calculate foot IK offsets
    FVector LeftFootTarget = CalculateFootIKOffset(TEXT("foot_l"), FootIKTraceDistance);
    FVector RightFootTarget = CalculateFootIKOffset(TEXT("foot_r"), FootIKTraceDistance);
    
    // Smooth interpolation
    LeftFootIKOffset = SmoothIKOffset(LeftFootIKOffset, LeftFootTarget, DeltaTime, IKInterpSpeed);
    RightFootIKOffset = SmoothIKOffset(RightFootIKOffset, RightFootTarget, DeltaTime, IKInterpSpeed);
    
    // Calculate foot rotations
    LeftFootIKRotation = CalculateFootIKRotation(TEXT("foot_l"));
    RightFootIKRotation = CalculateFootIKRotation(TEXT("foot_r"));
    
    // Calculate pelvis offset (average of both feet, but limited)
    float LeftOffset = LeftFootIKOffset.Z;
    float RightOffset = RightFootIKOffset.Z;
    float TargetPelvisOffset = (LeftOffset + RightOffset) * 0.5f;
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, MinPelvisOffset, MaxPelvisOffset);
    
    PelvisIKOffset.Z = FMath::FInterpTo(PelvisIKOffset.Z, TargetPelvisOffset, DeltaTime, IKInterpSpeed * 0.5f);
}

FVector UTranspersonalAnimInstance::CalculateFootIKOffset(const FName& SocketName, float TraceDistance)
{
    if (!Character)
    {
        return FVector::ZeroVector;
    }
    
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (!Mesh)
    {
        return FVector::ZeroVector;
    }
    
    // Get foot socket location
    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector StartLocation = SocketLocation;
    FVector EndLocation = StartLocation - FVector(0, 0, TraceDistance);
    
    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate offset needed to place foot on ground
        float DistanceToGround = (StartLocation - HitResult.Location).Z;
        float OffsetZ = TraceDistance - DistanceToGround;
        return FVector(0, 0, -OffsetZ);
    }
    
    return FVector::ZeroVector;
}

FRotator UTranspersonalAnimInstance::CalculateFootIKRotation(const FName& SocketName)
{
    if (!Character)
    {
        return FRotator::ZeroRotator;
    }
    
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (!Mesh)
    {
        return FRotator::ZeroRotator;
    }
    
    // Get foot socket location
    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector StartLocation = SocketLocation + FVector(0, 0, 20);
    FVector EndLocation = SocketLocation - FVector(0, 0, FootIKTraceDistance);
    
    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit && HitResult.Normal.Z > 0.1f)
    {
        // Calculate rotation to align foot with surface normal
        FVector UpVector = FVector::UpVector;
        FVector RightVector = FVector::CrossProduct(UpVector, HitResult.Normal).GetSafeNormal();
        FVector ForwardVector = FVector::CrossProduct(HitResult.Normal, RightVector).GetSafeNormal();
        
        FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitResult.Normal);
        FRotator CurrentRotation = Character->GetActorRotation();
        
        // Return relative rotation
        return (TargetRotation - CurrentRotation).GetNormalized();
    }
    
    return FRotator::ZeroRotator;
}

void UTranspersonalAnimInstance::UpdateCharacterState()
{
    if (!Character)
    {
        return;
    }
    
    // Update crouching state
    bIsCrouching = CharacterMovement && CharacterMovement->IsCrouching();
    
    // Update stamina (placeholder - would come from character stats component)
    StaminaPercentage = 1.0f; // TODO: Get from actual stamina system
    
    // Update action state (placeholder - would come from character action component)
    ActionState = EAnim_ActionState::None; // TODO: Get from actual action system
    
    // Update weapon type (placeholder - would come from inventory/equipment system)
    WeaponType = EAnim_WeaponType::Unarmed; // TODO: Get from actual weapon system
}

FVector UTranspersonalAnimInstance::SmoothIKOffset(const FVector& CurrentOffset, const FVector& TargetOffset, float DeltaTime, float InterpSpeed)
{
    return FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, InterpSpeed);
}