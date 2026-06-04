#include "Light_MountainCaveSystem.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ALight_MountainCaveSystem::ALight_MountainCaveSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create primary light component
    PrimaryLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PrimaryLightComponent"));
    PrimaryLightComponent->SetupAttachment(RootComponent);
    PrimaryLightComponent->SetIntensity(2.0f);
    PrimaryLightComponent->SetLightColor(FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
    PrimaryLightComponent->SetAttenuationRadius(800.0f);
    PrimaryLightComponent->SetCastShadows(false);

    // Create light source mesh component
    LightSourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightSourceMesh"));
    LightSourceMesh->SetupAttachment(RootComponent);
    LightSourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default configuration
    LightConfiguration.LightType = ELight_CaveLightingType::Ambient;
    LightConfiguration.Intensity = 2.0f;
    LightConfiguration.LightColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    LightConfiguration.AttenuationRadius = 800.0f;
    LightConfiguration.bCastShadows = false;
    LightConfiguration.bFlickerEffect = false;
    LightConfiguration.FlickerSpeed = 2.0f;

    // Initialize cave environment settings
    bEnableVolumetricFog = true;
    FogDensity = 0.02f;
    FogHeightFalloff = 0.1f;
    FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    bEnableDepthBasedLighting = true;
    CaveDepthFactor = 0.5f;
    bEnableTemperatureBasedLighting = true;
    CaveTemperature = 15.0f;

    // Initialize flicker variables
    FlickerTimer = 0.0f;
    BaseIntensity = 2.0f;
    bFlickerDirection = true;
    CaveFogActor = nullptr;
}

void ALight_MountainCaveSystem::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial configuration
    SetupCaveLighting(LightConfiguration);

    // Create volumetric fog if enabled
    if (bEnableVolumetricFog)
    {
        CreateVolumetricFog();
    }

    // Store base intensity for flicker effect
    BaseIntensity = PrimaryLightComponent->Intensity;
}

void ALight_MountainCaveSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update flicker effect if enabled
    if (LightConfiguration.bFlickerEffect)
    {
        UpdateFlickerEffect(DeltaTime);
    }

    // Update depth-based lighting if enabled
    if (bEnableDepthBasedLighting)
    {
        float CurrentDepth = FMath::Abs(GetActorLocation().Z - 200.0f) * 0.01f;
        ApplyDepthBasedDimming(CurrentDepth);
    }

    // Update temperature-based lighting if enabled
    if (bEnableTemperatureBasedLighting)
    {
        ApplyTemperatureColorShift(CaveTemperature);
    }
}

