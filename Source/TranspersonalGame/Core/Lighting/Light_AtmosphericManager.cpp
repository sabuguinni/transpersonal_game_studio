#include "Light_AtmosphericManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    bEnableDayNightCycle = true;
    DayDurationMinutes = 24.0f;
    CurrentTimeOfDay = 12.0f;

    SunLightActor = nullptr;
    SkyLightActor = nullptr;
    FogActor = nullptr;
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTimeOfDayPresets();
    InitializeAtmosphericActors();
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Progress time of day
        float TimeIncrement = (DeltaTime / 60.0f) / DayDurationMinutes * 24.0f;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }

        UpdateSunPosition();
        UpdateAtmosphericLighting();
    }
}

void ALight_AtmosphericManager::InitializeTimeOfDayPresets()
{
    // Dawn (5:00 AM)
    FLight_AtmosphericSettings DawnSettings;
    DawnSettings.SunIntensity = 2.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.SkyColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogInscatteringColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    // Morning (8:00 AM)
    FLight_AtmosphericSettings MorningSettings;
    MorningSettings.SunIntensity = 4.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunRotation = FRotator(-30.0f, 110.0f, 0.0f);
    MorningSettings.SkyLightIntensity = 1.0f;
    MorningSettings.SkyColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
    MorningSettings.FogDensity = 0.025f;
    MorningSettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Morning, MorningSettings);

    // Midday (12:00 PM)
    FLight_AtmosphericSettings MiddaySettings;
    MiddaySettings.SunIntensity = 6.5f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.92f, 0.71f, 1.0f);
    MiddaySettings.SunRotation = FRotator(-60.0f, 135.0f, 0.0f);
    MiddaySettings.SkyLightIntensity = 1.2f;
    MiddaySettings.SkyColor = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogInscatteringColor = FLinearColor(1.0f, 0.86f, 0.59f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);

    // Afternoon (4:00 PM)
    FLight_AtmosphericSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 5.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.SunRotation = FRotator(-40.0f, 200.0f, 0.0f);
    AfternoonSettings.SkyLightIntensity = 1.1f;
    AfternoonSettings.SkyColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogInscatteringColor = FLinearColor(1.0f, 0.75f, 0.5f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk (7:00 PM)
    FLight_AtmosphericSettings DuskSettings;
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunRotation = FRotator(-5.0f, 250.0f, 0.0f);
    DuskSettings.SkyLightIntensity = 0.7f;
    DuskSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f);
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogInscatteringColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    // Night (10:00 PM)
    FLight_AtmosphericSettings NightSettings;
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunRotation = FRotator(20.0f, 270.0f, 0.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.SkyColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogInscatteringColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Night, NightSettings);
}

void ALight_AtmosphericManager::InitializeAtmosphericActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find existing atmospheric actors
    TArray<AActor*> FoundActors;
    
    // Find DirectionalLight
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLightActor = Cast<ADirectionalLight>(FoundActors[0]);
        if (SunLightActor)
        {
            SunLightActor->SetActorLabel(TEXT("CretaceousSun"));
        }
    }

    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        if (SkyLightActor)
        {
            SkyLightActor->SetActorLabel(TEXT("CretaceousAmbient"));
        }
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        if (FogActor)
        {
            FogActor->SetActorLabel(TEXT("CretaceousAtmosphere"));
        }
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::SetAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    if (SunLightActor && SunLightActor->GetLightComponent())
    {
        SunLightActor->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLightActor->GetLightComponent()->SetLightColor(Settings.SunColor);
        SunLightActor->SetActorRotation(Settings.SunRotation);
    }

    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->SetIntensity(Settings.SkyLightIntensity);
        SkyLightActor->GetLightComponent()->SetLightColor(Settings.SkyColor);
    }

    if (FogActor && FogActor->GetComponent())
    {
        FogActor->GetComponent()->SetFogDensity(Settings.FogDensity);
        FogActor->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogActor->GetComponent()->SetFogInscatteringColor(Settings.FogInscatteringColor);
        FogActor->GetComponent()->SetVolumetricFog(Settings.bVolumetricFog);
        FogActor->GetComponent()->SetVolumetricFogScatteringDistribution(Settings.VolumetricScatteringDistribution);
        FogActor->GetComponent()->SetVolumetricFogAlbedo(Settings.VolumetricAlbedo);
    }
}

void ALight_AtmosphericManager::ApplyPresetForTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    if (TimeOfDaySettings.Contains(TimeOfDay))
    {
        SetAtmosphericSettings(TimeOfDaySettings[TimeOfDay]);
    }
}

void ALight_AtmosphericManager::UpdateAtmosphericLighting()
{
    FLight_AtmosphericSettings InterpolatedSettings = GetInterpolatedSettings();
    SetAtmosphericSettings(InterpolatedSettings);
}

void ALight_AtmosphericManager::UpdateSunPosition()
{
    if (!SunLightActor) return;

    // Convert time of day to sun angle
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunAzimuth = (CurrentTimeOfDay - 6.0f) * 15.0f + 90.0f;

    FRotator SunRotation = FRotator(-SunElevation, SunAzimuth, 0.0f);
    SunLightActor->SetActorRotation(SunRotation);
}

ELight_TimeOfDay ALight_AtmosphericManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 4.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 21.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

FLight_AtmosphericSettings ALight_AtmosphericManager::GetInterpolatedSettings() const
{
    ELight_TimeOfDay CurrentEnum = GetCurrentTimeOfDayEnum();
    
    // For now, return the exact preset. In future, could interpolate between adjacent time periods
    if (TimeOfDaySettings.Contains(CurrentEnum))
    {
        return TimeOfDaySettings[CurrentEnum];
    }
    
    // Fallback to midday settings
    return TimeOfDaySettings.Contains(ELight_TimeOfDay::Midday) ? 
           TimeOfDaySettings[ELight_TimeOfDay::Midday] : 
           FLight_AtmosphericSettings();
}