#include "DayNightCycleManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

const float ADayNightCycleManager::SecondsPerDay = 86400.0f;

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/second for smooth cycle

    InitializeDefaultPalettes();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    UpdateLightingFromTime(CurrentTimeNormalized);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        AdvanceCycle(DeltaTime);
    }
}

void ADayNightCycleManager::InitializeDefaultPalettes()
{
    TimeOfDayPalettes.Empty();

    // MIDNIGHT (0.0)
    FLight_TimeOfDayPalette Midnight;
    Midnight.TimeOfDay = ELight_TimeOfDay::Midnight;
    Midnight.SunPitch = 80.0f;   // below horizon
    Midnight.SunYaw = 0.0f;
    Midnight.SunIntensity = 0.05f;
    Midnight.SunColor = FLinearColor(0.1f, 0.15f, 0.4f, 1.0f);
    Midnight.FogDensity = 0.06f;
    Midnight.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    Midnight.SkyLightIntensity = 0.3f;
    Midnight.SkyLightColor = FLinearColor(0.15f, 0.2f, 0.5f, 1.0f);
    Midnight.BloomScale = 0.1f;
    TimeOfDayPalettes.Add(Midnight);

    // DAWN (0.25)
    FLight_TimeOfDayPalette Dawn;
    Dawn.TimeOfDay = ELight_TimeOfDay::Dawn;
    Dawn.SunPitch = -5.0f;
    Dawn.SunYaw = 90.0f;
    Dawn.SunIntensity = 3.5f;
    Dawn.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    Dawn.FogDensity = 0.045f;
    Dawn.FogColor = FLinearColor(1.0f, 0.6f, 0.35f, 1.0f);
    Dawn.SkyLightIntensity = 1.2f;
    Dawn.SkyLightColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    Dawn.BloomScale = 0.8f;
    TimeOfDayPalettes.Add(Dawn);

    // MORNING (0.33)
    FLight_TimeOfDayPalette Morning;
    Morning.TimeOfDay = ELight_TimeOfDay::Morning;
    Morning.SunPitch = -35.0f;
    Morning.SunYaw = 120.0f;
    Morning.SunIntensity = 7.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.85f, 0.65f, 1.0f);
    Morning.FogDensity = 0.025f;
    Morning.FogColor = FLinearColor(0.9f, 0.85f, 0.75f, 1.0f);
    Morning.SkyLightIntensity = 1.8f;
    Morning.SkyLightColor = FLinearColor(0.95f, 0.9f, 0.85f, 1.0f);
    Morning.BloomScale = 0.45f;
    TimeOfDayPalettes.Add(Morning);

    // MIDDAY (0.5)
    FLight_TimeOfDayPalette Midday;
    Midday.TimeOfDay = ELight_TimeOfDay::Midday;
    Midday.SunPitch = -75.0f;
    Midday.SunYaw = 180.0f;
    Midday.SunIntensity = 12.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    Midday.FogDensity = 0.018f;
    Midday.FogColor = FLinearColor(0.85f, 0.90f, 1.0f, 1.0f);
    Midday.SkyLightIntensity = 2.2f;
    Midday.SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    Midday.BloomScale = 0.35f;
    TimeOfDayPalettes.Add(Midday);

    // AFTERNOON (0.625)
    FLight_TimeOfDayPalette Afternoon;
    Afternoon.TimeOfDay = ELight_TimeOfDay::Afternoon;
    Afternoon.SunPitch = -35.0f;
    Afternoon.SunYaw = 210.0f;
    Afternoon.SunIntensity = 8.5f;
    Afternoon.SunColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    Afternoon.FogDensity = 0.03f;
    Afternoon.FogColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    Afternoon.SkyLightIntensity = 2.0f;
    Afternoon.SkyLightColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);
    Afternoon.BloomScale = 0.5f;
    TimeOfDayPalettes.Add(Afternoon);

    // DUSK (0.75)
    FLight_TimeOfDayPalette Dusk;
    Dusk.TimeOfDay = ELight_TimeOfDay::Dusk;
    Dusk.SunPitch = -8.0f;
    Dusk.SunYaw = 270.0f;
    Dusk.SunIntensity = 2.5f;
    Dusk.SunColor = FLinearColor(1.0f, 0.4f, 0.15f, 1.0f);
    Dusk.FogDensity = 0.04f;
    Dusk.FogColor = FLinearColor(0.6f, 0.3f, 0.5f, 1.0f);
    Dusk.SkyLightIntensity = 1.8f;
    Dusk.SkyLightColor = FLinearColor(0.8f, 0.5f, 0.7f, 1.0f);
    Dusk.BloomScale = 1.0f;
    TimeOfDayPalettes.Add(Dusk);

    // NIGHT (0.875)
    FLight_TimeOfDayPalette Night;
    Night.TimeOfDay = ELight_TimeOfDay::Night;
    Night.SunPitch = 45.0f;
    Night.SunYaw = 315.0f;
    Night.SunIntensity = 0.1f;
    Night.SunColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
    Night.FogDensity = 0.05f;
    Night.FogColor = FLinearColor(0.08f, 0.1f, 0.25f, 1.0f);
    Night.SkyLightIntensity = 0.5f;
    Night.SkyLightColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    Night.BloomScale = 0.15f;
    TimeOfDayPalettes.Add(Night);
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        SceneFog = *It;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SceneSkyLight = *It;
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("DayNightCycleManager: AutoFind — Sun=%s, Fog=%s, SkyLight=%s"),
        SunLight ? *SunLight->GetName() : TEXT("NOT FOUND"),
        SceneFog ? *SceneFog->GetName() : TEXT("NOT FOUND"),
        SceneSkyLight ? *SceneSkyLight->GetName() : TEXT("NOT FOUND"));
}

