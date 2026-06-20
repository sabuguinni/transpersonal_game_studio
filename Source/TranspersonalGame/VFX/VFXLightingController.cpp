#include "VFXLightingController.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
AVFX_LightingController::AVFX_LightingController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.016f; // ~60fps tick for smooth flicker

    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    RootComponent = PointLight;

    // Default campfire appearance
    PointLight->Intensity = 2500.0f;
    PointLight->AttenuationRadius = 400.0f;
    PointLight->LightColor = FColor(255, 107, 20); // warm orange
    PointLight->bCastShadows = true;
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::BeginPlay()
{
    Super::BeginPlay();
    ApplyZonePreset();
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bLightActive || !PointLight)
    {
        return;
    }

    ElapsedTime += DeltaTime;

    // LOD: check player proximity every 0.5s instead of every frame
    if (bUseLOD)
    {
        LODUpdateAccumulator += DeltaTime;
        if (LODUpdateAccumulator >= 0.5f)
        {
            LODUpdateAccumulator = 0.0f;
            UpdatePlayerProximity();
        }

        if (!bPlayerInRange && ActiveRadius > 0.0f)
        {
            return; // Skip flicker update when player is far
        }
    }

    UpdateFlicker(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::UpdateFlicker(float DeltaTime)
{
    if (!PointLight) return;

    const float NewIntensity = ComputeFlickerIntensity();
    const FLinearColor NewColor = ComputeFlickerColor();

    PointLight->SetIntensity(NewIntensity);
    PointLight->SetLightColor(NewColor);
}

// ─────────────────────────────────────────────────────────────────────────────
float AVFX_LightingController::ComputeFlickerIntensity() const
{
    // Combine two sine waves at different frequencies for organic flicker
    const float Primary = FMath::Sin(ElapsedTime * FlickerParams.FlickerFrequency);
    const float Secondary = FMath::Sin(ElapsedTime * FlickerParams.SecondaryFrequency * 2.7f);

    // Danger level amplifies flicker (T-Rex proximity = more chaotic fire)
    const float DangerAmplify = 1.0f + ExternalDangerLevel * 1.5f;

    const float NoiseFactor = (Primary * 0.6f + Secondary * 0.4f) * DangerAmplify;
    const float Intensity = FlickerParams.BaseIntensity + NoiseFactor * FlickerParams.FlickerAmplitude;

    return FMath::Max(Intensity, FlickerParams.BaseIntensity * 0.3f);
}

// ─────────────────────────────────────────────────────────────────────────────
FLinearColor AVFX_LightingController::ComputeFlickerColor() const
{
    // Lerp between trough and peak colour based on current intensity fraction
    const float CurrentIntensity = ComputeFlickerIntensity();
    const float IntensityFraction = FMath::Clamp(
        (CurrentIntensity - FlickerParams.BaseIntensity * 0.3f) /
        (FlickerParams.BaseIntensity * 1.7f + FlickerParams.FlickerAmplitude),
        0.0f, 1.0f
    );

    // Danger shifts colour toward red
    FLinearColor BaseColor = FLinearColor::LerpUsingHSV(
        FlickerParams.TroughColor,
        FlickerParams.PeakColor,
        IntensityFraction
    );

    if (ExternalDangerLevel > 0.0f)
    {
        const FLinearColor DangerRed(1.0f, 0.05f, 0.05f, 1.0f);
        BaseColor = FLinearColor::LerpUsingHSV(BaseColor, DangerRed, ExternalDangerLevel * 0.4f);
    }

    return BaseColor;
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::UpdatePlayerProximity()
{
    if (ActiveRadius <= 0.0f)
    {
        bPlayerInRange = true;
        return;
    }

    const UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        bPlayerInRange = false;
        return;
    }

    const float DistSq = FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation());
    bPlayerInRange = DistSq <= (ActiveRadius * ActiveRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::SetDangerLevel(float NewDangerLevel)
{
    ExternalDangerLevel = FMath::Clamp(NewDangerLevel, 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::SetLightActive(bool bActive)
{
    bLightActive = bActive;
    if (PointLight)
    {
        PointLight->SetVisibility(bActive);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AVFX_LightingController::ApplyZonePreset()
{
    if (!PointLight) return;

    switch (ZoneType)
    {
    case EVFX_LightZoneType::Campfire:
        FlickerParams.BaseIntensity    = 2500.0f;
        FlickerParams.FlickerAmplitude = 400.0f;
        FlickerParams.FlickerFrequency = 8.0f;
        FlickerParams.SecondaryFrequency = 3.2f;
        FlickerParams.PeakColor   = FLinearColor(1.0f, 0.55f, 0.10f, 1.0f);
        FlickerParams.TroughColor = FLinearColor(1.0f, 0.28f, 0.04f, 1.0f);
        PointLight->AttenuationRadius = 400.0f;
        PointLight->bCastShadows = true;
        break;

    case EVFX_LightZoneType::Volcanic:
        FlickerParams.BaseIntensity    = 8000.0f;
        FlickerParams.FlickerAmplitude = 1500.0f;
        FlickerParams.FlickerFrequency = 2.0f;
        FlickerParams.SecondaryFrequency = 0.8f;
        FlickerParams.PeakColor   = FLinearColor(1.0f, 0.25f, 0.02f, 1.0f);
        FlickerParams.TroughColor = FLinearColor(0.8f, 0.10f, 0.01f, 1.0f);
        PointLight->AttenuationRadius = 3000.0f;
        PointLight->bCastShadows = false;
        break;

    case EVFX_LightZoneType::ColdZone:
        FlickerParams.BaseIntensity    = 800.0f;
        FlickerParams.FlickerAmplitude = 50.0f;  // barely flickers — cold, stable
        FlickerParams.FlickerFrequency = 0.5f;
        FlickerParams.SecondaryFrequency = 0.2f;
        FlickerParams.PeakColor   = FLinearColor(0.45f, 0.65f, 1.0f, 1.0f);
        FlickerParams.TroughColor = FLinearColor(0.35f, 0.50f, 0.9f, 1.0f);
        PointLight->AttenuationRadius = 600.0f;
        PointLight->bCastShadows = false;
        break;

    case EVFX_LightZoneType::DangerZone:
        FlickerParams.BaseIntensity    = 1200.0f;
        FlickerParams.FlickerAmplitude = 600.0f;
        FlickerParams.FlickerFrequency = 12.0f;  // fast, aggressive
        FlickerParams.SecondaryFrequency = 5.5f;
        FlickerParams.PeakColor   = FLinearColor(1.0f, 0.05f, 0.05f, 1.0f);
        FlickerParams.TroughColor = FLinearColor(0.6f, 0.02f, 0.02f, 1.0f);
        PointLight->AttenuationRadius = 700.0f;
        PointLight->bCastShadows = false;
        break;

    case EVFX_LightZoneType::WaterReflect:
        FlickerParams.BaseIntensity    = 600.0f;
        FlickerParams.FlickerAmplitude = 200.0f;
        FlickerParams.FlickerFrequency = 4.0f;
        FlickerParams.SecondaryFrequency = 1.8f;
        FlickerParams.PeakColor   = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);
        FlickerParams.TroughColor = FLinearColor(0.3f, 0.6f, 0.9f, 1.0f);
        PointLight->AttenuationRadius = 500.0f;
        PointLight->bCastShadows = false;
        break;

    default:
        break;
    }
}
