#include "LightingAtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    // Initialize with midday defaults
    CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    CurrentWeather = ELight_WeatherState::Clear;
    TimeOfDaySeconds = 43200.0f; // 12:00 noon
    DayDurationSeconds = 1200.0f; // 20 real minutes = 1 game day
    bDynamicDayNightEnabled = true;
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultPalettes();
    
    // Apply initial palette
    FLight_Palette InitialPalette = GetPaletteForTime(CurrentTimeOfDay);
    ApplyPalette(InitialPalette);
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDynamicDayNightEnabled)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALightingAtmosphereManager::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time — 1 real second = (86400 / DayDurationSeconds) game seconds
    float GameSecondsPerRealSecond = 86400.0f / DayDurationSeconds;
    TimeOfDaySeconds += DeltaTime * GameSecondsPerRealSecond;

    // Wrap at 24 hours
    if (TimeOfDaySeconds >= 86400.0f)
    {
        TimeOfDaySeconds -= 86400.0f;
    }

    float CurrentHour = GetCurrentHour();
    ELight_TimeOfDay NewTimeOfDay = GetTimeOfDayFromHour(CurrentHour);

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        PreviousTimeOfDay = CurrentTimeOfDay;
        CurrentTimeOfDay = NewTimeOfDay;
        TransitionAlpha = 0.0f;
    }

    // Smooth transition between palettes
    TransitionAlpha = FMath::Min(TransitionAlpha + DeltaTime * 0.5f, 1.0f);

    if (TransitionAlpha < 1.0f)
    {
        FLight_Palette PrevPalette = GetPaletteForTime(PreviousTimeOfDay);
        FLight_Palette CurrPalette = GetPaletteForTime(CurrentTimeOfDay);
        FLight_Palette BlendedPalette = LerpPalettes(PrevPalette, CurrPalette, TransitionAlpha);
        ApplyLightingToScene(BlendedPalette);
    }
}

float ALightingAtmosphereManager::GetCurrentHour() const
{
    return TimeOfDaySeconds / 3600.0f;
}

ELight_TimeOfDay ALightingAtmosphereManager::GetTimeOfDayFromHour(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 10.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 10.0f && Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 14.0f && Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.0f && Hour < 18.5f) return ELight_TimeOfDay::GoldenHour;
    if (Hour >= 18.5f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f && Hour < 21.5f) return ELight_TimeOfDay::BlueHour;
    if (Hour >= 21.5f && Hour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ALightingAtmosphereManager::AdvanceTimeBySeconds(float Seconds)
{
    TimeOfDaySeconds = FMath::Fmod(TimeOfDaySeconds + Seconds, 86400.0f);
}

void ALightingAtmosphereManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    PreviousTimeOfDay = CurrentTimeOfDay;
    CurrentTimeOfDay = NewTime;
    TransitionAlpha = 0.0f;

    // Set TimeOfDaySeconds to match
    switch (NewTime)
    {
        case ELight_TimeOfDay::Dawn:       TimeOfDaySeconds = 21600.0f; break; // 6:00
        case ELight_TimeOfDay::Morning:    TimeOfDaySeconds = 28800.0f; break; // 8:00
        case ELight_TimeOfDay::Midday:     TimeOfDaySeconds = 43200.0f; break; // 12:00
        case ELight_TimeOfDay::Afternoon:  TimeOfDaySeconds = 54000.0f; break; // 15:00
        case ELight_TimeOfDay::GoldenHour: TimeOfDaySeconds = 61200.0f; break; // 17:00
        case ELight_TimeOfDay::Dusk:       TimeOfDaySeconds = 68400.0f; break; // 19:00
        case ELight_TimeOfDay::BlueHour:   TimeOfDaySeconds = 72000.0f; break; // 20:00
        case ELight_TimeOfDay::Night:      TimeOfDaySeconds = 79200.0f; break; // 22:00
        case ELight_TimeOfDay::Midnight:   TimeOfDaySeconds = 0.0f;     break; // 00:00
        default: break;
    }

    FLight_Palette Palette = GetPaletteForTime(NewTime);
    ApplyPalette(Palette);
}

