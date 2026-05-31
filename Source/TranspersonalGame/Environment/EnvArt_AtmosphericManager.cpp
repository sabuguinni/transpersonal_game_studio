#include "EnvArt_AtmosphericManager.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create fog component
    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("FogComponent"));
    FogComponent->SetupAttachment(RootComponent);

    // Create particle component
    ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
    ParticleComponent->SetupAttachment(RootComponent);

    // Initialize default settings
    AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::VolumericFog;
    AtmosphericSettings.Intensity = 0.5f;
    AtmosphericSettings.TintColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm golden tint
    AtmosphericSettings.VisibilityDistance = 15000.0f;
    AtmosphericSettings.bDynamicTimeOfDay = true;

    CurrentTimeOfDay = 12.0f; // Start at noon
    TimeSpeed = 0.1f; // Slow time progression
    TargetBiome = EBiomeType::Savanna;
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyBiomeAtmosphere();
    UpdateParticleEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericManager initialized for biome: %d"), (int32)TargetBiome);
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (AtmosphericSettings.bDynamicTimeOfDay)
    {
        UpdateTimeOfDay(DeltaTime);
    }
}

void AEnvArt_AtmosphericManager::SetAtmosphericType(EEnvArt_AtmosphericType NewType)
{
    AtmosphericSettings.AtmosphericType = NewType;
    UpdateParticleEffects();
    
    UE_LOG(LogTemp, Log, TEXT("Atmospheric type changed to: %d"), (int32)NewType);
}

void AEnvArt_AtmosphericManager::UpdateFogSettings(float NewIntensity, FLinearColor NewColor)
{
    AtmosphericSettings.Intensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    AtmosphericSettings.TintColor = NewColor;

    if (FogComponent)
    {
        FogComponent->SetFogDensity(AtmosphericSettings.Intensity * 0.02f);
        FogComponent->SetFogInscatteringColor(NewColor);
    }
}

void AEnvArt_AtmosphericManager::SetBiomeSpecificAtmosphere(EBiomeType Biome)
{
    TargetBiome = Biome;
    ApplyBiomeAtmosphere();
}

void AEnvArt_AtmosphericManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }

    // Update atmospheric effects based on time
    if (FogComponent)
    {
        float FogDensity = CalculateFogDensity(CurrentTimeOfDay);
        FogComponent->SetFogDensity(FogDensity);
        FogComponent->SetFogInscatteringColor(GetCurrentSunColor());
    }
}

FLinearColor AEnvArt_AtmosphericManager::GetCurrentSunColor() const
{
    return CalculateSunColor(CurrentTimeOfDay);
}

void AEnvArt_AtmosphericManager::UpdateTimeOfDay(float DeltaTime)
{
    CurrentTimeOfDay += DeltaTime * TimeSpeed;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    SetTimeOfDay(CurrentTimeOfDay);
}

void AEnvArt_AtmosphericManager::ApplyBiomeAtmosphere()
{
    switch (TargetBiome)
    {
        case EBiomeType::Savanna:
            AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::HeatHaze;
            AtmosphericSettings.TintColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm golden
            AtmosphericSettings.Intensity = 0.3f;
            break;

        case EBiomeType::Swamp:
            AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::MorningMist;
            AtmosphericSettings.TintColor = FLinearColor(0.7f, 0.9f, 0.8f, 1.0f); // Cool green
            AtmosphericSettings.Intensity = 0.8f;
            break;

        case EBiomeType::Forest:
            AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::PollenDrift;
            AtmosphericSettings.TintColor = FLinearColor(0.8f, 1.0f, 0.7f, 1.0f); // Soft green
            AtmosphericSettings.Intensity = 0.4f;
            break;

        case EBiomeType::Desert:
            AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::DustParticles;
            AtmosphericSettings.TintColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Sandy orange
            AtmosphericSettings.Intensity = 0.6f;
            break;

        case EBiomeType::Mountain:
            AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::VolumericFog;
            AtmosphericSettings.TintColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f); // Cool blue
            AtmosphericSettings.Intensity = 0.7f;
            break;

        default:
            AtmosphericSettings.AtmosphericType = EEnvArt_AtmosphericType::None;
            break;
    }

    UpdateFogSettings(AtmosphericSettings.Intensity, AtmosphericSettings.TintColor);
}

void AEnvArt_AtmosphericManager::UpdateParticleEffects()
{
    if (!ParticleComponent)
    {
        return;
    }

    // Configure particle system based on atmospheric type
    switch (AtmosphericSettings.AtmosphericType)
    {
        case EEnvArt_AtmosphericType::DustParticles:
            ParticleComponent->SetFloatParameter(TEXT("SpawnRate"), 50.0f);
            ParticleComponent->SetVectorParameter(TEXT("ParticleColor"), FVector(1.0f, 0.8f, 0.6f));
            break;

        case EEnvArt_AtmosphericType::PollenDrift:
            ParticleComponent->SetFloatParameter(TEXT("SpawnRate"), 30.0f);
            ParticleComponent->SetVectorParameter(TEXT("ParticleColor"), FVector(1.0f, 1.0f, 0.7f));
            break;

        case EEnvArt_AtmosphericType::MorningMist:
            ParticleComponent->SetFloatParameter(TEXT("SpawnRate"), 20.0f);
            ParticleComponent->SetVectorParameter(TEXT("ParticleColor"), FVector(0.9f, 0.9f, 1.0f));
            break;

        default:
            ParticleComponent->SetFloatParameter(TEXT("SpawnRate"), 10.0f);
            break;
    }

    ParticleComponent->SetFloatParameter(TEXT("Intensity"), AtmosphericSettings.Intensity);
}

FLinearColor AEnvArt_AtmosphericManager::CalculateSunColor(float TimeOfDay) const
{
    // Calculate sun color based on time of day
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) // Daytime
    {
        float NoonFactor = 1.0f - FMath::Abs(TimeOfDay - 12.0f) / 6.0f;
        return FLinearColor::LerpUsingHSV(
            FLinearColor(1.0f, 0.6f, 0.3f, 1.0f), // Sunrise/sunset orange
            FLinearColor(1.0f, 0.95f, 0.9f, 1.0f), // Noon white
            NoonFactor
        );
    }
    else // Nighttime
    {
        return FLinearColor(0.3f, 0.4f, 0.8f, 1.0f); // Cool blue moonlight
    }
}

float AEnvArt_AtmosphericManager::CalculateFogDensity(float TimeOfDay) const
{
    // Fog is denser during dawn and dusk
    float BaseDensity = AtmosphericSettings.Intensity * 0.02f;
    
    if ((TimeOfDay >= 5.0f && TimeOfDay <= 8.0f) || (TimeOfDay >= 17.0f && TimeOfDay <= 20.0f))
    {
        return BaseDensity * 1.5f; // Increase fog during transition times
    }
    
    return BaseDensity;
}