#include "Anim_PrehistoricLocomotionSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PrehistoricLocomotionSystem::UAnim_PrehistoricLocomotionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize locomotion settings
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 600.0f;
    TurnInPlaceThreshold = 45.0f;
    SlopeAngleThreshold = 30.0f;
    MovementDirectionDeadzone = 0.1f;

    // Initialize state tracking
    PreviousState = EAnim_LocomotionState::Idle;
    StateChangeTimer = 0.0f;
    LastGroundedTime = 0.0f;
    bWasMovingLastFrame = false;
    LastVelocity = FVector::ZeroVector;
    AccumulatedTurnAmount = 0.0f;

    // Initialize component references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;

    // Initialize animation assets
    LocomotionBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    CrouchMontage = nullptr;
}

void UAnim_PrehistoricLocomotionSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_PrehistoricLocomotionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (OwnerCharacter && MovementComponent)
    {
        UpdateLocomotionData(DeltaTime);
    }
}

void UAnim_PrehistoricLocomotionSystem::InitializeComponent()
{
    CacheCharacterReferences();
    
    if (OwnerCharacter)
    {
        // Initialize locomotion data
        LocomotionData.CurrentState = EAnim_LocomotionState::Idle;
        LocomotionData.MovementDirection = EAnim_MovementDirection::Forward;
        
        UE_LOG(LogTemp, Log, TEXT("Prehistoric Locomotion System initialized for %s"), 
               *OwnerCharacter->GetName());
    }
}

void UAnim_PrehistoricLocomotionSystem::CacheCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Prehistoric Locomotion System: Owner is not a Character"));
    }
}

void UAnim_PrehistoricLocomotionSystem::UpdateLocomotionData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Store previous state for transition detection
    PreviousState = LocomotionData.CurrentState;
    
    // Update core movement data
    UpdateMovementState(DeltaTime);
    
    // Analyze movement input and direction
    AnalyzeMovementInput();
    CalculateMovementDirection();
    
    // Update turn in place
    UpdateTurnInPlace(DeltaTime);
    
    // Adapt to terrain
    AdaptToTerrain();
    
    // Handle state transitions
    HandleStateTransitions();
    
    // Smooth values for better animation blending
    SmoothLocomotionValues(DeltaTime);
    
    // Update state timer
    StateChangeTimer += DeltaTime;
}

void UAnim_PrehistoricLocomotionSystem::UpdateMovementState(float DeltaTime)
{
    const FVector CurrentVelocity = MovementComponent->Velocity;
    const float CurrentSpeed = CurrentVelocity.Size();
    
    // Update basic movement data
    LocomotionData.Speed = CurrentSpeed;
    LocomotionData.bIsMoving = CurrentSpeed > MovementDirectionDeadzone;
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Update airborne or grounded state
    if (LocomotionData.bIsInAir)
    {
        UpdateAirborneState();
    }
    else
    {
        UpdateGroundedState();
        LastGroundedTime = GetWorld()->GetTimeSeconds();
    }
    
    // Store velocity for next frame
    LastVelocity = CurrentVelocity;
    bWasMovingLastFrame = LocomotionData.bIsMoving;
}

void UAnim_PrehistoricLocomotionSystem::UpdateAirborneState()
{
    const FVector CurrentVelocity = MovementComponent->Velocity;
    
    if (CurrentVelocity.Z > 0.0f)
    {
        // Moving upward - jumping
        SetLocomotionState(EAnim_LocomotionState::Jumping);
    }
    else
    {
        // Moving downward - falling
        SetLocomotionState(EAnim_LocomotionState::Falling);
    }
}

void UAnim_PrehistoricLocomotionSystem::UpdateGroundedState()
{
    if (LocomotionData.bIsCrouching)
    {
        SetLocomotionState(EAnim_LocomotionState::Crouching);
        return;
    }
    
    const float CurrentSpeed = LocomotionData.Speed;
    
    if (!LocomotionData.bIsMoving)
    {
        SetLocomotionState(EAnim_LocomotionState::Idle);
    }
    else if (CurrentSpeed < WalkSpeedThreshold)
    {
        SetLocomotionState(EAnim_LocomotionState::Walking);
    }
    else if (CurrentSpeed < RunSpeedThreshold)
    {
        SetLocomotionState(EAnim_LocomotionState::Running);
    }
    else
    {
        SetLocomotionState(EAnim_LocomotionState::Sprinting);
    }
}

