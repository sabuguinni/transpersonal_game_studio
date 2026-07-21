#include "EnvArt_LightingController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_LightingController::AEnvArt_LightingController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    TimeOfDaySettings.SunAngle = 45.0f;
    TimeOfDaySettings.SunIntensity = 3.5f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    TimeOfDaySettings.VolumetricScatteringIntensity = 2.0f;
    TimeOfDaySettings.bCastVolumetricShadow = true;

    FogSettings.FogDensity = 0.02f;
    FogSettings.FogHeightFalloff = 0.2f;
    FogSettings.FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    FogSettings.bVolumetricFog = true;
    FogSettings.VolumetricFogScatteringDistribution = 0.2f;
    FogSettings.VolumetricFogAlbedo = 1.0f;

    AtmosphericSettings.RayleighScatteringScale = 1.0f;
    AtmosphericSettings.MieScatteringScale = 1.0f;
    AtmosphericSettings.MieAnisotropy = 0.8f;
    AtmosphericSettings.RayleighScattering = FLinearColor(0.331f, 0.558f, 1.0f, 1.0f);
    AtmosphericSettings.MieScattering = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    TimeOfDay = 16.0f; // Golden hour
    bAutoUpdateTime = false;
    TimeSpeed = 1.0f;
}

void AEnvArt_LightingController::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    SetGoldenHourLighting();
}

void AEnvArt_LightingController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }
}

void AEnvArt_LightingController::SetGoldenHourLighting()
{
    TimeOfDay = 16.0f; // 4 PM
    ApplyGoldenHourPreset();
}

void AEnvArt_LightingController::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_LightingController::UpdateDirectionalLight()
{
    if (!DirectionalLightActor)
    {
        FindLightingActors();
        return;
    }

    UDirectionalLightComponent* LightComponent = DirectionalLightActor->GetComponent<UDirectionalLightComponent>();
    if (!LightComponent)
    {
        return;
    }

    // Calculate sun rotation based on time
    FRotator SunRotation = CalculateSunRotation(TimeOfDay);
    DirectionalLightActor->SetActorRotation(SunRotation);

    // Update light properties
    LightComponent->SetIntensity(GetSunIntensityForTime(TimeOfDay));
    LightComponent->SetLightColor(GetSunColorForTime(TimeOfDay));
    LightComponent->SetVolumetricScatteringIntensity(TimeOfDaySettings.VolumetricScatteringIntensity);
    LightComponent->SetCastVolumetricShadow(TimeOfDaySettings.bCastVolumetricShadow);
}

void AEnvArt_LightingController::UpdateFogSettings()
{
    if (!FogActor)
    {
        FindLightingActors();
        return;
    }

    UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent<UExponentialHeightFogComponent>();
    if (!FogComponent)
    {
        return;
    }

    FogComponent->SetFogDensity(FogSettings.FogDensity);
    FogComponent->SetFogHeightFalloff(FogSettings.FogHeightFalloff);
    FogComponent->SetFogInscatteringColor(FogSettings.FogInscatteringColor);
    FogComponent->SetVolumetricFog(FogSettings.bVolumetricFog);
    FogComponent->SetVolumetricFogScatteringDistribution(FogSettings.VolumetricFogScatteringDistribution);
    FogComponent->SetVolumetricFogAlbedo(FogSettings.VolumetricFogAlbedo);
}

void AEnvArt_LightingController::UpdateAtmosphericSettings()
{
    if (!SkyLightActor)
    {
        FindLightingActors();
        return;
    }

    USkyLightComponent* SkyLightComponent = SkyLightActor->GetComponent<USkyLightComponent>();
    if (SkyLightComponent)
    {
        SkyLightComponent->RecaptureSky();
    }
}

void AEnvArt_LightingController::FindLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            DirectionalLightActor = Cast<ADirectionalLight>(FoundActors[0]);
        }

        // Find fog actor
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }

        // Find sky light
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }
}

