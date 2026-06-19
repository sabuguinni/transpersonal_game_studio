#include "LightAtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALightAtmosphereManager::ALightAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    // Configure Dawn preset
    DawnSettings.SunPitch = -8.0f;
    DawnSettings.SunYaw = -90.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.4f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    DawnSettings.ExposureBias = 0.5f;

    // Configure Midday preset
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunYaw = 0.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.65f, 0.8f, 1.0f);
    MiddaySettings.ExposureBias = 1.0f;

    // Configure Dusk preset
    DuskSettings.SunPitch = -5.0f;
    DuskSettings.SunYaw = 90.0f;
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.15f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.3f;
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.FogColor = FLinearColor(0.7f, 0.4f, 0.3f, 1.0f);
    DuskSettings.ExposureBias = 0.4f;

    // Configure Night preset
    NightSettings.SunPitch = 30.0f;  // Sun below horizon
    NightSettings.SunYaw = 180.0f;
    NightSettings.SunIntensity = 0.0f;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    NightSettings.SkyLightIntensity = 0.1f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.15f, 1.0f);
    NightSettings.ExposureBias = -1.0f;
}

void ALightAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find light actors if not manually assigned
    if (!SunActor)
    {
        TArray<AActor*> DirLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirLights);
        if (DirLights.Num() > 0)
        {
            SunActor = DirLights[0];
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLightActor = SkyLights[0];
        }
    }

    // Apply initial time of day
    SetTimeOfDay(CurrentTimeOfDayNormalized);
}

void ALightAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableDayNightCycle)
    {
        return;
    }

    // Advance time
    float TimeAdvance = DeltaTime / DayDurationSeconds;
    CurrentTimeOfDayNormalized = FMath::Fmod(CurrentTimeOfDayNormalized + TimeAdvance, 1.0f);

    // Update all lighting systems
    UpdateSunPosition(CurrentTimeOfDayNormalized);
    UpdateFogSettings(CurrentTimeOfDayNormalized);
    UpdateSkyLight(CurrentTimeOfDayNormalized);
    UpdatePostProcess(CurrentTimeOfDayNormalized);

    // Update time of day classification
    CurrentTimeOfDay = ClassifyTimeOfDay(CurrentTimeOfDayNormalized);

    // Weather transition
    if (CurrentWeather != TargetWeather)
    {
        WeatherTransitionAlpha = FMath::Min(WeatherTransitionAlpha + DeltaTime * WeatherTransitionSpeed, 1.0f);
        if (WeatherTransitionAlpha >= 1.0f)
        {
            CurrentWeather = TargetWeather;
            WeatherTransitionAlpha = 0.0f;
        }
    }
}

void ALightAtmosphereManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDay = ClassifyTimeOfDay(CurrentTimeOfDayNormalized);

    UpdateSunPosition(CurrentTimeOfDayNormalized);
    UpdateFogSettings(CurrentTimeOfDayNormalized);
    UpdateSkyLight(CurrentTimeOfDayNormalized);
    UpdatePostProcess(CurrentTimeOfDayNormalized);
}

void ALightAtmosphereManager::SetWeather(ELight_WeatherState NewWeather)
{
    TargetWeather = NewWeather;
    WeatherTransitionAlpha = 0.0f;
}

ELight_TimeOfDay ALightAtmosphereManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

float ALightAtmosphereManager::GetCurrentTimeNormalized() const
{
    return CurrentTimeOfDayNormalized;
}

void ALightAtmosphereManager::ApplyMiddayPreset()
{
    SetTimeOfDay(0.5f);
    UE_LOG(LogTemp, Log, TEXT("LightAtmosphereManager: Applied Midday preset"));
}

void ALightAtmosphereManager::ApplyDawnPreset()
{
    SetTimeOfDay(0.25f);
    UE_LOG(LogTemp, Log, TEXT("LightAtmosphereManager: Applied Dawn preset"));
}

void ALightAtmosphereManager::ApplyDuskPreset()
{
    SetTimeOfDay(0.75f);
    UE_LOG(LogTemp, Log, TEXT("LightAtmosphereManager: Applied Dusk preset"));
}

void ALightAtmosphereManager::ApplyNightPreset()
{
    SetTimeOfDay(0.0f);
    UE_LOG(LogTemp, Log, TEXT("LightAtmosphereManager: Applied Night preset"));
}

