#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    CurrentTimeOfDayHours = 14.0f; // Start at 2pm (golden hour approaching)
    DayDurationSeconds = 1200.0f;  // 20 real minutes = 1 game day
    bPauseDayNightCycle = false;
    CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    SunLight = nullptr;
    HeightFog = nullptr;
    SkyLightActor = nullptr;

    InitializeDefaultPalettes();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightingActorsInWorld();
    SetTimeOfDay(CurrentTimeOfDayHours);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bPauseDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }
}

void ADayNightCycleManager::InitializeDefaultPalettes()
{
    // DAWN — soft pink-lavender, low sun, cool mist
    DawnPalette.SunColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.0f);
    DawnPalette.SunIntensity = 3.5f;
    DawnPalette.SunPitch = -8.0f;
    DawnPalette.FogColor = FLinearColor(0.78f, 0.65f, 0.82f, 1.0f);
    DawnPalette.FogDensity = 0.045f;
    DawnPalette.SkyLightColor = FLinearColor(0.72f, 0.68f, 0.88f, 1.0f);
    DawnPalette.SkyLightIntensity = 1.2f;

    // MIDDAY — harsh white sun, deep blue sky, minimal fog
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.98f, 0.88f, 1.0f);
    MiddayPalette.SunIntensity = 14.0f;
    MiddayPalette.SunPitch = -75.0f;
    MiddayPalette.FogColor = FLinearColor(0.55f, 0.68f, 0.88f, 1.0f);
    MiddayPalette.FogDensity = 0.012f;
    MiddayPalette.SkyLightColor = FLinearColor(0.82f, 0.88f, 1.0f, 1.0f);
    MiddayPalette.SkyLightIntensity = 3.5f;

    // GOLDEN HOUR — warm amber, low sun, hazy warm fog
    GoldenHourPalette.SunColor = FLinearColor(1.0f, 0.72f, 0.31f, 1.0f);
    GoldenHourPalette.SunIntensity = 8.5f;
    GoldenHourPalette.SunPitch = -18.0f;
    GoldenHourPalette.FogColor = FLinearColor(0.92f, 0.62f, 0.28f, 1.0f);
    GoldenHourPalette.FogDensity = 0.028f;
    GoldenHourPalette.SkyLightColor = FLinearColor(1.0f, 0.82f, 0.62f, 1.0f);
    GoldenHourPalette.SkyLightIntensity = 2.8f;

    // NIGHT — deep blue moonlight, dense fog, low intensity
    NightPalette.SunColor = FLinearColor(0.18f, 0.22f, 0.48f, 1.0f);
    NightPalette.SunIntensity = 0.8f;
    NightPalette.SunPitch = -72.0f;
    NightPalette.FogColor = FLinearColor(0.08f, 0.10f, 0.28f, 1.0f);
    NightPalette.FogDensity = 0.042f;
    NightPalette.SkyLightColor = FLinearColor(0.15f, 0.18f, 0.38f, 1.0f);
    NightPalette.SkyLightIntensity = 0.4f;
}

void ADayNightCycleManager::FindLightingActorsInWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }

    // Find ExponentialHeightFog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    if (FoundFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
    }

    // Find SkyLight
    TArray<AActor*> FoundSkylights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkylights);
    if (FoundSkylights.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundSkylights[0]);
    }
}

void ADayNightCycleManager::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDayHours = FMath::Fmod(Hours, 24.0f);
    if (CurrentTimeOfDayHours < 0.0f) CurrentTimeOfDayHours += 24.0f;

    CurrentTimeOfDay = GetTimeOfDayEnum();
    FLight_SkyPalette TargetPalette = GetPaletteForTime(CurrentTimeOfDayHours);
    ApplyPalette(TargetPalette);
}

void ADayNightCycleManager::ApplyPalette(const FLight_SkyPalette& Palette)
{
    CurrentPalette = Palette;

    // Apply sun settings
    if (SunLight)
    {
        FRotator SunRot = SunLight->GetActorRotation();
        SunRot.Pitch = Palette.SunPitch;
        SunLight->SetActorRotation(SunRot);

        UDirectionalLightComponent* SunComp = SunLight->GetComponent();
        if (SunComp)
        {
            SunComp->SetIntensity(Palette.SunIntensity);
            SunComp->SetLightColor(Palette.SunColor);
        }
    }

    // Apply fog settings
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply skylight settings
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
            SkyComp->SetLightColor(Palette.SkyLightColor);
        }
    }
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum() const
{
    float H = CurrentTimeOfDayHours;
    if (H >= 5.0f && H < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (H >= 7.0f && H < 10.0f)  return ELight_TimeOfDay::Morning;
    if (H >= 10.0f && H < 13.0f) return ELight_TimeOfDay::Midday;
    if (H >= 13.0f && H < 16.0f) return ELight_TimeOfDay::Afternoon;
    if (H >= 16.0f && H < 18.5f) return ELight_TimeOfDay::GoldenHour;
    if (H >= 18.5f && H < 20.0f) return ELight_TimeOfDay::Dusk;
    if (H >= 20.0f || H < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

FLight_SkyPalette ADayNightCycleManager::InterpolatePalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const
{
    FLight_SkyPalette Result;
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightColor = FMath::Lerp(A.SkyLightColor, B.SkyLightColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

FLight_SkyPalette ADayNightCycleManager::GetPaletteForTime(float Hours) const
{
    // Smooth interpolation between key palettes based on time of day
    if (Hours >= 5.0f && Hours < 10.0f)
    {
        float Alpha = (Hours - 5.0f) / 5.0f;
        return InterpolatePalettes(DawnPalette, MiddayPalette, Alpha);
    }
    else if (Hours >= 10.0f && Hours < 16.0f)
    {
        return MiddayPalette;
    }
    else if (Hours >= 16.0f && Hours < 20.0f)
    {
        float Alpha = (Hours - 16.0f) / 4.0f;
        return InterpolatePalettes(GoldenHourPalette, NightPalette, Alpha);
    }
    else
    {
        return NightPalette;
    }
}

float ADayNightCycleManager::GetNormalizedTimeOfDay() const
{
    return CurrentTimeOfDayHours / 24.0f;
}

bool ADayNightCycleManager::IsNightTime() const
{
    return CurrentTimeOfDayHours >= 20.0f || CurrentTimeOfDayHours < 5.0f;
}

bool ADayNightCycleManager::IsDaytime() const
{
    return CurrentTimeOfDayHours >= 7.0f && CurrentTimeOfDayHours < 18.5f;
}

void ADayNightCycleManager::TickDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    // Advance time: 24 hours over DayDurationSeconds real seconds
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    float NewHours = CurrentTimeOfDayHours + (HoursPerSecond * DeltaTime);
    SetTimeOfDay(NewHours);
}
