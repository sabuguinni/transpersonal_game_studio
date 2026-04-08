#include "ABP_PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UABP_PlayerCharacter::UABP_PlayerCharacter()
{
    // Initialize default values
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    
    StressLevel = EPlayerStress::Calm;
    FearIntensity = 0.0f;
    ExhaustionLevel = 0.0f;
    InjuryLevel = 0.0f;
    bIsBeingHunted = false;
    bIsHiding = false;
    
    CurrentAnimState = EPlayerAnimState::Idle;
    bIsCrouching = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsClimbing = false;
    
    GroundSlope = 0.0f;
    bIsOnUnstableGround = false;
    WeatherIntensity = 0.0f;
    
    TurnRate = 0.0f;
    bShouldUseMotionMatching = true;
    
    bEnableFootIK = true;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    LeftFootRotation = FRotator::ZeroRotator;
    RightFootRotation = FRotator::ZeroRotator;
    
    BreathingRate = 1.0f; // Normal breathing
    HeartRate = 1.0f; // Normal heart rate
    NervousTwitchIntensity = 0.0f;
    LookAroundFrequency = 0.0f;
}

void UABP_PlayerCharacter::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
}

void UABP_PlayerCharacter::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation variables
    UpdateMovementVariables();
    UpdateEmotionalState();
    UpdateActivityState();
    UpdateEnvironmentalAdaptation();
    UpdateFootIK();
    UpdateBreathingAndMicroMovements();
}

void UABP_PlayerCharacter::UpdateMovementVariables()
{
    if (!OwningCharacter || !MovementComponent)
        return;
        
    // Basic movement variables
    Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    bIsInAir = MovementComponent->IsFalling();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Calculate direction relative to character rotation
    if (Speed > 0.1f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if turning left or right
        FVector RightVector = OwningCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            Direction *= -1.0f;
        }
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Calculate turn rate for motion matching
    static FRotator LastRotation = OwningCharacter->GetActorRotation();
    FRotator CurrentRotation = OwningCharacter->GetActorRotation();
    TurnRate = (CurrentRotation.Yaw - LastRotation.Yaw) / GetWorld()->GetDeltaSeconds();
    LastRotation = CurrentRotation;
    
    // Desired velocity for motion matching prediction
    DesiredVelocity = MovementComponent->GetCurrentAcceleration().GetClampedToMaxSize(MovementComponent->GetMaxSpeed());
}

void UABP_PlayerCharacter::UpdateEmotionalState()
{
    // Calculate fear based on nearby threats (placeholder - will be connected to gameplay systems)
    FearIntensity = CalculateFearFromNearbyThreats();
    
    // Calculate exhaustion based on recent activity
    ExhaustionLevel = CalculateExhaustionFromActivity();
    
    // Determine overall stress level
    StressLevel = DetermineStressLevel();
    
    // Update breathing and heart rate based on emotional state
    BreathingRate = FMath::Lerp(1.0f, 3.0f, FearIntensity); // Faster breathing when scared
    HeartRate = FMath::Lerp(1.0f, 2.5f, FearIntensity + ExhaustionLevel * 0.5f);
    
    // Nervous behaviors increase with stress
    switch (StressLevel)
    {
        case EPlayerStress::Calm:
            NervousTwitchIntensity = 0.0f;
            LookAroundFrequency = 0.1f;
            break;
        case EPlayerStress::Nervous:
            NervousTwitchIntensity = 0.2f;
            LookAroundFrequency = 0.3f;
            break;
        case EPlayerStress::Scared:
            NervousTwitchIntensity = 0.5f;
            LookAroundFrequency = 0.7f;
            break;
        case EPlayerStress::Terrified:
            NervousTwitchIntensity = 1.0f;
            LookAroundFrequency = 1.0f;
            break;
    }
}

void UABP_PlayerCharacter::UpdateActivityState()
{
    // Determine current animation state based on movement and actions
    if (bIsCrafting)
    {
        CurrentAnimState = EPlayerAnimState::Crafting;
    }
    else if (bIsGathering)
    {
        CurrentAnimState = EPlayerAnimState::Gathering;
    }
    else if (bIsClimbing)
    {
        CurrentAnimState = EPlayerAnimState::Climbing;
    }
    else if (bIsInAir)
    {
        CurrentAnimState = EPlayerAnimState::Falling;
    }
    else if (bIsHiding)
    {
        CurrentAnimState = EPlayerAnimState::Hiding;
    }
    else if (bIsCrouching)
    {
        CurrentAnimState = EPlayerAnimState::Crouching;
    }
    else if (InjuryLevel > 0.7f)
    {
        CurrentAnimState = EPlayerAnimState::Injured;
    }
    else if (ExhaustionLevel > 0.8f)
    {
        CurrentAnimState = EPlayerAnimState::Exhausted;
    }
    else if (FearIntensity > 0.8f)
    {
        CurrentAnimState = EPlayerAnimState::Panicked;
    }
    else if (FearIntensity > 0.3f || bIsBeingHunted)
    {
        CurrentAnimState = EPlayerAnimState::Alert;
    }
    else if (Speed > 300.0f) // Running threshold
    {
        CurrentAnimState = EPlayerAnimState::Running;
    }
    else if (Speed > 10.0f) // Walking threshold
    {
        CurrentAnimState = EPlayerAnimState::Walking;
    }
    else
    {
        CurrentAnimState = EPlayerAnimState::Idle;
    }
}

