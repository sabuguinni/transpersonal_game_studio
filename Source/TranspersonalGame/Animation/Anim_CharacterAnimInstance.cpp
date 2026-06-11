#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../TranspersonalGameState.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Initialize animation variables
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    bIsRunning = false;
    
    HealthPercent = 1.0f;
    StaminaPercent = 1.0f;
    FearLevel = 0.0f;
    SurvivalState = ESurvivalState::Normal;
    
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    FearIntensity = 0.0f;
    
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    
    bIsInCombat = false;
    CurrentWeaponType = EWeaponType::None;
    bIsAttacking = false;
    
    Character = nullptr;
    MovementComponent = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache character reference
    Character = Cast<ACharacter>(GetOwningActor());
    if (Character)
    {
        MovementComponent = Character->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!Character || !MovementComponent)
    {
        return;
    }
    
    // Update all animation variables
    UpdateMovementVariables();
    UpdateSurvivalVariables();
    UpdateIKVariables(DeltaTimeX);
    UpdateCombatVariables();
}

void UAnim_CharacterAnimInstance::UpdateMovementVariables()
{
    if (!Character || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();
    
    // Calculate direction relative to character rotation
    if (Speed > 0.1f)
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Determine if moving left or right
        FVector RightVector = Character->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        if (RightDot < 0.0f)
        {
            Direction = -Direction;
        }
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Check if character is in air
    bIsInAir = MovementComponent->IsFalling();
    
    // Check if character is accelerating
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    bIsAccelerating = Acceleration.Size() > 0.1f;
    
    // Check if crouching
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine if running (speed threshold)
    bIsRunning = Speed > 300.0f;
    
    // Calculate blend values
    IdleToWalkBlend = FMath::Clamp(Speed / 150.0f, 0.0f, 1.0f);
    WalkToRunBlend = FMath::Clamp((Speed - 150.0f) / 300.0f, 0.0f, 1.0f);
}

void UAnim_CharacterAnimInstance::UpdateSurvivalVariables()
{
    // Try to get game state for survival stats
    if (UWorld* World = GetWorld())
    {
        if (ATranspersonalGameState* GameState = World->GetGameState<ATranspersonalGameState>())
        {
            HealthPercent = GameState->PlayerHealth / 100.0f;
            StaminaPercent = GameState->PlayerStamina / 100.0f;
            FearLevel = GameState->PlayerFear / 100.0f;
            SurvivalState = GameState->CurrentSurvivalState;
            
            // Calculate fear intensity for animation blending
            FearIntensity = FMath::Clamp(FearLevel, 0.0f, 1.0f);
        }
    }
}

void UAnim_CharacterAnimInstance::UpdateIKVariables(float DeltaTime)
{
    if (!Character)
    {
        return;
    }
    
    // Only update IK when character is on ground and moving slowly
    if (!bIsInAir && Speed < 200.0f)
    {
        // Get foot IK offsets
        float LeftOffset = GetFootIKOffset(TEXT("foot_l"));
        float RightOffset = GetFootIKOffset(TEXT("foot_r"));
        
        // Smoothly interpolate to new values
        LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, LeftOffset, DeltaTime, 15.0f);
        RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, RightOffset, DeltaTime, 15.0f);
        
        // Get foot rotations for slope adaptation
        LeftFootIKRotation = GetFootIKRotation(TEXT("foot_l"));
        RightFootIKRotation = GetFootIKRotation(TEXT("foot_r"));
    }
    else
    {
        // Reset IK when in air or moving fast
        LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, 0.0f, DeltaTime, 15.0f);
        RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, 0.0f, DeltaTime, 15.0f);
        LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, FRotator::ZeroRotator, DeltaTime, 15.0f);
        RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, FRotator::ZeroRotator, DeltaTime, 15.0f);
    }
}

void UAnim_CharacterAnimInstance::UpdateCombatVariables()
{
    // Basic combat state detection
    // This will be expanded when combat system is implemented
    bIsInCombat = FearLevel > 0.5f; // Simple fear-based combat detection
    
    // Weapon type detection would go here
    CurrentWeaponType = EWeaponType::None;
    
    // Attack state detection would go here
    bIsAttacking = false;
}

float UAnim_CharacterAnimInstance::GetFootIKOffset(FName SocketName, float TraceDistance)
{
    if (!Character)
    {
        return 0.0f;
    }
    
    FHitResult HitResult;
    if (LineTraceFromSocket(SocketName, TraceDistance, HitResult))
    {
        // Calculate offset needed to place foot on ground
        FVector SocketLocation = Character->GetMesh()->GetSocketLocation(SocketName);
        float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        float ExpectedGroundZ = Character->GetActorLocation().Z - CapsuleHalfHeight;
        float ActualGroundZ = HitResult.Location.Z;
        
        return ActualGroundZ - ExpectedGroundZ;
    }
    
    return 0.0f;
}

FRotator UAnim_CharacterAnimInstance::GetFootIKRotation(FName SocketName)
{
    if (!Character)
    {
        return FRotator::ZeroRotator;
    }
    
    FHitResult HitResult;
    if (LineTraceFromSocket(SocketName, 50.0f, HitResult))
    {
        // Calculate rotation to align foot with ground normal
        FVector GroundNormal = HitResult.Normal;
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(GroundNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, GroundNormal).GetSafeNormal();
        
        return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, GroundNormal);
    }
    
    return FRotator::ZeroRotator;
}

bool UAnim_CharacterAnimInstance::LineTraceFromSocket(FName SocketName, float TraceDistance, FHitResult& OutHit)
{
    if (!Character)
    {
        return false;
    }
    
    FVector SocketLocation = Character->GetMesh()->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0, 0, 20.0f); // Start slightly above socket
    FVector TraceEnd = SocketLocation - FVector(0, 0, TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    QueryParams.bTraceComplex = false;
    
    return GetWorld()->LineTraceSingleByChannel(
        OutHit,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
}