void ALightAtmosphereManager::LogAtmosphereStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== LightAtmosphereManager Status ==="));
    UE_LOG(LogTemp, Log, TEXT("  TimeNormalized: %.3f"), CurrentTimeOfDayNormalized);
    UE_LOG(LogTemp, Log, TEXT("  TimeOfDay: %d"), (int32)CurrentTimeOfDay);
    UE_LOG(LogTemp, Log, TEXT("  Weather: %d"), (int32)CurrentWeather);
    UE_LOG(LogTemp, Log, TEXT("  DayNightCycle: %s"), bEnableDayNightCycle ? TEXT("ON") : TEXT("OFF"));
    UE_LOG(LogTemp, Log, TEXT("  SunActor: %s"), SunActor ? *SunActor->GetActorLabel() : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("  SkyLightActor: %s"), SkyLightActor ? *SkyLightActor->GetActorLabel() : TEXT("NULL"));
}

// ============================================================
// Private helpers
// ============================================================

void ALightAtmosphereManager::UpdateSunPosition(float NormalizedTime)
{
    if (!SunActor)
    {
        return;
    }

    UDirectionalLightComponent* DirComp = SunActor->FindComponentByClass<UDirectionalLightComponent>();
    if (!DirComp)
    {
        return;
    }

    // Compute settings by lerping between presets based on time
    FLight_TimeOfDaySettings CurrentSettings;

    // Time zones: 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1=midnight
    if (NormalizedTime < 0.25f)
    {
        // Night -> Dawn
        float Alpha = NormalizedTime / 0.25f;
        CurrentSettings = LerpTimeSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (NormalizedTime < 0.5f)
    {
        // Dawn -> Midday
        float Alpha = (NormalizedTime - 0.25f) / 0.25f;
        CurrentSettings = LerpTimeSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (NormalizedTime < 0.75f)
    {
        // Midday -> Dusk
        float Alpha = (NormalizedTime - 0.5f) / 0.25f;
        CurrentSettings = LerpTimeSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else
    {
        // Dusk -> Night
        float Alpha = (NormalizedTime - 0.75f) / 0.25f;
        CurrentSettings = LerpTimeSettings(DuskSettings, NightSettings, Alpha);
    }

    // Apply weather multipliers
    float WeatherIntensityMult = 1.0f;
    if (CurrentWeather == ELight_WeatherState::Overcast)
    {
        WeatherIntensityMult = 0.4f;
    }
    else if (CurrentWeather == ELight_WeatherState::Stormy)
    {
        WeatherIntensityMult = 0.2f;
    }
    else if (CurrentWeather == ELight_WeatherState::Rainy)
    {
        WeatherIntensityMult = 0.3f;
    }

    // Apply to directional light
    SunActor->SetActorRotation(FRotator(CurrentSettings.SunPitch, CurrentSettings.SunYaw, 0.0f));
    DirComp->SetIntensity(CurrentSettings.SunIntensity * WeatherIntensityMult);
    DirComp->SetLightColor(CurrentSettings.SunColor);
}

void ALightAtmosphereManager::UpdateFogSettings(float NormalizedTime)
{
    if (!FogActor)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp)
    {
        return;
    }

    // Compute fog density based on time
    float BaseFogDensity = 0.02f;
    FLinearColor BaseFogColor = FLinearColor(0.45f, 0.55f, 0.65f, 1.0f);

    if (NormalizedTime < 0.25f)
    {
        // Night fog — thick and dark
        float Alpha = NormalizedTime / 0.25f;
        BaseFogDensity = FMath::Lerp(NightSettings.FogDensity, DawnSettings.FogDensity, Alpha);
        BaseFogColor = FMath::Lerp(NightSettings.FogColor, DawnSettings.FogColor, Alpha);
    }
    else if (NormalizedTime < 0.5f)
    {
        float Alpha = (NormalizedTime - 0.25f) / 0.25f;
        BaseFogDensity = FMath::Lerp(DawnSettings.FogDensity, MiddaySettings.FogDensity, Alpha);
        BaseFogColor = FMath::Lerp(DawnSettings.FogColor, MiddaySettings.FogColor, Alpha);
    }
    else if (NormalizedTime < 0.75f)
    {
        float Alpha = (NormalizedTime - 0.5f) / 0.25f;
        BaseFogDensity = FMath::Lerp(MiddaySettings.FogDensity, DuskSettings.FogDensity, Alpha);
        BaseFogColor = FMath::Lerp(MiddaySettings.FogColor, DuskSettings.FogColor, Alpha);
    }
    else
    {
        float Alpha = (NormalizedTime - 0.75f) / 0.25f;
        BaseFogDensity = FMath::Lerp(DuskSettings.FogDensity, NightSettings.FogDensity, Alpha);
        BaseFogColor = FMath::Lerp(DuskSettings.FogColor, NightSettings.FogColor, Alpha);
    }

    // Weather fog multiplier
    float WeatherFogMult = 1.0f;
    if (CurrentWeather == ELight_WeatherState::Foggy)
    {
        WeatherFogMult = 4.0f;
    }
    else if (CurrentWeather == ELight_WeatherState::Stormy || CurrentWeather == ELight_WeatherState::Rainy)
    {
        WeatherFogMult = 2.5f;
    }
    else if (CurrentWeather == ELight_WeatherState::Overcast)
    {
        WeatherFogMult = 1.5f;
    }

    FogComp->SetFogDensity(BaseFogDensity * WeatherFogMult);
    FogComp->SetFogInscatteringColor(BaseFogColor);
}

void ALightAtmosphereManager::UpdateSkyLight(float NormalizedTime)
{
    if (!SkyLightActor)
    {
        return;
    }

    USkyLightComponent* SkyComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (!SkyComp)
    {
        return;
    }

    float SkyIntensity = 1.5f;
    if (NormalizedTime < 0.25f)
    {
        SkyIntensity = FMath::Lerp(NightSettings.SkyLightIntensity, DawnSettings.SkyLightIntensity, NormalizedTime / 0.25f);
    }
    else if (NormalizedTime < 0.5f)
    {
        SkyIntensity = FMath::Lerp(DawnSettings.SkyLightIntensity, MiddaySettings.SkyLightIntensity, (NormalizedTime - 0.25f) / 0.25f);
    }
    else if (NormalizedTime < 0.75f)
    {
        SkyIntensity = FMath::Lerp(MiddaySettings.SkyLightIntensity, DuskSettings.SkyLightIntensity, (NormalizedTime - 0.5f) / 0.25f);
    }
    else
    {
        SkyIntensity = FMath::Lerp(DuskSettings.SkyLightIntensity, NightSettings.SkyLightIntensity, (NormalizedTime - 0.75f) / 0.25f);
    }

    SkyComp->SetIntensity(SkyIntensity);
}

void ALightAtmosphereManager::UpdatePostProcess(float NormalizedTime)
{
    if (!PostProcessVolume)
    {
        return;
    }

    // Exposure bias varies through the day
    float ExposureBias = 1.0f;
    if (NormalizedTime < 0.25f)
    {
        ExposureBias = FMath::Lerp(NightSettings.ExposureBias, DawnSettings.ExposureBias, NormalizedTime / 0.25f);
    }
    else if (NormalizedTime < 0.5f)
    {
        ExposureBias = FMath::Lerp(DawnSettings.ExposureBias, MiddaySettings.ExposureBias, (NormalizedTime - 0.25f) / 0.25f);
    }
    else if (NormalizedTime < 0.75f)
    {
        ExposureBias = FMath::Lerp(MiddaySettings.ExposureBias, DuskSettings.ExposureBias, (NormalizedTime - 0.5f) / 0.25f);
    }
    else
    {
        ExposureBias = FMath::Lerp(DuskSettings.ExposureBias, NightSettings.ExposureBias, (NormalizedTime - 0.75f) / 0.25f);
    }

    // Note: PostProcessVolume settings require FPostProcessSettings struct access
    // This is set via the component in editor; at runtime we use the volume's blend weight
    PostProcessVolume->BlendWeight = 1.0f;
}

FLight_TimeOfDaySettings ALightAtmosphereManager::LerpTimeSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha)
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

void ALightAtmosphereManager::ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings)
{
    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
        UDirectionalLightComponent* DirComp = SunActor->FindComponentByClass<UDirectionalLightComponent>();
        if (DirComp)
        {
            DirComp->SetIntensity(Settings.SunIntensity);
            DirComp->SetLightColor(Settings.SunColor);
        }
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

ELight_TimeOfDay ALightAtmosphereManager::ClassifyTimeOfDay(float NormalizedTime) const
{
    if (NormalizedTime < 0.05f || NormalizedTime >= 0.95f)
    {
        return ELight_TimeOfDay::Midnight;
    }
    else if (NormalizedTime < 0.15f)
    {
        return ELight_TimeOfDay::Night;
    }
    else if (NormalizedTime < 0.3f)
    {
        return ELight_TimeOfDay::Dawn;
    }
    else if (NormalizedTime < 0.45f)
    {
        return ELight_TimeOfDay::Morning;
    }
    else if (NormalizedTime < 0.6f)
    {
        return ELight_TimeOfDay::Midday;
    }
    else if (NormalizedTime < 0.75f)
    {
        return ELight_TimeOfDay::Afternoon;
    }
    else
    {
        return ELight_TimeOfDay::Dusk;
    }
}
