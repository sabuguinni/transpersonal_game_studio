#include "EnvironmentalAtmosphere.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEnvironmentalAtmosphere::UEnvironmentalAtmosphere()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
}

void UEnvironmentalAtmosphere::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing sun light in the level
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    // Find existing height fog
    if (!HeightFog)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
        for (AActor* Actor : FoundActors)
        {
            UExponentialHeightFogComponent* FogComp = Actor->FindComponentByClass<UExponentialHeightFogComponent>();
            if (FogComp)
            {
                HeightFog = FogComp;
                break;
            }
        }
    }

    InitializeAtmosphereForType();
    ApplyAtmosphereSettings();
}

void UEnvironmentalAtmosphere::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Simulate time progression for dynamic lighting
    CurrentTimeOfDay += DeltaTime * 0.1f; // Slow time progression
    if (CurrentTimeOfDay > 24.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
    
    UpdateSunPosition(CurrentTimeOfDay);
}

void UEnvironmentalAtmosphere::SetAtmosphereType(EEnvArt_AtmosphereType NewType)
{
    AtmosphereType = NewType;
    InitializeAtmosphereForType();
    ApplyAtmosphereSettings();
}

void UEnvironmentalAtmosphere::ApplyAtmosphereSettings()
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(AtmosphereSettings.SunColor);
        LightComp->SetIntensity(AtmosphereSettings.SunIntensity);
    }

    // Apply fog settings
    if (HeightFog)
    {
        HeightFog->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        HeightFog->SetFogDensity(AtmosphereSettings.FogDensity);
        HeightFog->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
    }

    // Apply particle settings
    if (PollenParticles)
    {
        PollenParticles->SetActive(AtmosphereSettings.bEnablePollenParticles);
        if (AtmosphereSettings.bEnablePollenParticles)
        {
            // Adjust spawn rate based on settings
            PollenParticles->SetFloatParameter(FName("SpawnRate"), AtmosphereSettings.ParticleSpawnRate);
        }
    }
}

void UEnvironmentalAtmosphere::UpdateSunPosition(float TimeOfDay)
{
    if (!SunLight)
        return;

    // Calculate sun angle based on time of day (0-24 hours)
    float SunAngleRadians = FMath::DegreesToRadians((TimeOfDay - 6.0f) * 15.0f); // 15 degrees per hour, sunrise at 6AM
    float SunElevation = FMath::Sin(SunAngleRadians) * 90.0f; // Max 90 degrees at noon
    
    // Clamp elevation to prevent negative values (night time)
    SunElevation = FMath::Clamp(SunElevation, -10.0f, 90.0f);
    
    FRotator SunRotation = FRotator(SunElevation, AtmosphereSettings.SunAngle, 0.0f);
    SunLight->SetActorRotation(SunRotation);

    // Adjust sun intensity based on elevation
    if (SunLight->GetLightComponent())
    {
        float IntensityMultiplier = FMath::Clamp(SunElevation / 90.0f, 0.1f, 1.0f);
        SunLight->GetLightComponent()->SetIntensity(AtmosphereSettings.SunIntensity * IntensityMultiplier);
    }
}

void UEnvironmentalAtmosphere::SetFogSettings(FLinearColor Color, float Density, float HeightFalloff)
{
    AtmosphereSettings.FogColor = Color;
    AtmosphereSettings.FogDensity = Density;
    AtmosphereSettings.FogHeightFalloff = HeightFalloff;
    
    if (HeightFog)
    {
        HeightFog->SetFogInscatteringColor(Color);
        HeightFog->SetFogDensity(Density);
        HeightFog->SetFogHeightFalloff(HeightFalloff);
    }
}

void UEnvironmentalAtmosphere::TogglePollenParticles(bool bEnabled)
{
    AtmosphereSettings.bEnablePollenParticles = bEnabled;
    
    if (PollenParticles)
    {
        PollenParticles->SetActive(bEnabled);
    }
}

void UEnvironmentalAtmosphere::InitializeAtmosphereForType()
{
    AtmosphereSettings = GetDefaultSettingsForType(AtmosphereType);
}

FEnvArt_AtmosphereSettings UEnvironmentalAtmosphere::GetDefaultSettingsForType(EEnvArt_AtmosphereType Type)
{
    FEnvArt_AtmosphereSettings Settings;
    
    switch (Type)
    {
        case EEnvArt_AtmosphereType::Forest:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm golden light
            Settings.SunIntensity = 2.5f;
            Settings.SunAngle = 45.0f;
            Settings.FogColor = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f); // Greenish fog
            Settings.FogDensity = 0.03f;
            Settings.FogHeightFalloff = 0.3f;
            Settings.bEnablePollenParticles = true;
            Settings.ParticleSpawnRate = 15.0f;
            break;
            
        case EEnvArt_AtmosphereType::Swamp:
            Settings.SunColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f); // Pale green light
            Settings.SunIntensity = 2.0f;
            Settings.SunAngle = 30.0f;
            Settings.FogColor = FLinearColor(0.5f, 0.6f, 0.5f, 1.0f); // Murky fog
            Settings.FogDensity = 0.05f;
            Settings.FogHeightFalloff = 0.1f;
            Settings.bEnablePollenParticles = true;
            Settings.ParticleSpawnRate = 8.0f;
            break;
            
        case EEnvArt_AtmosphereType::Desert:
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Hot orange light
            Settings.SunIntensity = 4.0f;
            Settings.SunAngle = 60.0f;
            Settings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f); // Sandy haze
            Settings.FogDensity = 0.01f;
            Settings.FogHeightFalloff = 0.5f;
            Settings.bEnablePollenParticles = false;
            Settings.ParticleSpawnRate = 0.0f;
            break;
            
        case EEnvArt_AtmosphereType::Mountain:
            Settings.SunColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Cool blue light
            Settings.SunIntensity = 3.5f;
            Settings.SunAngle = 75.0f;
            Settings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f); // Blue mountain fog
            Settings.FogDensity = 0.02f;
            Settings.FogHeightFalloff = 0.4f;
            Settings.bEnablePollenParticles = false;
            Settings.ParticleSpawnRate = 0.0f;
            break;
            
        case EEnvArt_AtmosphereType::Savanna:
        default:
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm white light
            Settings.SunIntensity = 3.0f;
            Settings.SunAngle = 50.0f;
            Settings.FogColor = FLinearColor(0.8f, 0.8f, 0.7f, 1.0f); // Dusty fog
            Settings.FogDensity = 0.015f;
            Settings.FogHeightFalloff = 0.25f;
            Settings.bEnablePollenParticles = true;
            Settings.ParticleSpawnRate = 5.0f;
            break;
    }
    
    return Settings;
}