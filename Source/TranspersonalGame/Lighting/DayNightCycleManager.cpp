#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    // Dawn preset — warm pink/orange, low sun, light fog
    DawnSettings.SunPitch = -8.0f;
    DawnSettings.SunYaw = -90.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;

    // Noon preset — bright white, high sun, minimal fog
    NoonSettings.SunPitch = -75.0f;
    NoonSettings.SunYaw = 0.0f;
    NoonSettings.SunIntensity = 12.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.5f, 0.65f, 0.8f, 1.0f);
    NoonSettings.SkyLightIntensity = 2.0f;

    // Dusk preset — warm orange/red, low sun, medium fog
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunYaw = 90.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.15f, 1.0f);
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogColor = FLinearColor(0.7f, 0.35f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.6f;

    // Night preset — cool blue, no direct sun, dense fog
    NightSettings.SunPitch = 30.0f; // Sun below horizon
    NightSettings.SunYaw = 180.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.15f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find references if not set in editor
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    if (!HeightFog)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    // Apply initial lighting for current time
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive)
    {
        return;
    }

    // Advance time: TimeScale hours per real second
    ElapsedSeconds += DeltaTime;
    float HoursElapsed = (ElapsedSeconds / 3600.0f) * TimeScale;
    CurrentTimeOfDay = FMath::Fmod(CurrentTimeOfDay + HoursElapsed, 24.0f);
    ElapsedSeconds = 0.0f; // Reset to avoid float accumulation

    UpdateSunPosition(CurrentTimeOfDay);
    UpdateFogSettings(CurrentTimeOfDay);
    UpdateSkyLight(CurrentTimeOfDay);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 13.0f) return ELight_TimeOfDay::Noon;
    if (CurrentTimeOfDay >= 13.0f && CurrentTimeOfDay < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition(CurrentTimeOfDay);
    UpdateFogSettings(CurrentTimeOfDay);
    UpdateSkyLight(CurrentTimeOfDay);
}

void ADayNightCycleManager::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        UDirectionalLightComponent* Comp = SunLight->GetComponent();
        if (Comp)
        {
            Comp->SetIntensity(Settings.SunIntensity);
            Comp->SetLightColor(Settings.SunColor);
        }
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SLComp = SkyLightActor->GetLightComponent();
        if (SLComp)
        {
            SLComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentTimeOfDay / 24.0f;
}

void ADayNightCycleManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight) return;

    // Map 0-24h to sun angle
    // 6h = sunrise (pitch=-5), 12h = noon (pitch=-75), 18h = sunset (pitch=-5), 0h = midnight (pitch=+30)
    float NormalizedTime = TimeHours / 24.0f;
    float SunAngle = NormalizedTime * 360.0f - 90.0f; // 0h = -90 degrees

    // Pitch: negative = illuminates ground, positive = below horizon
    float Pitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 80.0f;
    float Yaw = FMath::Cos(FMath::DegreesToRadians(SunAngle)) * 180.0f;

    SunLight->SetActorRotation(FRotator(Pitch, Yaw, 0.0f));

    // Intensity based on sun height
    UDirectionalLightComponent* Comp = SunLight->GetComponent();
    if (Comp)
    {
        float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));
        float Intensity = FMath::Max(0.0f, SunHeight) * 12.0f;

        // Warm color at dawn/dusk, white at noon
        float WarmFactor = 1.0f - FMath::Abs(SunHeight);
        FLinearColor SunColor(
            1.0f,
            FMath::Lerp(0.5f, 0.98f, FMath::Max(0.0f, SunHeight)),
            FMath::Lerp(0.2f, 0.9f, FMath::Max(0.0f, SunHeight)),
            1.0f
        );

        Comp->SetIntensity(FMath::Max(0.05f, Intensity));
        Comp->SetLightColor(SunColor);
    }
}

void ADayNightCycleManager::UpdateFogSettings(float TimeHours)
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (!FogComp) return;

    // More fog at dawn/dusk, less at noon, moderate at night
    float NormalizedTime = TimeHours / 24.0f;
    float SunAngle = NormalizedTime * 360.0f - 90.0f;
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));

    float FogDensity;
    FLinearColor FogColor;

    if (SunHeight > 0.5f) // Midday
    {
        FogDensity = 0.01f;
        FogColor = FLinearColor(0.5f, 0.65f, 0.85f, 1.0f);
    }
    else if (SunHeight > 0.0f) // Dawn/Dusk transition
    {
        float T = SunHeight / 0.5f;
        FogDensity = FMath::Lerp(0.04f, 0.01f, T);
        FogColor = FLinearColor(
            FMath::Lerp(0.7f, 0.5f, T),
            FMath::Lerp(0.4f, 0.65f, T),
            FMath::Lerp(0.3f, 0.85f, T),
            1.0f
        );
    }
    else // Night
    {
        FogDensity = 0.05f;
        FogColor = FLinearColor(0.05f, 0.08f, 0.15f, 1.0f);
    }

    FogComp->SetFogDensity(FogDensity);
    FogComp->SetFogInscatteringColor(FogColor);
}

void ADayNightCycleManager::UpdateSkyLight(float TimeHours)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SLComp = SkyLightActor->GetLightComponent();
    if (!SLComp) return;

    float NormalizedTime = TimeHours / 24.0f;
    float SunAngle = NormalizedTime * 360.0f - 90.0f;
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));

    float SkyIntensity = FMath::Lerp(0.15f, 2.0f, FMath::Max(0.0f, SunHeight));
    SLComp->SetIntensity(SkyIntensity);
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}
