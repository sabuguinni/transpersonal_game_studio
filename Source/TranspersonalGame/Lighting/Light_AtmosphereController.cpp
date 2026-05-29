#include "Light_AtmosphereController.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereController::ALight_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default Cretaceous atmospheric settings
    MiddaySettings.SunIntensity = 8.5f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
    MiddaySettings.SunTemperature = 4800.0f;
    MiddaySettings.FogDensity = 0.008f;
    MiddaySettings.FogHeightFalloff = 0.15f;
    MiddaySettings.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    MiddaySettings.RayleighScattering = 0.8f;
    MiddaySettings.MieScattering = 1.2f;

    DawnSettings = MiddaySettings;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.012f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    DuskSettings = DawnSettings;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);

    NightSettings = MiddaySettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.FogDensity = 0.015f;
    NightSettings.FogColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
}

void ALight_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingComponents();
    SetTimeOfDay(CurrentTimeOfDay);
}

void ALight_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_AtmosphereController::InitializeLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find or create directional light (sun)
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }
    else
    {
        // Spawn new directional light
        FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 1000);
        SunLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("CretaceousSun"));
        }
    }

    // Find or create sky atmosphere
    TArray<AActor*> FoundSkyAtmosphere;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundSkyAtmosphere);
    
    if (FoundSkyAtmosphere.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundSkyAtmosphere[0]);
    }
    else
    {
        SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass());
        if (SkyAtmosphere)
        {
            SkyAtmosphere->SetActorLabel(TEXT("CretaceousAtmosphere"));
        }
    }

    // Find or create height fog
    TArray<AActor*> FoundFog;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFog);
    
    if (FoundFog.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundFog[0]);
    }
    else
    {
        HeightFog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass());
        if (HeightFog)
        {
            HeightFog->SetActorLabel(TEXT("CretaceousFog"));
        }
    }

    // Find or create post process volume
    TArray<AActor*> FoundPP;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundPP);
    
    if (FoundPP.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundPP[0]);
    }

    // Create volcanic glow light if enabled
    if (bVolcanicActivity)
    {
        EnableVolcanicGlow(true);
    }
}

void ALight_AtmosphereController::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    DetermineTimeOfDayPeriod();
    UpdateSunPosition();
    InterpolateAtmosphericSettings();
}

void ALight_AtmosphereController::SetAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
        SunLight->GetLightComponent()->SetTemperature(Settings.SunTemperature);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        HeightFog->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
        HeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }

    if (SkyAtmosphere && SkyAtmosphere->GetAtmosphereComponent())
    {
        SkyAtmosphere->GetAtmosphereComponent()->SetRayleighScatteringScale(Settings.RayleighScattering);
        SkyAtmosphere->GetAtmosphereComponent()->SetMieScatteringScale(Settings.MieScattering);
    }
}

void ALight_AtmosphereController::EnableVolcanicGlow(bool bEnable)
{
    if (bEnable && !VolcanicGlowLight)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            VolcanicGlowLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), VolcanicGlowLocation, FRotator::ZeroRotator);
            if (VolcanicGlowLight)
            {
                VolcanicGlowLight->SetActorLabel(TEXT("VolcanicGlow_Distant"));
                if (VolcanicGlowLight->GetLightComponent())
                {
                    VolcanicGlowLight->GetLightComponent()->SetIntensity(VolcanicGlowIntensity);
                    VolcanicGlowLight->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.4f, 0.2f, 1.0f));
                    VolcanicGlowLight->GetLightComponent()->SetAttenuationRadius(50000.0f);
                    VolcanicGlowLight->GetLightComponent()->SetVolumetricScatteringIntensity(2.0f);
                }
            }
        }
    }
    else if (!bEnable && VolcanicGlowLight)
    {
        VolcanicGlowLight->Destroy();
        VolcanicGlowLight = nullptr;
    }
}

