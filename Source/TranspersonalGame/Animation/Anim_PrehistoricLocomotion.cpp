#include "Anim_PrehistoricLocomotion.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PrehistoricLocomotion::UAnim_PrehistoricLocomotion()
{
    // Initialize default values
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    CurrentLocomotionState = EAnim_LocomotionState::Idle;
    CurrentSurvivalAction = EAnim_SurvivalAction::None;
    
    Speed = 0.0f;
    Direction = 0.0f;
    VelocityZ = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsMoving = false;
    
    HealthPercent = 100.0f;
    StaminaPercent = 100.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsInjured = false;
    
    // Animation asset references (to be set in Blueprint)
    LocomotionBlendSpace = nullptr;
    IdleAnimation = nullptr;
    JumpMontage = nullptr;
    LandingMontage = nullptr;
    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    InjuredMontage = nullptr;
    
    // Configuration defaults
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    DirectionSmoothSpeed = 10.0f;
    SpeedSmoothSpeed = 5.0f;
    
    // Cached values
    CachedSpeed = 0.0f;
    CachedDirection = 0.0f;
    LastUpdateTime = 0.0f;
}

void UAnim_PrehistoricLocomotion::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ATranspersonalCharacter>(TryGetPawnOwner());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricLocomotion: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricLocomotion: Failed to get TranspersonalCharacter reference"));
    }
}

void UAnim_PrehistoricLocomotion::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation variables
    UpdateMovementVariables();
    UpdateSurvivalVariables();
    UpdateLocomotionState();
    SmoothValues(DeltaTimeX);
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_PrehistoricLocomotion::UpdateMovementVariables()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and movement data
    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector2D Velocity2D = FVector2D(Velocity.X, Velocity.Y);
    
    // Calculate speed
    float CurrentSpeed = Velocity2D.Size();
    Speed = CurrentSpeed;
    
    // Calculate direction relative to character forward
    if (CurrentSpeed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector2D Forward2D = FVector2D(ForwardVector.X, ForwardVector.Y).GetSafeNormal();
        FVector2D Velocity2DNormalized = Velocity2D.GetSafeNormal();
        
        float DotProduct = FVector2D::DotProduct(Forward2D, Velocity2DNormalized);
        float CrossProduct = Forward2D.X * Velocity2DNormalized.Y - Forward2D.Y * Velocity2DNormalized.X;
        
        Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Update movement flags
    bIsMoving = CurrentSpeed > 10.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    VelocityZ = Velocity.Z;
}

void UAnim_PrehistoricLocomotion::UpdateSurvivalVariables()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Get survival stats from character
    // Note: These would be actual properties on TranspersonalCharacter
    HealthPercent = 100.0f; // OwnerCharacter->GetHealthPercent();
    StaminaPercent = 100.0f; // OwnerCharacter->GetStaminaPercent();
    FearLevel = 0.0f; // OwnerCharacter->GetFearLevel();
    
    // Update survival flags
    bIsExhausted = StaminaPercent < 20.0f;
    bIsInjured = HealthPercent < 50.0f;
}

void UAnim_PrehistoricLocomotion::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = CurrentLocomotionState;
    
    if (bIsInAir)
    {
        if (VelocityZ > 100.0f)
        {
            NewState = EAnim_LocomotionState::Jumping;
        }
        else
        {
            NewState = EAnim_LocomotionState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        NewState = EAnim_LocomotionState::Crouching;
    }
    else if (bIsMoving)
    {
        if (Speed >= RunSpeedThreshold)
        {
            NewState = EAnim_LocomotionState::Running;
        }
        else if (Speed >= WalkSpeedThreshold)
        {
            NewState = EAnim_LocomotionState::Walking;
        }
        else
        {
            NewState = EAnim_LocomotionState::Idle;
        }
    }
    else
    {
        NewState = EAnim_LocomotionState::Idle;
    }
    
    // Handle state transitions
    if (NewState != CurrentLocomotionState)
    {
        EAnim_LocomotionState PreviousState = CurrentLocomotionState;
        CurrentLocomotionState = NewState;
        
        // Handle landing transition
        if (PreviousState == EAnim_LocomotionState::Falling && 
            NewState != EAnim_LocomotionState::Falling && 
            NewState != EAnim_LocomotionState::Jumping)
        {
            if (LandingMontage && !IsAnyMontagePlaying())
            {
                Montage_Play(LandingMontage, 1.0f);
            }
        }
    }
}

void UAnim_PrehistoricLocomotion::SmoothValues(float DeltaTime)
{
    // Smooth speed and direction for fluid animation
    CachedSpeed = FMath::FInterpTo(CachedSpeed, Speed, DeltaTime, SpeedSmoothSpeed);
    CachedDirection = FMath::FInterpTo(CachedDirection, Direction, DeltaTime, DirectionSmoothSpeed);
}

void UAnim_PrehistoricLocomotion::SetSurvivalAction(EAnim_SurvivalAction NewAction)
{
    if (CurrentSurvivalAction != NewAction)
    {
        CurrentSurvivalAction = NewAction;
        
        // Play appropriate montage for the action
        PlaySurvivalMontage(NewAction);
    }
}

void UAnim_PrehistoricLocomotion::PlaySurvivalMontage(EAnim_SurvivalAction Action)
{
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (Action)
    {
        case EAnim_SurvivalAction::Gathering:
            MontageToPlay = GatheringMontage;
            break;
        case EAnim_SurvivalAction::Crafting:
            MontageToPlay = CraftingMontage;
            break;
        case EAnim_SurvivalAction::Injured:
            MontageToPlay = InjuredMontage;
            break;
        default:
            // No montage for other actions
            break;
    }
    
    if (MontageToPlay && !IsAnyMontagePlaying())
    {
        Montage_Play(MontageToPlay, 1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricLocomotion: Playing montage for action %d"), 
               static_cast<int32>(Action));
    }
}

void UAnim_PrehistoricLocomotion::UpdateSurvivalStats(float Health, float Stamina, float Fear)
{
    HealthPercent = FMath::Clamp(Health, 0.0f, 100.0f);
    StaminaPercent = FMath::Clamp(Stamina, 0.0f, 100.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 100.0f);
    
    // Update derived flags
    bIsExhausted = StaminaPercent < 20.0f;
    bIsInjured = HealthPercent < 50.0f;
}

bool UAnim_PrehistoricLocomotion::ShouldPlayInjuredAnimation() const
{
    return bIsInjured && CurrentSurvivalAction == EAnim_SurvivalAction::Injured;
}

bool UAnim_PrehistoricLocomotion::ShouldPlayExhaustedAnimation() const
{
    return bIsExhausted && !bIsMoving;
}