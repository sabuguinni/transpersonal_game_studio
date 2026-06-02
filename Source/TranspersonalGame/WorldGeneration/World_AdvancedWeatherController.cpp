#include "World_AdvancedWeatherController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "Math/UnrealMathUtility.h"

AWorld_AdvancedWeatherController::AWorld_AdvancedWeatherController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Create weather controller mesh
    WeatherControllerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeatherControllerMesh"));
    WeatherControllerMesh->SetupAttachment(RootComponent);
    
    // Initialize weather state
    CurrentWeatherState = FWorld_WeatherState();
    TargetWeatherState = FWorld_WeatherState();
    WeatherTransitionSpeed = 1.0f;
    
    // Initialize storm system
    bStormActive = false;
    StormSpawnChance = 0.1f;
    TimeBetweenStorms = 1800.0f; // 30 minutes
    
    // Initialize performance settings
    MaxRenderBudget = 5.0f; // 5ms budget
    MaxParticleCount = 1000;
    LODDistance = 5000.0f;
    
    // Initialize timers
    LastStormTime = 0.0f;
    WeatherTransitionTimer = 0.0f;
    StormTimer = 0.0f;
    LightningTimer = 0.0f;
    PerformanceCheckTimer = 0.0f;
}

void AWorld_AdvancedWeatherController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing weather actors in the world
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        WeatherFog = Cast<AExponentialHeightFog>(FogActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Weather Controller: Found existing fog actor"));
    }
    
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), LightActors);
    if (LightActors.Num() > 0)
    {
        WeatherLight = Cast<ADirectionalLight>(LightActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Weather Controller: Found existing directional light"));
    }
    
    // Create weather effects
    CreateWeatherEffects();
    
    UE_LOG(LogTemp, Log, TEXT("Advanced Weather Controller initialized"));
}

void AWorld_AdvancedWeatherController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update weather systems
    UpdateWeatherTransition(DeltaTime);
    UpdateStormSystem(DeltaTime);
    UpdateWeatherEffects();
    
    // Performance monitoring
    PerformanceCheckTimer += DeltaTime;
    if (PerformanceCheckTimer >= 1.0f) // Check every second
    {
        UpdatePerformanceMetrics();
        PerformanceCheckTimer = 0.0f;
    }
}

void AWorld_AdvancedWeatherController::SetWeatherType(EWeatherType NewWeatherType, float TransitionTime)
{
    TargetWeatherState.CurrentWeather = NewWeatherType;
    
    // Set appropriate values based on weather type
    switch (NewWeatherType)
    {
        case EWeatherType::Clear:
            TargetWeatherState.Intensity = 0.0f;
            TargetWeatherState.WindSpeed = 5.0f;
            TargetWeatherState.Visibility = 10000.0f;
            break;
            
        case EWeatherType::Cloudy:
            TargetWeatherState.Intensity = 0.3f;
            TargetWeatherState.WindSpeed = 10.0f;
            TargetWeatherState.Visibility = 8000.0f;
            break;
            
        case EWeatherType::Rainy:
            TargetWeatherState.Intensity = 0.7f;
            TargetWeatherState.WindSpeed = 15.0f;
            TargetWeatherState.Visibility = 3000.0f;
            break;
            
        case EWeatherType::Stormy:
            TargetWeatherState.Intensity = 1.0f;
            TargetWeatherState.WindSpeed = 25.0f;
            TargetWeatherState.Visibility = 1000.0f;
            break;
            
        case EWeatherType::Foggy:
            TargetWeatherState.Intensity = 0.8f;
            TargetWeatherState.WindSpeed = 3.0f;
            TargetWeatherState.Visibility = 500.0f;
            break;
    }
    
    WeatherTransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
    WeatherTransitionTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Weather transition started to %d"), (int32)NewWeatherType);
}

