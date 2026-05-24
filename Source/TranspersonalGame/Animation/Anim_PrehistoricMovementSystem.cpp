#include "Anim_PrehistoricMovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_PrehistoricMovementSystem::UAnim_PrehistoricMovementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Default movement settings
    LocomotionType = EAnim_LocomotionType::Bipedal;
    WalkSpeed = 150.0f;
    RunSpeed = 300.0f;
    SprintSpeed = 500.0f;
    CrouchSpeed = 75.0f;
    
    // Terrain adaptation settings
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    MaxSlopeAngle = 45.0f;
    
    // Initialize movement data
    CurrentMovementData = FAnim_MovementData();
    TerrainData = FAnim_TerrainAdaptation();
    
    // Initialize state tracking
    PreviousMovementState = EAnim_MovementState::Idle;
    StateTransitionTimer = 0.0f;
    MovementBlendAlpha = 0.0f;
    
    // Initialize foot IK
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
}

void UAnim_PrehistoricMovementSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeReferences();
}

void UAnim_PrehistoricMovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update core movement data
    UpdateMovementData(DeltaTime);
    
    // Update terrain adaptation if enabled
    if (bEnableFootIK)
    {
        UpdateTerrainAdaptation(DeltaTime);
    }
    
    // Apply prehistoric movement behaviors
    ApplyPrehistoricMovementStyle();
    
    // Update blend space values for smooth animation transitions
    UpdateBlendSpaceValues();
    
    // Handle state transitions
    if (StateTransitionTimer > 0.0f)
    {
        StateTransitionTimer -= DeltaTime;
    }
}

void UAnim_PrehistoricMovementSystem::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (OwnerCharacter->GetMesh())
        {
            AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        }
    }
}

void UAnim_PrehistoricMovementSystem::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity and speed
    CurrentMovementData.Velocity = MovementComponent->Velocity;
    CurrentMovementData.Speed = CurrentMovementData.Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (CurrentMovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentMovementData.Velocity.GetSafeNormal();
        CurrentMovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            CurrentMovementData.Direction *= -1.0f;
        }
        
        CurrentMovementData.bIsMoving = true;
    }
    else
    {
        CurrentMovementData.Direction = 0.0f;
        CurrentMovementData.bIsMoving = false;
    }
    
    // Update movement state flags
    CurrentMovementData.bIsInAir = MovementComponent->IsFalling();
    CurrentMovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine movement state based on speed and conditions
    EAnim_MovementState NewState = EAnim_MovementState::Idle;
    
    if (CurrentMovementData.bIsInAir)
    {
        NewState = MovementComponent->Velocity.Z > 0.0f ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    else if (CurrentMovementData.bIsCrouching)
    {
        NewState = CurrentMovementData.bIsMoving ? EAnim_MovementState::Crouching : EAnim_MovementState::Idle;
    }
    else if (CurrentMovementData.bIsMoving)
    {
        if (CurrentMovementData.Speed < WalkSpeed * 0.5f)
        {
            NewState = EAnim_MovementState::Walking;
        }
        else if (CurrentMovementData.Speed < RunSpeed * 0.8f)
        {
            NewState = EAnim_MovementState::Running;
        }
        else
        {
            NewState = EAnim_MovementState::Sprinting;
        }
    }
    
    // Update movement state if changed
    if (NewState != CurrentMovementData.MovementState)
    {
        SetMovementState(NewState);
    }
}

void UAnim_PrehistoricMovementSystem::UpdateTerrainAdaptation(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerCharacter->GetMesh())
    {
        return;
    }
    
    // Calculate ground slope
    TerrainData.GroundSlope = CalculateGroundSlope();
    
    // Perform foot IK traces
    PerformFootIKTrace(TerrainData.LeftFootOffset, TerrainData.LeftFootRotation, TEXT("foot_l"));
    PerformFootIKTrace(TerrainData.RightFootOffset, TerrainData.RightFootRotation, TEXT("foot_r"));
    
    // Smooth foot IK transitions
    SmoothFootIKTransitions(DeltaTime);
    
    // Update foot IK alpha based on movement state
    float TargetIKAlpha = 1.0f;
    if (CurrentMovementData.bIsInAir || CurrentMovementData.MovementState == EAnim_MovementState::Sprinting)
    {
        TargetIKAlpha = 0.0f;
    }
    else if (CurrentMovementData.Speed > RunSpeed * 0.5f)
    {
        TargetIKAlpha = 0.5f;
    }
    
    TerrainData.FootIKAlpha = FMath::FInterpTo(TerrainData.FootIKAlpha, TargetIKAlpha, DeltaTime, FootIKInterpSpeed);
}