void ALightingAtmosphereManager::SetWeather(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    // Weather modifies the current palette
    FLight_Palette BasePalette = GetPaletteForTime(CurrentTimeOfDay);

    switch (NewWeather)
    {
        case ELight_WeatherState::Overcast:
            BasePalette.SunIntensity *= 0.3f;
            BasePalette.SkyLightIntensity *= 0.7f;
            BasePalette.FogDensity *= 2.5f;
            BasePalette.FogColor = FLinearColor(0.65f, 0.68f, 0.72f, 1.0f);
            break;
        case ELight_WeatherState::Stormy:
            BasePalette.SunIntensity *= 0.1f;
            BasePalette.SkyLightIntensity *= 0.4f;
            BasePalette.FogDensity *= 4.0f;
            BasePalette.FogColor = FLinearColor(0.35f, 0.38f, 0.42f, 1.0f);
            break;
        case ELight_WeatherState::Foggy:
            BasePalette.FogDensity *= 6.0f;
            BasePalette.FogColor = FLinearColor(0.78f, 0.80f, 0.82f, 1.0f);
            BasePalette.SunIntensity *= 0.5f;
            break;
        case ELight_WeatherState::Rainy:
            BasePalette.SunIntensity *= 0.2f;
            BasePalette.FogDensity *= 3.0f;
            BasePalette.FogColor = FLinearColor(0.55f, 0.60f, 0.65f, 1.0f);
            break;
        default:
            break;
    }

    ApplyPalette(BasePalette);
}

void ALightingAtmosphereManager::ApplyPalette(const FLight_Palette& Palette)
{
    ApplyLightingToScene(Palette);
}

void ALightingAtmosphereManager::ApplyLightingToScene(const FLight_Palette& Palette)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Apply to sun directional light
    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(Palette.SunPitch, Palette.SunYaw, 0.0f));
        UDirectionalLightComponent* SunComp = SunActor->FindComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Palette.SunIntensity);
            SunComp->SetLightColor(Palette.SunColor);
        }
    }

    // Apply to fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogInscatteringColor(Palette.FogColor);
            FogComp->SetFogDensity(Palette.FogDensity);
        }
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
        }
    }
}

FLight_Palette ALightingAtmosphereManager::LerpPalettes(const FLight_Palette& A, const FLight_Palette& B, float Alpha) const
{
    FLight_Palette Result;
    Result.TimeOfDay = B.TimeOfDay;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, Alpha);
    Result.AutoExposureBias = FMath::Lerp(A.AutoExposureBias, B.AutoExposureBias, Alpha);
    return Result;
}

FLight_Palette ALightingAtmosphereManager::GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const
{
    for (const FLight_Palette& P : TimePalettes)
    {
        if (P.TimeOfDay == TimeOfDay) return P;
    }
    // Return default midday if not found
    FLight_Palette Default;
    Default.TimeOfDay = ELight_TimeOfDay::Midday;
    return Default;
}

