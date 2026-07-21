#include "NightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ANightCycleManager::ANightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance

    InitializeDefaultPalettes();
}

void ANightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun/moon light if not set
    if (!SunMoonLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunMoonLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    // Auto-find sky light if not set
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }

    ApplyCurrentPalette();
}

void ANightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableRealTimeCycle)
    {
        return;
    }

    // Advance time
    ElapsedDayTime += DeltaTime;
    if (ElapsedDayTime >= DayDurationSeconds)
    {
        ElapsedDayTime = 0.0f;
    }

    // Convert elapsed time to 0-24 hour range
    float NewHour = (ElapsedDayTime / DayDurationSeconds) * 24.0f;
    if (FMath::Abs(NewHour - CurrentHour) > 0.05f)
    {
        CurrentHour = NewHour;
        UpdateCycleFromHour(CurrentHour);
    }
}

void ANightCycleManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    ApplyCurrentPalette();
}

void ANightCycleManager::SetHour(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    UpdateCycleFromHour(CurrentHour);
}

ELight_TimeOfDay ANightCycleManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

float ANightCycleManager::GetCurrentHour() const
{
    return CurrentHour;
}

bool ANightCycleManager::IsNightTime() const
{
    return CurrentTimeOfDay == ELight_TimeOfDay::Night ||
           CurrentTimeOfDay == ELight_TimeOfDay::Midnight ||
           CurrentTimeOfDay == ELight_TimeOfDay::Dusk;
}

float ANightCycleManager::GetNightDangerMultiplier() const
{
    // Night increases danger — predators are more active
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Midnight:    return 2.5f;
        case ELight_TimeOfDay::Night:       return 2.0f;
        case ELight_TimeOfDay::Dusk:        return 1.5f;
        case ELight_TimeOfDay::Dawn:        return 1.3f;
        case ELight_TimeOfDay::GoldenHour:  return 1.1f;
        case ELight_TimeOfDay::Morning:     return 0.8f;
        case ELight_TimeOfDay::Midday:      return 1.0f;
        default:                            return 1.0f;
    }
}

void ANightCycleManager::ApplyCurrentPalette()
{
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:        ApplyPalette(DawnPalette);        break;
        case ELight_TimeOfDay::Morning:     ApplyPalette(MorningPalette);     break;
        case ELight_TimeOfDay::Midday:      ApplyPalette(MiddayPalette);      break;
        case ELight_TimeOfDay::GoldenHour:  ApplyPalette(GoldenHourPalette);  break;
        case ELight_TimeOfDay::Dusk:        ApplyPalette(DuskPalette);        break;
        case ELight_TimeOfDay::Night:       ApplyPalette(NightPalette);       break;
        case ELight_TimeOfDay::Midnight:    ApplyPalette(MidnightPalette);    break;
    }
}

void ANightCycleManager::ApplyNightPaletteImmediate()
{
    CurrentTimeOfDay = ELight_TimeOfDay::Night;
    CurrentHour = 22.0f;
    ApplyPalette(NightPalette);
}

void ANightCycleManager::ApplyPalette(const FLight_SkyPalette& Palette)
{
    if (!SunMoonLight)
    {
        return;
    }

    // Apply sun/moon light settings
    UDirectionalLightComponent* DLComp = SunMoonLight->GetComponentByClass<UDirectionalLightComponent>();
    if (DLComp)
    {
        DLComp->SetIntensity(Palette.SunIntensity);
        DLComp->SetLightColor(Palette.SunColor);
    }

    // Apply sun pitch (rotation)
    FRotator CurrentRot = SunMoonLight->GetActorRotation();
    SunMoonLight->SetActorRotation(FRotator(Palette.SunPitchAngle, CurrentRot.Yaw, CurrentRot.Roll));

    // Apply sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SLComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLComp)
        {
            SLComp->SetIntensity(Palette.SkyLightIntensity);
        }
    }
}

void ANightCycleManager::UpdateCycleFromHour(float Hour)
{
    ELight_TimeOfDay NewTOD = HourToTimeOfDay(Hour);
    if (NewTOD != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTOD;
        ApplyCurrentPalette();
    }

    // Update sun pitch continuously for smooth transition
    if (SunMoonLight)
    {
        float TargetPitch = TimeOfDayToSunPitch(Hour);
        FRotator CurrentRot = SunMoonLight->GetActorRotation();
        float SmoothedPitch = FMath::FInterpTo(CurrentRot.Pitch, TargetPitch, 0.5f, 0.5f);
        SunMoonLight->SetActorRotation(FRotator(SmoothedPitch, CurrentRot.Yaw, CurrentRot.Roll));
    }
}

