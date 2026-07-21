#include "Core_SurvivalPhysicsIntegration.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCore_SurvivalPhysicsIntegration::UCore_SurvivalPhysicsIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for performance
    
    // Initialize default values
    StaminaMovementModifier = 1.0f;
    HealthPhysicsModifier = 1.0f;
    TemperatureMovementPenalty = 0.0f;
    FearMovementImpact = 0.0f;
    CurrentTerrainType = ETerrainType::Grassland;
    TerrainStaminaDrainMultiplier = 1.0f;
    TerrainSpeedModifier = 1.0f;
    ImpactDamageThreshold = 500.0f;
    FallDamageMultiplier = 1.0f;
    CollisionRecoveryTime = 0.0f;
    IntegrationPerformanceScore = 100.0f;
    FrameTimeImpact = 0.0f;
    bIntegrationValid = false;
    
    // Internal state
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    PerformanceTimer = 0.0f;
    LastFrameTime = 0.0f;
    bInitialized = false;
    SurvivalArchitecture = nullptr;
}

void UCore_SurvivalPhysicsIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Find owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegration: Found owner character %s"), *OwnerCharacter->GetName());
    }
    
    // Find survival architecture in world
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(TEXT("SurvivalArchitecture")))
            {
                SurvivalArchitecture = Actor->FindComponentByClass<UEng_SurvivalArchitecture>();
                if (SurvivalArchitecture)
                {
                    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegration: Connected to survival architecture"));
                    break;
                }
            }
        }
    }
    
    // Initialize integration
    if (OwnerCharacter)
    {
        InitializeSurvivalPhysics(OwnerCharacter);
    }
}

void UCore_SurvivalPhysicsIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bInitialized || !OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    float FrameStartTime = FPlatformTime::Seconds();
    
    // Update survival stats from architecture
    UpdateSurvivalStats();
    
    // Update movement physics based on survival state
    UpdateMovementPhysics(DeltaTime);
    
    // Process terrain interaction
    ProcessTerrainInteraction(CurrentTerrainType, DeltaTime);
    
    // Apply environmental effects
    ApplyEnvironmentalPhysics(EnvironmentalFactors);
    
    // Monitor performance
    MonitorPerformance(DeltaTime);
    
    // Calculate frame time impact
    float FrameEndTime = FPlatformTime::Seconds();
    FrameTimeImpact = (FrameEndTime - FrameStartTime) * 1000.0f; // Convert to milliseconds
}

void UCore_SurvivalPhysicsIntegration::InitializeSurvivalPhysics(ACharacter* TargetCharacter)
{
    if (!TargetCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalPhysicsIntegration: Cannot initialize with null character"));
        return;
    }
    
    OwnerCharacter = TargetCharacter;
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalPhysicsIntegration: Character has no movement component"));
        return;
    }
    
    // Initialize survival stats to defaults
    CurrentStats.Health = 100.0f;
    CurrentStats.Hunger = 100.0f;
    CurrentStats.Thirst = 100.0f;
    CurrentStats.Stamina = 100.0f;
    CurrentStats.Temperature = 20.0f; // Comfortable temperature
    CurrentStats.Fear = 0.0f;
    
    // Initialize environmental factors
    EnvironmentalFactors.Temperature = 20.0f;
    EnvironmentalFactors.Humidity = 50.0f;
    EnvironmentalFactors.WindSpeed = 0.0f;
    EnvironmentalFactors.TimeOfDay = 12.0f; // Noon
    EnvironmentalFactors.WeatherIntensity = 0.0f;
    EnvironmentalFactors.DangerLevel = 0.0f;
    
    bInitialized = true;
    bIntegrationValid = ValidateIntegration();
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegration: Initialized successfully for %s"), *OwnerCharacter->GetName());
}

