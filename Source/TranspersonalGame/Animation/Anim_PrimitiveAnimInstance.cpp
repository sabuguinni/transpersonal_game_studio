#include "Anim_PrimitiveAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../TranspersonalGameState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_PrimitiveAnimInstance::UAnim_PrimitiveAnimInstance()
{
    // Initialize animation properties
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsAccelerating = false;
    MovementState = EAnim_MovementState::Idle;
    
    // Initialize IK properties
    IKBodyOffset = 0.0f;
    
    // Initialize survival properties
    HealthPercent = 1.0f;
    StaminaPercent = 1.0f;
    FearLevel = 0.0f;
    bIsInjured = false;
    bIsExhausted = false;
    
    // Initialize combat properties
    bIsInCombat = false;
    bIsBlocking = false;
    bIsAttacking = false;
    CurrentWeaponType = EAnim_WeaponType::Unarmed;
    
    // Initialize environmental properties
    GroundSlope = 0.0f;
    bIsOnUnevenTerrain = false;
    bIsInWater = false;
    WaterDepth = 0.0f;
    
    // Initialize component references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MotionMatchingSystem = nullptr;
    GameState = nullptr;
    
    // Initialize internal state
    PreviousMovementState = EAnim_MovementState::Idle;
    bPreviousCombatState = false;
    bPreviousInjuryState = false;
    
    // Initialize smoothing rates
    SpeedSmoothRate = 5.0f;
    DirectionSmoothRate = 10.0f;
    IKSmoothRate = 15.0f;
}

void UAnim_PrimitiveAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        // Get movement component
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        // Get motion matching system
        MotionMatchingSystem = OwnerCharacter->FindComponentByClass<UAnim_MotionMatchingSystem>();
        
        // Get game state
        if (UWorld* World = OwnerCharacter->GetWorld())
        {
            GameState = Cast<UTranspersonalGameState>(World->GetGameState());
        }
    }
}

void UAnim_PrimitiveAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation properties
    UpdateMovementProperties();
    UpdateIKProperties();
    UpdateSurvivalProperties();
    UpdateCombatProperties();
    UpdateEnvironmentalProperties();
    
    // Check for state changes and trigger events
    if (MovementState != PreviousMovementState)
    {
        OnMovementStateChanged(MovementState);
        PreviousMovementState = MovementState;
    }
    
    if (bIsInCombat != bPreviousCombatState)
    {
        OnCombatStateChanged(bIsInCombat);
        bPreviousCombatState = bIsInCombat;
    }
    
    if (bIsInjured != bPreviousInjuryState)
    {
        OnInjuryStateChanged(bIsInjured);
        bPreviousInjuryState = bIsInjured;
    }
}

void UAnim_PrimitiveAnimInstance::UpdateMovementProperties()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size();
    
    // Smooth speed changes
    Speed = FMath::FInterpTo(Speed, CurrentSpeed, GetDeltaSeconds(), SpeedSmoothRate);
    
    // Calculate direction relative to character forward
    if (CurrentSpeed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        float TargetDirection = FMath::Atan2(RightDot, ForwardDot) * 180.0f / PI;
        Direction = FMath::FInterpAngle(Direction, TargetDirection, GetDeltaSeconds(), DirectionSmoothRate);
    }
    
    // Update movement flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Determine movement state
    if (bIsInAir)
    {
        MovementState = EAnim_MovementState::Jumping;
    }
    else if (bIsCrouching)
    {
        MovementState = EAnim_MovementState::Crouching;
    }
    else if (Speed > 400.0f)
    {
        MovementState = EAnim_MovementState::Running;
    }
    else if (Speed > 50.0f)
    {
        MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_PrimitiveAnimInstance::UpdateIKProperties()
{
    if (!MotionMatchingSystem)
    {
        return;
    }
    
    // Get IK data from motion matching system
    LeftFootIK = MotionMatchingSystem->GetLeftFootIK();
    RightFootIK = MotionMatchingSystem->GetRightFootIK();
    
    // Update body IK offset
    UpdateBodyIK();
}

void UAnim_PrimitiveAnimInstance::UpdateSurvivalProperties()
{
    if (!GameState)
    {
        return;
    }
    
    // Update survival stats from game state
    HealthPercent = GameState->GetHealthPercent();
    StaminaPercent = GameState->GetStaminaPercent();
    FearLevel = GameState->GetFearLevel();
    
    // Determine injury and exhaustion states
    bIsInjured = HealthPercent < 0.5f;
    bIsExhausted = StaminaPercent < 0.2f;
}

void UAnim_PrimitiveAnimInstance::UpdateCombatProperties()
{
    if (!GameState)
    {
        return;
    }
    
    // Update combat state from game state
    bIsInCombat = GameState->IsInCombat();
    bIsBlocking = GameState->IsBlocking();
    bIsAttacking = GameState->IsAttacking();
    CurrentWeaponType = GameState->GetCurrentWeaponType();
}

void UAnim_PrimitiveAnimInstance::UpdateEnvironmentalProperties()
{
    // Calculate ground slope
    GroundSlope = CalculateGroundSlope();
    
    // Check for uneven terrain
    bIsOnUnevenTerrain = CheckUnevenTerrain();
    
    // Check water depth (placeholder - would need water volume system)
    bIsInWater = false;
    WaterDepth = 0.0f;
}

float UAnim_PrimitiveAnimInstance::CalculateGroundSlope()
{
    if (!OwnerCharacter || !GetWorld())
    {
        return 0.0f;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        FVector Normal = HitResult.Normal;
        FVector UpVector = FVector::UpVector;
        
        float DotProduct = FVector::DotProduct(Normal, UpVector);
        float Angle = FMath::Acos(DotProduct) * 180.0f / PI;
        
        return Angle;
    }
    
    return 0.0f;
}

bool UAnim_PrimitiveAnimInstance::CheckUnevenTerrain()
{
    // Simple check based on ground slope
    return GroundSlope > 15.0f;
}

void UAnim_PrimitiveAnimInstance::UpdateBodyIK()
{
    if (!LeftFootIK.bIsValidIKTarget && !RightFootIK.bIsValidIKTarget)
    {
        IKBodyOffset = FMath::FInterpTo(IKBodyOffset, 0.0f, GetDeltaSeconds(), IKSmoothRate);
        return;
    }
    
    // Calculate average foot offset for body adjustment
    float LeftOffset = LeftFootIK.bIsValidIKTarget ? LeftFootIK.FootLocation.Z : 0.0f;
    float RightOffset = RightFootIK.bIsValidIKTarget ? RightFootIK.FootLocation.Z : 0.0f;
    
    float TargetOffset = (LeftOffset + RightOffset) * 0.5f;
    
    // Clamp the offset to reasonable values
    TargetOffset = FMath::Clamp(TargetOffset, -50.0f, 50.0f);
    
    // Smooth the transition
    IKBodyOffset = FMath::FInterpTo(IKBodyOffset, TargetOffset, GetDeltaSeconds(), IKSmoothRate);
}