void ADayNightCycleManager::AdvanceCycle(float DeltaTime)
{
    // Real seconds per in-game day = SecondsPerDay / CycleSpeedMultiplier
    float RealSecondsPerDay = SecondsPerDay / FMath::Max(CycleSpeedMultiplier, 0.001f);
    float DeltaNormalized = DeltaTime / RealSecondsPerDay;

    CurrentTimeNormalized = FMath::Fmod(CurrentTimeNormalized + DeltaNormalized, 1.0f);
    CurrentTimeOfDay = TimeNormalizedToEnum(CurrentTimeNormalized);

    UpdateLightingFromTime(CurrentTimeNormalized);
}

void ADayNightCycleManager::UpdateLightingFromTime(float TimeNormalized)
{
    if (TimeOfDayPalettes.Num() < 2) return;

    // Map time to palette index (7 palettes at: 0.0, 0.25, 0.33, 0.5, 0.625, 0.75, 0.875)
    static const float PaletteTimes[] = { 0.0f, 0.25f, 0.33f, 0.5f, 0.625f, 0.75f, 0.875f };
    const int32 NumPalettes = TimeOfDayPalettes.Num();

    int32 PrevIdx = 0;
    int32 NextIdx = 1;
    float Alpha = 0.0f;

    for (int32 i = 0; i < NumPalettes - 1; ++i)
    {
        if (TimeNormalized >= PaletteTimes[i] && TimeNormalized < PaletteTimes[i + 1])
        {
            PrevIdx = i;
            NextIdx = i + 1;
            float Range = PaletteTimes[i + 1] - PaletteTimes[i];
            Alpha = (Range > 0.0f) ? (TimeNormalized - PaletteTimes[i]) / Range : 0.0f;
            break;
        }
    }

    // Wrap around: between last palette and first (midnight wrap)
    if (TimeNormalized >= PaletteTimes[NumPalettes - 1])
    {
        PrevIdx = NumPalettes - 1;
        NextIdx = 0;
        float Range = 1.0f - PaletteTimes[NumPalettes - 1];
        Alpha = (Range > 0.0f) ? (TimeNormalized - PaletteTimes[NumPalettes - 1]) / Range : 0.0f;
    }

    FLight_TimeOfDayPalette Interpolated = InterpolatePalettes(
        TimeOfDayPalettes[PrevIdx],
        TimeOfDayPalettes[NextIdx],
        Alpha
    );

    ApplyPaletteToScene(Interpolated);
}

FLight_TimeOfDayPalette ADayNightCycleManager::InterpolatePalettes(
    const FLight_TimeOfDayPalette& A,
    const FLight_TimeOfDayPalette& B,
    float Alpha) const
{
    FLight_TimeOfDayPalette Result;
    Result.TimeOfDay = Alpha < 0.5f ? A.TimeOfDay : B.TimeOfDay;
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
    Result.BloomScale = FMath::Lerp(A.BloomScale, B.BloomScale, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplyPaletteToScene(const FLight_TimeOfDayPalette& Palette)
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
    if (SceneFog)
    {
        UExponentialHeightFogComponent* FogComp = SceneFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply to SkyLight
    if (SceneSkyLight)
    {
        USkyLightComponent* SLC = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Palette.SkyLightIntensity);
            SLC->SetLightColor(Palette.SkyLightColor);
        }
    }
}

void ADayNightCycleManager::ApplyTimeOfDayPalette(ELight_TimeOfDay TimeOfDay)
{
    for (const FLight_TimeOfDayPalette& Palette : TimeOfDayPalettes)
    {
        if (Palette.TimeOfDay == TimeOfDay)
        {
            ApplyPaletteToScene(Palette);
            return;
        }
    }
}

void ADayNightCycleManager::SetTimeNormalized(float NewTime)
{
    CurrentTimeNormalized = FMath::Clamp(NewTime, 0.0f, 1.0f);
    CurrentTimeOfDay = TimeNormalizedToEnum(CurrentTimeNormalized);
    UpdateLightingFromTime(CurrentTimeNormalized);
}

float ADayNightCycleManager::GetCurrentHour() const
{
    return CurrentTimeNormalized * 24.0f;
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

ELight_TimeOfDay ADayNightCycleManager::TimeNormalizedToEnum(float TimeNormalized) const
{
    if (TimeNormalized < 0.2f || TimeNormalized >= 0.95f) return ELight_TimeOfDay::Midnight;
    if (TimeNormalized < 0.29f) return ELight_TimeOfDay::Dawn;
    if (TimeNormalized < 0.42f) return ELight_TimeOfDay::Morning;
    if (TimeNormalized < 0.58f) return ELight_TimeOfDay::Midday;
    if (TimeNormalized < 0.7f)  return ELight_TimeOfDay::Afternoon;
    if (TimeNormalized < 0.82f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}