void UAnim_PrehistoricLocomotionSystem::SetLocomotionState(EAnim_LocomotionState NewState)
{
    if (LocomotionData.CurrentState != NewState)
    {
        PreviousState = LocomotionData.CurrentState;
        LocomotionData.CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Locomotion state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_PrehistoricLocomotionSystem::AnalyzeMovementInput()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get the current acceleration (input direction)
    const FVector CurrentAcceleration = MovementComponent->GetCurrentAcceleration();
    const FVector CurrentVelocity = MovementComponent->Velocity;
    
    // Calculate direction relative to character's forward vector
    if (CurrentVelocity.SizeSquared() > FMath::Square(MovementDirectionDeadzone))
    {
        const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        const FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        // Project velocity onto character's local axes
        const float ForwardDot = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), ForwardVector);
        const float RightDot = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), RightVector);
        
        // Calculate direction angle
        LocomotionData.Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
        
        // Calculate lean amount for strafing
        LocomotionData.LeanAmount = FMath::Clamp(RightDot * 2.0f, -1.0f, 1.0f);
    }
    else
    {
        LocomotionData.Direction = 0.0f;
        LocomotionData.LeanAmount = 0.0f;
    }
}

void UAnim_PrehistoricLocomotionSystem::CalculateMovementDirection()
{
    const float DirectionAngle = LocomotionData.Direction;
    const float AbsAngle = FMath::Abs(DirectionAngle);
    
    // Determine movement direction based on angle
    if (AbsAngle <= 22.5f)
    {
        LocomotionData.MovementDirection = EAnim_MovementDirection::Forward;
    }
    else if (AbsAngle >= 157.5f)
    {
        LocomotionData.MovementDirection = EAnim_MovementDirection::Backward;
    }
    else if (DirectionAngle > 0.0f)
    {
        if (DirectionAngle <= 67.5f)
        {
            LocomotionData.MovementDirection = EAnim_MovementDirection::ForwardRight;
        }
        else if (DirectionAngle <= 112.5f)
        {
            LocomotionData.MovementDirection = EAnim_MovementDirection::Right;
        }
        else
        {
            LocomotionData.MovementDirection = EAnim_MovementDirection::BackwardRight;
        }
    }
    else
    {
        if (DirectionAngle >= -67.5f)
        {
            LocomotionData.MovementDirection = EAnim_MovementDirection::ForwardLeft;
        }
        else if (DirectionAngle >= -112.5f)
        {
            LocomotionData.MovementDirection = EAnim_MovementDirection::Left;
        }
        else
        {
            LocomotionData.MovementDirection = EAnim_MovementDirection::BackwardLeft;
        }
    }
}

void UAnim_PrehistoricLocomotionSystem::UpdateTurnInPlace(float DeltaTime)
{
    if (!OwnerCharacter || LocomotionData.bIsMoving)
    {
        LocomotionData.TurnInPlace = 0.0f;
        AccumulatedTurnAmount = 0.0f;
        return;
    }
    
    // Calculate rotation change since last frame
    const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    static FRotator LastRotation = CurrentRotation;
    
    const float DeltaYaw = FMath::FindDeltaAngleDegrees(LastRotation.Yaw, CurrentRotation.Yaw);
    AccumulatedTurnAmount += DeltaYaw;
    
    // Apply turn in place if accumulated turn exceeds threshold
    if (FMath::Abs(AccumulatedTurnAmount) > TurnInPlaceThreshold)
    {
        LocomotionData.TurnInPlace = FMath::Sign(AccumulatedTurnAmount);
        
        // Reset accumulated turn after triggering
        if (FMath::Abs(DeltaYaw) < 1.0f) // Only reset when rotation stabilizes
        {
            AccumulatedTurnAmount = 0.0f;
        }
    }
    else
    {
        LocomotionData.TurnInPlace = FMath::FInterpTo(LocomotionData.TurnInPlace, 0.0f, DeltaTime, 5.0f);
    }
    
    LastRotation = CurrentRotation;
}