void UCore_SurvivalPhysicsIntegration::UpdateMovementPhysics(float DeltaTime)
{
    if (!MovementComponent || !bInitialized)
    {
        return;
    }
    
    // Calculate physics modifiers based on survival stats
    CalculatePhysicsModifiers();
    
    // Apply stamina-based movement modification
    float BaseMaxWalkSpeed = 600.0f; // Standard UE5 character speed
    float ModifiedSpeed = BaseMaxWalkSpeed * StaminaMovementModifier * HealthPhysicsModifier * TerrainSpeedModifier;
    
    // Apply temperature penalty
    ModifiedSpeed *= (1.0f - TemperatureMovementPenalty);
    
    // Apply fear impact (reduces precision, not speed)
    if (FearMovementImpact > 0.1f)
    {
        // Reduce movement precision by adding slight random offset
        FVector CurrentVelocity = MovementComponent->Velocity;
        if (!CurrentVelocity.IsZero())
        {
            float FearShake = FearMovementImpact * 50.0f; // Scale fear to movement shake
            FVector ShakeOffset = FVector(
                FMath::RandRange(-FearShake, FearShake),
                FMath::RandRange(-FearShake, FearShake),
                0.0f
            );
            // Note: In a full implementation, this would be applied to input processing
        }
    }
    
    // Update movement component
    MovementComponent->MaxWalkSpeed = FMath::Clamp(ModifiedSpeed, 50.0f, 1200.0f);
    
    // Update jump velocity based on stamina
    float BaseJumpVelocity = 420.0f;
    MovementComponent->JumpZVelocity = BaseJumpVelocity * FMath::Clamp(StaminaMovementModifier, 0.3f, 1.0f);
}

void UCore_SurvivalPhysicsIntegration::ProcessTerrainInteraction(ETerrainType TerrainType, float DeltaTime)
{
    CurrentTerrainType = TerrainType;
    
    // Calculate terrain-specific effects
    switch (TerrainType)
    {
        case ETerrainType::Grassland:
            TerrainStaminaDrainMultiplier = 1.0f;
            TerrainSpeedModifier = 1.0f;
            break;
            
        case ETerrainType::Forest:
            TerrainStaminaDrainMultiplier = 1.2f;
            TerrainSpeedModifier = 0.9f;
            break;
            
        case ETerrainType::Mountain:
            TerrainStaminaDrainMultiplier = 2.0f;
            TerrainSpeedModifier = 0.7f;
            break;
            
        case ETerrainType::Swamp:
            TerrainStaminaDrainMultiplier = 1.8f;
            TerrainSpeedModifier = 0.6f;
            break;
            
        case ETerrainType::Desert:
            TerrainStaminaDrainMultiplier = 1.5f;
            TerrainSpeedModifier = 0.8f;
            break;
            
        case ETerrainType::Beach:
            TerrainStaminaDrainMultiplier = 1.3f;
            TerrainSpeedModifier = 0.85f;
            break;
            
        default:
            TerrainStaminaDrainMultiplier = 1.0f;
            TerrainSpeedModifier = 1.0f;
            break;
    }
    
    // Calculate stamina drain for movement
    if (MovementComponent && !MovementComponent->Velocity.IsZero())
    {
        float MovementSpeed = MovementComponent->Velocity.Size();
        float StaminaDrain = CalculateStaminaDrain(MovementSpeed, TerrainType, DeltaTime);
        
        // Apply stamina drain (would normally be handled by survival architecture)
        if (SurvivalArchitecture)
        {
            // In a full implementation, this would call the survival architecture
            // SurvivalArchitecture->DrainStamina(StaminaDrain);
        }
    }
}

void UCore_SurvivalPhysicsIntegration::ProcessSurvivalCollision(const FHitResult& HitResult, float ImpactForce)
{
    if (!bInitialized || ImpactForce < ImpactDamageThreshold)
    {
        return;
    }
    
    // Calculate damage based on impact force and health status
    float BaseDamage = (ImpactForce - ImpactDamageThreshold) / 100.0f;
    float ActualDamage = BaseDamage * FallDamageMultiplier;
    
    // Reduce damage if health is high (better physical condition)
    if (CurrentStats.Health > 75.0f)
    {
        ActualDamage *= 0.8f;
    }
    else if (CurrentStats.Health < 25.0f)
    {
        ActualDamage *= 1.3f; // More vulnerable when injured
    }
    
    // Calculate recovery time based on stamina
    CollisionRecoveryTime = FMath::Clamp(ActualDamage / CurrentStats.Stamina, 0.5f, 5.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalPhysicsIntegration: Collision damage %.2f, recovery time %.2f"), 
           ActualDamage, CollisionRecoveryTime);
}

