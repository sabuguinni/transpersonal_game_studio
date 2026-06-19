#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec — sufficient for smooth lighting

    // Default presets — cinematic prehistoric world

    // Dawn: cool blue-pink, low sun, heavy fog
    DawnPreset.SunPitch = -5.0f;
    DawnPreset.SunYaw = -90.0f;
    DawnPreset.SunIntensity = 3.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnPreset.FogDensity = 0.05f;
    DawnPreset.FogColor = FLinearColor(0.5f, 0.55f, 0.7f, 1.0f);
    DawnPreset.SkyLightIntensity = 0.5f;

    // Midday: bright white sun, minimal fog, high contrast
    MiddayPreset.SunPitch = -75.0f;
    MiddayPreset.SunYaw = 0.0f;
    MiddayPreset.SunIntensity = 12.0f;
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddayPreset.FogDensity = 0.01f;
    MiddayPreset.FogColor = FLinearColor(0.7f, 0.75f, 0.85f, 1.0f);
    MiddayPreset.SkyLightIntensity = 2.0f;

    // Dusk: warm amber-orange, long shadows
    DuskPreset.SunPitch = -8.0f;
    DuskPreset.SunYaw = 90.0f;
    DuskPreset.SunIntensity = 4.0f;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.45f, 0.1f, 1.0f);
    DuskPreset.FogDensity = 0.04f;
    DuskPreset.FogColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
    DuskPreset.SkyLightIntensity = 0.4f;

    // Night: deep blue, moonlight, dense fog — maximum danger
    NightPreset.SunPitch = 30.0f; // Sun below horizon
    NightPreset.SunYaw = 180.0f;
    NightPreset.SunIntensity = 0.1f;
    NightPreset.SunColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
    NightPreset.FogDensity = 0.08f;
    NightPreset.FogColor = FLinearColor(0.05f, 0.07f, 0.15f, 1.0f);
    NightPreset.SkyLightIntensity = 0.15f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    ApplyPreset(TimeToEnum(CurrentTimeOfDay));
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive)
    {
        return;
    }

    UpdateLighting(DeltaTime);
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Advance time
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Get current preset via interpolation
    FLight_TimeOfDayPreset CurrentPreset = GetPresetForTime(CurrentTimeOfDay);
    ApplyLightingPreset(CurrentPreset);

    // Check for time of day phase change
    ELight_TimeOfDay NewEnum = TimeToEnum(CurrentTimeOfDay);
    if (NewEnum != CurrentTimeEnum)
    {
        CurrentTimeEnum = NewEnum;
        OnTimeOfDayChanged(CurrentTimeEnum);
    }

    OnTimeUpdated(GetNormalizedTime());
}

void ADayNightCycleManager::ApplyLightingPreset(const FLight_TimeOfDayPreset& Preset)
{
    // Apply to sun (directional light)
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Preset.SunPitch, Preset.SunYaw, 0.0f));
        UDirectionalLightComponent* DirComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DirComp)
        {
            DirComp->SetIntensity(Preset.SunIntensity);
            DirComp->SetLightColor(Preset.SunColor);
        }
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Preset.SkyLightIntensity);
        }
    }

    // Apply to fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Preset.FogDensity);
            FogComp->SetFogInscatteringColor(Preset.FogColor);
        }
    }
}

FLight_TimeOfDayPreset ADayNightCycleManager::InterpolatePresets(
    const FLight_TimeOfDayPreset& A,
    const FLight_TimeOfDayPreset& B,
    float Alpha) const
{
    FLight_TimeOfDayPreset Result;
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
    return Result;
}

FLight_TimeOfDayPreset ADayNightCycleManager::GetPresetForTime(float TimeHours) const
{
    // Key times: Dawn=5, Midday=12, Dusk=19, Night=22
    if (TimeHours < 5.0f)
    {
        // Night → Dawn
        float Alpha = FMath::Clamp((TimeHours - 0.0f) / 5.0f, 0.0f, 1.0f);
        return InterpolatePresets(NightPreset, DawnPreset, Alpha);
    }
    else if (TimeHours < 12.0f)
    {
        // Dawn → Midday
        float Alpha = FMath::Clamp((TimeHours - 5.0f) / 7.0f, 0.0f, 1.0f);
        return InterpolatePresets(DawnPreset, MiddayPreset, Alpha);
    }
    else if (TimeHours < 19.0f)
    {
        // Midday → Dusk
        float Alpha = FMath::Clamp((TimeHours - 12.0f) / 7.0f, 0.0f, 1.0f);
        return InterpolatePresets(MiddayPreset, DuskPreset, Alpha);
    }
    else if (TimeHours < 22.0f)
    {
        // Dusk → Night
        float Alpha = FMath::Clamp((TimeHours - 19.0f) / 3.0f, 0.0f, 1.0f);
        return InterpolatePresets(DuskPreset, NightPreset, Alpha);
    }
    else
    {
        return NightPreset;
    }
}

ELight_TimeOfDay ADayNightCycleManager::TimeToEnum(float TimeHours) const
{
    if (TimeHours < 5.0f)  return ELight_TimeOfDay::Night;
    if (TimeHours < 7.0f)  return ELight_TimeOfDay::Dawn;
    if (TimeHours < 11.0f) return ELight_TimeOfDay::Morning;
    if (TimeHours < 14.0f) return ELight_TimeOfDay::Midday;
    if (TimeHours < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (TimeHours < 21.0f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    FLight_TimeOfDayPreset Preset = GetPresetForTime(CurrentTimeOfDay);
    ApplyLightingPreset(Preset);
    CurrentTimeEnum = TimeToEnum(CurrentTimeOfDay);
}

float ADayNightCycleManager::GetNormalizedTime() const
{
    return CurrentTimeOfDay / 24.0f;
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum() const
{
    return CurrentTimeEnum;
}

void ADayNightCycleManager::ApplyPreset(ELight_TimeOfDay TimeOfDay)
{
    FLight_TimeOfDayPreset Preset;
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:      Preset = DawnPreset;   break;
        case ELight_TimeOfDay::Midday:    Preset = MiddayPreset; break;
        case ELight_TimeOfDay::Dusk:      Preset = DuskPreset;   break;
        case ELight_TimeOfDay::Night:
        case ELight_TimeOfDay::Midnight:  Preset = NightPreset;  break;
        default:                          Preset = MiddayPreset; break;
    }
    ApplyLightingPreset(Preset);
    CurrentTimeEnum = TimeOfDay;
}

void ADayNightCycleManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    // Find sky light
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }

    // Find height fog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        HeightFog = *It;
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("DayNightCycleManager: AutoFind — Sun=%s, SkyLight=%s, Fog=%s"),
        SunLight ? *SunLight->GetActorLabel() : TEXT("NULL"),
        SkyLightActor ? *SkyLightActor->GetActorLabel() : TEXT("NULL"),
        HeightFog ? *HeightFog->GetActorLabel() : TEXT("NULL"));
}
