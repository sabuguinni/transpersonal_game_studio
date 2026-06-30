#include "DuskAtmosphereController.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADuskAtmosphereController::ADuskAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; // Only tick when animating
}

void ADuskAtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    DiscoverSceneLightingActors();

    if (bAnimateDayCycle)
    {
        SetActorTickEnabled(true);
    }

    // Apply dusk palette on begin play
    ApplyDuskPalette();
}

void ADuskAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bAnimateDayCycle || DayCycleSpeed <= 0.0f)
    {
        return;
    }

    DayCycleAccumulator += DeltaTime * DayCycleSpeed;
    TimeOfDayNormalized = FMath::Fmod(DayCycleAccumulator, 1.0f);

    // Determine time of day enum from normalized value
    if (TimeOfDayNormalized < 0.1f || TimeOfDayNormalized > 0.95f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
    }
    else if (TimeOfDayNormalized < 0.2f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    }
    else if (TimeOfDayNormalized < 0.35f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    }
    else if (TimeOfDayNormalized < 0.55f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Noon;
    }
    else if (TimeOfDayNormalized < 0.7f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    }
    else if (TimeOfDayNormalized < 0.85f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
        ApplyDuskPalette();
    }

    // Sun pitch: -90 (midnight) → +80 (noon) → -90 (midnight)
    // Map 0..1 normalized to -90..80..-90 pitch
    float SunPitch = FMath::Sin(TimeOfDayNormalized * PI * 2.0f - PI * 0.5f) * 80.0f;
    float SunYaw = TimeOfDayNormalized * 360.0f;

    // Guard: never exceed -30 pitch (CAP enforcement)
    SunPitch = FMath::Clamp(SunPitch, -90.0f, 80.0f);

    ApplySunSettings(SunPitch, SunYaw,
        DuskPalette.SunIntensity,
        DuskPalette.SunColor,
        DuskPalette.ColorTemperatureKelvin);
}

void ADuskAtmosphereController::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    DiscoverSceneLightingActors();
}

void ADuskAtmosphereController::ApplyDuskPalette()
{
    CurrentTimeOfDay = ELight_TimeOfDay::Dusk;

    ApplySunSettings(
        DuskPalette.SunPitchAngle,
        DuskPalette.SunYawAngle,
        DuskPalette.SunIntensity,
        DuskPalette.SunColor,
        DuskPalette.ColorTemperatureKelvin
    );

    ApplyFogSettings(
        DuskPalette.FogDensity,
        DuskPalette.FogInscatteringColor,
        DuskPalette.FogMaxOpacity,
        DuskPalette.bEnableVolumetricFog
    );

    ApplySkyLightSettings(
        DuskPalette.SkyLightIntensity,
        DuskPalette.SkyLightColor
    );
}