void AEnvArt_LightingController::ApplyDawnPreset()
{
    TimeOfDay = 6.0f;
    TimeOfDaySettings.SunAngle = -5.0f;
    TimeOfDaySettings.SunIntensity = 1.5f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    
    FogSettings.FogDensity = 0.04f;
    FogSettings.FogInscatteringColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_LightingController::ApplyNoonPreset()
{
    TimeOfDay = 12.0f;
    TimeOfDaySettings.SunAngle = 85.0f;
    TimeOfDaySettings.SunIntensity = 5.0f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    
    FogSettings.FogDensity = 0.01f;
    FogSettings.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_LightingController::ApplyGoldenHourPreset()
{
    TimeOfDay = 16.0f;
    TimeOfDaySettings.SunAngle = 15.0f;
    TimeOfDaySettings.SunIntensity = 3.5f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    FogSettings.FogDensity = 0.02f;
    FogSettings.FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_LightingController::ApplyDuskPreset()
{
    TimeOfDay = 19.0f;
    TimeOfDaySettings.SunAngle = -10.0f;
    TimeOfDaySettings.SunIntensity = 2.0f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    
    FogSettings.FogDensity = 0.03f;
    FogSettings.FogInscatteringColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_LightingController::ApplyNightPreset()
{
    TimeOfDay = 22.0f;
    TimeOfDaySettings.SunAngle = -45.0f;
    TimeOfDaySettings.SunIntensity = 0.1f;
    TimeOfDaySettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    
    FogSettings.FogDensity = 0.025f;
    FogSettings.FogInscatteringColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    
    UpdateDirectionalLight();
    UpdateFogSettings();
}

FRotator AEnvArt_LightingController::CalculateSunRotation(float TimeHour) const
{
    // Convert time to sun angle (0-24 hours to -90 to 90 degrees)
    float SunElevation = FMath::Sin((TimeHour - 6.0f) * PI / 12.0f) * 90.0f;
    float SunAzimuth = (TimeHour - 12.0f) * 15.0f; // 15 degrees per hour
    
    return FRotator(-SunElevation, SunAzimuth, 0.0f);
}

FLinearColor AEnvArt_LightingController::GetSunColorForTime(float TimeHour) const
{
    if (TimeHour >= 5.0f && TimeHour <= 7.0f) // Dawn
    {
        return FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    }
    else if (TimeHour >= 7.0f && TimeHour <= 10.0f) // Morning
    {
        return FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    }
    else if (TimeHour >= 10.0f && TimeHour <= 14.0f) // Noon
    {
        return FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    }
    else if (TimeHour >= 14.0f && TimeHour <= 18.0f) // Golden hour
    {
        return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    }
    else if (TimeHour >= 18.0f && TimeHour <= 20.0f) // Dusk
    {
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    }
    else // Night
    {
        return FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    }
}

float AEnvArt_LightingController::GetSunIntensityForTime(float TimeHour) const
{
    if (TimeHour >= 6.0f && TimeHour <= 18.0f) // Day
    {
        float DayProgress = (TimeHour - 6.0f) / 12.0f;
        float IntensityCurve = FMath::Sin(DayProgress * PI);
        return FMath::Lerp(1.0f, 5.0f, IntensityCurve);
    }
    else // Night
    {
        return 0.1f;
    }
}

void AEnvArt_LightingController::RefreshLightingReferences()
{
    FindLightingActors();
}

void AEnvArt_LightingController::ApplyCurrentSettings()
{
    UpdateDirectionalLight();
    UpdateFogSettings();
    UpdateAtmosphericSettings();
}

void AEnvArt_LightingController::UpdateTimeOfDay(float DeltaTime)
{
    TimeOfDay += (DeltaTime * TimeSpeed) / 3600.0f; // Convert seconds to hours
    
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_LightingController::InterpolateLightingSettings(float Alpha)
{
    // This function can be used for smooth transitions between lighting presets
    // Implementation would depend on specific transition requirements
}

FLinearColor AEnvArt_LightingController::InterpolateColor(const FLinearColor& ColorA, const FLinearColor& ColorB, float Alpha) const
{
    return FLinearColor(
        FMath::Lerp(ColorA.R, ColorB.R, Alpha),
        FMath::Lerp(ColorA.G, ColorB.G, Alpha),
        FMath::Lerp(ColorA.B, ColorB.B, Alpha),
        FMath::Lerp(ColorA.A, ColorB.A, Alpha)
    );
}