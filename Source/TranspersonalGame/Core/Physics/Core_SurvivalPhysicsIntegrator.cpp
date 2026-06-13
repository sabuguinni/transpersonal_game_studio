#include "Core_SurvivalPhysicsIntegrator.h"
#include "Core/GameFramework/TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_SurvivalPhysicsIntegrator::UCore_SurvivalPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    UpdateFrequency = 0.1f;
    bEnableDebugOutput = false;
    
    // Set default thresholds
    LowStaminaThreshold = 0.3f;
    LowHealthThreshold = 0.25f;
    HighFearThreshold = 0.7f;
    CriticalHungerThreshold = 0.2f;
    CriticalThirstThreshold = 0.15f;
    
    // Set default multiplier ranges
    MovementSpeedRange = FVector2D(0.4f, 1.2f);
    JumpForceRange = FVector2D(0.5f, 1.1f);
    CollisionDamageRange = FVector2D(0.8f, 2.0f);
    RagdollThresholdRange = FVector2D(0.3f, 1.5f);
    
    LastUpdateTime = 0.0f;
}

void UCore_SurvivalPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponent();
    
    // Set up tick interval
    SetComponentTickInterval(UpdateFrequency);
}

void UCore_SurvivalPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check if enough time has passed for update
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        UpdatePhysicsFromSurvival();
        LastUpdateTime = CurrentTime;
    }
}

void UCore_SurvivalPhysicsIntegrator::InitializeComponent()
{
    CacheComponentReferences();
    ResetPhysicsState();
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegrator: Component initialized for %s"), 
               OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
    }
}

void UCore_SurvivalPhysicsIntegrator::CacheComponentReferences()
{
    OwnerCharacter = Cast<ATranspersonalCharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        // Get survival system from GameInstance
        if (UGameInstance* GameInstance = OwnerCharacter->GetGameInstance())
        {
            SurvivalSystem = GameInstance->GetSubsystem<UEng_SurvivalSystemManager>();
        }
    }
    
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalPhysicsIntegrator: Failed to get TranspersonalCharacter reference"));
    }
    
    if (!SurvivalSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalPhysicsIntegrator: Failed to get SurvivalSystemManager reference"));
    }
}

void UCore_SurvivalPhysicsIntegrator::UpdatePhysicsFromSurvival()
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return;
    }
    
    // Calculate new physics state based on survival stats
    CurrentPhysicsState.MovementSpeedMultiplier = CalculateMovementSpeedMultiplier();
    CurrentPhysicsState.JumpForceMultiplier = CalculateJumpForceMultiplier();
    CurrentPhysicsState.AccelerationMultiplier = CurrentPhysicsState.MovementSpeedMultiplier * 0.8f;
    CurrentPhysicsState.CollisionDamageMultiplier = CalculateCollisionDamageMultiplier();
    CurrentPhysicsState.RagdollThresholdMultiplier = CalculateRagdollThresholdMultiplier();
    CurrentPhysicsState.FearTremorIntensity = CalculateFearTremorIntensity();
    
    // Apply fear-based effects
    CurrentPhysicsState.bFearInducedClumsiness = (GetSurvivalStatNormalized(TEXT("Fear")) > HighFearThreshold);
    
    // Environmental physics modifications
    float HealthRatio = GetSurvivalStatNormalized(TEXT("Health"));
    float StaminaRatio = GetSurvivalStatNormalized(TEXT("Stamina"));
    
    CurrentPhysicsState.GroundFrictionMultiplier = FMath::Lerp(0.7f, 1.1f, StaminaRatio);
    CurrentPhysicsState.AirResistanceMultiplier = FMath::Lerp(0.9f, 1.2f, HealthRatio);
    CurrentPhysicsState.GravityScaleMultiplier = FMath::Lerp(1.1f, 0.95f, StaminaRatio);
    
    // Apply the calculated state to the character
    ApplyPhysicsStateToCharacter();
    
    if (bEnableDebugOutput)
    {
        LogCurrentPhysicsState();
    }
}

