#include "Core_SurvivalPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_SurvivalPhysics::UCore_SurvivalPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize survival stats
    SurvivalStats = FCore_SurvivalStats();
    CurrentState = ECore_SurvivalState::Healthy;
    
    // Physics multipliers
    MovementSpeedMultiplier = 1.0f;
    JumpHeightMultiplier = 1.0f;
    
    // Decay rates
    HungerDecayRate = 0.5f;
    ThirstDecayRate = 0.8f;
    StaminaRecoveryRate = 2.0f;
    FearDecayRate = 1.0f;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UCore_SurvivalPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Get references to owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
    
    // Initialize survival state
    CalculateSurvivalState();
    ApplySurvivalPhysicsEffects();
}

void UCore_SurvivalPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateSurvivalStats(DeltaTime);
    CalculateSurvivalState();
    ApplySurvivalPhysicsEffects();
}

void UCore_SurvivalPhysics::UpdateSurvivalStats(float DeltaTime)
{
    // Hunger decay
    SurvivalStats.Hunger = FMath::Max(0.0f, SurvivalStats.Hunger - (HungerDecayRate * DeltaTime));
    
    // Thirst decay (faster than hunger)
    SurvivalStats.Thirst = FMath::Max(0.0f, SurvivalStats.Thirst - (ThirstDecayRate * DeltaTime));
    
    // Stamina recovery (when not exhausted)
    if (SurvivalStats.Stamina < 100.0f && CurrentState != ECore_SurvivalState::Exhausted)
    {
        SurvivalStats.Stamina = FMath::Min(100.0f, SurvivalStats.Stamina + (StaminaRecoveryRate * DeltaTime));
    }
    
    // Fear decay over time
    if (SurvivalStats.Fear > 0.0f)
    {
        SurvivalStats.Fear = FMath::Max(0.0f, SurvivalStats.Fear - (FearDecayRate * DeltaTime));
    }
    
    // Health degradation from starvation/dehydration
    if (SurvivalStats.Hunger <= 0.0f || SurvivalStats.Thirst <= 0.0f)
    {
        float HealthLoss = 5.0f * DeltaTime; // Lose 5 health per second when starving/dehydrated
        SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - HealthLoss);
    }
    
    // Temperature regulation (simplified)
    float TargetTemp = 37.0f; // Normal body temperature
    float TempDiff = TargetTemp - SurvivalStats.Temperature;
    SurvivalStats.Temperature += TempDiff * 0.1f * DeltaTime; // Gradual temperature regulation
}

void UCore_SurvivalPhysics::ApplyDamage(float DamageAmount)
{
    SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - DamageAmount);
    
    // Increase fear when taking damage
    SurvivalStats.Fear = FMath::Min(100.0f, SurvivalStats.Fear + (DamageAmount * 0.5f));
    
    CalculateSurvivalState();
}

void UCore_SurvivalPhysics::RestoreHealth(float HealAmount)
{
    SurvivalStats.Health = FMath::Min(100.0f, SurvivalStats.Health + HealAmount);
    CalculateSurvivalState();
}

void UCore_SurvivalPhysics::ConsumeFood(float FoodValue)
{
    SurvivalStats.Hunger = FMath::Min(100.0f, SurvivalStats.Hunger + FoodValue);
    CalculateSurvivalState();
}

void UCore_SurvivalPhysics::DrinkWater(float WaterValue)
{
    SurvivalStats.Thirst = FMath::Min(100.0f, SurvivalStats.Thirst + WaterValue);
    CalculateSurvivalState();
}

void UCore_SurvivalPhysics::RestoreStamina(float StaminaAmount)
{
    SurvivalStats.Stamina = FMath::Min(100.0f, SurvivalStats.Stamina + StaminaAmount);
    CalculateSurvivalState();
}

void UCore_SurvivalPhysics::SetFearLevel(float NewFearLevel)
{
    SurvivalStats.Fear = FMath::Clamp(NewFearLevel, 0.0f, 100.0f);
    CalculateSurvivalState();
}

ECore_SurvivalState UCore_SurvivalPhysics::GetSurvivalState() const
{
    return CurrentState;
}