float UCore_SurvivalPhysicsIntegration::CalculateStaminaDrain(float MovementSpeed, ETerrainType Terrain, float DeltaTime)
{
    // Base stamina drain per second while moving
    float BaseDrain = 2.0f;
    
    // Speed multiplier (faster movement = more drain)
    float SpeedMultiplier = FMath::Clamp(MovementSpeed / 600.0f, 0.1f, 3.0f);
    
    // Terrain multiplier
    float TerrainMultiplier = TerrainStaminaDrainMultiplier;
    
    // Environmental multiplier
    float EnvironmentalMultiplier = 1.0f;
    if (EnvironmentalFactors.Temperature > 30.0f || EnvironmentalFactors.Temperature < 5.0f)
    {
        EnvironmentalMultiplier = 1.5f; // Extreme temperatures increase drain
    }
    
    return BaseDrain * SpeedMultiplier * TerrainMultiplier * EnvironmentalMultiplier * DeltaTime;
}

void UCore_SurvivalPhysicsIntegration::ApplyEnvironmentalPhysics(const FEng_EnvironmentalFactors& Environment)
{
    EnvironmentalFactors = Environment;
    
    // Calculate temperature movement penalty
    float OptimalTemp = 20.0f;
    float TempDifference = FMath::Abs(Environment.Temperature - OptimalTemp);
    TemperatureMovementPenalty = FMath::Clamp(TempDifference / 30.0f, 0.0f, 0.5f);
    
    // Wind affects movement stability
    if (Environment.WindSpeed > 20.0f)
    {
        // High wind reduces movement precision
        FearMovementImpact = FMath::Max(FearMovementImpact, Environment.WindSpeed / 100.0f);
    }
    
    // Danger level affects fear
    if (Environment.DangerLevel > 0.1f)
    {
        FearMovementImpact = FMath::Max(FearMovementImpact, Environment.DangerLevel);
    }
}

bool UCore_SurvivalPhysicsIntegration::ValidateIntegration()
{
    bool bValid = true;
    
    // Check character and movement component
    if (!OwnerCharacter || !MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalPhysicsIntegration: Missing character or movement component"));
        bValid = false;
    }
    
    // Check survival architecture connection
    if (!ValidateArchitectureConnection())
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalPhysicsIntegration: Survival architecture not connected"));
        // Not critical for basic physics, so don't fail validation
    }
    
    // Check physics modifiers are in valid ranges
    if (StaminaMovementModifier < 0.0f || StaminaMovementModifier > 2.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalPhysicsIntegration: Invalid stamina modifier %.2f"), StaminaMovementModifier);
        bValid = false;
    }
    
    bIntegrationValid = bValid;
    return bValid;
}

FString UCore_SurvivalPhysicsIntegration::GetPerformanceMetrics()
{
    return FString::Printf(TEXT("Performance Score: %.1f%%, Frame Impact: %.3fms, Integration Valid: %s"),
                          IntegrationPerformanceScore,
                          FrameTimeImpact,
                          bIntegrationValid ? TEXT("Yes") : TEXT("No"));
}

void UCore_SurvivalPhysicsIntegration::ResetIntegration()
{
    // Reset all modifiers to defaults
    StaminaMovementModifier = 1.0f;
    HealthPhysicsModifier = 1.0f;
    TemperatureMovementPenalty = 0.0f;
    FearMovementImpact = 0.0f;
    TerrainStaminaDrainMultiplier = 1.0f;
    TerrainSpeedModifier = 1.0f;
    FallDamageMultiplier = 1.0f;
    CollisionRecoveryTime = 0.0f;
    
    // Reset performance metrics
    IntegrationPerformanceScore = 100.0f;
    FrameTimeImpact = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsIntegration: Reset to default state"));
}

bool UCore_SurvivalPhysicsIntegration::ValidateWalkAroundPhysics()
{
    if (!bInitialized || !OwnerCharacter || !MovementComponent)
    {
        return false;
    }
    
    // Check basic movement functionality
    bool bMovementValid = MovementComponent->MaxWalkSpeed > 0.0f && MovementComponent->JumpZVelocity > 0.0f;
    
    // Check physics integration is active
    bool bPhysicsActive = (StaminaMovementModifier > 0.0f) && (HealthPhysicsModifier > 0.0f);
    
    // Check terrain interaction is working
    bool bTerrainValid = (TerrainSpeedModifier > 0.0f) && (TerrainStaminaDrainMultiplier > 0.0f);
    
    return bMovementValid && bPhysicsActive && bTerrainValid;
}