void UAnim_PrehistoricMovementSystem::PerformFootIKTrace(FVector& FootOffset, FRotator& FootRotation, FName SocketName)
{
    if (!OwnerCharacter || !OwnerCharacter->GetMesh())
    {
        return;
    }
    
    // Get foot socket location
    FVector SocketLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);
    
    // Perform line trace
    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerCharacter);
    
    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResult,
        true
    );
    
    if (bHit)
    {
        // Calculate foot offset
        float DistanceFromGround = FVector::Dist(SocketLocation, HitResult.Location);
        FootOffset.Z = -(FootIKTraceDistance - DistanceFromGround);
        
        // Calculate foot rotation to match ground normal
        FVector GroundNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(GroundNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, GroundNormal).GetSafeNormal();
        
        FootRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, GroundNormal);
        FootRotation = UKismetMathLibrary::NormalizedDeltaRotator(FootRotation, OwnerCharacter->GetActorRotation());
    }
    else
    {
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
    }
}

float UAnim_PrehistoricMovementSystem::CalculateGroundSlope()
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    FVector TraceStart = ActorLocation + FVector(0.0f, 0.0f, 50.0f);
    FVector TraceEnd = ActorLocation - FVector(0.0f, 0.0f, 100.0f);
    
    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerCharacter);
    
    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResult,
        true
    );
    
    if (bHit)
    {
        TerrainData.GroundNormal = HitResult.Normal;
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
    }
    
    TerrainData.GroundNormal = FVector::UpVector;
    return 0.0f;
}

void UAnim_PrehistoricMovementSystem::SmoothFootIKTransitions(float DeltaTime)
{
    // Smoothly interpolate foot IK offsets and rotations
    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, TerrainData.LeftFootOffset, DeltaTime, FootIKInterpSpeed);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, TerrainData.RightFootOffset, DeltaTime, FootIKInterpSpeed);
    
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, TerrainData.LeftFootRotation, DeltaTime, FootIKInterpSpeed);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, TerrainData.RightFootRotation, DeltaTime, FootIKInterpSpeed);
    
    // Update terrain data with smoothed values
    TerrainData.LeftFootOffset = LeftFootIKOffset;
    TerrainData.RightFootOffset = RightFootIKOffset;
    TerrainData.LeftFootRotation = LeftFootIKRotation;
    TerrainData.RightFootRotation = RightFootIKRotation;
}

void UAnim_PrehistoricMovementSystem::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementData.MovementState != NewState && CanTransitionToState(NewState))
    {
        PreviousMovementState = CurrentMovementData.MovementState;
        CurrentMovementData.MovementState = NewState;
        StateTransitionTimer = 0.2f; // Default transition time
        
        // Handle specific state transitions
        switch (NewState)
        {
            case EAnim_MovementState::Jumping:
                if (JumpMontage)
                {
                    PlayMovementMontage(JumpMontage);
                }
                break;
            case EAnim_MovementState::Landing:
                if (LandMontage)
                {
                    PlayMovementMontage(LandMontage);
                }
                break;
            case EAnim_MovementState::Crouching:
                if (CrouchMontage)
                {
                    PlayMovementMontage(CrouchMontage);
                }
                break;
            case EAnim_MovementState::Climbing:
                if (ClimbMontage)
                {
                    PlayMovementMontage(ClimbMontage);
                }
                break;
        }
    }
}

EAnim_MovementState UAnim_PrehistoricMovementSystem::GetMovementState() const
{
    return CurrentMovementData.MovementState;
}