void AWorld_AdvancedWeatherController::StartStorm(const FWorld_StormData& StormData)
{
    ActiveStorm = StormData;
    bStormActive = true;
    StormTimer = 0.0f;
    LightningTimer = 0.0f;
    
    // Automatically set weather to stormy
    SetWeatherType(EWeatherType::Stormy, 5.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Storm started at location: %s"), *ActiveStorm.StormCenter.ToString());
}

void AWorld_AdvancedWeatherController::EndStorm(float FadeTime)
{
    if (bStormActive)
    {
        bStormActive = false;
        LastStormTime = GetWorld()->GetTimeSeconds();
        
        // Transition to cloudy weather
        SetWeatherType(EWeatherType::Cloudy, FadeTime);
        
        UE_LOG(LogTemp, Log, TEXT("Storm ended with fade time: %f"), FadeTime);
    }
}

void AWorld_AdvancedWeatherController::UpdateWeatherIntensity(float NewIntensity)
{
    CurrentWeatherState.Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    TargetWeatherState.Intensity = CurrentWeatherState.Intensity;
}

float AWorld_AdvancedWeatherController::GetWeatherIntensityAtLocation(const FVector& Location) const
{
    if (!bStormActive)
    {
        return CurrentWeatherState.Intensity;
    }
    
    // Calculate distance-based intensity for storms
    float DistanceToStorm = FVector::Dist(Location, ActiveStorm.StormCenter);
    float IntensityMultiplier = 1.0f - FMath::Clamp(DistanceToStorm / ActiveStorm.StormRadius, 0.0f, 1.0f);
    
    return CurrentWeatherState.Intensity * IntensityMultiplier;
}

FVector AWorld_AdvancedWeatherController::GetWindDirection() const
{
    if (bStormActive)
    {
        // Wind blows towards storm center
        FVector PlayerLocation = GetActorLocation();
        return (ActiveStorm.StormCenter - PlayerLocation).GetSafeNormal();
    }
    
    // Default wind direction
    return FVector(1.0f, 0.0f, 0.0f);
}

void AWorld_AdvancedWeatherController::OptimizeWeatherEffects(float TargetFrameTime)
{
    float PerformanceRatio = TargetFrameTime / (1.0f / 60.0f); // Target 60 FPS
    
    if (PerformanceRatio > 1.2f) // Performance is poor
    {
        AdjustEffectQuality(0.7f);
        UE_LOG(LogTemp, Warning, TEXT("Weather effects quality reduced due to performance"));
    }
    else if (PerformanceRatio < 0.8f) // Performance is good
    {
        AdjustEffectQuality(1.0f);
    }
}

void AWorld_AdvancedWeatherController::SetWeatherLOD(int32 LODLevel)
{
    // Adjust effect complexity based on LOD level
    switch (LODLevel)
    {
        case 0: // High quality
            MaxParticleCount = 1000;
            MaxRenderBudget = 5.0f;
            break;
            
        case 1: // Medium quality
            MaxParticleCount = 500;
            MaxRenderBudget = 3.0f;
            break;
            
        case 2: // Low quality
            MaxParticleCount = 200;
            MaxRenderBudget = 1.5f;
            break;
            
        default: // Minimal quality
            MaxParticleCount = 50;
            MaxRenderBudget = 0.5f;
            break;
    }
    
    // Update existing effects
    UpdateWeatherEffects();
}

void AWorld_AdvancedWeatherController::UpdateWeatherTransition(float DeltaTime)
{
    if (WeatherTransitionTimer < 1.0f)
    {
        WeatherTransitionTimer += DeltaTime * WeatherTransitionSpeed;
        WeatherTransitionTimer = FMath::Clamp(WeatherTransitionTimer, 0.0f, 1.0f);
        
        // Interpolate weather values
        CurrentWeatherState.Intensity = FMath::Lerp(CurrentWeatherState.Intensity, TargetWeatherState.Intensity, WeatherTransitionTimer);
        CurrentWeatherState.WindSpeed = FMath::Lerp(CurrentWeatherState.WindSpeed, TargetWeatherState.WindSpeed, WeatherTransitionTimer);
        CurrentWeatherState.Visibility = FMath::Lerp(CurrentWeatherState.Visibility, TargetWeatherState.Visibility, WeatherTransitionTimer);
        
        if (WeatherTransitionTimer >= 1.0f)
        {
            CurrentWeatherState = TargetWeatherState;
            UE_LOG(LogTemp, Log, TEXT("Weather transition completed"));
        }
    }
}

void AWorld_AdvancedWeatherController::UpdateStormSystem(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (bStormActive)
    {
        StormTimer += DeltaTime;
        LightningTimer += DeltaTime;
        
        // Update storm movement
        UpdateStormMovement(DeltaTime);
        
        // Spawn lightning
        if (LightningTimer >= (1.0f / ActiveStorm.LightningFrequency))
        {
            SpawnLightning();
            LightningTimer = 0.0f;
        }
        
        // Check if storm should end
        if (StormTimer >= ActiveStorm.Duration)
        {
            EndStorm(10.0f);
        }
    }
    else
    {
        // Check if new storm should spawn
        if ((CurrentTime - LastStormTime) >= TimeBetweenStorms && ShouldSpawnStorm())
        {
            FWorld_StormData NewStorm;
            NewStorm.StormCenter = GetActorLocation() + FVector(
                FMath::RandRange(-10000.0f, 10000.0f),
                FMath::RandRange(-10000.0f, 10000.0f),
                FMath::RandRange(2000.0f, 4000.0f)
            );
            StartStorm(NewStorm);
        }
    }
}

void AWorld_AdvancedWeatherController::UpdateWeatherEffects()
{
    UpdateFogEffects();
    UpdateLightingEffects();
    UpdateRainEffects();
}

void AWorld_AdvancedWeatherController::UpdatePerformanceMetrics()
{
    // Simple performance check based on actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() > 8000)
    {
        // Reduce weather effect quality
        SetWeatherLOD(3); // Minimal quality
        UE_LOG(LogTemp, Warning, TEXT("High actor count detected, reducing weather quality"));
    }
}

void AWorld_AdvancedWeatherController::CreateWeatherEffects()
{
    // Initialize rain particle systems array
    RainSystems.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Weather effects created"));
}

void AWorld_AdvancedWeatherController::UpdateRainEffects()
{
    // Update rain intensity based on current weather
    float RainIntensity = (CurrentWeatherState.CurrentWeather == EWeatherType::Rainy || 
                          CurrentWeatherState.CurrentWeather == EWeatherType::Stormy) 
                          ? CurrentWeatherState.Intensity : 0.0f;
    
    // Apply rain effects (implementation would control particle systems)
}

void AWorld_AdvancedWeatherController::UpdateFogEffects()
{
    if (WeatherFog)
    {
        float FogDensity = CurrentWeatherState.CurrentWeather == EWeatherType::Foggy 
                          ? CurrentWeatherState.Intensity * 0.1f 
                          : CurrentWeatherState.Intensity * 0.02f;
        
        // Apply fog density (would use fog component properties in full implementation)
    }
}

void AWorld_AdvancedWeatherController::UpdateLightingEffects()
{
    if (WeatherLight)
    {
        float LightIntensity = 1.0f - (CurrentWeatherState.Intensity * 0.5f);
        
        // Apply lighting changes (would use light component properties in full implementation)
    }
}

void AWorld_AdvancedWeatherController::SpawnLightning()
{
    // Create lightning effect at random location within storm
    FVector LightningLocation = ActiveStorm.StormCenter + FVector(
        FMath::RandRange(-ActiveStorm.StormRadius, ActiveStorm.StormRadius),
        FMath::RandRange(-ActiveStorm.StormRadius, ActiveStorm.StormRadius),
        0.0f
    );
    
    UE_LOG(LogTemp, Log, TEXT("Lightning spawned at: %s"), *LightningLocation.ToString());
}

void AWorld_AdvancedWeatherController::UpdateStormMovement(float DeltaTime)
{
    // Move storm based on wind patterns
    FVector MovementDirection = GetWindDirection();
    ActiveStorm.StormCenter += MovementDirection * ActiveStorm.MovementSpeed * DeltaTime;
}

bool AWorld_AdvancedWeatherController::ShouldSpawnStorm() const
{
    return FMath::RandRange(0.0f, 1.0f) < StormSpawnChance;
}

void AWorld_AdvancedWeatherController::AdjustEffectQuality(float PerformanceRatio)
{
    MaxParticleCount = FMath::RoundToInt(MaxParticleCount * PerformanceRatio);
    MaxRenderBudget *= PerformanceRatio;
    
    // Update all active effects with new quality settings
    UpdateWeatherEffects();
}

void AWorld_AdvancedWeatherController::CullDistantEffects()
{
    // Remove effects beyond LOD distance
    // Implementation would check distance to player and disable distant effects
}