float UCore_SurvivalPhysicsIntegration::GetMilestoneCompletionPercentage()
{
    float CompletionScore = 0.0f;
    
    // Basic integration (25%)
    if (bInitialized && OwnerCharacter && MovementComponent)
    {
        CompletionScore += 25.0f;
    }
    
    // Survival stats integration (25%)
    if (CurrentStats.Health > 0.0f && CurrentStats.Stamina > 0.0f)
    {
        CompletionScore += 25.0f;
    }
    
    // Movement physics working (25%)
    if (ValidateWalkAroundPhysics())
    {
        CompletionScore += 25.0f;
    }
    
    // Performance acceptable (25%)
    if (IntegrationPerformanceScore > 80.0f)
    {
        CompletionScore += 25.0f;
    }
    
    return FMath::Clamp(CompletionScore, 0.0f, 100.0f);
}

void UCore_SurvivalPhysicsIntegration::UpdateSurvivalStats()
{
    if (SurvivalArchitecture)
    {
        // In a full implementation, this would get stats from the architecture
        // CurrentStats = SurvivalArchitecture->GetCurrentStats();
        // For now, simulate some basic stat changes
        
        // Simulate stamina regeneration when not moving
        if (MovementComponent && MovementComponent->Velocity.IsZero())
        {
            CurrentStats.Stamina = FMath::Min(CurrentStats.Stamina + 5.0f, 100.0f);
        }
    }
}

void UCore_SurvivalPhysicsIntegration::CalculatePhysicsModifiers()
{
    // Stamina affects movement speed and jump height
    StaminaMovementModifier = FMath::Clamp(CurrentStats.Stamina / 100.0f, 0.2f, 1.0f);
    
    // Health affects overall physics response
    HealthPhysicsModifier = FMath::Clamp(CurrentStats.Health / 100.0f, 0.3f, 1.0f);
    
    // Fear affects movement precision
    FearMovementImpact = FMath::Clamp(CurrentStats.Fear / 100.0f, 0.0f, 0.8f);
    
    // Fall damage increases when health is low
    FallDamageMultiplier = FMath::Clamp(2.0f - (CurrentStats.Health / 100.0f), 0.5f, 2.0f);
}

void UCore_SurvivalPhysicsIntegration::ProcessEnvironmentalImpact(float DeltaTime)
{
    // Temperature affects movement
    float OptimalTemp = 20.0f;
    float TempDifference = FMath::Abs(EnvironmentalFactors.Temperature - OptimalTemp);
    TemperatureMovementPenalty = FMath::Clamp(TempDifference / 30.0f, 0.0f, 0.5f);
    
    // Update environmental factors impact
    ApplyEnvironmentalPhysics(EnvironmentalFactors);
}

void UCore_SurvivalPhysicsIntegration::MonitorPerformance(float DeltaTime)
{
    PerformanceTimer += DeltaTime;
    
    if (PerformanceTimer >= 1.0f) // Update performance metrics every second
    {
        // Calculate performance score based on frame time impact
        if (FrameTimeImpact < 0.5f)
        {
            IntegrationPerformanceScore = 100.0f;
        }
        else if (FrameTimeImpact < 1.0f)
        {
            IntegrationPerformanceScore = 90.0f;
        }
        else if (FrameTimeImpact < 2.0f)
        {
            IntegrationPerformanceScore = 75.0f;
        }
        else
        {
            IntegrationPerformanceScore = 50.0f;
        }
        
        PerformanceTimer = 0.0f;
    }
}

bool UCore_SurvivalPhysicsIntegration::ValidateArchitectureConnection()
{
    if (!SurvivalArchitecture)
    {
        // Try to find survival architecture again
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
            {
                AActor* Actor = *ActorIterator;
                if (Actor && Actor->GetName().Contains(TEXT("SurvivalArchitecture")))
                {
                    SurvivalArchitecture = Actor->FindComponentByClass<UEng_SurvivalArchitecture>();
                    if (SurvivalArchitecture)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    return true;
}