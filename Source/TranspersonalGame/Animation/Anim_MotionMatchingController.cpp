#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkThreshold = 50.0f;
    RunThreshold = 200.0f;
    SprintThreshold = 400.0f;
    MotionSmoothingRate = 5.0f;
    TerrainAnalysisRadius = 100.0f;
    
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache character references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (MovementComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("Motion Matching Controller initialized for: %s"), *OwnerCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Motion Matching Controller: No movement component found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Motion Matching Controller: Owner is not a Character"));
    }
}

void UAnim_MotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Throttle updates for performance
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateMotionData(LastUpdateTime);
        UpdateBlendSpaceParameters();
        LastUpdateTime = 0.0f;
    }
    
    StateTransitionTimer += DeltaTime;
}

void UAnim_MotionMatchingController::UpdateMotionData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Store previous frame data
    PreviousMotionData = CurrentMotionData;
    
    // Update velocity and speed
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    
    // Calculate acceleration
    FVector AccelerationDelta = (CurrentVelocity - PreviousMotionData.Velocity) / FMath::Max(DeltaTime, 0.001f);
    CurrentMotionData.Acceleration = FMath::VInterpTo(CurrentMotionData.Acceleration, AccelerationDelta, DeltaTime, MotionSmoothingRate);
    
    // Update direction relative to character forward
    if (CurrentMotionData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Determine if moving left or right
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection);
        if (CrossProduct.Z < 0.0f)
        {
            CurrentMotionData.Direction *= -1.0f;
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Update air state
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    if (!CurrentMotionData.bIsInAir)
    {
        LastGroundTime = GetWorld()->GetTimeSeconds();
        LastGroundLocation = OwnerCharacter->GetActorLocation();
    }
    
    // Update crouching state
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine movement state
    EAnim_MovementState NewState = DetermineMovementState();
    if (NewState != CurrentMotionData.MovementState)
    {
        SetMovementState(NewState);
    }
    
    // Analyze terrain
    CurrentMotionData.TerrainType = AnalyzeTerrainType();
}

EAnim_MovementState UAnim_MotionMatchingController::DetermineMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check air states first
    if (CurrentMotionData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 50.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else if (MovementComponent->Velocity.Z < -50.0f)
        {
            return EAnim_MovementState::Falling;
        }
    }
    else
    {
        // Just landed
        float TimeSinceGrounded = GetWorld()->GetTimeSeconds() - LastGroundTime;
        if (TimeSinceGrounded < 0.5f && PreviousMotionData.bIsInAir)
        {
            return EAnim_MovementState::Landing;
        }
    }
    
    // Check crouching
    if (CurrentMotionData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Determine ground movement state based on speed
    float Speed = CurrentMotionData.Speed;
    
    if (Speed < WalkThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (Speed < SprintThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

void UAnim_MotionMatchingController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMotionData.MovementState != NewState)
    {
        EAnim_MovementState PreviousState = CurrentMotionData.MovementState;
        CurrentMotionData.MovementState = NewState;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
        
        // Trigger animation montages for specific transitions
        if (NewState == EAnim_MovementState::Jumping && JumpMontage && MeshComponent)
        {
            UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Play(JumpMontage);
            }
        }
        else if (NewState == EAnim_MovementState::Landing && LandMontage && MeshComponent)
        {
            UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Play(LandMontage);
            }
        }
    }
}

EAnim_TerrainType UAnim_MotionMatchingController::AnalyzeTerrainType() const
{
    if (!OwnerCharacter)
    {
        return EAnim_TerrainType::Flat;
    }
    
    float Slope = GetTerrainSlope();
    
    // Classify terrain based on slope
    if (FMath::Abs(Slope) < 5.0f)
    {
        return EAnim_TerrainType::Flat;
    }
    else if (Slope > 5.0f)
    {
        return EAnim_TerrainType::Uphill;
    }
    else if (Slope < -5.0f)
    {
        return EAnim_TerrainType::Downhill;
    }
    
    return EAnim_TerrainType::Flat;
}

float UAnim_MotionMatchingController::GetTerrainSlope() const
{
    if (!OwnerCharacter || !GetWorld())
    {
        return 0.0f;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Cast ray forward to analyze slope
    FVector TraceStart = StartLocation + FVector(0, 0, 50);
    FVector TraceEnd = StartLocation + (ForwardVector * TerrainAnalysisRadius) - FVector(0, 0, 200);
    
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
        FVector HitLocation = HitResult.Location;
        FVector CurrentLocation = OwnerCharacter->GetActorLocation();
        
        float HeightDifference = HitLocation.Z - CurrentLocation.Z;
        float HorizontalDistance = FVector::Dist2D(HitLocation, CurrentLocation);
        
        if (HorizontalDistance > 0.0f)
        {
            float SlopeRadians = FMath::Atan(HeightDifference / HorizontalDistance);
            return FMath::RadiansToDegrees(SlopeRadians);
        }
    }
    
    return 0.0f;
}

UAnimSequence* UAnim_MotionMatchingController::SelectBestAnimation() const
{
    // Simple animation selection based on current state
    switch (CurrentMotionData.MovementState)
    {
        case EAnim_MovementState::Idle:
            if (IdleAnimations.Num() > 0)
            {
                return IdleAnimations[0];
            }
            break;
            
        case EAnim_MovementState::Walking:
            if (WalkAnimations.Num() > 0)
            {
                return WalkAnimations[0];
            }
            break;
            
        case EAnim_MovementState::Running:
        case EAnim_MovementState::Sprinting:
            if (RunAnimations.Num() > 0)
            {
                return RunAnimations[0];
            }
            break;
            
        default:
            break;
    }
    
    return nullptr;
}

float UAnim_MotionMatchingController::CalculateAnimationScore(UAnimSequence* Animation) const
{
    if (!Animation)
    {
        return 0.0f;
    }
    
    // Placeholder scoring system - in a full implementation this would
    // analyze the animation's motion data against current character motion
    float Score = 1.0f;
    
    // Factor in speed matching
    // This would require animation metadata or motion analysis
    
    return Score;
}

void UAnim_MotionMatchingController::UpdateBlendSpaceParameters()
{
    // Update blend space coordinates based on current motion
    // This would be used by the Animation Blueprint
    
    if (MovementBlendSpace && MeshComponent)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            FVector2D BlendCoords = GetBlendSpaceCoordinates();
            // In a full implementation, this would set blend space parameters
            // via the Animation Blueprint interface
        }
    }
}

FVector2D UAnim_MotionMatchingController::GetBlendSpaceCoordinates() const
{
    // Calculate 2D blend space coordinates
    // X-axis: Speed (normalized)
    // Y-axis: Direction (-180 to 180 degrees, normalized to -1 to 1)
    
    float NormalizedSpeed = FMath::Clamp(CurrentMotionData.Speed / SprintThreshold, 0.0f, 1.0f);
    float NormalizedDirection = FMath::Clamp(CurrentMotionData.Direction / 180.0f, -1.0f, 1.0f);
    
    return FVector2D(NormalizedSpeed, NormalizedDirection);
}