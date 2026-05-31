#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Initialize animation state variables
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsRunning = false;
    bIsCrouching = false;
    bIsFalling = false;
    bIsJumping = false;
    
    // Initialize survival state variables
    HealthPercent = 100.0f;
    StaminaPercent = 100.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsInjured = false;
    
    // Initialize references
    Character = nullptr;
    CharacterMovement = nullptr;
    
    // Set animation thresholds for realistic primitive human movement
    WalkSpeedThreshold = 150.0f;     // Slow walking pace
    RunSpeedThreshold = 400.0f;      // Running pace
    JumpVelocityThreshold = 200.0f;  // Jump detection threshold
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    Character = Cast<ACharacter>(GetOwningActor());
    if (Character)
    {
        CharacterMovement = Character->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), *Character->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Instance: Failed to get character reference"));
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!Character || !CharacterMovement)
    {
        return;
    }
    
    // Update all animation values
    UpdateMovementValues();
    UpdateSurvivalValues();
    UpdateCharacterState();
}

void UAnim_CharacterAnimInstance::UpdateMovementValues()
{
    if (!Character || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to character rotation
    if (Speed > 0.1f)
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector RightVector = Character->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardAmount = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightAmount = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = UKismetMathLibrary::Atan2(RightAmount, ForwardAmount) * (180.0f / PI);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Update movement state booleans
    bIsInAir = CharacterMovement->IsFalling();
    bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
    bIsRunning = Speed > RunSpeedThreshold;
    bIsCrouching = CharacterMovement->IsCrouching();
    bIsFalling = CharacterMovement->IsFalling() && Velocity.Z < -JumpVelocityThreshold;
    bIsJumping = CharacterMovement->IsFalling() && Velocity.Z > JumpVelocityThreshold;
    
    // Check for exhaustion based on speed and stamina
    bIsExhausted = StaminaPercent < 20.0f && Speed > WalkSpeedThreshold;
}

void UAnim_CharacterAnimInstance::UpdateSurvivalValues()
{
    if (!Character)
    {
        return;
    }
    
    // Try to get survival stats from character
    // This would connect to the TranspersonalCharacter survival system
    // For now, simulate realistic survival state changes
    
    // Simulate health degradation when injured
    if (bIsInjured && HealthPercent > 0.0f)
    {
        HealthPercent = FMath::Max(0.0f, HealthPercent - 0.1f);
    }
    
    // Simulate stamina consumption during movement
    if (bIsRunning && StaminaPercent > 0.0f)
    {
        StaminaPercent = FMath::Max(0.0f, StaminaPercent - 2.0f);
    }
    else if (!bIsRunning && StaminaPercent < 100.0f)
    {
        StaminaPercent = FMath::Min(100.0f, StaminaPercent + 0.5f);
    }
    
    // Simulate fear level based on environment
    // This would connect to dinosaur proximity and threat detection
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - 1.0f);
    }
    
    // Update injury state based on health
    bIsInjured = HealthPercent < 50.0f;
    
    // Update exhaustion state
    bIsExhausted = StaminaPercent < 20.0f;
}

void UAnim_CharacterAnimInstance::UpdateCharacterState()
{
    if (!Character)
    {
        return;
    }
    
    // Log state changes for debugging
    static bool bWasRunning = false;
    static bool bWasJumping = false;
    static bool bWasInAir = false;
    
    // Detect state transitions and trigger events
    if (bIsRunning && !bWasRunning)
    {
        OnStartRunning();
        UE_LOG(LogTemp, Log, TEXT("Character started running"));
    }
    else if (!bIsRunning && bWasRunning)
    {
        OnStopRunning();
        UE_LOG(LogTemp, Log, TEXT("Character stopped running"));
    }
    
    if (bIsJumping && !bWasJumping)
    {
        OnJumped();
        UE_LOG(LogTemp, Log, TEXT("Character jumped"));
    }
    
    if (!bIsInAir && bWasInAir)
    {
        OnLanded();
        UE_LOG(LogTemp, Log, TEXT("Character landed"));
    }
    
    // Check for survival events
    if (HealthPercent < 25.0f && HealthPercent > 0.0f)
    {
        OnHealthCritical();
    }
    
    if (bIsExhausted)
    {
        OnStaminaExhausted();
    }
    
    if (FearLevel > 75.0f)
    {
        OnFearReaction();
    }
    
    // Update previous state
    bWasRunning = bIsRunning;
    bWasJumping = bIsJumping;
    bWasInAir = bIsInAir;
}