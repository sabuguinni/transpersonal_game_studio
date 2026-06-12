#include "Light_AtmosphericSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create components
    DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
    RootComponent = DirectionalLight;

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));

    // Initialize default settings for Cretaceous period
    MiddaySettings.SunIntensity = 5.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.Temperature = 5500.0f;
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogHeightFalloff = 0.2f;
    MiddaySettings.FogInscatteringColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.BloomIntensity = 0.675f;

    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.Temperature = 4500.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogHeightFalloff = 0.15f;
    DawnSettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.BloomIntensity = 0.8f;

    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DuskSettings.Temperature = 3500.0f;
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogHeightFalloff = 0.18f;
    DuskSettings.FogInscatteringColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DuskSettings.BloomIntensity = 0.9f;

    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightSettings.Temperature = 8000.0f;
    NightSettings.FogDensity = 0.01f;
    NightSettings.FogHeightFalloff = 0.25f;
    NightSettings.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightSettings.BloomIntensity = 0.5f;

    CurrentTimeOfDay = 12.0f;
    TimeOfDaySpeed = 1.0f;
    bEnableDayNightCycle = true;
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndConfigureWorldActors();
    SetCretaceousAtmosphere();
    EnableLumenGlobalIllumination();
    ConfigureVolumetricFog();
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        CurrentTimeOfDay += TimeOfDaySpeed * DeltaTime / 60.0f; // Convert to minutes
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay = 0.0f;
        }
        UpdateLighting();
    }
}

void ALight_AtmosphericSystem::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateLighting();
}

void ALight_AtmosphericSystem::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    if (DirectionalLight)
    {
        DirectionalLight->SetIntensity(Settings.SunIntensity);
        DirectionalLight->SetLightColor(Settings.SunColor);
        DirectionalLight->SetTemperature(Settings.Temperature);
    }

    if (WorldDirectionalLight && WorldDirectionalLight->GetLightComponent())
    {
        WorldDirectionalLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        WorldDirectionalLight->GetLightComponent()->SetLightColor(Settings.SunColor);
        WorldDirectionalLight->GetLightComponent()->SetTemperature(Settings.Temperature);
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(Settings.FogInscatteringColor);
    }

    if (WorldHeightFog && WorldHeightFog->GetComponent())
    {
        WorldHeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        WorldHeightFog->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
        WorldHeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogInscatteringColor);
    }

    if (PostProcess)
    {
        PostProcess->Settings.BloomIntensity = Settings.BloomIntensity;
        PostProcess->Settings.bOverride_BloomIntensity = true;
    }

    if (WorldPostProcessVolume && WorldPostProcessVolume->Settings)
    {
        WorldPostProcessVolume->Settings->BloomIntensity = Settings.BloomIntensity;
        WorldPostProcessVolume->Settings->bOverride_BloomIntensity = true;
    }
}

ELight_TimeOfDay ALight_AtmosphericSystem::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 8.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 12.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 12.0f && CurrentTimeOfDay < 16.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 19.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 22.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphericSystem::SetCretaceousAtmosphere()
{
    ApplyAtmosphericSettings(MiddaySettings);
    
    // Set sun angle for Cretaceous period (higher angle due to different axial tilt)
    if (DirectionalLight)
    {
        FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
        DirectionalLight->SetWorldRotation(SunRotation);
    }

    if (WorldDirectionalLight)
    {
        FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
        WorldDirectionalLight->SetActorRotation(SunRotation);
    }
}

void ALight_AtmosphericSystem::EnableLumenGlobalIllumination()
{
    if (PostProcess)
    {
        PostProcess->Settings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
        PostProcess->Settings.ReflectionMethod = EReflectionMethod::Lumen;
        PostProcess->Settings.bOverride_DynamicGlobalIlluminationMethod = true;
        PostProcess->Settings.bOverride_ReflectionMethod = true;
    }

    if (WorldPostProcessVolume && WorldPostProcessVolume->Settings)
    {
        WorldPostProcessVolume->Settings->DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
        WorldPostProcessVolume->Settings->ReflectionMethod = EReflectionMethod::Lumen;
        WorldPostProcessVolume->Settings->bOverride_DynamicGlobalIlluminationMethod = true;
        WorldPostProcessVolume->Settings->bOverride_ReflectionMethod = true;
    }
}