void ALight_AtmosphereController::UpdateSunPosition()
{
    if (!SunLight) return;

    // Calculate sun angle based on time of day
    // Cretaceous period had ~22 hour days, but we'll use 24h cycle for gameplay
    float SunAngle = (CurrentTimeOfDay - 0.25f) * 360.0f; // -90 to 270 degrees
    float SunElevation = FMath::Sin(CurrentTimeOfDay * PI) * 90.0f - 45.0f; // -45 to 45 degrees
    
    FRotator SunRotation = FRotator(SunElevation, SunAngle, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ALight_AtmosphereController::UpdateAtmosphericScattering()
{
    if (!SkyAtmosphere || !SkyAtmosphere->GetAtmosphereComponent()) return;

    // Adjust atmospheric scattering based on time of day
    float ScatteringMultiplier = FMath::Lerp(0.7f, 1.0f, FMath::Abs(FMath::Sin(CurrentTimeOfDay * PI)));
    
    SkyAtmosphere->GetAtmosphereComponent()->SetRayleighScatteringScale(MiddaySettings.RayleighScattering * ScatteringMultiplier);
    SkyAtmosphere->GetAtmosphereComponent()->SetMieScatteringScale(MiddaySettings.MieScattering * ScatteringMultiplier);
}

void ALight_AtmosphereController::UpdateFogSettings()
{
    if (!HeightFog || !HeightFog->GetComponent()) return;

    // Enable volumetric fog for Lumen
    HeightFog->GetComponent()->SetVolumetricFog(true);
    HeightFog->GetComponent()->SetVolumetricFogScatteringDistribution(0.6f);
    HeightFog->GetComponent()->SetVolumetricFogAlbedo(FLinearColor(0.95f, 0.9f, 0.8f, 1.0f));
}

void ALight_AtmosphereController::UpdateDayNightCycle(float DeltaTime)
{
    if (DayDurationMinutes <= 0.0f) return;

    float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
    CurrentTimeOfDay += TimeIncrement;
    
    if (CurrentTimeOfDay >= 1.0f)
    {
        CurrentTimeOfDay -= 1.0f; // Wrap around to next day
    }

    DetermineTimeOfDayPeriod();
    UpdateSunPosition();
    InterpolateAtmosphericSettings();
}

void ALight_AtmosphereController::DetermineTimeOfDayPeriod()
{
    if (CurrentTimeOfDay >= 0.0f && CurrentTimeOfDay < 0.15f)
    {
        CurrentPeriod = ELight_TimeOfDay::Night;
    }
    else if (CurrentTimeOfDay >= 0.15f && CurrentTimeOfDay < 0.25f)
    {
        CurrentPeriod = ELight_TimeOfDay::Dawn;
    }
    else if (CurrentTimeOfDay >= 0.25f && CurrentTimeOfDay < 0.4f)
    {
        CurrentPeriod = ELight_TimeOfDay::Morning;
    }
    else if (CurrentTimeOfDay >= 0.4f && CurrentTimeOfDay < 0.6f)
    {
        CurrentPeriod = ELight_TimeOfDay::Midday;
    }
    else if (CurrentTimeOfDay >= 0.6f && CurrentTimeOfDay < 0.75f)
    {
        CurrentPeriod = ELight_TimeOfDay::Afternoon;
    }
    else if (CurrentTimeOfDay >= 0.75f && CurrentTimeOfDay < 0.85f)
    {
        CurrentPeriod = ELight_TimeOfDay::Dusk;
    }
    else
    {
        CurrentPeriod = ELight_TimeOfDay::Night;
    }
}

void ALight_AtmosphereController::InterpolateAtmosphericSettings()
{
    FLight_AtmosphericSettings CurrentSettings;
    
    // Determine which settings to interpolate between
    if (CurrentTimeOfDay >= 0.15f && CurrentTimeOfDay < 0.4f)
    {
        // Dawn to Morning
        float Alpha = (CurrentTimeOfDay - 0.15f) / 0.25f;
        CurrentSettings = LerpAtmosphericSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 0.4f && CurrentTimeOfDay < 0.75f)
    {
        // Morning to Afternoon (stay at midday settings)
        CurrentSettings = MiddaySettings;
    }
    else if (CurrentTimeOfDay >= 0.75f && CurrentTimeOfDay < 0.9f)
    {
        // Afternoon to Dusk
        float Alpha = (CurrentTimeOfDay - 0.75f) / 0.15f;
        CurrentSettings = LerpAtmosphericSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else
    {
        // Night
        CurrentSettings = NightSettings;
    }

    SetAtmosphericSettings(CurrentSettings);
}

FLight_AtmosphericSettings ALight_AtmosphereController::LerpAtmosphericSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha)
{
    FLight_AtmosphericSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunTemperature = FMath::Lerp(A.SunTemperature, B.SunTemperature, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.RayleighScattering = FMath::Lerp(A.RayleighScattering, B.RayleighScattering, Alpha);
    Result.MieScattering = FMath::Lerp(A.MieScattering, B.MieScattering, Alpha);
    
    return Result;
}