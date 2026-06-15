#include "Light_CinematicLightingManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SpotLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "UObject/ConstructorHelpers.h"

ALight_CinematicLightingManager::ALight_CinematicLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create main sun light component
    MainSunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MainSunLight"));
    MainSunLight->SetupAttachment(RootComponent);
    MainSunLight->SetIntensity(8.0f);
    MainSunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f));
    MainSunLight->SetCastVolumetricShadow(true);
    MainSunLight->SetVolumetricScatteringIntensity(1.8f);
    MainSunLight->SetLightShaftOcclusion(true);
    MainSunLight->SetLightShaftBloomScale(0.3f);
    MainSunLight->SetLightShaftBloomThreshold(0.8f);

    // Create rim light component
    RimLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RimLight"));
    RimLight->SetupAttachment(RootComponent);
    RimLight->SetIntensity(12.0f);
    RimLight->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
    RimLight->SetInnerConeAngle(15.0f);
    RimLight->SetOuterConeAngle(45.0f);
    RimLight->SetAttenuationRadius(5000.0f);
    RimLight->SetCastVolumetricShadow(true);
    RimLight->SetVolumetricScatteringIntensity(2.0f);

    // Create atmospheric fog component
    AtmosphericFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphericFog"));
    AtmosphericFog->SetupAttachment(RootComponent);
    AtmosphericFog->SetFogDensity(0.008f);
    AtmosphericFog->SetFogHeightFalloff(0.1f);
    AtmosphericFog->SetVolumetricFog(true);
    AtmosphericFog->SetVolumetricFogScatteringDistribution(0.6f);
    AtmosphericFog->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.85f, 0.7f, 1.0f));
    AtmosphericFog->SetVolumetricFogEmissive(FLinearColor(0.02f, 0.015f, 0.01f, 1.0f));
    AtmosphericFog->SetVolumetricFogExtinctionScale(1.2f);

    // Initialize default values
    CurrentTimeOfDay = ELight_TimeOfDay::Noon;
    CurrentWeather = ELight_WeatherType::Clear;
    DayDurationMinutes = 20.0f;
    bAutoProgressTime = true;
    bCinematicMode = false;
    CinematicTransitionDuration = 2.0f;
    
    // Initialize state
    CurrentTimeProgress = 0.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    TransitionDuration = 0.0f;
    MainPostProcessVolume = nullptr;
}

void ALight_CinematicLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingPresets();
    FindOrCreatePostProcessVolume();
    SetupAtmosphericComponents();
    
    // Apply initial lighting preset
    if (TimeOfDayPresets.Contains(CurrentTimeOfDay))
    {
        ApplyLightingPreset(TimeOfDayPresets[CurrentTimeOfDay]);
    }
}

void ALight_CinematicLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoProgressTime && !bCinematicMode)
    {
        UpdateTimeProgression(DeltaTime);
    }
    
    if (bIsTransitioning)
    {
        UpdateLightingTransition(DeltaTime);
    }
}