ELight_TimeOfDay ANightCycleManager::HourToTimeOfDay(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 11.0f && Hour < 15.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 15.0f && Hour < 18.0f) return ELight_TimeOfDay::GoldenHour;
    if (Hour >= 18.0f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f && Hour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight; // 23:00 - 05:00
}

float ANightCycleManager::TimeOfDayToSunPitch(float Hour) const
{
    // Map 0-24h to sun pitch angle
    // Noon (12h) = -90 degrees (directly overhead)
    // Midnight (0h/24h) = +10 degrees (below horizon)
    // Dawn (6h) = -5 degrees (just rising)
    // Dusk (18h) = -5 degrees (just setting)

    // Normalize to 0-1 range where 0.5 = noon
    float NormalizedTime = Hour / 24.0f;
    float SunAngle = FMath::Sin(NormalizedTime * PI * 2.0f - PI * 0.5f);
    // Map sin(-1 to 1) to pitch (-90 to +15)
    return FMath::Lerp(15.0f, -90.0f, (SunAngle + 1.0f) * 0.5f);
}

void ANightCycleManager::InitializeDefaultPalettes()
{
    // --- DAWN: Warm pink-orange horizon, cool purple sky ---
    DawnPalette.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnPalette.SunIntensity = 2.0f;
    DawnPalette.SunPitchAngle = -5.0f;
    DawnPalette.FogColor = FLinearColor(0.4f, 0.25f, 0.35f, 1.0f);
    DawnPalette.FogDensity = 0.06f;
    DawnPalette.SkyLightIntensity = 0.6f;

    // --- MORNING: Warm yellow-white sun, clear blue sky ---
    MorningPalette.SunColor = FLinearColor(1.0f, 0.92f, 0.7f, 1.0f);
    MorningPalette.SunIntensity = 8.0f;
    MorningPalette.SunPitchAngle = -30.0f;
    MorningPalette.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    MorningPalette.FogDensity = 0.03f;
    MorningPalette.SkyLightIntensity = 1.0f;

    // --- MIDDAY: Harsh white sun, bright sky ---
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddayPalette.SunIntensity = 10.0f;
    MiddayPalette.SunPitchAngle = -75.0f;
    MiddayPalette.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MiddayPalette.FogDensity = 0.015f;
    MiddayPalette.SkyLightIntensity = 1.5f;

    // --- GOLDEN HOUR: Deep amber-orange, long shadows ---
    GoldenHourPalette.SunColor = FLinearColor(1.0f, 0.55f, 0.1f, 1.0f);
    GoldenHourPalette.SunIntensity = 6.0f;
    GoldenHourPalette.SunPitchAngle = -20.0f;
    GoldenHourPalette.FogColor = FLinearColor(0.6f, 0.35f, 0.15f, 1.0f);
    GoldenHourPalette.FogDensity = 0.04f;
    GoldenHourPalette.SkyLightIntensity = 0.9f;

    // --- DUSK: Deep orange-red, purple horizon ---
    DuskPalette.SunColor = FLinearColor(1.0f, 0.3f, 0.05f, 1.0f);
    DuskPalette.SunIntensity = 3.0f;
    DuskPalette.SunPitchAngle = -8.0f;
    DuskPalette.FogColor = FLinearColor(0.35f, 0.15f, 0.3f, 1.0f);
    DuskPalette.FogDensity = 0.07f;
    DuskPalette.SkyLightIntensity = 0.5f;

    // --- NIGHT: Cool moonlight blue, dense fog ---
    NightPalette.SunColor = FLinearColor(0.55f, 0.65f, 1.0f, 1.0f);
    NightPalette.SunIntensity = 0.8f;
    NightPalette.SunPitchAngle = -15.0f;
    NightPalette.FogColor = FLinearColor(0.05f, 0.07f, 0.18f, 1.0f);
    NightPalette.FogDensity = 0.08f;
    NightPalette.SkyLightIntensity = 0.3f;

    // --- MIDNIGHT: Near-black, deep indigo, maximum danger ---
    MidnightPalette.SunColor = FLinearColor(0.3f, 0.35f, 0.7f, 1.0f);
    MidnightPalette.SunIntensity = 0.3f;
    MidnightPalette.SunPitchAngle = 10.0f; // Below horizon
    MidnightPalette.FogColor = FLinearColor(0.02f, 0.03f, 0.1f, 1.0f);
    MidnightPalette.FogDensity = 0.12f;
    MidnightPalette.SkyLightIntensity = 0.15f;
}