void UABP_PlayerCharacter::UpdateEnvironmentalAdaptation()
{
    if (!OwningCharacter)
        return;
        
    // Trace down to find ground slope and surface type
    FVector StartLocation = OwningCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Calculate ground slope
        FVector GroundNormal = HitResult.Normal;
        GroundSlope = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
        
        // Check for unstable ground (placeholder - would check surface material)
        bIsOnUnstableGround = GroundSlope > 30.0f;
    }
    else
    {
        GroundSlope = 0.0f;
        bIsOnUnstableGround = false;
    }
}

void UABP_PlayerCharacter::UpdateFootIK()
{
    if (!bEnableFootIK || !OwningCharacter)
        return;
        
    // Simplified foot IK - traces down from each foot to find ground offset
    // This would be expanded with proper bone positions in the actual implementation
    
    FVector LeftFootWorldPos = OwningCharacter->GetActorLocation() + OwningCharacter->GetActorRightVector() * -20.0f;
    FVector RightFootWorldPos = OwningCharacter->GetActorLocation() + OwningCharacter->GetActorRightVector() * 20.0f;
    
    // Trace for left foot
    FVector LeftStart = LeftFootWorldPos + FVector(0, 0, 50.0f);
    FVector LeftEnd = LeftFootWorldPos - FVector(0, 0, 100.0f);
    
    FHitResult LeftHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_WorldStatic, QueryParams))
    {
        float LeftDistance = (LeftHit.Location - LeftFootWorldPos).Z;
        LeftFootOffset = FVector(0, 0, LeftDistance);
        
        // Calculate foot rotation to match ground normal
        FVector GroundNormal = LeftHit.Normal;
        LeftFootRotation = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, GroundNormal);
    }
    
    // Trace for right foot
    FVector RightStart = RightFootWorldPos + FVector(0, 0, 50.0f);
    FVector RightEnd = RightFootWorldPos - FVector(0, 0, 100.0f);
    
    FHitResult RightHit;
    if (GetWorld()->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_WorldStatic, QueryParams))
    {
        float RightDistance = (RightHit.Location - RightFootWorldPos).Z;
        RightFootOffset = FVector(0, 0, RightDistance);
        
        // Calculate foot rotation to match ground normal
        FVector GroundNormal = RightHit.Normal;
        RightFootRotation = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, GroundNormal);
    }
}

void UABP_PlayerCharacter::UpdateBreathingAndMicroMovements()
{
    // These would drive subtle additive animations for breathing, heartbeat, nervous twitches
    // The actual implementation would use timeline curves and additive blend nodes
    
    // Breathing affects chest and shoulder movement
    // Heart rate affects subtle body shake
    // Nervous twitches affect head and hand movements
    // Look around frequency affects head turn animations
}

float UABP_PlayerCharacter::CalculateFearFromNearbyThreats()
{
    // Placeholder - in actual implementation, this would:
    // 1. Query nearby dinosaur AI agents
    // 2. Check if any are in "hunting" or "aggressive" state
    // 3. Calculate distance and threat level
    // 4. Return fear intensity based on proximity and danger
    
    return FMath::Clamp(FearIntensity, 0.0f, 1.0f);
}

float UABP_PlayerCharacter::CalculateExhaustionFromActivity()
{
    // Placeholder - in actual implementation, this would:
    // 1. Track recent movement distance and speed
    // 2. Track recent actions (crafting, gathering, climbing)
    // 3. Calculate stamina drain over time
    // 4. Return exhaustion level
    
    return FMath::Clamp(ExhaustionLevel, 0.0f, 1.0f);
}

EPlayerStress UABP_PlayerCharacter::DetermineStressLevel()
{
    float TotalStress = FearIntensity + ExhaustionLevel * 0.3f + InjuryLevel * 0.5f;
    
    if (TotalStress > 0.8f)
        return EPlayerStress::Terrified;
    else if (TotalStress > 0.5f)
        return EPlayerStress::Scared;
    else if (TotalStress > 0.2f)
        return EPlayerStress::Nervous;
    else
        return EPlayerStress::Calm;
}