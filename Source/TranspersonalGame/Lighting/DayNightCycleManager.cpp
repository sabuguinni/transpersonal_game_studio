#include "DayNightCycleManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    InitKeyframeDefaults();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneActors();
    ApplyCurrentStateToScene();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bCycleEnabled)
    {
        AdvanceCycle(DeltaTime);
    }
}

void ADayNightCycleManager::InitKeyframeDefaults()
{
    // Dawn — cool pink/purple light, low intensity
    DawnState.SunPitch = -5.0f;
    DawnState.SunYaw = -90.0f;
    DawnState.SunIntensity = 2.0f;
    DawnState.SunColor = FLinearColor(1.0f, 0.6f, 0.5f, 1.0f);
    DawnState.FogDensity = 0.04f;
    DawnState.FogColor = FLinearColor(0.8f, 0.7f, 0.9f, 1.0f);
    DawnState.SkyLightIntensity = 0.5f;
    DawnState.SkyLightColor = FLinearColor(0.5f, 0.5f, 0.8f, 1.0f);

    // Midday — bright white/blue, high intensity
    MiddayState.SunPitch = -75.0f;
    MiddayState.SunYaw = -60.0f;
    MiddayState.SunIntensity = 12.0f;
    MiddayState.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddayState.FogDensity = 0.01f;
    MiddayState.FogColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
    MiddayState.SkyLightIntensity = 2.0f;
    MiddayState.SkyLightColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    // Golden Hour — warm amber/orange, medium intensity
    GoldenHourState.SunPitch = -22.0f;
    GoldenHourState.SunYaw = -60.0f;
    GoldenHourState.SunIntensity = 8.0f;
    GoldenHourState.SunColor = FLinearColor(1.0f, 0.75f, 0.3f, 1.0f);
    GoldenHourState.FogDensity = 0.025f;
    GoldenHourState.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    GoldenHourState.SkyLightIntensity = 1.2f;
    GoldenHourState.SkyLightColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);

    // Night — deep blue, very low intensity
    NightState.SunPitch = 20.0f;
    NightState.SunYaw = 120.0f;
    NightState.SunIntensity = 0.1f;
    NightState.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightState.FogDensity = 0.05f;
    NightState.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightState.SkyLightIntensity = 0.3f;
    NightState.SkyLightColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);

    // Start at golden hour
    CurrentSunState = GoldenHourState;
    CurrentTimeOfDayNormalized = 0.35f;
}

void ADayNightCycleManager::AutoFindSceneActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    if (!SceneSkyLight)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SceneSkyLight = *It;
            break;
        }
    }
}

void ADayNightCycleManager::AdvanceCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    float AdvanceAmount = (DeltaTime * CycleSpeedMultiplier) / DayDurationSeconds;
    CurrentTimeOfDayNormalized = FMath::Fmod(CurrentTimeOfDayNormalized + AdvanceAmount, 1.0f);

    ELight_TimeOfDay NewEnum = TimeToEnum(CurrentTimeOfDayNormalized);
    if (NewEnum != CurrentTimeOfDayEnum)
    {
        ELight_TimeOfDay OldEnum = CurrentTimeOfDayEnum;
        CurrentTimeOfDayEnum = NewEnum;
        OnTimeOfDayChanged(NewEnum);

        if (NewEnum == ELight_TimeOfDay::Dawn)
        {
            OnDawnBegins();
        }
        else if (NewEnum == ELight_TimeOfDay::Night)
        {
            OnNightBegins();
        }
    }

    UpdateSceneLighting();
}

FLight_SunState ADayNightCycleManager::InterpolateSunStates(const FLight_SunState& A, const FLight_SunState& B, float Alpha) const
{
    FLight_SunState Result;
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

void ADayNightCycleManager::UpdateSceneLighting()
{
    // Determine which two keyframes to interpolate between
    // Timeline: 0.0=Dawn, 0.25=Midday, 0.5=GoldenHour, 0.75=Night, 1.0=Dawn
    FLight_SunState TargetState;
    float T = CurrentTimeOfDayNormalized;

    if (T < 0.25f)
    {
        // Dawn -> Midday
        TargetState = InterpolateSunStates(DawnState, MiddayState, T / 0.25f);
    }
    else if (T < 0.5f)
    {
        // Midday -> GoldenHour
        TargetState = InterpolateSunStates(MiddayState, GoldenHourState, (T - 0.25f) / 0.25f);
    }
    else if (T < 0.75f)
    {
        // GoldenHour -> Night
        TargetState = InterpolateSunStates(GoldenHourState, NightState, (T - 0.5f) / 0.25f);
    }
    else
    {
        // Night -> Dawn
        TargetState = InterpolateSunStates(NightState, DawnState, (T - 0.75f) / 0.25f);
    }

    CurrentSunState = TargetState;

    // Apply to scene actors
    if (SunLight)
    {
        // Clamp pitch to prevent going above horizon during day
        float ClampedPitch = FMath::Clamp(TargetState.SunPitch, -89.0f, 30.0f);
        SunLight->SetActorRotation(FRotator(ClampedPitch, TargetState.SunYaw, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(TargetState.SunIntensity);
            DLC->SetLightColor(TargetState.SunColor);
        }
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(TargetState.FogDensity);
            FogComp->SetFogInscatteringColor(TargetState.FogColor);
        }
    }

    if (SceneSkyLight)
    {
        USkyLightComponent* SLC = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(TargetState.SkyLightIntensity);
            SLC->SetLightColor(TargetState.SkyLightColor);
        }
    }
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDayEnum = TimeToEnum(CurrentTimeOfDayNormalized);
    UpdateSceneLighting();
}

float ADayNightCycleManager::GetCurrentHour() const
{
    return CurrentTimeOfDayNormalized * 24.0f;
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum() const
{
    return CurrentTimeOfDayEnum;
}

ELight_TimeOfDay ADayNightCycleManager::TimeToEnum(float NormalizedTime) const
{
    if (NormalizedTime < 0.05f)  return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.15f)  return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.30f)  return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.42f)  return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.55f)  return ELight_TimeOfDay::GoldenHour;
    if (NormalizedTime < 0.65f)  return ELight_TimeOfDay::Dusk;
    if (NormalizedTime < 0.85f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::ApplyCurrentStateToScene()
{
    AutoFindSceneActors();
    UpdateSceneLighting();
}
