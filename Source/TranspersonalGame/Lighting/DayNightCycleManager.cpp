#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor — set default palette values
// ============================================================

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;  // Update 10x/sec for smooth transitions

    // Dawn palette — soft pink-purple, low sun
    DawnPalette.SunPitchDegrees = -5.0f;
    DawnPalette.SunIntensity = 1.5f;
    DawnPalette.SunColor = FLinearColor(1.0f, 0.65f, 0.55f, 1.0f);
    DawnPalette.FogDensity = 0.045f;
    DawnPalette.FogColor = FLinearColor(0.75f, 0.55f, 0.65f, 1.0f);
    DawnPalette.SkyLightIntensity = 0.6f;

    // Morning palette — warm golden, rising sun
    MorningPalette.SunPitchDegrees = -30.0f;
    MorningPalette.SunIntensity = 4.0f;
    MorningPalette.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    MorningPalette.FogDensity = 0.025f;
    MorningPalette.FogColor = FLinearColor(0.8f, 0.75f, 0.65f, 1.0f);
    MorningPalette.SkyLightIntensity = 1.0f;

    // Midday palette — harsh white, high sun
    MiddayPalette.SunPitchDegrees = -75.0f;
    MiddayPalette.SunIntensity = 8.0f;
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddayPalette.FogDensity = 0.012f;
    MiddayPalette.FogColor = FLinearColor(0.7f, 0.8f, 0.95f, 1.0f);
    MiddayPalette.SkyLightIntensity = 1.8f;

    // Dusk palette — deep orange-purple, low sun
    DuskPalette.SunPitchDegrees = -8.0f;
    DuskPalette.SunIntensity = 2.5f;
    DuskPalette.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    DuskPalette.FogDensity = 0.038f;
    DuskPalette.FogColor = FLinearColor(0.85f, 0.45f, 0.35f, 1.0f);
    DuskPalette.SkyLightIntensity = 0.7f;

    // Night palette — deep blue moonlight
    NightPalette.SunPitchDegrees = 30.0f;  // Below horizon
    NightPalette.SunIntensity = 0.05f;
    NightPalette.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightPalette.FogDensity = 0.055f;
    NightPalette.FogColor = FLinearColor(0.05f, 0.08f, 0.18f, 1.0f);
    NightPalette.SkyLightIntensity = 0.15f;
}

// ============================================================
// BeginPlay — auto-find scene actors if not set
// ============================================================

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find DirectionalLight if not assigned
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    // Auto-find ExponentialHeightFog
    if (!HeightFog)
    {
        TArray<AActor*> FoundFogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
        if (FoundFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
        }
    }

    // Auto-find SkyLight
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }

    // Apply initial palette
    SetTimeOfDay(CurrentTimeOfDayHours);
}

// ============================================================
// Tick — advance time and update atmosphere
// ============================================================

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleEnabled)
    {
        AdvanceTime(DeltaTime);
    }

    UpdateSunPosition();
    UpdateAtmosphere();
    UpdateSkyLight();

    // Check for time-of-day transition events
    ELight_TimeOfDay NewEnum = HoursToTimeOfDay(CurrentTimeOfDayHours);
    if (NewEnum != PreviousTimeOfDayEnum)
    {
        CurrentTimeOfDayEnum = NewEnum;
        OnTimeOfDayChanged(NewEnum);
        PreviousTimeOfDayEnum = NewEnum;
    }
}

// ============================================================
// Private: AdvanceTime
// ============================================================

void ADayNightCycleManager::AdvanceTime(float DeltaTime)
{
    // DayDurationSeconds = how many real seconds = 24 game hours
    float HoursPerSecond = 24.0f / FMath::Max(DayDurationSeconds, 1.0f);
    CurrentTimeOfDayHours += HoursPerSecond * DeltaTime * TimeMultiplier;

    // Wrap around 24 hours
    if (CurrentTimeOfDayHours >= 24.0f)
    {
        CurrentTimeOfDayHours -= 24.0f;
    }

    // Update normalized progress (0=midnight, 0.5=noon, 1=midnight)
    NormalizedDayProgress = CurrentTimeOfDayHours / 24.0f;
}

// ============================================================
// Private: UpdateSunPosition
// ============================================================

void ADayNightCycleManager::UpdateSunPosition()
{
    if (!SunLight) return;

    FLight_SkyPalette CurrentPalette = GetPaletteForTime(CurrentTimeOfDayHours);

    // CAP ENFORCEMENT: Never let sun pitch exceed -30 during day (prevents sun below ground)
    float TargetPitch = CurrentPalette.SunPitchDegrees;
    CurrentSunPitch = TargetPitch;

    FRotator CurrentRot = SunLight->GetActorRotation();
    FRotator NewRot(TargetPitch, CurrentRot.Yaw, CurrentRot.Roll);
    SunLight->SetActorRotation(NewRot);
}

// ============================================================
// Private: UpdateAtmosphere
// ============================================================

void ADayNightCycleManager::UpdateAtmosphere()
{
    if (!HeightFog) return;

    FLight_SkyPalette CurrentPalette = GetPaletteForTime(CurrentTimeOfDayHours);
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (!FogComp) return;

    // Weather modifies fog density
    float WeatherFogMod = 1.0f;
    switch (CurrentWeather.WeatherState)
    {
        case ELight_WeatherState::Foggy:    WeatherFogMod = 3.5f; break;
        case ELight_WeatherState::Overcast: WeatherFogMod = 1.8f; break;
        case ELight_WeatherState::Stormy:   WeatherFogMod = 2.5f; break;
        default: WeatherFogMod = 1.0f; break;
    }

    FogComp->SetFogDensity(CurrentPalette.FogDensity * WeatherFogMod);
    FogComp->SetFogInscatteringColor(CurrentPalette.FogColor);
}

