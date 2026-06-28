#include "DayNightCycleManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    // ── Dawn defaults ─────────────────────────────────────────
    DawnData.SunPitchDegrees = -5.0f;
    DawnData.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f); // Orange-red
    DawnData.SunIntensity = 2.0f;
    DawnData.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DawnData.FogDensity = 0.04f;
    DawnData.AmbientIntensity = 0.1f;

    // ── Midday defaults ───────────────────────────────────────
    MiddayData.SunPitchDegrees = -75.0f;
    MiddayData.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f); // Bright white-yellow
    MiddayData.SunIntensity = 15.0f;
    MiddayData.FogInscatteringColor = FLinearColor(0.5f, 0.65f, 0.9f, 1.0f);
    MiddayData.FogDensity = 0.01f;
    MiddayData.AmbientIntensity = 0.5f;

    // ── Dusk defaults ─────────────────────────────────────────
    DuskData.SunPitchDegrees = -8.0f;
    DuskData.SunColor = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f); // Deep amber
    DuskData.SunIntensity = 3.0f;
    DuskData.FogInscatteringColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    DuskData.FogDensity = 0.05f;
    DuskData.AmbientIntensity = 0.15f;

    // ── Night defaults ────────────────────────────────────────
    NightData.SunPitchDegrees = -90.0f;
    NightData.SunColor = FLinearColor(0.05f, 0.05f, 0.2f, 1.0f); // Deep blue moonlight
    NightData.SunIntensity = 0.5f;
    NightData.FogInscatteringColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    NightData.FogDensity = 0.06f;
    NightData.AmbientIntensity = 0.05f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    SetTimeOfDay(CurrentTimeHours);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bCycleActive)
    {
        AdvanceTime(DeltaTime);
    }
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Find ExponentialHeightFog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }
}

void ADayNightCycleManager::SetTimeOfDay(float NewHours)
{
    CurrentTimeHours = FMath::Fmod(NewHours, 24.0f);
    if (CurrentTimeHours < 0.0f) CurrentTimeHours += 24.0f;
    PreviousTimeHours = CurrentTimeHours;
    UpdateSunPosition();
}

void ADayNightCycleManager::AdvanceTime(float DeltaTime)
{
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeHours += HoursPerSecond * DeltaTime;
    if (CurrentTimeHours >= 24.0f) CurrentTimeHours -= 24.0f;
    UpdateSunPosition();
}

void ADayNightCycleManager::UpdateSunPosition()
{
    ELight_TimeOfDay NewPhase = ComputePhaseFromHour(CurrentTimeHours);
    CurrentPhase = NewPhase;

    // Compute blend between phases based on hour
    float BlendAlpha = 0.0f;
    FLight_SunPhaseData PhaseA, PhaseB;

    switch (CurrentPhase)
    {
    case ELight_TimeOfDay::Dawn:
        PhaseA = NightData;
        PhaseB = DawnData;
        BlendAlpha = FMath::GetRangePct(4.0f, 6.5f, CurrentTimeHours);
        break;
    case ELight_TimeOfDay::Morning:
        PhaseA = DawnData;
        PhaseB = MiddayData;
        BlendAlpha = FMath::GetRangePct(6.5f, 12.0f, CurrentTimeHours);
        break;
    case ELight_TimeOfDay::Midday:
        PhaseA = MiddayData;
        PhaseB = MiddayData;
        BlendAlpha = 1.0f;
        break;
    case ELight_TimeOfDay::Afternoon:
        PhaseA = MiddayData;
        PhaseB = DuskData;
        BlendAlpha = FMath::GetRangePct(13.0f, 18.0f, CurrentTimeHours);
        break;
    case ELight_TimeOfDay::Dusk:
        PhaseA = DuskData;
        PhaseB = NightData;
        BlendAlpha = FMath::GetRangePct(18.0f, 20.0f, CurrentTimeHours);
        break;
    case ELight_TimeOfDay::Night:
    case ELight_TimeOfDay::DeepNight:
        PhaseA = NightData;
        PhaseB = NightData;
        BlendAlpha = 1.0f;
        break;
    }

    FLight_SunPhaseData CurrentData = LerpPhaseData(PhaseA, PhaseB, FMath::Clamp(BlendAlpha, 0.0f, 1.0f));
    ApplyPhaseData(CurrentData);
}

ELight_TimeOfDay ADayNightCycleManager::ComputePhaseFromHour(float Hour) const
{
    if (Hour >= 4.0f && Hour < 6.5f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 6.5f && Hour < 12.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 12.0f && Hour < 13.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 13.0f && Hour < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 18.0f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f && Hour < 22.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::DeepNight;
}

FLight_SunPhaseData ADayNightCycleManager::LerpPhaseData(const FLight_SunPhaseData& A, const FLight_SunPhaseData& B, float Alpha) const
{
    FLight_SunPhaseData Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.FogInscatteringColor = FLinearColor::LerpUsingHSV(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.AmbientIntensity = FMath::Lerp(A.AmbientIntensity, B.AmbientIntensity, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplyPhaseData(const FLight_SunPhaseData& Data)
{
    // Apply sun pitch — enforce guard: never above -3 degrees (avoid zenith artifacts)
    float SafePitch = FMath::Min(Data.SunPitchDegrees, -3.0f);

    if (SunLight)
    {
        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(SafePitch, CurrentRot.Yaw, CurrentRot.Roll));

        UDirectionalLightComponent* DLC = SunLight->GetComponent();
        if (DLC)
        {
            DLC->SetLightColor(Data.SunColor);
            DLC->SetIntensity(Data.SunIntensity);
        }
    }

    // Apply fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogInscatteringColor(Data.FogInscatteringColor);
            FogComp->SetFogDensity(Data.FogDensity);
        }
    }

    // Recapture sky light for accurate ambient
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetLightComponent();
        if (SLC && SLC->IsRealTimeCapture())
        {
            SLC->SetIntensity(Data.AmbientIntensity * 3.0f);
        }
    }
}
