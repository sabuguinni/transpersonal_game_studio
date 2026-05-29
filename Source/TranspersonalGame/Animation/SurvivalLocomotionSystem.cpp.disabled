#include "SurvivalLocomotionSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

USurvivalLocomotionSystem::USurvivalLocomotionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize locomotion data
    CurrentLocomotionData = FAnim_LocomotionData();
}

void USurvivalLocomotionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache character and movement component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("SurvivalLocomotionSystem: Initialized for character %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalLocomotionSystem: Owner is not a Character!"));
    }
}

void USurvivalLocomotionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateLocomotionState(DeltaTime);
    }
}

void USurvivalLocomotionSystem::UpdateLocomotionState(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get current velocity and speed
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    
    // Update basic locomotion data
    CurrentLocomotionData.Speed = CurrentSpeed;
    CurrentLocomotionData.bIsMoving = CurrentSpeed > 1.0f;
    CurrentLocomotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentLocomotionData.Direction = CalculateMovementDirection();
    
    // Update survival-based stats
    CurrentLocomotionData.StaminaPercentage = CachedStamina;
    CurrentLocomotionData.HealthPercentage = CachedHealth;
    CurrentLocomotionData.FearLevel = CachedFear;
    
    // Determine locomotion state
    CurrentLocomotionData.LocomotionState = DetermineLocomotionState();
    
    // Determine movement quality
    CurrentLocomotionData.MovementQuality = DetermineMovementQuality();
}

void USurvivalLocomotionSystem::SetMovementQuality(float Health, float Stamina, float Hunger, float Thirst, float Temperature, float Fear)
{
    // Cache survival stats for locomotion calculations
    CachedHealth = FMath::Clamp(Health, 0.0f, 1.0f);
    CachedStamina = FMath::Clamp(Stamina, 0.0f, 1.0f);
    CachedHunger = FMath::Clamp(Hunger, 0.0f, 1.0f);
    CachedThirst = FMath::Clamp(Thirst, 0.0f, 1.0f);
    CachedTemperature = FMath::Clamp(Temperature, 0.0f, 1.0f);
    CachedFear = FMath::Clamp(Fear, 0.0f, 1.0f);
}

void USurvivalLocomotionSystem::ForceLocomotionState(EAnim_SurvivalLocomotionState NewState)
{
    CurrentLocomotionData.LocomotionState = NewState;
    UE_LOG(LogTemp, Log, TEXT("SurvivalLocomotionSystem: Forced locomotion state to %d"), (int32)NewState);
}

float USurvivalLocomotionSystem::CalculateMovementDirection() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }

    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.Size2D() < 1.0f)
    {
        return 0.0f;
    }

    // Get character forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    // Normalize velocity to 2D
    FVector VelocityNormalized = Velocity.GetSafeNormal2D();
    
    // Calculate angle between forward vector and velocity
    float ForwardDot = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
    
    // Convert to angle in degrees (-180 to 180)
    float Angle = FMath::Atan2(RightDot, ForwardDot) * 180.0f / PI;
    
    return Angle;
}

EAnim_SurvivalLocomotionState USurvivalLocomotionSystem::DetermineLocomotionState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_SurvivalLocomotionState::Idle;
    }

    // Check if in air first
    if (CurrentLocomotionData.bIsInAir)
    {
        return EAnim_SurvivalLocomotionState::Idle; // Could add jumping/falling states later
    }

    // Check fear level - high fear affects movement
    if (CachedFear > FearfulThreshold)
    {
        return EAnim_SurvivalLocomotionState::Fearful;
    }

    // Check health for injured state
    if (CachedHealth < WoundedHealthThreshold)
    {
        return EAnim_SurvivalLocomotionState::Injured;
    }

    // Check stamina for exhausted state
    if (CachedStamina < TiredStaminaThreshold && CurrentLocomotionData.bIsMoving)
    {
        return EAnim_SurvivalLocomotionState::Exhausted;
    }

    // Check if crouching (would need to be set externally or via input)
    if (MovementComponent->IsCrouching())
    {
        if (CurrentLocomotionData.Speed > 1.0f)
        {
            return EAnim_SurvivalLocomotionState::Crawling;
        }
        else
        {
            return EAnim_SurvivalLocomotionState::Crouching;
        }
    }

    // Determine movement state based on speed
    if (CurrentLocomotionData.Speed < 1.0f)
    {
        return EAnim_SurvivalLocomotionState::Idle;
    }
    else if (CurrentLocomotionData.Speed < RunThreshold)
    {
        return EAnim_SurvivalLocomotionState::Walking;
    }
    else
    {
        return EAnim_SurvivalLocomotionState::Running;
    }
}

EAnim_MovementQuality USurvivalLocomotionSystem::DetermineMovementQuality() const
{
    // Priority order for movement quality (most severe first)
    
    if (CachedHealth < WoundedHealthThreshold)
    {
        return EAnim_MovementQuality::Wounded;
    }
    
    if (CachedStamina < TiredStaminaThreshold)
    {
        return EAnim_MovementQuality::Tired;
    }
    
    if (CachedThirst < ThirstyThreshold)
    {
        return EAnim_MovementQuality::Thirsty;
    }
    
    if (CachedHunger < HungryThreshold)
    {
        return EAnim_MovementQuality::Hungry;
    }
    
    // Temperature effects
    if (CachedTemperature < 0.2f)
    {
        return EAnim_MovementQuality::Cold;
    }
    else if (CachedTemperature > 0.8f)
    {
        return EAnim_MovementQuality::Hot;
    }
    
    return EAnim_MovementQuality::Normal;
}