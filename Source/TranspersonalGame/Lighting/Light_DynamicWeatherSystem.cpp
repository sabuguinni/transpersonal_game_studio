#include "Light_DynamicWeatherSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ULight_DynamicWeatherSystem::ULight_DynamicWeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentWeatherState = ELight_WeatherState::Clear;
    TargetWeatherState = ELight_WeatherState::Clear;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    TransitionDuration = 5.0f;

    bDynamicWeatherEnabled = true;
    MinWeatherInterval = 300.0f; // 5 minutes
    MaxWeatherInterval = 900.0f; // 15 minutes

    CurrentTimeOfDay = 12.0f; // Noon
    DayDurationMinutes = 24.0f; // 24 real minutes = 1 game day
    bTimeProgression = true;

    SunLight = nullptr;
    AtmosphericFog = nullptr;
    PostProcessVolume = nullptr;
}

void ULight_DynamicWeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeWeatherPresets();
    FindLightingComponents();
    SetWeatherState(CurrentWeatherState);
    
    if (bDynamicWeatherEnabled)
    {
        ScheduleNextWeatherChange();
    }
}

void ULight_DynamicWeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update time progression
    if (bTimeProgression)
    {
        UpdateTimeOfDay(DeltaTime);
        UpdateSunPosition();
    }

    // Handle weather transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentWeatherState = TargetWeatherState;
            OnWeatherTransitionComplete();
        }

        // Lerp between current and target weather settings
        const FLight_WeatherSettings* CurrentSettings = WeatherPresets.Find(CurrentWeatherState);
        const FLight_WeatherSettings* TargetSettings = WeatherPresets.Find(TargetWeatherState);
        
        if (CurrentSettings && TargetSettings)
        {
            FLight_WeatherSettings LerpedSettings = LerpWeatherSettings(*CurrentSettings, *TargetSettings, TransitionProgress);
            ApplyWeatherSettings(LerpedSettings);
        }
    }
}

void ULight_DynamicWeatherSystem::SetWeatherState(ELight_WeatherState NewState)
{
    CurrentWeatherState = NewState;
    TargetWeatherState = NewState;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;

    const FLight_WeatherSettings* Settings = WeatherPresets.Find(NewState);
    if (Settings)
    {
        ApplyWeatherSettings(*Settings);
    }
}

void ULight_DynamicWeatherSystem::TransitionToWeather(ELight_WeatherState NewState, float TransitionDuration)
{
    if (NewState == CurrentWeatherState)
    {
        return;
    }

    TargetWeatherState = NewState;
    this->TransitionDuration = FMath::Max(0.1f, TransitionDuration);
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
}

void ULight_DynamicWeatherSystem::EnableDynamicWeather(bool bEnable)
{
    bDynamicWeatherEnabled = bEnable;
    
    if (bEnable)
    {
        ScheduleNextWeatherChange();
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(WeatherTransitionTimer);
    }
}

void ULight_DynamicWeatherSystem::SetWeatherTransitionInterval(float MinInterval, float MaxInterval)
{
    MinWeatherInterval = FMath::Max(60.0f, MinInterval); // Minimum 1 minute
    MaxWeatherInterval = FMath::Max(MinWeatherInterval, MaxInterval);
}

void ULight_DynamicWeatherSystem::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }
    UpdateSunPosition();
}

void ULight_DynamicWeatherSystem::SetDayDuration(float DurationInMinutes)
{
    DayDurationMinutes = FMath::Max(1.0f, DurationInMinutes);
}

