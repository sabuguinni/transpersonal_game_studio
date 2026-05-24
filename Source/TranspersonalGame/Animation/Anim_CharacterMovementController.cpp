#include "Anim_CharacterMovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterMovementController::UAnim_CharacterMovementController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    LastSpeed = 0.0f;
    SpeedChangeRate = 5.0f;
    DirectionChangeRate = 10.0f;
    
    // Initialize montages to null - will be set in Blueprint
    JumpMontage = nullptr;
    LandMontage = nullptr;
    FearReactionMontage = nullptr;
    InjuryMontage = nullptr;
    LocomotionBlendSpace = nullptr;
    CrouchBlendSpace = nullptr;
}

void UAnim_CharacterMovementController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache references to owner and components
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
    
    // Initialize movement data
    CurrentMovementData = FAnim_MovementData();
}

void UAnim_CharacterMovementController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementState();
        UpdateSurvivalState();
    }
}

void UAnim_CharacterMovementController::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMovementValues();
    
    // Update movement state
    CurrentMovementData.MovementState = DetermineMovementState();
    
    // Update boolean flags
    CurrentMovementData.bIsMoving = CurrentMovementData.Speed > 0.1f;
    CurrentMovementData.bIsInAir = MovementComponent->IsFalling();
    CurrentMovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_CharacterMovementController::UpdateSurvivalState()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UpdateSurvivalValues();
    
    // Update survival state
    CurrentMovementData.SurvivalState = DetermineSurvivalState();
}

void UAnim_CharacterMovementController::UpdateMovementValues()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    
    // Smooth speed changes
    CurrentMovementData.Speed = FMath::FInterpTo(CurrentMovementData.Speed, CurrentSpeed, 
        GetWorld()->GetDeltaSeconds(), SpeedChangeRate);
    
    // Calculate direction relative to actor forward
    if (CurrentSpeed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        float TargetDirection = FMath::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
        
        CurrentMovementData.Direction = FMath::FInterpAngle(CurrentMovementData.Direction, TargetDirection,
            GetWorld()->GetDeltaSeconds(), DirectionChangeRate);
    }
    
    LastSpeed = CurrentSpeed;
}

void UAnim_CharacterMovementController::UpdateSurvivalValues()
{
    // This would typically read from a survival component
    // For now, using placeholder values that can be overridden in Blueprint
    
    // These values should come from the character's survival stats
    // CurrentMovementData.HealthPercentage = SurvivalComponent->GetHealthPercentage();
    // CurrentMovementData.StaminaPercentage = SurvivalComponent->GetStaminaPercentage();
    // CurrentMovementData.FearLevel = SurvivalComponent->GetFearLevel();
    
    // Placeholder implementation - maintain current values if not set elsewhere
    if (CurrentMovementData.HealthPercentage <= 0.0f)
    {
        CurrentMovementData.HealthPercentage = 1.0f;
    }
    
    if (CurrentMovementData.StaminaPercentage <= 0.0f)
    {
        CurrentMovementData.StaminaPercentage = 1.0f;
    }
}

EAnim_MovementState UAnim_CharacterMovementController::DetermineMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (MovementComponent->IsFalling())
    {
        return EAnim_MovementState::Falling;
    }
    
    if (MovementComponent->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    float Speed = CurrentMovementData.Speed;
    
    if (Speed < 0.1f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < MovementComponent->MaxWalkSpeed * 0.6f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

EAnim_SurvivalState UAnim_CharacterMovementController::DetermineSurvivalState()
{
    // Priority order: Injured > Exhausted > Fearful > Hungry/Thirsty > Normal
    
    if (CurrentMovementData.HealthPercentage < InjuredThreshold)
    {
        return EAnim_SurvivalState::Injured;
    }
    
    if (CurrentMovementData.StaminaPercentage < ExhaustedThreshold)
    {
        return EAnim_SurvivalState::Exhausted;
    }
    
    if (CurrentMovementData.FearLevel > FearThreshold)
    {
        return EAnim_SurvivalState::Fearful;
    }
    
    // Additional survival states can be added here
    // if (HungerLevel > HungryThreshold) return EAnim_SurvivalState::Hungry;
    // if (ThirstLevel > ThirstyThreshold) return EAnim_SurvivalState::Thirsty;
    
    return EAnim_SurvivalState::Normal;
}

void UAnim_CharacterMovementController::TriggerJumpAnimation()
{
    if (AnimInstance && JumpMontage)
    {
        AnimInstance->Montage_Play(JumpMontage);
        UE_LOG(LogTemp, Log, TEXT("Animation: Jump montage triggered"));
    }
}

void UAnim_CharacterMovementController::TriggerLandAnimation()
{
    if (AnimInstance && LandMontage)
    {
        AnimInstance->Montage_Play(LandMontage);
        UE_LOG(LogTemp, Log, TEXT("Animation: Land montage triggered"));
    }
}

void UAnim_CharacterMovementController::TriggerFearReaction(float FearIntensity)
{
    if (AnimInstance && FearReactionMontage)
    {
        // Set fear level for animation state
        CurrentMovementData.FearLevel = FMath::Clamp(FearIntensity, 0.0f, 1.0f);
        
        // Play fear reaction montage
        AnimInstance->Montage_Play(FearReactionMontage);
        UE_LOG(LogTemp, Log, TEXT("Animation: Fear reaction triggered with intensity %f"), FearIntensity);
    }
}

void UAnim_CharacterMovementController::TriggerInjuryAnimation()
{
    if (AnimInstance && InjuryMontage)
    {
        AnimInstance->Montage_Play(InjuryMontage);
        UE_LOG(LogTemp, Log, TEXT("Animation: Injury montage triggered"));
    }
}