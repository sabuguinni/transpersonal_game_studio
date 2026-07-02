#include "LightingAtmosphereManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick every second for performance

    // Populate Freesound ambient IDs (forest ambience, day cycle)
    ForestAmbienceSoundIDs = {846981, 846980, 846978, 846982, 488328};

    // Default: afternoon golden hour palette
    ActivePalette.SunPitch = -25.0f;
    ActivePalette.SunYaw = 200.0f;
    ActivePalette.SunIntensity = 7.5f;
    ActivePalette.SunColor = FLinearColor(1.0f, 0.82f, 0.55f, 1.0f);
    ActivePalette.FogDensity = 0.022f;
    ActivePalette.FogColor = FLinearColor(0.78f, 0.65f, 0.45f, 1.0f);
    ActivePalette.SkyLightIntensity = 1.4f;
    ActivePalette.SkyLightColor = FLinearColor(0.95f, 0.88f, 0.72f, 1.0f);
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ApplyActivePalette();
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (DayCycleSpeed <= 0.0f)
    {
        return;
    }

    // Advance time: DayCycleSpeed=1 means 1 real second = 1 game minute
    TimeAccumulator += DeltaTime * DayCycleSpeed;
    if (TimeAccumulator >= 60.0f)
    {
        float DeltaHours = TimeAccumulator / 3600.0f;
        AdvanceTime(DeltaHours);
        TimeAccumulator = 0.0f;
    }
}

void ALightingAtmosphereManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find DirectionalLight (sun)
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        HeightFog = *It;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }
}

void ALightingAtmosphereManager::ApplyActivePalette()
{
    ApplyPaletteToScene(ActivePalette);
}

void ALightingAtmosphereManager::ApplyTimeOfDayPalette(ELight_TimeOfDay TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;
    ActivePalette = GetPaletteForTime(TimeOfDay);
    ApplyPaletteToScene(ActivePalette);
}

void ALightingAtmosphereManager::ApplyWeatherState(ELight_WeatherState Weather)
{
    CurrentWeather = Weather;

    // Modify active palette based on weather
    switch (Weather)
    {
    case ELight_WeatherState::Overcast:
        ActivePalette.SunIntensity *= 0.4f;
        ActivePalette.FogDensity = 0.045f;
        ActivePalette.FogColor = FLinearColor(0.6f, 0.62f, 0.65f, 1.0f);
        ActivePalette.SkyLightIntensity = 0.8f;
        break;

    case ELight_WeatherState::Stormy:
        ActivePalette.SunIntensity *= 0.15f;
        ActivePalette.FogDensity = 0.065f;
        ActivePalette.FogColor = FLinearColor(0.35f, 0.38f, 0.42f, 1.0f);
        ActivePalette.SkyLightIntensity = 0.5f;
        break;

    case ELight_WeatherState::Foggy:
        ActivePalette.FogDensity = 0.08f;
        ActivePalette.FogColor = FLinearColor(0.75f, 0.78f, 0.8f, 1.0f);
        ActivePalette.SkyLightIntensity = 0.9f;
        break;

    case ELight_WeatherState::Dusty:
        ActivePalette.FogDensity = 0.035f;
        ActivePalette.FogColor = FLinearColor(0.85f, 0.72f, 0.45f, 1.0f);
        ActivePalette.SkyLightColor = FLinearColor(0.9f, 0.78f, 0.55f, 1.0f);
        break;

    case ELight_WeatherState::Clear:
    default:
        // No modification — use base palette
        break;
    }

    ApplyPaletteToScene(ActivePalette);
}

