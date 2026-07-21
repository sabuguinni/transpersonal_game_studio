#include "Light_DynamicAtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/VolumetricFog.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_DynamicAtmosphereManager::ALight_DynamicAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize Cretaceous atmosphere settings
    CretaceousSettings.SunIntensity = 5.0f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    CretaceousSettings.FogDensity = 0.3f;
    CretaceousSettings.FogColor = FLinearColor(0.78f, 0.70f, 0.63f, 1.0f);
    CretaceousSettings.VolumetricScattering = 0.3f;
    
    // Set default time to noon
    CurrentTimeOfDay = 0.5f;
    DayNightCycleDuration = 1200.0f; // 20 minutes real time
}

void ALight_DynamicAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing lighting components in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        if (ADirectionalLight* DirectionalLightActor = Cast<ADirectionalLight>(FoundActors[0]))
        {
            SunLight = DirectionalLightActor->GetLightComponent();
        }
    }
    
    // Find sky light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        if (ASkyLight* SkyLightActor = Cast<ASkyLight>(FoundActors[0]))
        {
            SkyLight = SkyLightActor->GetLightComponent();
        }
    }
    
    // Apply initial Cretaceous atmosphere
    ApplyCretaceousAtmosphere();
    FixPersistentAtmosphere();
}

void ALight_DynamicAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update day/night cycle
    CurrentTimeOfDay += DeltaTime / DayNightCycleDuration;
    if (CurrentTimeOfDay > 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
    }
    
    UpdateSunPosition();
    UpdateAtmosphericProperties();
}

void ALight_DynamicAtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UpdateSunPosition();
    UpdateAtmosphericProperties();
}

void ALight_DynamicAtmosphereManager::ApplyCretaceousAtmosphere()
{
    if (SunLight)
    {
        SunLight->SetIntensity(CretaceousSettings.SunIntensity);
        SunLight->SetLightColor(CretaceousSettings.SunColor);
        SunLight->SetCastShadows(true);
        SunLight->SetCastVolumetricShadow(true);
    }
    
    if (SkyLight)
    {
        SkyLight->SetIntensity(1.0f);
        SkyLight->SetLightColor(FLinearColor(0.9f, 0.85f, 0.8f, 1.0f));
        SkyLight->RecaptureSky();
    }
}

void ALight_DynamicAtmosphereManager::FixPersistentAtmosphere()
{
    RemoveConflictingAtmosphereActors();
    
    // Spawn volumetric fog if not present
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVolumetricFog::StaticClass(), FogActors);
    
    if (FogActors.Num() == 0)
    {
        FVector SpawnLocation(0.0f, 0.0f, 500.0f);
        FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
        
        if (AVolumetricFog* NewFog = GetWorld()->SpawnActor<AVolumetricFog>(AVolumetricFog::StaticClass(), SpawnLocation, SpawnRotation))
        {
            AtmosphericFog = NewFog->GetComponent();
            if (AtmosphericFog)
            {
                AtmosphericFog->SetScatteringDistribution(CretaceousSettings.VolumetricScattering);
                AtmosphericFog->SetAlbedo(CretaceousSettings.FogColor);
                AtmosphericFog->SetEmissionColor(FLinearColor(0.2f, 0.15f, 0.12f, 1.0f));
            }
        }
    }
}

FLinearColor ALight_DynamicAtmosphereManager::CalculateSunColor(float TimeOfDay) const
{
    // Sunrise/sunset colors (orange/red) vs midday (white/yellow)
    if (TimeOfDay < 0.25f || TimeOfDay > 0.75f)
    {
        // Night - cooler blue tint
        return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    }
    else if (TimeOfDay < 0.35f || TimeOfDay > 0.65f)
    {
        // Dawn/dusk - warm orange
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    }
    else
    {
        // Day - warm white/yellow
        return CretaceousSettings.SunColor;
    }
}

float ALight_DynamicAtmosphereManager::CalculateSunIntensity(float TimeOfDay) const
{
    // Calculate intensity based on sun elevation
    float SunElevation = FMath::Sin((TimeOfDay - 0.25f) * 2.0f * PI);
    SunElevation = FMath::Max(0.0f, SunElevation);
    
    return SunElevation * CretaceousSettings.SunIntensity;
}

void ALight_DynamicAtmosphereManager::UpdateSunPosition()
{
    if (!SunLight)
        return;
    
    // Calculate sun rotation based on time of day
    float SunAngle = (CurrentTimeOfDay - 0.25f) * 360.0f; // 0.25 = sunrise
    FRotator SunRotation(SunAngle, 0.0f, 0.0f);
    
    if (AActor* SunActor = SunLight->GetOwner())
    {
        SunActor->SetActorRotation(SunRotation);
    }
}

void ALight_DynamicAtmosphereManager::UpdateAtmosphericProperties()
{
    if (!SunLight)
        return;
    
    // Update sun color and intensity
    FLinearColor NewSunColor = CalculateSunColor(CurrentTimeOfDay);
    float NewSunIntensity = CalculateSunIntensity(CurrentTimeOfDay);
    
    SunLight->SetLightColor(NewSunColor);
    SunLight->SetIntensity(NewSunIntensity);
    
    // Update fog properties based on time of day
    if (AtmosphericFog)
    {
        float FogIntensity = IsDay() ? CretaceousSettings.FogDensity : CretaceousSettings.FogDensity * 0.5f;
        FLinearColor FogColor = IsDay() ? CretaceousSettings.FogColor : FLinearColor(0.2f, 0.25f, 0.4f, 1.0f);
        
        AtmosphericFog->SetScatteringDistribution(FogIntensity);
        AtmosphericFog->SetAlbedo(FogColor);
    }
}

void ALight_DynamicAtmosphereManager::RemoveConflictingAtmosphereActors()
{
    // Remove problematic atmosphere actors that cause orange/red tint
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsInLevel(GetWorld(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
            continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        if (ClassName.Contains(TEXT("SkyAtmosphere")) || 
            ClassName.Contains(TEXT("AtmosphericFog")) ||
            ClassName.Contains(TEXT("BP_Sky")))
        {
            Actor->Destroy();
        }
    }
}