void ALight_AtmosphericSystem::ConfigureVolumetricFog()
{
    if (HeightFog)
    {
        HeightFog->SetVolumetricFog(true);
        HeightFog->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFog->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
        HeightFog->SetVolumetricFogEmissive(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
        HeightFog->SetVolumetricFogExtinctionScale(1.0f);
    }

    if (WorldHeightFog && WorldHeightFog->GetComponent())
    {
        WorldHeightFog->GetComponent()->SetVolumetricFog(true);
        WorldHeightFog->GetComponent()->SetVolumetricFogScatteringDistribution(0.2f);
        WorldHeightFog->GetComponent()->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
        WorldHeightFog->GetComponent()->SetVolumetricFogEmissive(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
        WorldHeightFog->GetComponent()->SetVolumetricFogExtinctionScale(1.0f);
    }
}

void ALight_AtmosphericSystem::UpdateLighting()
{
    ELight_TimeOfDay CurrentPeriod = GetCurrentTimeOfDayEnum();
    FLight_AtmosphericSettings InterpolatedSettings;

    // Determine interpolation based on current time
    float Alpha = 0.0f;
    FLight_AtmosphericSettings SettingsA, SettingsB;

    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 12.0f)
    {
        // Dawn to Midday
        Alpha = (CurrentTimeOfDay - 5.0f) / 7.0f;
        SettingsA = DawnSettings;
        SettingsB = MiddaySettings;
    }
    else if (CurrentTimeOfDay >= 12.0f && CurrentTimeOfDay < 19.0f)
    {
        // Midday to Dusk
        Alpha = (CurrentTimeOfDay - 12.0f) / 7.0f;
        SettingsA = MiddaySettings;
        SettingsB = DuskSettings;
    }
    else
    {
        // Dusk to Dawn (through night)
        if (CurrentTimeOfDay >= 19.0f)
        {
            Alpha = (CurrentTimeOfDay - 19.0f) / 10.0f;
        }
        else
        {
            Alpha = (CurrentTimeOfDay + 5.0f) / 10.0f;
        }
        SettingsA = DuskSettings;
        SettingsB = NightSettings;
    }

    InterpolateSettings(SettingsA, SettingsB, Alpha, InterpolatedSettings);
    ApplyAtmosphericSettings(InterpolatedSettings);

    // Update sun rotation based on time
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
    FRotator SunRotation = FRotator(SunAngle, 45.0f, 0.0f);
    
    if (DirectionalLight)
    {
        DirectionalLight->SetWorldRotation(SunRotation);
    }

    if (WorldDirectionalLight)
    {
        WorldDirectionalLight->SetActorRotation(SunRotation);
    }
}

void ALight_AtmosphericSystem::InterpolateSettings(const FLight_AtmosphericSettings& SettingsA, const FLight_AtmosphericSettings& SettingsB, float Alpha, FLight_AtmosphericSettings& OutSettings)
{
    OutSettings.SunIntensity = FMath::Lerp(SettingsA.SunIntensity, SettingsB.SunIntensity, Alpha);
    OutSettings.SunColor = FMath::Lerp(SettingsA.SunColor, SettingsB.SunColor, Alpha);
    OutSettings.Temperature = FMath::Lerp(SettingsA.Temperature, SettingsB.Temperature, Alpha);
    OutSettings.FogDensity = FMath::Lerp(SettingsA.FogDensity, SettingsB.FogDensity, Alpha);
    OutSettings.FogHeightFalloff = FMath::Lerp(SettingsA.FogHeightFalloff, SettingsB.FogHeightFalloff, Alpha);
    OutSettings.FogInscatteringColor = FMath::Lerp(SettingsA.FogInscatteringColor, SettingsB.FogInscatteringColor, Alpha);
    OutSettings.BloomIntensity = FMath::Lerp(SettingsA.BloomIntensity, SettingsB.BloomIntensity, Alpha);
}

void ALight_AtmosphericSystem::FindAndConfigureWorldActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find existing world actors
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        WorldDirectionalLight = *ActorItr;
        break;
    }

    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        WorldSkyAtmosphere = *ActorItr;
        break;
    }

    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        WorldHeightFog = *ActorItr;
        break;
    }

    for (TActorIterator<APostProcessVolume> ActorItr(World); ActorItr; ++ActorItr)
    {
        WorldPostProcessVolume = *ActorItr;
        if (WorldPostProcessVolume->bUnbound)
        {
            break; // Use the unbound post-process volume
        }
    }
}