float UCore_SurvivalPhysicsIntegrator::CalculateMovementSpeedMultiplier() const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return 1.0f;
    }
    
    float StaminaRatio = GetSurvivalStatNormalized(TEXT("Stamina"));
    float HungerRatio = GetSurvivalStatNormalized(TEXT("Hunger"));
    float ThirstRatio = GetSurvivalStatNormalized(TEXT("Thirst"));
    float HealthRatio = GetSurvivalStatNormalized(TEXT("Health"));
    
    // Combine survival stats with different weights
    float CombinedRatio = (StaminaRatio * 0.4f) + (HungerRatio * 0.25f) + (ThirstRatio * 0.2f) + (HealthRatio * 0.15f);
    
    return InterpolateMultiplier(CombinedRatio, MovementSpeedRange);
}

float UCore_SurvivalPhysicsIntegrator::CalculateJumpForceMultiplier() const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return 1.0f;
    }
    
    float StaminaRatio = GetSurvivalStatNormalized(TEXT("Stamina"));
    float HealthRatio = GetSurvivalStatNormalized(TEXT("Health"));
    
    // Jump is primarily affected by stamina and health
    float CombinedRatio = (StaminaRatio * 0.7f) + (HealthRatio * 0.3f);
    
    return InterpolateMultiplier(CombinedRatio, JumpForceRange);
}

float UCore_SurvivalPhysicsIntegrator::CalculateCollisionDamageMultiplier() const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return 1.0f;
    }
    
    float HealthRatio = GetSurvivalStatNormalized(TEXT("Health"));
    float FearRatio = GetSurvivalStatNormalized(TEXT("Fear"));
    
    // Lower health = more damage from collisions
    // Higher fear = more damage from panic
    float VulnerabilityRatio = 1.0f - ((HealthRatio * 0.6f) + ((1.0f - FearRatio) * 0.4f));
    
    return InterpolateMultiplier(VulnerabilityRatio, CollisionDamageRange);
}

float UCore_SurvivalPhysicsIntegrator::CalculateRagdollThresholdMultiplier() const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return 1.0f;
    }
    
    float HealthRatio = GetSurvivalStatNormalized(TEXT("Health"));
    float StaminaRatio = GetSurvivalStatNormalized(TEXT("Stamina"));
    
    // Lower health/stamina = easier to ragdoll
    float ResistanceRatio = (HealthRatio * 0.6f) + (StaminaRatio * 0.4f);
    
    return InterpolateMultiplier(ResistanceRatio, RagdollThresholdRange);
}

float UCore_SurvivalPhysicsIntegrator::CalculateFearTremorIntensity() const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return 0.0f;
    }
    
    float FearRatio = GetSurvivalStatNormalized(TEXT("Fear"));
    
    // Tremor starts at 50% fear and increases exponentially
    if (FearRatio < 0.5f)
    {
        return 0.0f;
    }
    
    float TremorRange = FearRatio - 0.5f; // 0.0 to 0.5
    return FMath::Pow(TremorRange * 2.0f, 2.0f) * 10.0f; // 0 to 10 intensity
}

float UCore_SurvivalPhysicsIntegrator::GetSurvivalStatNormalized(const FString& StatName) const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return 1.0f;
    }
    
    // Get normalized survival stats from the character
    if (StatName == TEXT("Health"))
    {
        return OwnerCharacter->GetHealthPercentage();
    }
    else if (StatName == TEXT("Stamina"))
    {
        return OwnerCharacter->GetStaminaPercentage();
    }
    else if (StatName == TEXT("Hunger"))
    {
        return OwnerCharacter->GetHungerPercentage();
    }
    else if (StatName == TEXT("Thirst"))
    {
        return OwnerCharacter->GetThirstPercentage();
    }
    else if (StatName == TEXT("Fear"))
    {
        return OwnerCharacter->GetFearPercentage();
    }
    
    return 1.0f;
}

float UCore_SurvivalPhysicsIntegrator::InterpolateMultiplier(float NormalizedValue, const FVector2D& Range) const
{
    return FMath::Lerp(Range.X, Range.Y, FMath::Clamp(NormalizedValue, 0.0f, 1.0f));
}

