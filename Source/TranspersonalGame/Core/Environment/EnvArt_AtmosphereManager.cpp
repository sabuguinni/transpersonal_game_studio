#include "EnvArt_AtmosphereManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    SunLight = nullptr;
    SkyLightActor = nullptr;
    HeightFog = nullptr;
    
    TimeOfDay = 12.0f;
    bAutoTimeProgression = false;
    TimeProgressionSpeed = 1.0f;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindAtmosphereActors();
    SetGoldenHourLighting();
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoTimeProgression)
    {
        TimeOfDay += DeltaTime * TimeProgressionSpeed;
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay = 0.0f;
        }
        UpdateSunPosition();
    }
}

void AEnvArt_AtmosphereManager::FindAtmosphereActors()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        
        // Find DirectionalLight
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
        
        // Find SkyLight
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
        
        // Find ExponentialHeightFog
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void AEnvArt_AtmosphereManager::SetGoldenHourLighting()
{
    AtmosphereSettings.SunElevation = 15.0f;
    AtmosphereSettings.SunAzimuth = 45.0f;
    AtmosphereSettings.SunIntensity = 8.0f;
    AtmosphereSettings.FogDensity = 0.015f;
    AtmosphereSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::SetMidDayLighting()
{
    AtmosphereSettings.SunElevation = 80.0f;
    AtmosphereSettings.SunAzimuth = 180.0f;
    AtmosphereSettings.SunIntensity = 12.0f;
    AtmosphereSettings.FogDensity = 0.01f;
    AtmosphereSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::SetDuskLighting()
{
    AtmosphereSettings.SunElevation = -5.0f;
    AtmosphereSettings.SunAzimuth = 270.0f;
    AtmosphereSettings.SunIntensity = 4.0f;
    AtmosphereSettings.FogDensity = 0.025f;
    AtmosphereSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::UpdateAtmosphere()
{
    UpdateSunPosition();
    UpdateFogSettings();
    UpdateSkyLightSettings();
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    
    // Convert time to sun elevation (-90 to 90 degrees)
    float SunAngle = (TimeOfDay - 6.0f) * 15.0f - 90.0f; // 6AM = -90°, 12PM = 0°, 6PM = 90°
    AtmosphereSettings.SunElevation = FMath::Clamp(SunAngle, -90.0f, 90.0f);
    
    // Adjust intensity based on elevation
    if (AtmosphereSettings.SunElevation > 0)
    {
        AtmosphereSettings.SunIntensity = FMath::Lerp(2.0f, 12.0f, AtmosphereSettings.SunElevation / 90.0f);
    }
    else
    {
        AtmosphereSettings.SunIntensity = 0.1f; // Moonlight
    }
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::UpdateSunPosition()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        FRotator SunRotation;
        SunRotation.Pitch = -AtmosphereSettings.SunElevation;
        SunRotation.Yaw = AtmosphereSettings.SunAzimuth;
        SunRotation.Roll = 0.0f;
        
        SunLight->SetActorRotation(SunRotation);
        SunLight->GetLightComponent()->SetIntensity(AtmosphereSettings.SunIntensity);
        
        // Set sun color based on elevation
        FLinearColor SunColor;
        if (AtmosphereSettings.SunElevation > 30.0f)
        {
            SunColor = FLinearColor::White;
        }
        else if (AtmosphereSettings.SunElevation > 0.0f)
        {
            SunColor = FLinearColor(1.0f, 0.8f, 0.6f); // Golden
        }
        else
        {
            SunColor = FLinearColor(0.6f, 0.7f, 1.0f); // Blue moonlight
        }
        
        SunLight->GetLightComponent()->SetLightColor(SunColor);
    }
}

void AEnvArt_AtmosphereManager::UpdateFogSettings()
{
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(AtmosphereSettings.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        
        // Adjust fog height based on time of day
        float FogHeight = FMath::Lerp(200.0f, 50.0f, FMath::Abs(AtmosphereSettings.SunElevation) / 90.0f);
        HeightFog->GetComponent()->SetFogHeightFalloff(0.02f);
    }
}

void AEnvArt_AtmosphereManager::UpdateSkyLightSettings()
{
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        // Adjust sky light intensity based on sun elevation
        float SkyIntensity = FMath::Lerp(0.1f, 1.5f, (AtmosphereSettings.SunElevation + 90.0f) / 180.0f);
        SkyLightActor->GetLightComponent()->SetIntensity(SkyIntensity);
        
        // Recapture sky light for dynamic lighting
        SkyLightActor->GetLightComponent()->RecaptureSky();
    }
}

void AEnvArt_AtmosphereManager::CreateVolumetricFogZones()
{
    // Implementation for creating localized fog zones in forests/swamps
    if (UWorld* World = GetWorld())
    {
        // This would spawn additional ExponentialHeightFog actors in specific biome areas
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Creating volumetric fog zones"));
    }
}

void AEnvArt_AtmosphereManager::SpawnAtmosphericParticles()
{
    // Implementation for dust, pollen, and atmospheric particle effects
    if (UWorld* World = GetWorld())
    {
        // This would spawn Niagara particle systems for atmospheric effects
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Spawning atmospheric particles"));
    }
}