void ALightingAtmosphereManager::InitializeDefaultPalettes()
{
    TimePalettes.Empty();

    // DAWN — cold blue-pink, low sun on horizon
    FLight_Palette Dawn;
    Dawn.TimeOfDay = ELight_TimeOfDay::Dawn;
    Dawn.SunPitch = -5.0f; Dawn.SunYaw = -90.0f;
    Dawn.SunIntensity = 2.5f;
    Dawn.SunColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.0f);
    Dawn.FogColor = FLinearColor(0.55f, 0.62f, 0.85f, 1.0f);
    Dawn.FogDensity = 0.035f; Dawn.SkyLightIntensity = 0.8f;
    Dawn.BloomIntensity = 0.6f; Dawn.AutoExposureBias = -0.5f;
    TimePalettes.Add(Dawn);

    // MORNING — warm golden, soft shadows
    FLight_Palette Morning;
    Morning.TimeOfDay = ELight_TimeOfDay::Morning;
    Morning.SunPitch = -35.0f; Morning.SunYaw = -60.0f;
    Morning.SunIntensity = 7.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.88f, 0.68f, 1.0f);
    Morning.FogColor = FLinearColor(0.78f, 0.75f, 0.65f, 1.0f);
    Morning.FogDensity = 0.022f; Morning.SkyLightIntensity = 2.0f;
    Morning.BloomIntensity = 0.45f; Morning.AutoExposureBias = 0.1f;
    TimePalettes.Add(Morning);

    // MIDDAY — harsh white sun, high contrast
    FLight_Palette Midday;
    Midday.TimeOfDay = ELight_TimeOfDay::Midday;
    Midday.SunPitch = -78.0f; Midday.SunYaw = 45.0f;
    Midday.SunIntensity = 12.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    Midday.FogColor = FLinearColor(0.85f, 0.82f, 0.72f, 1.0f);
    Midday.FogDensity = 0.018f; Midday.SkyLightIntensity = 2.8f;
    Midday.BloomIntensity = 0.35f; Midday.AutoExposureBias = 0.3f;
    TimePalettes.Add(Midday);

    // AFTERNOON — warm, long shadows begin
    FLight_Palette Afternoon;
    Afternoon.TimeOfDay = ELight_TimeOfDay::Afternoon;
    Afternoon.SunPitch = -45.0f; Afternoon.SunYaw = 90.0f;
    Afternoon.SunIntensity = 9.0f;
    Afternoon.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
    Afternoon.FogColor = FLinearColor(0.80f, 0.76f, 0.65f, 1.0f);
    Afternoon.FogDensity = 0.020f; Afternoon.SkyLightIntensity = 2.4f;
    Afternoon.BloomIntensity = 0.40f; Afternoon.AutoExposureBias = 0.2f;
    TimePalettes.Add(Afternoon);

    // GOLDEN HOUR — deep amber, long dramatic shadows
    FLight_Palette GoldenHour;
    GoldenHour.TimeOfDay = ELight_TimeOfDay::GoldenHour;
    GoldenHour.SunPitch = -15.0f; GoldenHour.SunYaw = 120.0f;
    GoldenHour.SunIntensity = 5.5f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.62f, 0.25f, 1.0f);
    GoldenHour.FogColor = FLinearColor(0.92f, 0.68f, 0.42f, 1.0f);
    GoldenHour.FogDensity = 0.028f; GoldenHour.SkyLightIntensity = 1.8f;
    GoldenHour.BloomIntensity = 0.65f; GoldenHour.AutoExposureBias = -0.2f;
    TimePalettes.Add(GoldenHour);

    // DUSK — orange-red, sun at horizon
    FLight_Palette Dusk;
    Dusk.TimeOfDay = ELight_TimeOfDay::Dusk;
    Dusk.SunPitch = -3.0f; Dusk.SunYaw = 150.0f;
    Dusk.SunIntensity = 2.0f;
    Dusk.SunColor = FLinearColor(1.0f, 0.45f, 0.18f, 1.0f);
    Dusk.FogColor = FLinearColor(0.88f, 0.52f, 0.32f, 1.0f);
    Dusk.FogDensity = 0.038f; Dusk.SkyLightIntensity = 1.2f;
    Dusk.BloomIntensity = 0.75f; Dusk.AutoExposureBias = -0.8f;
    TimePalettes.Add(Dusk);

    // BLUE HOUR — deep blue, twilight
    FLight_Palette BlueHour;
    BlueHour.TimeOfDay = ELight_TimeOfDay::BlueHour;
    BlueHour.SunPitch = -30.0f; BlueHour.SunYaw = 180.0f;
    BlueHour.SunIntensity = 0.8f;
    BlueHour.SunColor = FLinearColor(0.55f, 0.65f, 0.95f, 1.0f);
    BlueHour.FogColor = FLinearColor(0.28f, 0.35f, 0.62f, 1.0f);
    BlueHour.FogDensity = 0.032f; BlueHour.SkyLightIntensity = 0.9f;
    BlueHour.BloomIntensity = 0.55f; BlueHour.AutoExposureBias = -1.2f;
    TimePalettes.Add(BlueHour);

    // NIGHT — deep blue-black, moonlight
    FLight_Palette Night;
    Night.TimeOfDay = ELight_TimeOfDay::Night;
    Night.SunPitch = -15.0f; Night.SunYaw = 220.0f;
    Night.SunIntensity = 0.3f;
    Night.SunColor = FLinearColor(0.62f, 0.72f, 1.0f, 1.0f);
    Night.FogColor = FLinearColor(0.08f, 0.10f, 0.22f, 1.0f);
    Night.FogDensity = 0.025f; Night.SkyLightIntensity = 0.4f;
    Night.BloomIntensity = 0.3f; Night.AutoExposureBias = -2.5f;
    TimePalettes.Add(Night);

    // MIDNIGHT — near black, stars only
    FLight_Palette Midnight;
    Midnight.TimeOfDay = ELight_TimeOfDay::Midnight;
    Midnight.SunPitch = -15.0f; Midnight.SunYaw = 270.0f;
    Midnight.SunIntensity = 0.15f;
    Midnight.SunColor = FLinearColor(0.45f, 0.55f, 0.88f, 1.0f);
    Midnight.FogColor = FLinearColor(0.04f, 0.05f, 0.12f, 1.0f);
    Midnight.FogDensity = 0.020f; Midnight.SkyLightIntensity = 0.2f;
    Midnight.BloomIntensity = 0.2f; Midnight.AutoExposureBias = -3.5f;
    TimePalettes.Add(Midnight);

    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereManager: Initialized %d time palettes"), TimePalettes.Num());
}