void UAnim_PrehistoricMovementSystem::PlayMovementMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_PrehistoricMovementSystem::TransitionToState(EAnim_MovementState TargetState, float TransitionTime)
{
    if (CanTransitionToState(TargetState))
    {
        SetMovementState(TargetState);
        StateTransitionTimer = TransitionTime;
    }
}

bool UAnim_PrehistoricMovementSystem::CanTransitionToState(EAnim_MovementState TargetState)
{
    return IsValidMovementTransition(CurrentMovementData.MovementState, TargetState);
}

bool UAnim_PrehistoricMovementSystem::IsValidMovementTransition(EAnim_MovementState From, EAnim_MovementState To)
{
    // Define valid state transitions for prehistoric movement
    switch (From)
    {
        case EAnim_MovementState::Idle:
            return To != EAnim_MovementState::Landing; // Can't land from idle
        case EAnim_MovementState::Jumping:
            return To == EAnim_MovementState::Falling || To == EAnim_MovementState::Landing;
        case EAnim_MovementState::Falling:
            return To == EAnim_MovementState::Landing || To == EAnim_MovementState::Jumping;
        case EAnim_MovementState::Landing:
            return To == EAnim_MovementState::Idle || To == EAnim_MovementState::Walking;
        default:
            return true; // Most transitions are valid
    }
}

void UAnim_PrehistoricMovementSystem::UpdateBlendSpaceValues()
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Update movement speed for blend spaces
    float NormalizedSpeed = 0.0f;
    switch (CurrentMovementData.MovementState)
    {
        case EAnim_MovementState::Walking:
            NormalizedSpeed = FMath::Clamp(CurrentMovementData.Speed / WalkSpeed, 0.0f, 1.0f);
            break;
        case EAnim_MovementState::Running:
            NormalizedSpeed = FMath::Clamp(CurrentMovementData.Speed / RunSpeed, 0.0f, 1.0f);
            break;
        case EAnim_MovementState::Sprinting:
            NormalizedSpeed = FMath::Clamp(CurrentMovementData.Speed / SprintSpeed, 0.0f, 1.0f);
            break;
    }
    
    // Set blend space parameters (these would be used in the Animation Blueprint)
    // AnimInstance->SetBlendSpaceInput(NormalizedSpeed, CurrentMovementData.Direction);
}

void UAnim_PrehistoricMovementSystem::ApplyPrehistoricMovementStyle()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Apply prehistoric movement characteristics
    // Slightly more cautious movement on rough terrain
    if (TerrainData.GroundSlope > 15.0f)
    {
        HandleRoughTerrain();
    }
    
    // Adjust movement based on locomotion type
    switch (LocomotionType)
    {
        case EAnim_LocomotionType::Bipedal:
            // Human-like movement with careful foot placement
            break;
        case EAnim_LocomotionType::Quadrupedal:
            // Animal-like movement patterns
            break;
        case EAnim_LocomotionType::Flying:
            // Flying creature movement
            break;
        case EAnim_LocomotionType::Swimming:
            // Aquatic movement
            break;
    }
}

void UAnim_PrehistoricMovementSystem::HandleRoughTerrain()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Reduce movement speed on steep terrain
    float SlopeSpeedMultiplier = 1.0f - (TerrainData.GroundSlope / MaxSlopeAngle) * 0.3f;
    SlopeSpeedMultiplier = FMath::Clamp(SlopeSpeedMultiplier, 0.5f, 1.0f);
    
    // Apply speed reduction (this would be applied to the character movement component)
    // MovementComponent->MaxWalkSpeed *= SlopeSpeedMultiplier;
}

void UAnim_PrehistoricMovementSystem::AdjustForCarriedWeight(float WeightFactor)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Reduce movement speed based on carried weight
    float WeightSpeedMultiplier = 1.0f - (WeightFactor * 0.4f);
    WeightSpeedMultiplier = FMath::Clamp(WeightSpeedMultiplier, 0.3f, 1.0f);
    
    // Apply weight-based speed reduction
    WalkSpeed *= WeightSpeedMultiplier;
    RunSpeed *= WeightSpeedMultiplier;
    SprintSpeed *= WeightSpeedMultiplier;
}