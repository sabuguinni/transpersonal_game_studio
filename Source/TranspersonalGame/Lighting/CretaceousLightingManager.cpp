#include "CretaceousLightingManager.h"

#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 Hz — sufficient for smooth blending

    BuildDefaultPresets();
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    AutoFindSceneLights();
    EnforceCAPRules();
    ApplyPreset(ActivePresetIndex);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── Preset blending ───────────────────────────────────────────────────────
    if (bBlending)
    {
        BlendElapsed += DeltaTime;
        const float Duration = FMath::Max(BlendTarget.BlendDuration, 0.1f);
        BlendAlpha = FMath::Clamp(BlendElapsed / Duration, 0.0f, 1.0f);

        FLight_DayPreset Interpolated = LerpPresets(BlendSource, BlendTarget, BlendAlpha);
        ApplyPresetToScene(Interpolated);

        if (BlendAlpha >= 1.0f)
        {
            bBlending = false;
            BlendAlpha = 1.0f;
        }
    }

    // ── Auto day/night cycle ──────────────────────────────────────────────────
    if (bAutoCycle && !bBlending)
    {
        NormalisedTimeOfDay += DeltaTime / FMath::Max(SecondsPerDay, 60.0f);
        if (NormalisedTimeOfDay > 1.0f)
        {
            NormalisedTimeOfDay -= 1.0f;
        }

        // Map normalised time to sun pitch: noon=-90, horizon=-10 (clamped to -30 by CAP)
        // 0.0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1.0=midnight
        const float SunAngle = FMath::Sin(NormalisedTimeOfDay * PI * 2.0f - PI * 0.5f) * 80.0f - 10.0f;
        const float ClampedPitch = FMath::Min(SunAngle, -30.0f); // CAP: never above -30

        if (SunLight)
        {
            FRotator CurrentRot = SunLight->GetActorRotation();
            SunLight->SetActorRotation(FRotator(ClampedPitch, CurrentRot.Yaw, CurrentRot.Roll));
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyPreset
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::ApplyPreset(int32 PresetIndex)
{
    if (!DayPresets.IsValidIndex(PresetIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLightingManager: Invalid preset index %d"), PresetIndex);
        return;
    }

    ActivePresetIndex = PresetIndex;
    bBlending = false;
    BlendAlpha = 1.0f;
    ApplyPresetToScene(DayPresets[PresetIndex]);
}

// ─────────────────────────────────────────────────────────────────────────────
// BlendToPreset
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::BlendToPreset(int32 PresetIndex)
{
    if (!DayPresets.IsValidIndex(PresetIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLightingManager: Invalid blend target index %d"), PresetIndex);
        return;
    }

    // Capture current state as blend source
    if (DayPresets.IsValidIndex(ActivePresetIndex))
    {
        BlendSource = DayPresets[ActivePresetIndex];
    }
    BlendTarget = DayPresets[PresetIndex];
    ActivePresetIndex = PresetIndex;

    BlendElapsed = 0.0f;
    BlendAlpha = 0.0f;
    bBlending = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyGoldenHourDefault
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::ApplyGoldenHourDefault()
{
    // Find golden hour preset
    for (int32 i = 0; i < DayPresets.Num(); ++i)
    {
        if (DayPresets[i].TimeOfDay == ELight_TimeOfDay::GoldenHour)
        {
            ApplyPreset(i);
            return;
        }
    }
    // Fallback: apply index 3 (default golden hour position)
    ApplyPreset(3);
}

// ─────────────────────────────────────────────────────────────────────────────
// EnforceCAPRules
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::EnforceCAPRules()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // CAP Rule 1: Sun pitch must never exceed -30 degrees
    if (SunLight)
    {
        FRotator Rot = SunLight->GetActorRotation();
        if (Rot.Pitch > -30.0f)
        {
            SunLight->SetActorRotation(FRotator(-30.0f, Rot.Yaw, Rot.Roll));
            UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: CAP — Sun pitch clamped to -30"));
        }
    }

    // CAP Rule 2: Exactly 1 ExponentialHeightFog in the level
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 1)
    {
        for (int32 i = 1; i < FogActors.Num(); ++i)
        {
            FogActors[i]->Destroy();
            UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: CAP — Duplicate fog removed"));
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetCurrentPresetName
// ─────────────────────────────────────────────────────────────────────────────

FString ACretaceousLightingManager::GetCurrentPresetName() const
{
    if (!DayPresets.IsValidIndex(ActivePresetIndex))
    {
        return TEXT("Unknown");
    }

    const ELight_TimeOfDay TOD = DayPresets[ActivePresetIndex].TimeOfDay;
    switch (TOD)
    {
        case ELight_TimeOfDay::Dawn:       return TEXT("Dawn");
        case ELight_TimeOfDay::Morning:    return TEXT("Morning");
        case ELight_TimeOfDay::Midday:     return TEXT("Midday");
        case ELight_TimeOfDay::GoldenHour: return TEXT("Golden Hour");
        case ELight_TimeOfDay::Dusk:       return TEXT("Dusk");
        case ELight_TimeOfDay::Night:      return TEXT("Night");
        case ELight_TimeOfDay::Overcast:   return TEXT("Overcast");
        case ELight_TimeOfDay::Storm:      return TEXT("Storm");
        default:                           return TEXT("Unknown");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyPresetToScene (private)
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::ApplyPresetToScene(const FLight_DayPreset& Preset)
{
    // ── DirectionalLight (Sun) ────────────────────────────────────────────────
    if (SunLight)
    {
        // CAP: clamp pitch
        const float SafePitch = FMath::Min(Preset.SunPitch, -30.0f);
        SunLight->SetActorRotation(FRotator(SafePitch, Preset.SunYaw, 0.0f));

        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Preset.SunIntensity);
            DLC->SetLightColor(Preset.SunColor.ToFColor(true));
        }
    }

    // ── SkyLight ──────────────────────────────────────────────────────────────
    if (SceneSkyLight)
    {
        USkyLightComponent* SLC = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Preset.SkyIntensity);
        }
    }

    // ── ExponentialHeightFog ──────────────────────────────────────────────────
    if (SceneFog)
    {
        UExponentialHeightFogComponent* FogComp = SceneFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Preset.FogDensity);
            FogComp->SetFogInscatteringColor(Preset.FogColor.ToFColor(true));
            FogComp->SetVolumetricFog(Preset.bVolumetricFog);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// LerpPresets (private)
// ─────────────────────────────────────────────────────────────────────────────

FLight_DayPreset ACretaceousLightingManager::LerpPresets(
    const FLight_DayPreset& A,
    const FLight_DayPreset& B,
    float Alpha) const
{
    FLight_DayPreset Result;
    Result.TimeOfDay = B.TimeOfDay;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.bVolumetricFog = B.bVolumetricFog;
    Result.VolumetricFogScattering = FMath::Lerp(A.VolumetricFogScattering, B.VolumetricFogScattering, Alpha);
    Result.BlendDuration = B.BlendDuration;
    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// AutoFindSceneLights (private)
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::AutoFindSceneLights()
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

    if (!SceneSkyLight)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SceneSkyLight = *It;
            break;
        }
    }

    if (!SceneFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            SceneFog = *It;
            break;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildDefaultPresets (private)
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::BuildDefaultPresets()
{
    DayPresets.Empty();

    // 0 — Dawn
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Dawn;
        P.SunPitch = -8.0f; // clamped to -30 by CAP
        P.SunYaw = -90.0f;
        P.SunIntensity = 2.0f;
        P.SunColor = FLinearColor(1.0f, 0.55f, 0.3f, 1.0f);
        P.SkyIntensity = 0.8f;
        P.FogDensity = 0.06f;
        P.FogColor = FLinearColor(0.7f, 0.6f, 0.8f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.5f;
        P.BlendDuration = 8.0f;
        DayPresets.Add(P);
    }

    // 1 — Morning
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Morning;
        P.SunPitch = -45.0f;
        P.SunYaw = -60.0f;
        P.SunIntensity = 6.0f;
        P.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        P.SkyIntensity = 1.4f;
        P.FogDensity = 0.025f;
        P.FogColor = FLinearColor(0.65f, 0.8f, 1.0f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.3f;
        P.BlendDuration = 6.0f;
        DayPresets.Add(P);
    }

    // 2 — Midday
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Midday;
        P.SunPitch = -80.0f;
        P.SunYaw = 0.0f;
        P.SunIntensity = 12.0f;
        P.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
        P.SkyIntensity = 2.2f;
        P.FogDensity = 0.015f;
        P.FogColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        P.bVolumetricFog = false;
        P.VolumetricFogScattering = 0.2f;
        P.BlendDuration = 5.0f;
        DayPresets.Add(P);
    }

    // 3 — Golden Hour (DEFAULT)
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::GoldenHour;
        P.SunPitch = -35.0f;
        P.SunYaw = 45.0f;
        P.SunIntensity = 8.5f;
        P.SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);
        P.SkyIntensity = 1.8f;
        P.FogDensity = 0.035f;
        P.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.35f;
        P.BlendDuration = 5.0f;
        DayPresets.Add(P);
    }

    // 4 — Dusk
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Dusk;
        P.SunPitch = -12.0f; // clamped to -30 by CAP
        P.SunYaw = 135.0f;
        P.SunIntensity = 3.5f;
        P.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
        P.SkyIntensity = 1.0f;
        P.FogDensity = 0.05f;
        P.FogColor = FLinearColor(0.8f, 0.5f, 0.6f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.45f;
        P.BlendDuration = 8.0f;
        DayPresets.Add(P);
    }

    // 5 — Night
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Night;
        P.SunPitch = -90.0f; // below horizon
        P.SunYaw = 180.0f;
        P.SunIntensity = 0.1f;
        P.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
        P.SkyIntensity = 0.4f;
        P.FogDensity = 0.04f;
        P.FogColor = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.25f;
        P.BlendDuration = 10.0f;
        DayPresets.Add(P);
    }

    // 6 — Overcast
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Overcast;
        P.SunPitch = -60.0f;
        P.SunYaw = 0.0f;
        P.SunIntensity = 4.0f;
        P.SunColor = FLinearColor(0.85f, 0.88f, 0.95f, 1.0f);
        P.SkyIntensity = 1.2f;
        P.FogDensity = 0.07f;
        P.FogColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.6f;
        P.BlendDuration = 6.0f;
        DayPresets.Add(P);
    }

    // 7 — Storm
    {
        FLight_DayPreset P;
        P.TimeOfDay = ELight_TimeOfDay::Storm;
        P.SunPitch = -50.0f;
        P.SunYaw = 0.0f;
        P.SunIntensity = 1.5f;
        P.SunColor = FLinearColor(0.5f, 0.55f, 0.6f, 1.0f);
        P.SkyIntensity = 0.6f;
        P.FogDensity = 0.12f;
        P.FogColor = FLinearColor(0.35f, 0.38f, 0.42f, 1.0f);
        P.bVolumetricFog = true;
        P.VolumetricFogScattering = 0.7f;
        P.BlendDuration = 4.0f;
        DayPresets.Add(P);
    }
}