void ALight_MountainCaveSystem::SetupCaveLighting(const FLight_CaveLightConfig& Config)
{
    LightConfiguration = Config;

    // Apply configuration to primary light
    PrimaryLightComponent->SetIntensity(Config.Intensity);
    PrimaryLightComponent->SetLightColor(Config.LightColor);
    PrimaryLightComponent->SetAttenuationRadius(Config.AttenuationRadius);
    PrimaryLightComponent->SetCastShadows(Config.bCastShadows);

    // Store base intensity for flicker effect
    BaseIntensity = Config.Intensity;

    // Apply type-specific settings
    switch (Config.LightType)
    {
    case ELight_CaveLightingType::Ambient:
        PrimaryLightComponent->SetLightColor(FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
        PrimaryLightComponent->SetCastShadows(false);
        break;

    case ELight_CaveLightingType::Entrance:
        PrimaryLightComponent->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
        PrimaryLightComponent->SetCastShadows(true);
        PrimaryLightComponent->SetIntensity(Config.Intensity * 1.5f);
        break;

    case ELight_CaveLightingType::Interior:
        PrimaryLightComponent->SetLightColor(FLinearColor(0.6f, 0.5f, 0.4f, 1.0f));
        PrimaryLightComponent->SetAttenuationRadius(Config.AttenuationRadius * 0.7f);
        break;

    case ELight_CaveLightingType::Crystal:
        PrimaryLightComponent->SetLightColor(FLinearColor(0.7f, 0.9f, 1.0f, 1.0f));
        LightConfiguration.bFlickerEffect = true;
        LightConfiguration.FlickerSpeed = 0.5f;
        break;

    case ELight_CaveLightingType::Fire:
        PrimaryLightComponent->SetLightColor(FLinearColor(1.0f, 0.6f, 0.2f, 1.0f));
        LightConfiguration.bFlickerEffect = true;
        LightConfiguration.FlickerSpeed = 3.0f;
        break;
    }
}

void ALight_MountainCaveSystem::AddSecondaryLight(const FLight_CaveLightConfig& Config, const FVector& RelativeLocation)
{
    CreateSecondaryLightComponent(Config, RelativeLocation);
    AdditionalLights.Add(Config);
}

void ALight_MountainCaveSystem::UpdateLightingForDepth(float CaveDepth)
{
    CaveDepthFactor = CaveDepth;
    ApplyDepthBasedDimming(CaveDepth);
}

void ALight_MountainCaveSystem::UpdateLightingForTemperature(float Temperature)
{
    CaveTemperature = Temperature;
    ApplyTemperatureColorShift(Temperature);
}

void ALight_MountainCaveSystem::SetFlickerEffect(bool bEnable, float Speed)
{
    LightConfiguration.bFlickerEffect = bEnable;
    LightConfiguration.FlickerSpeed = Speed;
    FlickerTimer = 0.0f;
}

void ALight_MountainCaveSystem::CreateVolumetricFog()
{
    if (CaveFogActor)
    {
        return; // Already exists
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn exponential height fog actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = nullptr;

    CaveFogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

    if (CaveFogActor)
    {
        UExponentialHeightFogComponent* FogComponent = CaveFogActor->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(FogDensity);
            FogComponent->SetFogHeightFalloff(FogHeightFalloff);
            FogComponent->SetFogInscatteringColor(FogInscatteringColor);
            FogComponent->SetStartDistance(100.0f);
            FogComponent->SetFogCutoffDistance(10000.0f);
        }
    }
}

void ALight_MountainCaveSystem::RemoveVolumetricFog()
{
    if (CaveFogActor)
    {
        CaveFogActor->Destroy();
        CaveFogActor = nullptr;
    }
}

void ALight_MountainCaveSystem::PreviewLightingSetup()
{
    // Editor-only function for previewing lighting setup
    SetupCaveLighting(LightConfiguration);

    if (bEnableVolumetricFog && !CaveFogActor)
    {
        CreateVolumetricFog();
    }
    else if (!bEnableVolumetricFog && CaveFogActor)
    {
        RemoveVolumetricFog();
    }
}

void ALight_MountainCaveSystem::SetLightingPreset(ELight_CaveLightingType PresetType)
{
    FLight_CaveLightConfig PresetConfig;
    PresetConfig.LightType = PresetType;

    switch (PresetType)
    {
    case ELight_CaveLightingType::Ambient:
        PresetConfig.Intensity = 2.0f;
        PresetConfig.LightColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        PresetConfig.AttenuationRadius = 800.0f;
        PresetConfig.bCastShadows = false;
        PresetConfig.bFlickerEffect = false;
        break;

    case ELight_CaveLightingType::Entrance:
        PresetConfig.Intensity = 4.0f;
        PresetConfig.LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        PresetConfig.AttenuationRadius = 1200.0f;
        PresetConfig.bCastShadows = true;
        PresetConfig.bFlickerEffect = false;
        break;

    case ELight_CaveLightingType::Interior:
        PresetConfig.Intensity = 1.5f;
        PresetConfig.LightColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
        PresetConfig.AttenuationRadius = 600.0f;
        PresetConfig.bCastShadows = false;
        PresetConfig.bFlickerEffect = false;
        break;

    case ELight_CaveLightingType::Crystal:
        PresetConfig.Intensity = 3.0f;
        PresetConfig.LightColor = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f);
        PresetConfig.AttenuationRadius = 1000.0f;
        PresetConfig.bCastShadows = false;
        PresetConfig.bFlickerEffect = true;
        PresetConfig.FlickerSpeed = 0.5f;
        break;

    case ELight_CaveLightingType::Fire:
        PresetConfig.Intensity = 3.5f;
        PresetConfig.LightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
        PresetConfig.AttenuationRadius = 900.0f;
        PresetConfig.bCastShadows = true;
        PresetConfig.bFlickerEffect = true;
        PresetConfig.FlickerSpeed = 3.0f;
        break;
    }

    SetupCaveLighting(PresetConfig);
}