void UCore_SurvivalPhysicsIntegrator::ApplyPhysicsStateToCharacter()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    ApplyMovementModifications();
    ApplyCollisionModifications();
    ApplyEnvironmentalModifications();
    ApplyFearEffects();
}

void UCore_SurvivalPhysicsIntegrator::ApplyMovementModifications()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Apply movement speed modifications
    float BaseMaxWalkSpeed = 600.0f; // Default UE5 walk speed
    MovementComp->MaxWalkSpeed = BaseMaxWalkSpeed * CurrentPhysicsState.MovementSpeedMultiplier;
    
    // Apply jump force modifications
    float BaseJumpZVelocity = 420.0f; // Default UE5 jump velocity
    MovementComp->JumpZVelocity = BaseJumpZVelocity * CurrentPhysicsState.JumpForceMultiplier;
    
    // Apply acceleration modifications
    float BaseMaxAcceleration = 2048.0f; // Default UE5 acceleration
    MovementComp->MaxAcceleration = BaseMaxAcceleration * CurrentPhysicsState.AccelerationMultiplier;
    
    // Apply ground friction modifications
    MovementComp->GroundFriction = 8.0f * CurrentPhysicsState.GroundFrictionMultiplier;
}

void UCore_SurvivalPhysicsIntegrator::ApplyCollisionModifications()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        return;
    }
    
    // Modify collision response based on physics state
    // This would typically involve custom collision handling
    // For now, we store the multiplier for use in damage calculations
}

void UCore_SurvivalPhysicsIntegrator::ApplyEnvironmentalModifications()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Apply gravity scale modifications
    MovementComp->GravityScale = CurrentPhysicsState.GravityScaleMultiplier;
}

void UCore_SurvivalPhysicsIntegrator::ApplyFearEffects()
{
    if (!OwnerCharacter || CurrentPhysicsState.FearTremorIntensity <= 0.0f)
    {
        return;
    }
    
    // Apply camera shake or movement tremor based on fear
    // This would typically involve adding random movement offsets
    // For now, we just log the effect
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegrator: Applying fear tremor intensity: %f"), 
               CurrentPhysicsState.FearTremorIntensity);
    }
}

bool UCore_SurvivalPhysicsIntegrator::IsInCriticalSurvivalState() const
{
    if (!OwnerCharacter || !SurvivalSystem)
    {
        return false;
    }
    
    float HealthRatio = GetSurvivalStatNormalized(TEXT("Health"));
    float HungerRatio = GetSurvivalStatNormalized(TEXT("Hunger"));
    float ThirstRatio = GetSurvivalStatNormalized(TEXT("Thirst"));
    
    return (HealthRatio < LowHealthThreshold) || 
           (HungerRatio < CriticalHungerThreshold) || 
           (ThirstRatio < CriticalThirstThreshold);
}

bool UCore_SurvivalPhysicsIntegrator::IsPhysicsCompromised() const
{
    return (CurrentPhysicsState.MovementSpeedMultiplier < 0.6f) ||
           (CurrentPhysicsState.JumpForceMultiplier < 0.7f) ||
           (CurrentPhysicsState.FearTremorIntensity > 5.0f);
}

void UCore_SurvivalPhysicsIntegrator::ResetPhysicsState()
{
    CurrentPhysicsState = FCore_SurvivalPhysicsState();
    
    if (OwnerCharacter)
    {
        ApplyPhysicsStateToCharacter();
    }
}

void UCore_SurvivalPhysicsIntegrator::LogCurrentPhysicsState() const
{
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegrator State:"));
    UE_LOG(LogTemp, Log, TEXT("  Movement Speed: %f"), CurrentPhysicsState.MovementSpeedMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  Jump Force: %f"), CurrentPhysicsState.JumpForceMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  Collision Damage: %f"), CurrentPhysicsState.CollisionDamageMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  Ragdoll Threshold: %f"), CurrentPhysicsState.RagdollThresholdMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  Fear Tremor: %f"), CurrentPhysicsState.FearTremorIntensity);
    UE_LOG(LogTemp, Log, TEXT("  Critical State: %s"), IsInCriticalSurvivalState() ? TEXT("Yes") : TEXT("No"));
}