#include "Light_CaveLightingManager.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

ALight_CaveLightingManager::ALight_CaveLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize light components
    InitializeLightComponents();
    
    // Setup default presets
    SetupDefaultPresets();
}

void ALight_CaveLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial lighting mode
    SetCaveLightingMode(CurrentMode);
    
    // Start flickering if enabled
    if (bEnableFlickering)
    {
        EnableFlickering(true);
    }
}

void ALight_CaveLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ALight_CaveLightingManager::InitializeLightComponents()
{
    // Create fire pit light
    FirePitLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FirePitLight"));
    FirePitLight->SetupAttachment(RootComponent);
    FirePitLight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    FirePitLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.1f, 1.0f));
    FirePitLight->SetIntensity(1200.0f);
    FirePitLight->SetAttenuationRadius(800.0f);
    FirePitLight->SetCastShadows(true);

    // Create entrance light
    EntranceLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("EntranceLight"));
    EntranceLight->SetupAttachment(RootComponent);
    EntranceLight->SetRelativeLocation(FVector(300.0f, 0.0f, 200.0f));
    EntranceLight->SetRelativeRotation(FRotator(-45.0f, 180.0f, 0.0f));
    EntranceLight->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
    EntranceLight->SetIntensity(800.0f);
    EntranceLight->SetInnerConeAngle(30.0f);
    EntranceLight->SetOuterConeAngle(60.0f);
}

void ALight_CaveLightingManager::SetupDefaultPresets()
{
    // Deep Cave Preset
    DeepCavePreset.FireColor = FLinearColor(0.8f, 0.3f, 0.05f, 1.0f);
    DeepCavePreset.FireIntensity = 800.0f;
    DeepCavePreset.TorchIntensity = 200.0f;
    DeepCavePreset.FogColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    DeepCavePreset.FogDensity = 0.12f;

    // Cave Entrance Preset
    EntrancePreset.FireColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    EntrancePreset.FireIntensity = 1000.0f;
    EntrancePreset.TorchIntensity = 300.0f;
    EntrancePreset.FogColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f);
    EntrancePreset.FogDensity = 0.06f;

    // Torch Lit Preset
    TorchLitPreset.FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    TorchLitPreset.FireIntensity = 1200.0f;
    TorchLitPreset.TorchIntensity = 400.0f;
    TorchLitPreset.FogColor = FLinearColor(0.7f, 0.4f, 0.2f, 1.0f);
    TorchLitPreset.FogDensity = 0.08f;

    // Campfire Preset
    CampfirePreset.FireColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    CampfirePreset.FireIntensity = 1500.0f;
    CampfirePreset.TorchIntensity = 500.0f;
    CampfirePreset.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    CampfirePreset.FogDensity = 0.04f;
}

void ALight_CaveLightingManager::SetCaveLightingMode(ELight_CaveLightingMode NewMode)
{
    CurrentMode = NewMode;

    switch (CurrentMode)
    {
        case ELight_CaveLightingMode::DeepCave:
            ApplyLightingPreset(DeepCavePreset);
            break;
        case ELight_CaveLightingMode::CaveEntrance:
            ApplyLightingPreset(EntrancePreset);
            break;
        case ELight_CaveLightingMode::TorchLit:
            ApplyLightingPreset(TorchLitPreset);
            break;
        case ELight_CaveLightingMode::Campfire:
            ApplyLightingPreset(CampfirePreset);
            break;
    }
}

void ALight_CaveLightingManager::ApplyLightingPreset(const FLight_CaveLightingPreset& Preset)
{
    if (FirePitLight)
    {
        FirePitLight->SetLightColor(Preset.FireColor);
        FirePitLight->SetIntensity(Preset.FireIntensity);
    }

    // Update torch lights
    for (UPointLightComponent* TorchLight : TorchLights)
    {
        if (TorchLight)
        {
            TorchLight->SetLightColor(Preset.FireColor);
            TorchLight->SetIntensity(Preset.TorchIntensity);
        }
    }
}

void ALight_CaveLightingManager::AddTorchLight(const FVector& Location)
{
    if (TorchLights.Num() >= MaxTorches)
    {
        return;
    }

    UPointLightComponent* NewTorch = CreateDefaultSubobject<UPointLightComponent>(*FString::Printf(TEXT("TorchLight_%d"), TorchLights.Num()));
    if (NewTorch)
    {
        NewTorch->SetupAttachment(RootComponent);
        NewTorch->SetRelativeLocation(Location);
        NewTorch->SetLightColor(FLinearColor(1.0f, 0.5f, 0.15f, 1.0f));
        NewTorch->SetIntensity(400.0f);
        NewTorch->SetAttenuationRadius(300.0f);
        NewTorch->SetCastShadows(true);
        
        TorchLights.Add(NewTorch);
    }
}

void ALight_CaveLightingManager::RemoveTorchLight(int32 Index)
{
    if (TorchLights.IsValidIndex(Index))
    {
        UPointLightComponent* TorchToRemove = TorchLights[Index];
        if (TorchToRemove)
        {
            TorchToRemove->DestroyComponent();
        }
        TorchLights.RemoveAt(Index);
    }
}

void ALight_CaveLightingManager::ClearAllTorches()
{
    for (UPointLightComponent* TorchLight : TorchLights)
    {
        if (TorchLight)
        {
            TorchLight->DestroyComponent();
        }
    }
    TorchLights.Empty();
}

void ALight_CaveLightingManager::EnableFlickering(bool bEnable)
{
    bEnableFlickering = bEnable;

    if (bEnableFlickering)
    {
        GetWorldTimerManager().SetTimer(FlickerTimerHandle, this, &ALight_CaveLightingManager::UpdateFlickering, 
                                       1.0f / FlickerSpeed, true);
    }
    else
    {
        GetWorldTimerManager().ClearTimer(FlickerTimerHandle);
    }
}

void ALight_CaveLightingManager::SetFlickerParameters(float Speed, float Intensity)
{
    FlickerSpeed = FMath::Clamp(Speed, 0.1f, 10.0f);
    FlickerIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    if (bEnableFlickering)
    {
        EnableFlickering(true); // Restart with new parameters
    }
}

void ALight_CaveLightingManager::UpdateFlickering()
{
    if (!bEnableFlickering)
    {
        return;
    }

    // Flicker fire pit light
    if (FirePitLight)
    {
        float BaseIntensity = 0.0f;
        switch (CurrentMode)
        {
            case ELight_CaveLightingMode::DeepCave:
                BaseIntensity = DeepCavePreset.FireIntensity;
                break;
            case ELight_CaveLightingMode::CaveEntrance:
                BaseIntensity = EntrancePreset.FireIntensity;
                break;
            case ELight_CaveLightingMode::TorchLit:
                BaseIntensity = TorchLitPreset.FireIntensity;
                break;
            case ELight_CaveLightingMode::Campfire:
                BaseIntensity = CampfirePreset.FireIntensity;
                break;
        }

        float FlickerValue = FMath::RandRange(-FlickerIntensity, FlickerIntensity);
        float NewIntensity = BaseIntensity * (1.0f + FlickerValue);
        FirePitLight->SetIntensity(FMath::Max(NewIntensity, 50.0f));
    }

    // Flicker torch lights
    for (UPointLightComponent* TorchLight : TorchLights)
    {
        if (TorchLight)
        {
            float BaseIntensity = 400.0f;
            float FlickerValue = FMath::RandRange(-FlickerIntensity, FlickerIntensity);
            float NewIntensity = BaseIntensity * (1.0f + FlickerValue);
            TorchLight->SetIntensity(FMath::Max(NewIntensity, 50.0f));
        }
    }
}