void ALight_MountainCaveSystem::UpdateFlickerEffect(float DeltaTime)
{
    FlickerTimer += DeltaTime * LightConfiguration.FlickerSpeed;

    float FlickerIntensity = BaseIntensity;

    if (LightConfiguration.LightType == ELight_CaveLightingType::Fire)
    {
        // Realistic fire flicker
        FlickerIntensity = BaseIntensity + FMath::Sin(FlickerTimer * 2.0f) * 0.3f + FMath::Sin(FlickerTimer * 5.0f) * 0.1f;
    }
    else if (LightConfiguration.LightType == ELight_CaveLightingType::Crystal)
    {
        // Gentle crystal glow
        FlickerIntensity = BaseIntensity + FMath::Sin(FlickerTimer) * 0.2f;
    }
    else
    {
        // Generic flicker
        FlickerIntensity = BaseIntensity + FMath::Sin(FlickerTimer) * 0.15f;
    }

    FlickerIntensity = FMath::Max(FlickerIntensity, BaseIntensity * 0.3f);
    PrimaryLightComponent->SetIntensity(FlickerIntensity);
}

void ALight_MountainCaveSystem::ApplyDepthBasedDimming(float Depth)
{
    float DimmingFactor = FMath::Clamp(1.0f - (Depth * CaveDepthFactor), 0.2f, 1.0f);
    float AdjustedIntensity = BaseIntensity * DimmingFactor;
    
    if (!LightConfiguration.bFlickerEffect)
    {
        PrimaryLightComponent->SetIntensity(AdjustedIntensity);
    }
}

void ALight_MountainCaveSystem::ApplyTemperatureColorShift(float Temperature)
{
    FLinearColor TemperatureColor = CalculateTemperatureColor(Temperature);
    FLinearColor FinalColor = FLinearColor::LerpUsingHSV(LightConfiguration.LightColor, TemperatureColor, 0.3f);
    PrimaryLightComponent->SetLightColor(FinalColor);
}

FLinearColor ALight_MountainCaveSystem::CalculateTemperatureColor(float Temperature)
{
    // Convert temperature to color shift
    // Cold caves (0-10°C) -> blue tint
    // Moderate caves (10-20°C) -> neutral
    // Warm caves (20-30°C) -> red tint
    
    if (Temperature < 10.0f)
    {
        float BlueShift = FMath::Clamp((10.0f - Temperature) / 10.0f, 0.0f, 1.0f);
        return FLinearColor(0.7f - BlueShift * 0.2f, 0.8f - BlueShift * 0.1f, 1.0f, 1.0f);
    }
    else if (Temperature > 20.0f)
    {
        float RedShift = FMath::Clamp((Temperature - 20.0f) / 10.0f, 0.0f, 1.0f);
        return FLinearColor(1.0f, 0.8f - RedShift * 0.3f, 0.6f - RedShift * 0.3f, 1.0f);
    }
    else
    {
        return FLinearColor(0.8f, 0.7f, 0.5f, 1.0f); // Neutral cave color
    }
}

void ALight_MountainCaveSystem::CreateSecondaryLightComponent(const FLight_CaveLightConfig& Config, const FVector& Location)
{
    UPointLightComponent* SecondaryLight = CreateDefaultSubobject<UPointLightComponent>(*FString::Printf(TEXT("SecondaryLight_%d"), SecondaryLights.Num()));
    SecondaryLight->SetupAttachment(RootComponent);
    SecondaryLight->SetRelativeLocation(Location);
    SecondaryLight->SetIntensity(Config.Intensity);
    SecondaryLight->SetLightColor(Config.LightColor);
    SecondaryLight->SetAttenuationRadius(Config.AttenuationRadius);
    SecondaryLight->SetCastShadows(Config.bCastShadows);

    SecondaryLights.Add(SecondaryLight);
}