#include "EnvironmentAtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AEnvironmentAtmosphereManager::AEnvironmentAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default Cretaceous atmosphere settings
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AtmosphereSettings.SunIntensity = 4.0f;
    AtmosphereSettings.SunRotation = FRotator(-35.0f, 60.0f, 0.0f);
    AtmosphereSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    AtmosphereSettings.FogDensity = 0.015f;
    AtmosphereSettings.FogHeightFalloff = 0.15f;
    AtmosphereSettings.SkyLightColor = FLinearColor(0.4f, 0.6f, 0.9f, 1.0f);
    AtmosphereSettings.SkyLightIntensity = 1.2f;

    CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
}

void AEnvironmentAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    SetCretaceousAtmosphere();
}

void AEnvironmentAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update lighting based on current settings
    UpdateLighting();
}

void AEnvironmentAtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    FEnvArt_AtmosphereSettings NewSettings = GetTimeOfDaySettings(NewTimeOfDay);
    ApplyAtmosphereSettings(NewSettings);
}

void AEnvironmentAtmosphereManager::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& NewSettings)
{
    AtmosphereSettings = NewSettings;
    UpdateLighting();
}

void AEnvironmentAtmosphereManager::SetCretaceousAtmosphere()
{
    // Warm, tropical Cretaceous atmosphere
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AtmosphereSettings.SunIntensity = 4.5f;
    AtmosphereSettings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
    AtmosphereSettings.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    AtmosphereSettings.FogDensity = 0.02f;
    AtmosphereSettings.FogHeightFalloff = 0.2f;
    AtmosphereSettings.SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    AtmosphereSettings.SkyLightIntensity = 1.5f;
    
    UpdateLighting();
}

void AEnvironmentAtmosphereManager::SetForestAtmosphere()
{
    // Dappled forest lighting with god rays
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AtmosphereSettings.SunIntensity = 3.0f;
    AtmosphereSettings.SunRotation = FRotator(-45.0f, 30.0f, 0.0f);
    AtmosphereSettings.FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    AtmosphereSettings.FogDensity = 0.03f;
    AtmosphereSettings.FogHeightFalloff = 0.25f;
    AtmosphereSettings.SkyLightColor = FLinearColor(0.3f, 0.6f, 0.4f, 1.0f);
    AtmosphereSettings.SkyLightIntensity = 0.8f;
    
    UpdateLighting();
}

void AEnvironmentAtmosphereManager::UpdateLighting()
{
    UpdateSunPosition();
    UpdateFogSettings();
    UpdateSkyLighting();
}

void AEnvironmentAtmosphereManager::FindLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light (sun)
        for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            SunLight = *ActorItr;
            break; // Use first found
        }
        
        // Find sky light
        for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            SkyLight = *ActorItr;
            break; // Use first found
        }
        
        // Find height fog
        for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
        {
            HeightFog = *ActorItr;
            break; // Use first found
        }
    }
}

void AEnvironmentAtmosphereManager::CreateDefaultLighting()
{
    if (UWorld* World = GetWorld())
    {
        // Create directional light if not found
        if (!SunLight)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("Sun_DirectionalLight");
            SunLight = World->SpawnActor<ADirectionalLight>(SpawnParams);
            if (SunLight)
            {
                SunLight->SetActorLocation(FVector(0, 0, 1000));
                SunLight->SetActorRotation(AtmosphereSettings.SunRotation);
            }
        }
        
        // Create sky light if not found
        if (!SkyLight)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("Sky_SkyLight");
            SkyLight = World->SpawnActor<ASkyLight>(SpawnParams);
            if (SkyLight)
            {
                SkyLight->SetActorLocation(FVector(0, 0, 500));
            }
        }
        
        // Create height fog if not found
        if (!HeightFog)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("Atmosphere_HeightFog");
            HeightFog = World->SpawnActor<AExponentialHeightFog>(SpawnParams);
            if (HeightFog)
            {
                HeightFog->SetActorLocation(FVector(0, 0, 0));
            }
        }
        
        UpdateLighting();
    }
}

void AEnvironmentAtmosphereManager::UpdateSunPosition()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->SetActorRotation(AtmosphereSettings.SunRotation);
        SunLight->GetLightComponent()->SetLightColor(AtmosphereSettings.SunColor);
        SunLight->GetLightComponent()->SetIntensity(AtmosphereSettings.SunIntensity);
        SunLight->GetLightComponent()->SetCastShadows(true);
        SunLight->GetLightComponent()->SetCastVolumetricShadow(true);
    }
}

void AEnvironmentAtmosphereManager::UpdateFogSettings()
{
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        HeightFog->GetComponent()->SetFogDensity(AtmosphereSettings.FogDensity);
        HeightFog->GetComponent()->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
        HeightFog->GetComponent()->SetVolumetricFog(true);
        HeightFog->GetComponent()->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFog->GetComponent()->SetVolumetricFogAlbedo(FColor::White);
    }
}

void AEnvironmentAtmosphereManager::UpdateSkyLighting()
{
    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetLightColor(AtmosphereSettings.SkyLightColor);
        SkyLight->GetLightComponent()->SetIntensity(AtmosphereSettings.SkyLightIntensity);
        SkyLight->GetLightComponent()->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
        SkyLight->GetLightComponent()->RecaptureSky();
    }
}

FEnvArt_AtmosphereSettings AEnvironmentAtmosphereManager::GetTimeOfDaySettings(EEnvArt_TimeOfDay TimeOfDay) const
{
    FEnvArt_AtmosphereSettings Settings = AtmosphereSettings;
    
    switch (TimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            Settings.SunIntensity = 2.0f;
            Settings.SunRotation = FRotator(-80.0f, 90.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Morning:
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            Settings.SunIntensity = 3.5f;
            Settings.SunRotation = FRotator(-60.0f, 75.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Noon:
            Settings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
            Settings.SunIntensity = 5.0f;
            Settings.SunRotation = FRotator(-10.0f, 0.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            Settings.SunIntensity = 4.0f;
            Settings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
            Settings.SunIntensity = 2.5f;
            Settings.SunRotation = FRotator(-70.0f, 270.0f, 0.0f);
            Settings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Night:
            Settings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
            Settings.SunIntensity = 0.5f;
            Settings.SunRotation = FRotator(-120.0f, 0.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
            Settings.SkyLightIntensity = 0.3f;
            break;
    }
    
    return Settings;
}