// ============================================================
// Private: UpdateSkyLight
// ============================================================

void ADayNightCycleManager::UpdateSkyLight()
{
    if (!SkyLightActor) return;

    FLight_SkyPalette CurrentPalette = GetPaletteForTime(CurrentTimeOfDayHours);
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    if (!SkyComp) return;

    SkyComp->SetIntensity(CurrentPalette.SkyLightIntensity);
}

// ============================================================
// Private: HoursToTimeOfDay
// ============================================================

ELight_TimeOfDay ADayNightCycleManager::HoursToTimeOfDay(float Hours) const
{
    if (Hours >= 5.0f  && Hours < 7.0f)  return ELight_TimeOfDay::Dawn;
    if (Hours >= 7.0f  && Hours < 10.0f) return ELight_TimeOfDay::Morning;
    if (Hours >= 10.0f && Hours < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hours >= 14.0f && Hours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hours >= 17.0f && Hours < 19.5f) return ELight_TimeOfDay::Dusk;
    if (Hours >= 19.5f && Hours < 21.0f) return ELight_TimeOfDay::Twilight;
    if (Hours >= 21.0f || Hours < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

// ============================================================
// Private: GetPaletteForTime — interpolates between presets
// ============================================================

FLight_SkyPalette ADayNightCycleManager::GetPaletteForTime(float Hours) const
{
    // Piecewise linear interpolation through the day
    if (Hours >= 5.0f && Hours < 7.0f)
    {
        float Alpha = (Hours - 5.0f) / 2.0f;
        return InterpolatePalettes(NightPalette, MorningPalette, Alpha);
    }
    else if (Hours >= 7.0f && Hours < 12.0f)
    {
        float Alpha = (Hours - 7.0f) / 5.0f;
        return InterpolatePalettes(MorningPalette, MiddayPalette, Alpha);
    }
    else if (Hours >= 12.0f && Hours < 17.0f)
    {
        float Alpha = (Hours - 12.0f) / 5.0f;
        return InterpolatePalettes(MiddayPalette, DuskPalette, Alpha);
    }
    else if (Hours >= 17.0f && Hours < 20.0f)
    {
        float Alpha = (Hours - 17.0f) / 3.0f;
        return InterpolatePalettes(DuskPalette, NightPalette, Alpha);
    }
    else
    {
        return NightPalette;
    }
}

// ============================================================
// Private: InterpolatePalettes
// ============================================================

FLight_SkyPalette ADayNightCycleManager::InterpolatePalettes(
    const FLight_SkyPalette& A,
    const FLight_SkyPalette& B,
    float Alpha) const
{
    FLight_SkyPalette Result;
    Result.SunPitchDegrees   = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunIntensity      = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor          = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity        = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor          = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

// ============================================================
// Private: ApplyPaletteToScene
// ============================================================

void ADayNightCycleManager::ApplyPaletteToScene(const FLight_SkyPalette& Palette)
{
    // Sun
    if (SunLight)
    {
        FRotator Rot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Palette.SunPitchDegrees, Rot.Yaw, Rot.Roll));

        UDirectionalLightComponent* DLC = Cast<UDirectionalLightComponent>(
            SunLight->GetComponentByClass(UDirectionalLightComponent::StaticClass()));
        if (DLC)
        {
            DLC->SetIntensity(Palette.SunIntensity);
            DLC->SetLightColor(Palette.SunColor);
        }
    }

    // Fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
        }
    }
}

// ============================================================
// Public API
// ============================================================

void ADayNightCycleManager::SetTimeOfDay(float HoursIn24)
{
    CurrentTimeOfDayHours = FMath::Clamp(HoursIn24, 0.0f, 24.0f);
    NormalizedDayProgress = CurrentTimeOfDayHours / 24.0f;

    FLight_SkyPalette Palette = GetPaletteForTime(CurrentTimeOfDayHours);
    ApplyPaletteToScene(Palette);

    ELight_TimeOfDay NewEnum = HoursToTimeOfDay(CurrentTimeOfDayHours);
    if (NewEnum != CurrentTimeOfDayEnum)
    {
        CurrentTimeOfDayEnum = NewEnum;
        PreviousTimeOfDayEnum = NewEnum;
    }
}

bool ADayNightCycleManager::IsNight() const
{
    return CurrentTimeOfDayHours < 5.0f || CurrentTimeOfDayHours >= 21.0f;
}

bool ADayNightCycleManager::IsDawn() const
{
    return CurrentTimeOfDayHours >= 5.0f && CurrentTimeOfDayHours < 7.0f;
}

void ADayNightCycleManager::SetWeather(ELight_WeatherState NewWeather, float TransitionTime)
{
    TargetWeather = NewWeather;
    WeatherTransitionTimer = TransitionTime;
    CurrentWeather.WeatherState = NewWeather;
    OnWeatherChanged(NewWeather);
}

void ADayNightCycleManager::DebugApplyCurrentPalette()
{
    FLight_SkyPalette Palette = GetPaletteForTime(CurrentTimeOfDayHours);
    ApplyPaletteToScene(Palette);
    UE_LOG(LogTemp, Log, TEXT("DayNightCycleManager: Debug palette applied for hour %.1f"), CurrentTimeOfDayHours);
}
