#include "PlayerAnimationBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UPlayerAnimationBlueprint::UPlayerAnimationBlueprint()
{
    // Initialize default values
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    
    HealthPercentage = 100.0f;
    HungerLevel = 0.0f;
    ThirstLevel = 0.0f;
    FearLevel = 0.0f;
    bIsInjured = false;
    bIsExhausted = false;
    
    GroundSlope = 0.0f;
    bIsOnRoughTerrain = false;
    bIsInWater = false;
    
    bShouldPlayFearAnimation = false;
    bShouldPlayPainAnimation = false;
    AnimationBlendWeight = 1.0f;
    
    Character = nullptr;
    MovementComponent = nullptr;
}

void UPlayerAnimationBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get reference to the character
    Character = Cast<ACharacter>(GetOwningActor());
    
    if (Character)
    {
        MovementComponent = Character->GetCharacterMovement();
    }
}

void UPlayerAnimationBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!Character || !MovementComponent)
    {
        return;
    }
    
    // Update all animation variables
    UpdateLocomotionVariables();
    UpdateSurvivalStates();
    UpdateEnvironmentalReactions();
    UpdateAnimationStates();
}

void UPlayerAnimationBlueprint::UpdateLocomotionVariables()
{
    if (!Character || !MovementComponent)
    {
        return;
    }
    
    // Calculate speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to character forward
    if (Speed > 3.0f) // Only calculate direction if moving
    {
        FVector Forward = Character->GetActorForwardVector();
        FVector Right = Character->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(Forward, NormalizedVelocity);
        float RightDot = FVector::DotProduct(Right, NormalizedVelocity);
        
        Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Check if in air
    bIsInAir = MovementComponent->IsFalling();
    
    // Check if accelerating
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    bIsAccelerating = Acceleration.SizeSquared() > 0.0f;
    
    // Check if crouching
    bIsCrouching = MovementComponent->IsCrouching();
}

void UPlayerAnimationBlueprint::UpdateSurvivalStates()
{
    if (!Character)
    {
        return;
    }
    
    // Try to get survival stats from character
    // This would normally interface with a survival component
    // For now, we'll use placeholder logic
    
    // Health percentage (would come from health component)
    HealthPercentage = FMath::Clamp(HealthPercentage, 0.0f, 100.0f);
    bIsInjured = HealthPercentage < 50.0f;
    
    // Fear level affects animation intensity
    bShouldPlayFearAnimation = FearLevel > 50.0f;
    
    // Exhaustion affects movement speed
    bIsExhausted = Speed > 400.0f; // Running for too long
    
    // Pain animations
    bShouldPlayPainAnimation = bIsInjured && FearLevel > 30.0f;
}

void UPlayerAnimationBlueprint::UpdateEnvironmentalReactions()
{
    if (!Character)
    {
        return;
    }
    
    // Calculate ground slope
    GroundSlope = CalculateGroundSlope();
    
    // Check terrain type
    bIsOnRoughTerrain = IsOnRoughTerrain();
    
    // Check if in water (simplified check)
    FVector CharacterLocation = Character->GetActorLocation();
    UWorld* World = Character->GetWorld();
    
    if (World)
    {
        // Simple water check - would be more sophisticated in real implementation
        bIsInWater = CharacterLocation.Z < 0.0f; // Below "sea level"
    }
}

void UPlayerAnimationBlueprint::UpdateAnimationStates()
{
    // Calculate blend weights based on survival states
    float FearWeight = FearLevel / 100.0f;
    float HealthWeight = HealthPercentage / 100.0f;
    
    // Blend weight affects how much survival animations override locomotion
    AnimationBlendWeight = FMath::Clamp(1.0f - (FearWeight * 0.5f), 0.3f, 1.0f);
}

float UPlayerAnimationBlueprint::CalculateGroundSlope() const
{
    if (!Character)
    {
        return 0.0f;
    }
    
    UWorld* World = Character->GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    // Perform a line trace downward to get ground normal
    FVector Start = Character->GetActorLocation();
    FVector End = Start - FVector(0, 0, 200.0f); // Trace 200 units down
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        FVector GroundNormal = HitResult.Normal;
        FVector UpVector = FVector::UpVector;
        
        // Calculate angle between ground normal and up vector
        float DotProduct = FVector::DotProduct(GroundNormal, UpVector);
        float AngleRadians = FMath::Acos(DotProduct);
        float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
        
        return AngleDegrees;
    }
    
    return 0.0f;
}

bool UPlayerAnimationBlueprint::IsOnRoughTerrain() const
{
    // Simple terrain check based on slope
    return GroundSlope > 15.0f; // Consider terrain rough if slope > 15 degrees
}