void UAnim_PrehistoricLocomotionSystem::AdaptToTerrain()
{
    // This will be expanded to handle terrain-specific adaptations
    // For now, just check if we're on a steep slope
    if (IsOnSteepSlope())
    {
        // Modify movement parameters for steep terrain
        const float SlopeAngle = GetSlopeAngle();
        
        // Reduce effective speed on steep slopes
        const float SlopeFactor = FMath::Clamp(1.0f - (SlopeAngle / 90.0f), 0.5f, 1.0f);
        LocomotionData.Speed *= SlopeFactor;
    }
}

bool UAnim_PrehistoricLocomotionSystem::IsOnSteepSlope() const
{
    return GetSlopeAngle() > SlopeAngleThreshold;
}

float UAnim_PrehistoricLocomotionSystem::GetSlopeAngle() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }
    
    const FVector FloorNormal = MovementComponent->CurrentFloor.HitResult.Normal;
    const float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FloorNormal.Z));
    
    return SlopeAngle;
}

void UAnim_PrehistoricLocomotionSystem::HandleStateTransitions()
{
    // Handle landing state
    if (PreviousState == EAnim_LocomotionState::Falling && 
        LocomotionData.CurrentState != EAnim_LocomotionState::Falling &&
        LocomotionData.CurrentState != EAnim_LocomotionState::Jumping)
    {
        // Just landed
        SetLocomotionState(EAnim_LocomotionState::Landing);
        
        // Play landing montage if available
        if (LandMontage && MeshComponent)
        {
            PlayLocomotionMontage(LandMontage);
        }
    }
}

void UAnim_PrehistoricLocomotionSystem::SmoothLocomotionValues(float DeltaTime)
{
    // Smooth speed changes for better animation blending
    const float SpeedInterpRate = 5.0f;
    LocomotionData.Speed = FMath::FInterpTo(LocomotionData.Speed, MovementComponent->Velocity.Size(), DeltaTime, SpeedInterpRate);
    
    // Smooth direction changes
    const float DirectionInterpRate = 8.0f;
    const float TargetDirection = CalculateDirectionAngle();
    LocomotionData.Direction = FMath::FInterpAngle(LocomotionData.Direction, TargetDirection, DeltaTime, DirectionInterpRate);
    
    // Smooth lean amount
    const float LeanInterpRate = 6.0f;
    // LocomotionData.LeanAmount is already calculated in AnalyzeMovementInput
}

float UAnim_PrehistoricLocomotionSystem::CalculateSpeedRatio() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }
    
    const float MaxSpeed = MovementComponent->GetMaxSpeed();
    return MaxSpeed > 0.0f ? (LocomotionData.Speed / MaxSpeed) : 0.0f;
}

float UAnim_PrehistoricLocomotionSystem::CalculateDirectionAngle() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }
    
    const FVector CurrentVelocity = MovementComponent->Velocity;
    if (CurrentVelocity.SizeSquared() < FMath::Square(MovementDirectionDeadzone))
    {
        return 0.0f;
    }
    
    const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    const FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    const FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
    const float ForwardDot = FVector::DotProduct(VelocityDirection, ForwardVector);
    const float RightDot = FVector::DotProduct(VelocityDirection, RightVector);
    
    return FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
}

void UAnim_PrehistoricLocomotionSystem::PlayLocomotionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !MeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        
        UE_LOG(LogTemp, Log, TEXT("Playing locomotion montage: %s"), *Montage->GetName());
    }
}

void UAnim_PrehistoricLocomotionSystem::StopLocomotionMontage(float BlendOutTime)
{
    if (!MeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(BlendOutTime);
        
        UE_LOG(LogTemp, Log, TEXT("Stopping locomotion montage with blend time: %f"), BlendOutTime);
    }
}