void ALight_CinematicLightingManager::InitializeLightingPresets()
{
    // Dawn preset
    FLight_LightingPreset DawnPreset;
    DawnPreset.SunIntensity = 3.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnPreset.SunRotation = FRotator(-10.0f, 75.0f, 0.0f);
    DawnPreset.FogDensity = 0.015f;
    DawnPreset.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DawnPreset.VolumetricScattering = 2.5f;
    DawnPreset.WhiteTemperature = 3200.0f;
    DawnPreset.BloomIntensity = 1.2f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dawn, DawnPreset);

    // Morning preset
    FLight_LightingPreset MorningPreset;
    MorningPreset.SunIntensity = 6.0f;
    MorningPreset.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningPreset.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
    MorningPreset.FogDensity = 0.010f;
    MorningPreset.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    MorningPreset.VolumetricScattering = 2.0f;
    MorningPreset.WhiteTemperature = 5500.0f;
    MorningPreset.BloomIntensity = 0.9f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Morning, MorningPreset);

    // Noon preset
    FLight_LightingPreset NoonPreset;
    NoonPreset.SunIntensity = 10.0f;
    NoonPreset.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    NoonPreset.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    NoonPreset.FogDensity = 0.005f;
    NoonPreset.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    NoonPreset.VolumetricScattering = 1.5f;
    NoonPreset.WhiteTemperature = 6500.0f;
    NoonPreset.BloomIntensity = 0.6f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Noon, NoonPreset);

    // Afternoon preset
    FLight_LightingPreset AfternoonPreset;
    AfternoonPreset.SunIntensity = 8.0f;
    AfternoonPreset.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonPreset.SunRotation = FRotator(-45.0f, -30.0f, 0.0f);
    AfternoonPreset.FogDensity = 0.008f;
    AfternoonPreset.FogColor = FLinearColor(0.9f, 0.75f, 0.5f, 1.0f);
    AfternoonPreset.VolumetricScattering = 1.8f;
    AfternoonPreset.WhiteTemperature = 5800.0f;
    AfternoonPreset.BloomIntensity = 0.8f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Afternoon, AfternoonPreset);

    // Dusk preset
    FLight_LightingPreset DuskPreset;
    DuskPreset.SunIntensity = 4.0f;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskPreset.SunRotation = FRotator(-5.0f, -75.0f, 0.0f);
    DuskPreset.FogDensity = 0.020f;
    DuskPreset.FogColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    DuskPreset.VolumetricScattering = 3.0f;
    DuskPreset.WhiteTemperature = 2800.0f;
    DuskPreset.BloomIntensity = 1.5f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dusk, DuskPreset);

    // Night preset
    FLight_LightingPreset NightPreset;
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SunColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f);
    NightPreset.SunRotation = FRotator(30.0f, 0.0f, 0.0f);
    NightPreset.FogDensity = 0.012f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightPreset.VolumetricScattering = 1.0f;
    NightPreset.WhiteTemperature = 4000.0f;
    NightPreset.BloomIntensity = 0.4f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Night, NightPreset);

    // Weather presets
    FLight_LightingPreset ClearWeather;
    ClearWeather.FogDensity = 0.005f;
    ClearWeather.VolumetricScattering = 1.5f;
    WeatherPresets.Add(ELight_WeatherType::Clear, ClearWeather);

    FLight_LightingPreset OvercastWeather;
    OvercastWeather.SunIntensity = 4.0f;
    OvercastWeather.FogDensity = 0.015f;
    OvercastWeather.VolumetricScattering = 0.8f;
    WeatherPresets.Add(ELight_WeatherType::Overcast, OvercastWeather);

    FLight_LightingPreset FoggyWeather;
    FoggyWeather.SunIntensity = 2.0f;
    FoggyWeather.FogDensity = 0.050f;
    FoggyWeather.VolumetricScattering = 0.5f;
    WeatherPresets.Add(ELight_WeatherType::Foggy, FoggyWeather);
}

void ALight_CinematicLightingManager::UpdateTimeProgression(float DeltaTime)
{
    if (DayDurationMinutes <= 0.0f) return;

    float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
    CurrentTimeProgress += TimeIncrement;

    if (CurrentTimeProgress >= 1.0f)
    {
        CurrentTimeProgress = 0.0f;
        
        // Progress to next time of day
        int32 CurrentIndex = static_cast<int32>(CurrentTimeOfDay);
        CurrentIndex = (CurrentIndex + 1) % 6; // 6 time periods
        CurrentTimeOfDay = static_cast<ELight_TimeOfDay>(CurrentIndex);
        
        if (TimeOfDayPresets.Contains(CurrentTimeOfDay))
        {
            SetTimeOfDay(CurrentTimeOfDay, false);
        }
    }
}

void ALight_CinematicLightingManager::UpdateLightingTransition(float DeltaTime)
{
    if (!bIsTransitioning || TransitionDuration <= 0.0f) return;

    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
    }

    FLight_LightingPreset InterpolatedPreset = InterpolateLightingPresets(
        TransitionStartPreset, 
        TransitionTargetPreset, 
        TransitionProgress
    );
    
    ApplyPresetToComponents(InterpolatedPreset);
}

void ALight_CinematicLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay, bool bInstant)
{
    if (!TimeOfDayPresets.Contains(NewTimeOfDay)) return;

    CurrentTimeOfDay = NewTimeOfDay;
    FLight_LightingPreset TargetPreset = TimeOfDayPresets[NewTimeOfDay];

    if (bInstant)
    {
        ApplyLightingPreset(TargetPreset, 0.0f);
    }
    else
    {
        ApplyLightingPreset(TargetPreset, 3.0f);
    }
}

void ALight_CinematicLightingManager::SetWeather(ELight_WeatherType NewWeather, bool bInstant)
{
    if (!WeatherPresets.Contains(NewWeather)) return;

    CurrentWeather = NewWeather;
    FLight_LightingPreset WeatherModifier = WeatherPresets[NewWeather];
    
    // Combine current time of day with weather modifier
    FLight_LightingPreset CombinedPreset = TimeOfDayPresets[CurrentTimeOfDay];
    CombinedPreset.SunIntensity *= WeatherModifier.SunIntensity / 8.0f; // Normalize
    CombinedPreset.FogDensity = WeatherModifier.FogDensity;
    CombinedPreset.VolumetricScattering = WeatherModifier.VolumetricScattering;

    if (bInstant)
    {
        ApplyLightingPreset(CombinedPreset, 0.0f);
    }
    else
    {
        ApplyLightingPreset(CombinedPreset, 2.0f);
    }
}

