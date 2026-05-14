#include "AtmosphericLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void UAtmosphericLightingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Find atmospheric actors in the world
    FindAtmosphericActors();
    
    // Apply default Cretaceous atmosphere
    ApplyCretaceousAtmosphere();
    
    // Start day/night cycle if enabled
    if (bEnableDayNightCycle)
    {
        GetWorld()->GetTimerManager().SetTimer(
            DayNightTimer,
            [this]()
            {
                CurrentDayProgress += 0.01f; // Increment by 1% each update
                if (CurrentDayProgress > 1.0f)
                {
                    CurrentDayProgress = 0.0f;
                }
                UpdateDayNightCycle(CurrentDayProgress);
            },
            (DayDurationMinutes * 60.0f) / 100.0f, // Update interval for 1% progress
            true
        );
    }
}

void UAtmosphericLightingManager::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DayNightTimer);
    }
    Super::Deinitialize();
}

void UAtmosphericLightingManager::FindAtmosphericActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break; // Use first found
    }

    // Find sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphere = *ActorItr;
        break;
    }

    // Find height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        HeightFog = *ActorItr;
        break;
    }

    // Find volumetric clouds
    for (TActorIterator<AVolumetricCloud> ActorItr(World); ActorItr; ++ActorItr)
    {
        VolumetricClouds = *ActorItr;
        break;
    }
}

void UAtmosphericLightingManager::ApplyCretaceousAtmosphere()
{
    // Set default Cretaceous period settings
    CurrentSettings.SunIntensity = 5.0f;
    CurrentSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    CurrentSettings.SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
    CurrentSettings.RayleighScattering = 0.0331f;
    CurrentSettings.FogDensity = 0.02f;
    CurrentSettings.FogHeightFalloff = 0.2f;
    CurrentSettings.FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);

    ApplySettingsToActors();
}

void UAtmosphericLightingManager::SetTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    CurrentSettings = GetSettingsForTimeOfDay(TimeOfDay);
    ApplySettingsToActors();
}

void UAtmosphericLightingManager::UpdateDayNightCycle(float DayProgress)
{
    CurrentDayProgress = FMath::Clamp(DayProgress, 0.0f, 1.0f);
    
    // Calculate sun angle based on day progress
    float SunAngle = (CurrentDayProgress * 360.0f) - 90.0f; // -90 to 270 degrees
    CurrentSettings.SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    
    // Adjust intensity based on sun height
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle + 90.0f));
    CurrentSettings.SunIntensity = FMath::Max(0.1f, SunHeight * 5.0f);
    
    // Adjust color temperature
    if (SunHeight > 0.0f)
    {
        // Daytime - warm tropical sun
        float ColorTemp = FMath::Lerp(0.8f, 1.0f, SunHeight);
        CurrentSettings.SunColor = FLinearColor(1.0f, ColorTemp * 0.95f, ColorTemp * 0.9f, 1.0f);
    }
    else
    {
        // Nighttime - cool moonlight
        CurrentSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
        CurrentSettings.SunIntensity = 0.1f;
    }
    
    ApplySettingsToActors();
}

void UAtmosphericLightingManager::SetAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    CurrentSettings = Settings;
    ApplySettingsToActors();
}

void UAtmosphericLightingManager::ApplySettingsToActors()
{
    // Apply sun settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CurrentSettings.SunIntensity);
        LightComp->SetLightColor(CurrentSettings.SunColor);
        SunLight->SetActorRotation(CurrentSettings.SunRotation);
    }

    // Apply sky atmosphere settings
    if (SkyAtmosphere)
    {
        USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent();
        if (SkyComp)
        {
            SkyComp->RayleighScatteringScale = CurrentSettings.RayleighScattering;
        }
    }

    // Apply fog settings
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->FogDensity = CurrentSettings.FogDensity;
            FogComp->FogHeightFalloff = CurrentSettings.FogHeightFalloff;
            FogComp->FogInscatteringColor = CurrentSettings.FogColor;
        }
    }

    // Hide volumetric clouds for clear Cretaceous sky
    if (VolumetricClouds)
    {
        VolumetricClouds->SetActorHiddenInGame(true);
    }
}

FLight_AtmosphericSettings UAtmosphericLightingManager::GetSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    FLight_AtmosphericSettings Settings = CurrentSettings;
    
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            Settings.SunIntensity = 2.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            Settings.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
            break;
            
        case ELight_TimeOfDay::Morning:
            Settings.SunIntensity = 4.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
            Settings.SunRotation = FRotator(-60.0f, 0.0f, 0.0f);
            break;
            
        case ELight_TimeOfDay::Midday:
            Settings.SunIntensity = 5.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
            Settings.SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
            break;
            
        case ELight_TimeOfDay::Afternoon:
            Settings.SunIntensity = 4.5f;
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
            Settings.SunRotation = FRotator(-30.0f, 0.0f, 0.0f);
            break;
            
        case ELight_TimeOfDay::Dusk:
            Settings.SunIntensity = 2.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
            Settings.SunRotation = FRotator(-10.0f, 0.0f, 0.0f);
            break;
            
        case ELight_TimeOfDay::Night:
            Settings.SunIntensity = 0.1f;
            Settings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
            Settings.SunRotation = FRotator(30.0f, 0.0f, 0.0f);
            break;
    }
    
    return Settings;
}