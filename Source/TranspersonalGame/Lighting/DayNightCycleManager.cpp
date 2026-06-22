#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth cycle

    // Dawn preset — warm pink-orange light, dense morning fog
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.AmbientTemperature = 3500.0f;

    // Noon preset — bright white light, minimal fog
    NoonSettings.SunIntensity = 12.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    NoonSettings.SunPitch = -80.0f;
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.5f, 0.65f, 0.8f, 1.0f);
    NoonSettings.SkyLightIntensity = 2.0f;
    NoonSettings.AmbientTemperature = 6500.0f;

    // Dusk preset — deep amber/red, moderate fog
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.1f, 1.0f);
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.7f, 0.4f, 0.3f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.6f;
    DuskSettings.AmbientTemperature = 2800.0f;

    // Night preset — moonlight blue, heavy fog
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
    NightSettings.SunPitch = -20.0f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.AmbientTemperature = 8000.0f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    LastPhase = CalculatePhase(CurrentTimeHours);
    CurrentPhase = LastPhase;
    ApplyCurrentLighting();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        AdvanceTime(DeltaTime);
        UpdateLighting();
    }
}

void ADayNightCycleManager::AdvanceTime(float DeltaTime)
{
    // TimeScale = how many in-game hours pass per real second
    // e.g. TimeScale=60 → 1 real second = 1 in-game minute
    const float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeHours += DeltaTime * HoursPerSecond;

    if (CurrentTimeHours >= 24.0f)
    {
        CurrentTimeHours -= 24.0f;
    }

    // Check for phase transition
    ELight_TimeOfDay NewPhase = CalculatePhase(CurrentTimeHours);
    if (NewPhase != LastPhase)
    {
        CurrentPhase = NewPhase;
        OnPhaseChanged(NewPhase);
        LastPhase = NewPhase;
    }
}

void ADayNightCycleManager::UpdateLighting()
{
    UpdateSunPosition();
    UpdateFog();
    UpdateSkyLight();
}

void ADayNightCycleManager::UpdateSunPosition()
{
    if (!SunActor)
    {
        // Auto-find directional light if not assigned
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunActor = FoundActors[0];
        }
        else
        {
            return;
        }
    }

    FLight_TimeOfDaySettings Settings = GetSettingsForHour(CurrentTimeHours);

    // Calculate sun yaw from time (0h = east, 12h = west)
    const float SunYaw = (CurrentTimeHours / 24.0f) * 360.0f - 90.0f;

    SunActor->SetActorRotation(FRotator(Settings.SunPitch, SunYaw, 0.0f));

    UDirectionalLightComponent* DirLight = SunActor->FindComponentByClass<UDirectionalLightComponent>();
    if (DirLight)
    {
        DirLight->SetIntensity(Settings.SunIntensity);
        DirLight->SetLightColor(Settings.SunColor);
        DirLight->SetTemperature(Settings.AmbientTemperature);
    }
}

void ADayNightCycleManager::UpdateFog()
{
    if (!FogActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            FogActor = FoundActors[0];
        }
        else
        {
            return;
        }
    }

    FLight_TimeOfDaySettings Settings = GetSettingsForHour(CurrentTimeHours);

    UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ADayNightCycleManager::UpdateSkyLight()
{
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = FoundActors[0];
        }
        else
        {
            return;
        }
    }

    FLight_TimeOfDaySettings Settings = GetSettingsForHour(CurrentTimeHours);

    USkyLightComponent* SkyComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (SkyComp)
    {
        SkyComp->SetIntensity(Settings.SkyLightIntensity);
    }
}

ELight_TimeOfDay ADayNightCycleManager::CalculatePhase(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 11.0f && Hour < 14.0f) return ELight_TimeOfDay::Noon;
    if (Hour >= 14.0f && Hour < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 18.0f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f && Hour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForHour(float Hour) const
{
    // Smooth interpolation between key time presets
    if (Hour >= 5.0f && Hour < 12.0f)
    {
        // Dawn → Noon
        const float Alpha = (Hour - 5.0f) / 7.0f;
        return InterpolateSettings(DawnSettings, NoonSettings, Alpha);
    }
    else if (Hour >= 12.0f && Hour < 19.0f)
    {
        // Noon → Dusk
        const float Alpha = (Hour - 12.0f) / 7.0f;
        return InterpolateSettings(NoonSettings, DuskSettings, Alpha);
    }
    else if (Hour >= 19.0f && Hour < 22.0f)
    {
        // Dusk → Night
        const float Alpha = (Hour - 19.0f) / 3.0f;
        return InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night / pre-dawn
        return NightSettings;
    }
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentTimeHours = FMath::Clamp(NewHour, 0.0f, 24.0f);
    CurrentPhase = CalculatePhase(CurrentTimeHours);
    ApplyCurrentLighting();
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentPhase() const
{
    return CurrentPhase;
}

void ADayNightCycleManager::ApplyCurrentLighting()
{
    UpdateLighting();
}

void ADayNightCycleManager::OnPhaseChanged_Implementation(ELight_TimeOfDay NewPhase)
{
    // Blueprint can override this to trigger events (weather changes, creature behaviour, etc.)
    UE_LOG(LogTemp, Log, TEXT("DayNightCycle: Phase changed to %d at hour %.1f"), (int32)NewPhase, CurrentTimeHours);
}