void ALight_CinematicLightingManager::EnableCinematicMode(const FLight_LightingPreset& Preset, float TransitionTime)
{
    bCinematicMode = true;
    CinematicPreset = Preset;
    ApplyLightingPreset(Preset, TransitionTime);
}

void ALight_CinematicLightingManager::DisableCinematicMode(float TransitionTime)
{
    bCinematicMode = false;
    
    if (TimeOfDayPresets.Contains(CurrentTimeOfDay))
    {
        ApplyLightingPreset(TimeOfDayPresets[CurrentTimeOfDay], TransitionTime);
    }
}

void ALight_CinematicLightingManager::ApplyLightingPreset(const FLight_LightingPreset& Preset, float TransitionTime)
{
    if (TransitionTime <= 0.0f)
    {
        ApplyPresetToComponents(Preset);
        bIsTransitioning = false;
    }
    else
    {
        TransitionStartPreset = GetCurrentLightingSettings();
        TransitionTargetPreset = Preset;
        TransitionDuration = TransitionTime;
        TransitionProgress = 0.0f;
        bIsTransitioning = true;
    }
}

FLight_LightingPreset ALight_CinematicLightingManager::GetCurrentLightingSettings() const
{
    FLight_LightingPreset CurrentSettings;
    
    if (MainSunLight)
    {
        CurrentSettings.SunIntensity = MainSunLight->Intensity;
        CurrentSettings.SunColor = MainSunLight->GetLightColor();
        CurrentSettings.SunRotation = GetActorRotation();
    }
    
    if (AtmosphericFog)
    {
        CurrentSettings.FogDensity = AtmosphericFog->FogDensity;
        CurrentSettings.VolumetricScattering = MainSunLight ? MainSunLight->VolumetricScatteringIntensity : 1.0f;
    }
    
    return CurrentSettings;
}

void ALight_CinematicLightingManager::ApplyPresetToComponents(const FLight_LightingPreset& Preset)
{
    if (MainSunLight)
    {
        MainSunLight->SetIntensity(Preset.SunIntensity);
        MainSunLight->SetLightColor(Preset.SunColor);
        MainSunLight->SetVolumetricScatteringIntensity(Preset.VolumetricScattering);
        SetActorRotation(Preset.SunRotation);
    }
    
    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogDensity(Preset.FogDensity);
        AtmosphericFog->SetVolumetricFogAlbedo(Preset.FogColor);
    }
    
    UpdatePostProcessSettings(Preset.WhiteTemperature, Preset.BloomIntensity, 1.0f);
}

FLight_LightingPreset ALight_CinematicLightingManager::InterpolateLightingPresets(
    const FLight_LightingPreset& A, 
    const FLight_LightingPreset& B, 
    float Alpha) const
{
    FLight_LightingPreset Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.VolumetricScattering = FMath::Lerp(A.VolumetricScattering, B.VolumetricScattering, Alpha);
    Result.WhiteTemperature = FMath::Lerp(A.WhiteTemperature, B.WhiteTemperature, Alpha);
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, Alpha);
    
    return Result;
}

void ALight_CinematicLightingManager::SetVolumetricFogDensity(float Density)
{
    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogDensity(Density);
    }
}

void ALight_CinematicLightingManager::SetAtmosphericScattering(float ScatteringIntensity)
{
    if (MainSunLight)
    {
        MainSunLight->SetVolumetricScatteringIntensity(ScatteringIntensity);
    }
}

void ALight_CinematicLightingManager::UpdatePostProcessSettings(float WhiteTemp, float BloomIntensity, float Saturation)
{
    if (MainPostProcessVolume && MainPostProcessVolume->Settings.bOverride_WhiteTemp)
    {
        MainPostProcessVolume->Settings.WhiteTemp = WhiteTemp;
        MainPostProcessVolume->Settings.BloomIntensity = BloomIntensity;
        MainPostProcessVolume->Settings.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.0f);
    }
}

void ALight_CinematicLightingManager::FindOrCreatePostProcessVolume()
{
    // Try to find existing post process volume
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        MainPostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
    else
    {
        // Create new post process volume
        MainPostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
        if (MainPostProcessVolume)
        {
            MainPostProcessVolume->bUnbound = true;
            MainPostProcessVolume->Priority = 1.0f;
        }
    }
}

void ALight_CinematicLightingManager::SetupAtmosphericComponents()
{
    // Additional atmospheric setup if needed
    if (AtmosphericFog)
    {
        AtmosphericFog->SetVolumetricFog(true);
        AtmosphericFog->SetVolumetricFogScatteringDistribution(0.6f);
        AtmosphericFog->SetVolumetricFogExtinctionScale(1.2f);
    }
}