void ADuskAtmosphereController::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;

    switch (NewTimeOfDay)
    {
    case ELight_TimeOfDay::Dusk:
        ApplyDuskPalette();
        break;

    case ELight_TimeOfDay::Dawn:
        // Dawn: cool pink-purple, low sun from east
        ApplySunSettings(-15.0f, -45.0f, 1.8f,
            FLinearColor(1.0f, 0.7f, 0.5f, 1.0f), 4500.0f);
        ApplyFogSettings(0.05f,
            FLinearColor(0.7f, 0.4f, 0.5f, 1.0f), 0.7f, true);
        ApplySkyLightSettings(0.8f, FLinearColor(0.8f, 0.7f, 0.9f, 1.0f));
        break;

    case ELight_TimeOfDay::Noon:
        // Noon: bright white-yellow, high sun
        ApplySunSettings(-75.0f, 0.0f, 10.0f,
            FLinearColor(1.0f, 0.98f, 0.9f, 1.0f), 6500.0f);
        ApplyFogSettings(0.02f,
            FLinearColor(0.6f, 0.75f, 0.9f, 1.0f), 0.5f, false);
        ApplySkyLightSettings(2.0f, FLinearColor(0.9f, 0.95f, 1.0f, 1.0f));
        break;

    case ELight_TimeOfDay::Night:
        // Night: deep blue moonlight
        ApplySunSettings(-10.0f, 180.0f, 0.3f,
            FLinearColor(0.6f, 0.7f, 1.0f, 1.0f), 8000.0f);
        ApplyFogSettings(0.04f,
            FLinearColor(0.05f, 0.08f, 0.18f, 1.0f), 0.9f, true);
        ApplySkyLightSettings(0.3f, FLinearColor(0.3f, 0.4f, 0.7f, 1.0f));
        break;

    case ELight_TimeOfDay::Storm:
        // Storm: grey-white overcast
        ApplySunSettings(-45.0f, 0.0f, 1.8f,
            FLinearColor(0.85f, 0.88f, 0.92f, 1.0f), 7500.0f);
        ApplyFogSettings(0.08f,
            FLinearColor(0.3f, 0.35f, 0.45f, 1.0f), 0.95f, true);
        ApplySkyLightSettings(0.6f, FLinearColor(0.6f, 0.65f, 0.75f, 1.0f));
        break;

    default:
        ApplyDuskPalette();
        break;
    }
}

FVector ADuskAtmosphereController::GetSunDirection() const
{
    if (!SceneSun)
    {
        return FVector(-1.0f, 0.0f, -0.5f); // Default fallback
    }
    return SceneSun->GetActorForwardVector();
}

float ADuskAtmosphereController::GetSunElevationNormalized() const
{
    FVector SunDir = GetSunDirection();
    // Z component of sun direction: -1 = pointing straight down (zenith), +1 = up (nadir)
    return FMath::Clamp(-SunDir.Z, -1.0f, 1.0f);
}

void ADuskAtmosphereController::DiscoverSceneLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find DirectionalLight
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SceneSun = *It;
        break; // Use first found
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SceneSkyLight = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        SceneFog = *It;
        break;
    }
}

void ADuskAtmosphereController::SetVolumetricFogEnabled(bool bEnabled)
{
    if (!SceneFog)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp =
        SceneFog->GetComponent();

    if (FogComp)
    {
        FogComp->SetVolumetricFog(bEnabled);
    }
}

void ADuskAtmosphereController::ApplySunSettings(
    float PitchDeg, float YawDeg,
    float Intensity, FLinearColor Color, float TempKelvin)
{
    if (!SceneSun)
    {
        return;
    }

    SceneSun->SetActorRotation(FRotator(PitchDeg, YawDeg, 0.0f));

    UDirectionalLightComponent* SunComp =
        SceneSun->GetComponent();

    if (SunComp)
    {
        SunComp->SetIntensity(Intensity);
        SunComp->SetLightColor(Color);
        SunComp->SetUseTemperature(true);
        SunComp->SetTemperature(TempKelvin);
        SunComp->SetCastShadows(true);
        SunComp->SetAtmosphereSunLight(true);
    }
}

void ADuskAtmosphereController::ApplyFogSettings(
    float Density, FLinearColor InscatterColor,
    float MaxOpacity, bool bVolumetric)
{
    if (!SceneFog)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp =
        SceneFog->GetComponent();

    if (FogComp)
    {
        FogComp->SetFogDensity(Density);
        FogComp->SetFogInscatteringColor(InscatterColor);
        FogComp->SetFogMaxOpacity(MaxOpacity);
        FogComp->SetVolumetricFog(bVolumetric);
    }
}

void ADuskAtmosphereController::ApplySkyLightSettings(
    float Intensity, FLinearColor Color)
{
    if (!SceneSkyLight)
    {
        return;
    }

    USkyLightComponent* SkyComp =
        SceneSkyLight->GetLightComponent();

    if (SkyComp)
    {
        SkyComp->SetIntensity(Intensity);
        SkyComp->SetLightColor(Color);
    }
}
