#include "PrimitiveAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UPrimitiveAnimInstance::UPrimitiveAnimInstance()
{
    // Initialize default values
    MovementState = EAnim_MovementState::Idle;
    CombatState = EAnim_CombatState::Unarmed;
    
    Speed = 0.0f;
    Direction = 0.0f;
    Velocity = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsMoving = false;
    
    bIsAttacking = false;
    bIsBlocking = false;
    bHasWeapon = false;
    ComboIndex = 0;
    
    FearLevel = 0.0f;
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    
    bIsOnUnevenTerrain = false;
    GroundSlope = 0.0f;
    bIsInWater = false;
    
    PreviousSpeed = 0.0f;
    PreviousDirection = 0.0f;
}

void UPrimitiveAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache character references
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        AnimController = OwnerCharacter->FindComponentByClass<UPrimitiveAnimationController>();
    }
}

void UPrimitiveAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation parameters
    UpdateMovementParameters();
    UpdateCombatParameters();
    UpdateSurvivalParameters();
    UpdateEnvironmentParameters();
}

void UPrimitiveAnimInstance::UpdateMovementParameters()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector CurrentVelocity = MovementComponent->Velocity;
    float CurrentSpeed = CurrentVelocity.Size();
    
    // Smooth speed transitions
    Speed = FMath::FInterpTo(PreviousSpeed, CurrentSpeed, GetWorld()->GetDeltaSeconds(), SpeedSmoothingRate);
    PreviousSpeed = Speed;
    
    // Calculate velocity magnitude for blend spaces
    Velocity = Speed;
    
    // Calculate movement direction relative to character forward
    if (Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentVelocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        float TargetDirection = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        Direction = FMath::FInterpTo(PreviousDirection, TargetDirection, GetWorld()->GetDeltaSeconds(), DirectionSmoothingRate);
        PreviousDirection = Direction;
        
        bIsMoving = true;
    }
    else
    {
        bIsMoving = false;
        Direction = 0.0f;
        PreviousDirection = 0.0f;
    }
    
    // Update movement state flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine movement state based on speed thresholds
    if (bIsInAir)
    {
        MovementState = MovementComponent->Velocity.Z > 0 ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    else if (bIsCrouching)
    {
        MovementState = EAnim_MovementState::Crouching;
    }
    else if (Speed < 10.0f)
    {
        MovementState = EAnim_MovementState::Idle;
    }
    else if (Speed < 300.0f)
    {
        MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementState = EAnim_MovementState::Running;
    }\n}

void UPrimitiveAnimInstance::UpdateCombatParameters()
{
    if (AnimController)
    {
        FAnim_CombatData CombatData = AnimController->GetCombatData();
        
        bIsAttacking = CombatData.bIsAttacking;
        bIsBlocking = CombatData.bIsBlocking;
        bHasWeapon = CombatData.bHasWeapon;
        ComboIndex = CombatData.ComboIndex;
        
        CombatState = AnimController->GetCurrentCombatState();
    }
}

void UPrimitiveAnimInstance::UpdateSurvivalParameters()
{
    // Get survival stats from character (placeholder values for now)
    // In a real implementation, these would come from a survival component
    FearLevel = 0.0f;
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    
    // Fear affects animation speed and intensity
    if (FearLevel > 0.5f)
    {
        // Increase animation playback rate when afraid
        // This would be handled in the animation blueprint
    }
}

void UPrimitiveAnimInstance::UpdateEnvironmentParameters()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Perform ground trace to detect terrain slope
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        FVector GroundNormal = HitResult.Normal;
        FVector UpVector = FVector::UpVector;
        
        // Calculate ground slope angle
        float DotProduct = FVector::DotProduct(GroundNormal, UpVector);
        float SlopeAngle = FMath::Acos(DotProduct) * (180.0f / PI);
        GroundSlope = SlopeAngle;
        
        // Consider terrain uneven if slope > 15 degrees
        bIsOnUnevenTerrain = SlopeAngle > 15.0f;
    }
    
    // Check for water (simplified - would use water volume in real implementation)
    bIsInWater = false;
}

// Animation event functions
void UPrimitiveAnimInstance::OnJumpStart()
{
    // Called when jump animation starts
    UE_LOG(LogTemp, Log, TEXT("Jump animation started"));
}

void UPrimitiveAnimInstance::OnLanding()
{
    // Called when landing animation plays
    UE_LOG(LogTemp, Log, TEXT("Landing animation triggered"));
}

void UPrimitiveAnimInstance::OnAttackStart()
{
    // Called when attack animation starts
    bIsAttacking = true;
    UE_LOG(LogTemp, Log, TEXT("Attack animation started"));
}

void UPrimitiveAnimInstance::OnAttackEnd()
{
    // Called when attack animation ends
    bIsAttacking = false;
    UE_LOG(LogTemp, Log, TEXT("Attack animation ended"));
}

void UPrimitiveAnimInstance::OnFearReaction()
{
    // Called when fear reaction animation triggers
    UE_LOG(LogTemp, Log, TEXT("Fear reaction animation triggered"));
}

// State transition helper functions
bool UPrimitiveAnimInstance::ShouldTransitionToIdle() const
{
    return Speed < 10.0f && !bIsInAir && !bIsCrouching;
}

bool UPrimitiveAnimInstance::ShouldTransitionToWalk() const
{
    return Speed >= 10.0f && Speed < 300.0f && !bIsInAir && !bIsCrouching;
}

bool UPrimitiveAnimInstance::ShouldTransitionToRun() const
{
    return Speed >= 300.0f && !bIsInAir && !bIsCrouching;
}

bool UPrimitiveAnimInstance::ShouldTransitionToJump() const
{
    return bIsInAir && MovementComponent && MovementComponent->Velocity.Z > 0;
}