FLight_SunPalette ALightingAtmosphereManager::GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const
{
    FLight_SunPalette Palette;

    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:
        Palette.SunPitch = -8.0f;
        Palette.SunYaw = 85.0f;
        Palette.SunIntensity = 4.5f;
        Palette.SunColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.0f);  // peachy-pink
        Palette.FogDensity = 0.035f;
        Palette.FogColor = FLinearColor(0.72f, 0.58f, 0.65f, 1.0f);  // pink-lavender
        Palette.SkyLightIntensity = 1.2f;
        Palette.SkyLightColor = FLinearColor(0.7f, 0.78f, 0.95f, 1.0f);  // cool blue
        break;

    case ELight_TimeOfDay::Morning:
        Palette.SunPitch = -35.0f;
        Palette.SunYaw = 120.0f;
        Palette.SunIntensity = 6.0f;
        Palette.SunColor = FLinearColor(1.0f, 0.92f, 0.72f, 1.0f);  // warm white
        Palette.FogDensity = 0.018f;
        Palette.FogColor = FLinearColor(0.8f, 0.82f, 0.75f, 1.0f);
        Palette.SkyLightIntensity = 1.5f;
        Palette.SkyLightColor = FLinearColor(0.85f, 0.9f, 1.0f, 1.0f);
        break;

    case ELight_TimeOfDay::Midday:
        Palette.SunPitch = -80.0f;
        Palette.SunYaw = 180.0f;
        Palette.SunIntensity = 10.0f;
        Palette.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);  // harsh white
        Palette.FogDensity = 0.012f;
        Palette.FogColor = FLinearColor(0.85f, 0.88f, 0.92f, 1.0f);
        Palette.SkyLightIntensity = 2.0f;
        Palette.SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
        break;

    case ELight_TimeOfDay::Afternoon:
        Palette.SunPitch = -25.0f;
        Palette.SunYaw = 200.0f;
        Palette.SunIntensity = 7.5f;
        Palette.SunColor = FLinearColor(1.0f, 0.82f, 0.55f, 1.0f);  // warm golden
        Palette.FogDensity = 0.022f;
        Palette.FogColor = FLinearColor(0.78f, 0.65f, 0.45f, 1.0f);
        Palette.SkyLightIntensity = 1.4f;
        Palette.SkyLightColor = FLinearColor(0.95f, 0.88f, 0.72f, 1.0f);
        break;

    case ELight_TimeOfDay::GoldenHour:
        Palette.SunPitch = -12.0f;
        Palette.SunYaw = 240.0f;
        Palette.SunIntensity = 5.5f;
        Palette.SunColor = FLinearColor(1.0f, 0.62f, 0.22f, 1.0f);  // deep orange
        Palette.FogDensity = 0.032f;
        Palette.FogColor = FLinearColor(0.88f, 0.55f, 0.28f, 1.0f);
        Palette.SkyLightIntensity = 1.0f;
        Palette.SkyLightColor = FLinearColor(1.0f, 0.72f, 0.45f, 1.0f);
        break;

    case ELight_TimeOfDay::Dusk:
        Palette.SunPitch = -18.0f;
        Palette.SunYaw = 55.0f;
        Palette.SunIntensity = 6.5f;
        Palette.SunColor = FLinearColor(1.0f, 0.45f, 0.18f, 1.0f);  // deep golden-orange
        Palette.FogDensity = 0.028f;
        Palette.FogColor = FLinearColor(0.72f, 0.42f, 0.28f, 1.0f);
        Palette.SkyLightIntensity = 1.8f;
        Palette.SkyLightColor = FLinearColor(0.85f, 0.62f, 0.45f, 1.0f);
        break;

    case ELight_TimeOfDay::Night:
        Palette.SunPitch = -5.0f;
        Palette.SunYaw = 0.0f;
        Palette.SunIntensity = 0.5f;
        Palette.SunColor = FLinearColor(0.3f, 0.35f, 0.55f, 1.0f);  // moonlight blue
        Palette.FogDensity = 0.04f;
        Palette.FogColor = FLinearColor(0.1f, 0.12f, 0.22f, 1.0f);
        Palette.SkyLightIntensity = 0.3f;
        Palette.SkyLightColor = FLinearColor(0.25f, 0.3f, 0.55f, 1.0f);
        break;

    default:
        break;
    }

    return Palette;
}

void ALightingAtmosphereManager::AdvanceTime(float DeltaHours)
{
    CurrentHour = FMath::Fmod(CurrentHour + DeltaHours, 24.0f);

    // Determine time of day from hour
    ELight_TimeOfDay NewTimeOfDay;
    if (CurrentHour >= 5.0f && CurrentHour < 7.0f)
        NewTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (CurrentHour >= 7.0f && CurrentHour < 11.0f)
        NewTimeOfDay = ELight_TimeOfDay::Morning;
    else if (CurrentHour >= 11.0f && CurrentHour < 14.0f)
        NewTimeOfDay = ELight_TimeOfDay::Midday;
    else if (CurrentHour >= 14.0f && CurrentHour < 17.0f)
        NewTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (CurrentHour >= 17.0f && CurrentHour < 19.0f)
        NewTimeOfDay = ELight_TimeOfDay::GoldenHour;
    else if (CurrentHour >= 19.0f && CurrentHour < 21.0f)
        NewTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        NewTimeOfDay = ELight_TimeOfDay::Night;

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        ApplyTimeOfDayPalette(NewTimeOfDay);
    }
}

FLight_SunPalette ALightingAtmosphereManager::LerpPalettes(const FLight_SunPalette& A, const FLight_SunPalette& B, float Alpha) const
{
    FLight_SunPalette Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.SkyLightColor = FLinearColor(
        FMath::Lerp(A.SkyLightColor.R, B.SkyLightColor.R, Alpha),
        FMath::Lerp(A.SkyLightColor.G, B.SkyLightColor.G, Alpha),
        FMath::Lerp(A.SkyLightColor.B, B.SkyLightColor.B, Alpha),
        1.0f
    );
    return Result;
}

void ALightingAtmosphereManager::ApplyPaletteToScene(const FLight_SunPalette& Palette)
{
    // Apply to DirectionalLight (sun)
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Palette.SunPitch, Palette.SunYaw, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Palette.SunIntensity);
            DLC->SetLightColor(Palette.SunColor);
        }
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Palette.SkyLightIntensity);
            SLC->SetLightColor(Palette.SkyLightColor);
        }
    }
}