void UCore_SurvivalPhysics::ApplySurvivalPhysicsEffects()
{
    if (!MovementComponent)
    {
        return;
    }
    
    UpdatePhysicsMultipliers();
    
    // Apply movement speed modifications
    float BaseWalkSpeed = 600.0f; // Default UE5 walk speed
    MovementComponent->MaxWalkSpeed = BaseWalkSpeed * MovementSpeedMultiplier;
    
    // Apply jump height modifications
    float BaseJumpVelocity = 420.0f; // Default UE5 jump velocity
    MovementComponent->JumpZVelocity = BaseJumpVelocity * JumpHeightMultiplier;
    
    // Handle critical state
    if (CurrentState == ECore_SurvivalState::Critical)
    {
        HandleCriticalState();
    }
}

float UCore_SurvivalPhysics::GetCurrentMovementSpeed() const
{
    return MovementComponent ? MovementComponent->MaxWalkSpeed : 0.0f;
}

float UCore_SurvivalPhysics::GetCurrentJumpHeight() const
{
    return MovementComponent ? MovementComponent->JumpZVelocity : 0.0f;
}

void UCore_SurvivalPhysics::CalculateSurvivalState()
{
    // Determine survival state based on stats
    if (SurvivalStats.Health <= 10.0f)
    {
        CurrentState = ECore_SurvivalState::Critical;
    }
    else if (SurvivalStats.Health <= 30.0f || SurvivalStats.Hunger <= 10.0f || SurvivalStats.Thirst <= 10.0f)
    {
        CurrentState = ECore_SurvivalState::Injured;
    }
    else if (SurvivalStats.Stamina <= 10.0f)
    {
        CurrentState = ECore_SurvivalState::Exhausted;
    }
    else if (SurvivalStats.Fear >= 70.0f)
    {
        CurrentState = ECore_SurvivalState::Critical; // High fear is critical
    }
    else if (SurvivalStats.Hunger <= 30.0f)
    {
        CurrentState = ECore_SurvivalState::Hungry;
    }
    else if (SurvivalStats.Thirst <= 30.0f)
    {
        CurrentState = ECore_SurvivalState::Thirsty;
    }
    else if (SurvivalStats.Stamina <= 30.0f)
    {
        CurrentState = ECore_SurvivalState::Tired;
    }
    else
    {
        CurrentState = ECore_SurvivalState::Healthy;
    }
}

void UCore_SurvivalPhysics::UpdatePhysicsMultipliers()
{
    // Base multipliers
    MovementSpeedMultiplier = 1.0f;
    JumpHeightMultiplier = 1.0f;
    
    // Apply state-based modifications
    switch (CurrentState)
    {
        case ECore_SurvivalState::Healthy:
            MovementSpeedMultiplier = 1.0f;
            JumpHeightMultiplier = 1.0f;
            break;
            
        case ECore_SurvivalState::Tired:
            MovementSpeedMultiplier = 0.9f;
            JumpHeightMultiplier = 0.9f;
            break;
            
        case ECore_SurvivalState::Hungry:
            MovementSpeedMultiplier = 0.85f;
            JumpHeightMultiplier = 0.85f;
            break;
            
        case ECore_SurvivalState::Thirsty:
            MovementSpeedMultiplier = 0.8f;
            JumpHeightMultiplier = 0.8f;
            break;
            
        case ECore_SurvivalState::Injured:
            MovementSpeedMultiplier = 0.7f;
            JumpHeightMultiplier = 0.6f;
            break;
            
        case ECore_SurvivalState::Exhausted:
            MovementSpeedMultiplier = 0.5f;
            JumpHeightMultiplier = 0.3f;
            break;
            
        case ECore_SurvivalState::Critical:
            MovementSpeedMultiplier = 0.3f;
            JumpHeightMultiplier = 0.1f;
            break;
    }
    
    // Additional fear-based modifications
    if (SurvivalStats.Fear > 50.0f)
    {
        // High fear increases speed (panic) but reduces jump accuracy
        MovementSpeedMultiplier *= 1.2f;
        JumpHeightMultiplier *= 0.8f;
    }
}

void UCore_SurvivalPhysics::HandleCriticalState()
{
    // In critical state, apply additional effects
    if (SurvivalStats.Health <= 0.0f)
    {
        // Character death logic would go here
        UE_LOG(LogTemp, Warning, TEXT("Character has died - Health: %f"), SurvivalStats.Health);
    }
    
    // Screen effects, sound cues, etc. would be triggered here
    // This is where we'd integrate with other systems like audio/VFX
}