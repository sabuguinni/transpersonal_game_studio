#include "DuskTwilightLightingProfile.h"

#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADuskTwilightLightingProfile::ADuskTwilightLightingProfile()
{
    PrimaryActorTick.bCanEverTick = false;

    DirectionalLightRef = nullptr;
    HeightFogRef = nullptr;
    SkyLightRef = nullptr;
    PostProcessRef = nullptr;
    WarmFillLight = nullptr;
    CoolRimLight = nullptr;
    PurpleShadowLight = nullptr;
}

void ADuskTwilightLightingProfile::BeginPlay()
{
    Super::BeginPlay();

    FindLightingActors();

    if (bAutoApplyOnBeginPlay)
    {
        ApplyDuskPalette();
    }
}

void ADuskTwilightLightingProfile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADuskTwilightLightingProfile::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        DirectionalLightRef = *It;
        break;
    }

    // Find exponential height fog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        HeightFogRef = *It;
        break;
    }

    // Find sky light
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightRef = *It;
        break;
    }

    // Find post process volume
    for (TActorIterator<APostProcessVolume> It(World); It; ++It)
    {
        PostProcessRef = *It;
        break;
    }
}

void ADuskTwilightLightingProfile::SpawnFillLights()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Remove existing fill lights first
    RemoveFillLights();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // --- Warm Fill Light (orange key from low horizon) ---
    WarmFillLight = World->SpawnActor<APointLight>(
        FVector(800.0f, -400.0f, 180.0f),
        FRotator::ZeroRotator,
        SpawnParams
    );
    if (WarmFillLight)
    {
        WarmFillLight->SetActorLabel(TEXT("DuskWarmFill"));
        UPointLightComponent* WFC = WarmFillLight->GetLightComponent();
        if (WFC)
        {
            WFC->SetIntensity(DuskPalette.WarmFillIntensity);
            WFC->SetLightColor(DuskPalette.WarmFillColor);
            WFC->SetAttenuationRadius(6000.0f);
            WFC->SetCastShadows(true);
        }
    }

    // --- Cool Rim Light (blue-violet from opposite horizon) ---
    CoolRimLight = World->SpawnActor<APointLight>(
        FVector(-900.0f, 600.0f, 220.0f),
        FRotator::ZeroRotator,
        SpawnParams
    );
    if (CoolRimLight)
    {
        CoolRimLight->SetActorLabel(TEXT("DuskCoolRim"));
        UPointLightComponent* CRC = CoolRimLight->GetLightComponent();
        if (CRC)
        {
            CRC->SetIntensity(DuskPalette.CoolRimIntensity);
            CRC->SetLightColor(DuskPalette.CoolRimColor);
            CRC->SetAttenuationRadius(5000.0f);
            CRC->SetCastShadows(false);
        }
    }

    // --- Purple Shadow Fill (deep ambient bounce) ---
    PurpleShadowLight = World->SpawnActor<APointLight>(
        FVector(0.0f, 0.0f, 80.0f),
        FRotator::ZeroRotator,
        SpawnParams
    );
    if (PurpleShadowLight)
    {
        PurpleShadowLight->SetActorLabel(TEXT("DuskPurpleShadow"));
        UPointLightComponent* PSC = PurpleShadowLight->GetLightComponent();
        if (PSC)
        {
            PSC->SetIntensity(DuskPalette.PurpleShadowIntensity);
            PSC->SetLightColor(DuskPalette.PurpleShadowColor);
            PSC->SetAttenuationRadius(8000.0f);
            PSC->SetCastShadows(false);
        }
    }
}

void ADuskTwilightLightingProfile::RemoveFillLights()
{
    if (WarmFillLight)
    {
        WarmFillLight->Destroy();
        WarmFillLight = nullptr;
    }
    if (CoolRimLight)
    {
        CoolRimLight->Destroy();
        CoolRimLight = nullptr;
    }
    if (PurpleShadowLight)
    {
        PurpleShadowLight->Destroy();
        PurpleShadowLight = nullptr;
    }
}

void ADuskTwilightLightingProfile::ApplyDuskPalette()
{
    ApplyToDirectionalLight();
    ApplyToHeightFog();
    ApplyToPostProcess();
    ApplyToSkyLight();
}

void ADuskTwilightLightingProfile::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;

    // Blend palette based on time of day
    // For now, dusk is the primary palette — future cycles will add full interpolation
    if (CurrentTimeOfDay == ELight_TimeOfDay::Dusk || CurrentTimeOfDay == ELight_TimeOfDay::Twilight)
    {
        ApplyDuskPalette();
    }
}

float ADuskTwilightLightingProfile::GetTimeOfDayAlpha() const
{
    // Returns normalised 0-1 value for time of day
    // 0 = Dawn, 0.5 = Midday, 1 = Midnight
    switch (CurrentTimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:        return 0.0f;
    case ELight_TimeOfDay::Morning:     return 0.15f;
    case ELight_TimeOfDay::Midday:      return 0.35f;
    case ELight_TimeOfDay::GoldenHour:  return 0.55f;
    case ELight_TimeOfDay::Dusk:        return 0.70f;
    case ELight_TimeOfDay::Twilight:    return 0.82f;
    case ELight_TimeOfDay::Night:       return 0.92f;
    case ELight_TimeOfDay::Midnight:    return 1.0f;
    default:                            return 0.5f;
    }
}

void ADuskTwilightLightingProfile::ApplyToDirectionalLight()
{
    if (!DirectionalLightRef)
    {
        return;
    }

    UDirectionalLightComponent* DLC = DirectionalLightRef->GetComponent();
    if (DLC)
    {
        DLC->SetIntensity(DuskPalette.SunIntensity);
        DLC->SetLightColor(DuskPalette.SunColor);
        DLC->SetCastShadows(true);
    }
}

void ADuskTwilightLightingProfile::ApplyToHeightFog()
{
    if (!HeightFogRef)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = HeightFogRef->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(DuskPalette.FogDensity);
        FogComp->SetFogInscatteringColor(DuskPalette.FogInscatterColor);
        FogComp->SetVolumetricFog(bVolumetricFogEnabled);
    }
}

void ADuskTwilightLightingProfile::ApplyToPostProcess()
{
    if (!PostProcessRef)
    {
        return;
    }

    FPostProcessSettings& PPSettings = PostProcessRef->Settings;
    PPSettings.BloomIntensity = DuskPalette.BloomIntensity;
    PPSettings.AutoExposureMinBrightness = DuskPalette.AutoExposureMin;
    PPSettings.AutoExposureMaxBrightness = DuskPalette.AutoExposureMax;
}

void ADuskTwilightLightingProfile::ApplyToSkyLight()
{
    if (!SkyLightRef)
    {
        return;
    }

    USkyLightComponent* SLC = SkyLightRef->GetLightComponent();
    if (SLC)
    {
        SLC->SetCastShadows(true);
        SLC->RecaptureSky();
    }
}