void ULight_DynamicWeatherSystem::InitializeWeatherPresets()
{
    // Clear Weather
    FLight_WeatherSettings ClearWeather;
    ClearWeather.SunIntensity = 5.0f;
    ClearWeather.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.FogHeightFalloff = 0.2f;
    ClearWeather.FogColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);
    ClearWeather.BloomIntensity = 0.675f;
    ClearWeather.ExposureBias = 0.5f;
    WeatherPresets.Add(ELight_WeatherState::Clear, ClearWeather);

    // Overcast Weather
    FLight_WeatherSettings OvercastWeather;
    OvercastWeather.SunIntensity = 2.5f;
    OvercastWeather.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    OvercastWeather.FogDensity = 0.04f;
    OvercastWeather.FogHeightFalloff = 0.15f;
    OvercastWeather.FogColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    OvercastWeather.BloomIntensity = 0.4f;
    OvercastWeather.ExposureBias = 0.2f;
    WeatherPresets.Add(ELight_WeatherState::Overcast, OvercastWeather);

    // Storm Weather
    FLight_WeatherSettings StormWeather;
    StormWeather.SunIntensity = 1.0f;
    StormWeather.SunColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
    StormWeather.FogDensity = 0.08f;
    StormWeather.FogHeightFalloff = 0.1f;
    StormWeather.FogColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
    StormWeather.BloomIntensity = 0.2f;
    StormWeather.ExposureBias = -0.5f;
    WeatherPresets.Add(ELight_WeatherState::Storm, StormWeather);

    // Fog Weather
    FLight_WeatherSettings FogWeather;
    FogWeather.SunIntensity = 1.5f;
    FogWeather.SunColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);
    FogWeather.FogDensity = 0.15f;
    FogWeather.FogHeightFalloff = 0.05f;
    FogWeather.FogColor = FLinearColor(0.8f, 0.8f, 0.75f, 1.0f);
    FogWeather.BloomIntensity = 0.3f;
    FogWeather.ExposureBias = 0.0f;
    WeatherPresets.Add(ELight_WeatherState::Fog, FogWeather);

    // Dawn Weather
    FLight_WeatherSettings DawnWeather;
    DawnWeather.SunIntensity = 3.0f;
    DawnWeather.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnWeather.FogDensity = 0.03f;
    DawnWeather.FogHeightFalloff = 0.25f;
    DawnWeather.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnWeather.BloomIntensity = 0.8f;
    DawnWeather.ExposureBias = 0.3f;
    WeatherPresets.Add(ELight_WeatherState::Dawn, DawnWeather);

    // Dusk Weather
    FLight_WeatherSettings DuskWeather;
    DuskWeather.SunIntensity = 2.0f;
    DuskWeather.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    DuskWeather.FogDensity = 0.025f;
    DuskWeather.FogHeightFalloff = 0.3f;
    DuskWeather.FogColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f);
    DuskWeather.BloomIntensity = 1.0f;
    DuskWeather.ExposureBias = 0.1f;
    WeatherPresets.Add(ELight_WeatherState::Dusk, DuskWeather);
}

void ULight_DynamicWeatherSystem::FindLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find DirectionalLight
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }

    // Find ExponentialHeightFog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FogActors[0]);
    }

    // Find PostProcessVolume
    TArray<AActor*> PPVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PPVolumes);
    if (PPVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PPVolumes[0]);
    }
}

void ULight_DynamicWeatherSystem::ApplyWeatherSettings(const FLight_WeatherSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }

    // Apply fog settings
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }

    // Apply post-process settings
    if (PostProcessVolume)
    {
        FPostProcessSettings& PPSettings = PostProcessVolume->Settings;
        PPSettings.bOverride_BloomIntensity = true;
        PPSettings.BloomIntensity = Settings.BloomIntensity;
        PPSettings.bOverride_AutoExposureBias = true;
        PPSettings.AutoExposureBias = Settings.ExposureBias;
    }
}

void ULight_DynamicWeatherSystem::UpdateTimeOfDay(float DeltaTime)
{
    float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
    CurrentTimeOfDay += HoursPerSecond * DeltaTime;
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
}

void ULight_DynamicWeatherSystem::UpdateSunPosition()
{
    if (!SunLight)
    {
        return;
    }

    // Convert time of day to sun angle (0-24 hours to 0-360 degrees)
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
    
    // Calculate sun rotation
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // East-West movement can be added here
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);

    // Adjust intensity based on sun height
    if (SunLight->GetLightComponent())
    {
        float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));
        float IntensityMultiplier = FMath::Max(0.1f, SunHeight);
        
        const FLight_WeatherSettings* CurrentSettings = WeatherPresets.Find(CurrentWeatherState);
        if (CurrentSettings)
        {
            float AdjustedIntensity = CurrentSettings->SunIntensity * IntensityMultiplier;
            SunLight->GetLightComponent()->SetIntensity(AdjustedIntensity);
        }
    }
}

void ULight_DynamicWeatherSystem::ScheduleNextWeatherChange()
{
    if (!bDynamicWeatherEnabled)
    {
        return;
    }

    float NextChangeDelay = FMath::RandRange(MinWeatherInterval, MaxWeatherInterval);
    
    GetWorld()->GetTimerManager().SetTimer(WeatherTransitionTimer, [this]()
    {
        // Choose random weather state
        TArray<ELight_WeatherState> WeatherStates = {
            ELight_WeatherState::Clear,
            ELight_WeatherState::Overcast,
            ELight_WeatherState::Storm,
            ELight_WeatherState::Fog
        };
        
        ELight_WeatherState NewWeather = WeatherStates[FMath::RandRange(0, WeatherStates.Num() - 1)];
        TransitionToWeather(NewWeather, FMath::RandRange(5.0f, 15.0f));
        
        ScheduleNextWeatherChange();
        
    }, NextChangeDelay, false);
}

void ULight_DynamicWeatherSystem::OnWeatherTransitionComplete()
{
    // Weather transition completed - can trigger events here
}

FLight_WeatherSettings ULight_DynamicWeatherSystem::LerpWeatherSettings(const FLight_WeatherSettings& A, const FLight_WeatherSettings& B, float Alpha)
{
    FLight_WeatherSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    
    return Result;
}