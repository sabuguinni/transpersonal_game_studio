#include "Light_VolumetricFogManager.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/VolumetricCloud.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_VolumetricFogManager::ALight_VolumetricFogManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    CurrentFogType = ELight_FogType::Light;
    FogTransitionSpeed = 1.0f;
    bEnableAtmosphericPerspective = true;
    AtmosphericPerspectiveDistance = 100000.0f;
    
    // Initialize fog settings for Cretaceous atmosphere
    FogSettings.FogDensity = 0.02f;
    FogSettings.FogHeightFalloff = 0.2f;
    FogSettings.StartDistance = 5000.0f;
    FogSettings.FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);
    FogSettings.bVolumetricFog = true;
    FogSettings.VolumetricFogScatteringDistribution = 0.2f;
    FogSettings.VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    FogSettings.VolumetricFogEmissive = 0.0f;
    FogSettings.VolumetricFogExtinctionScale = 1.0f;
    
    HeightFogActor = nullptr;
    VolumetricCloudActor = nullptr;
}

void ALight_VolumetricFogManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFogComponents();
    ApplyFogSettings(FogSettings);
}

void ALight_VolumetricFogManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsTransitioning)
    {
        UpdateFogTransition(DeltaTime);
    }
}

void ALight_VolumetricFogManager::SetFogType(ELight_FogType NewFogType)
{
    if (CurrentFogType != NewFogType)
    {
        CurrentFogType = NewFogType;
        ApplyFogTypeSettings(NewFogType);
    }
}

void ALight_VolumetricFogManager::ApplyFogSettings(const FLight_FogSettings& Settings)
{
    FogSettings = Settings;
    
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
            FogComp->SetStartDistance(Settings.StartDistance);
            FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
            FogComp->SetVolumetricFog(Settings.bVolumetricFog);
            
            if (Settings.bVolumetricFog)
            {
                FogComp->SetVolumetricFogScatteringDistribution(Settings.VolumetricFogScatteringDistribution);
                FogComp->SetVolumetricFogAlbedo(Settings.VolumetricFogAlbedo);
                FogComp->SetVolumetricFogEmissive(Settings.VolumetricFogEmissive);
                FogComp->SetVolumetricFogExtinctionScale(Settings.VolumetricFogExtinctionScale);
            }
        }
    }
}

void ALight_VolumetricFogManager::TransitionToFogType(ELight_FogType TargetFogType, float TransitionDuration)
{
    if (CurrentFogType == TargetFogType) return;
    
    this->TargetFogType = TargetFogType;
    this->TransitionDuration = TransitionDuration;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
    
    // Store current settings as start point
    StartFogSettings = FogSettings;
    
    // Generate target settings based on fog type
    ApplyFogTypeSettings(TargetFogType);
    TargetFogSettings = FogSettings;
    
    // Restore start settings for smooth transition
    FogSettings = StartFogSettings;
}

void ALight_VolumetricFogManager::CreateMorningMist()
{
    FLight_FogSettings MistSettings;
    MistSettings.FogDensity = 0.01f;
    MistSettings.FogHeightFalloff = 0.5f;
    MistSettings.StartDistance = 1000.0f;
    MistSettings.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MistSettings.bVolumetricFog = true;
    MistSettings.VolumetricFogScatteringDistribution = 0.1f;
    MistSettings.VolumetricFogAlbedo = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    
    ApplyFogSettings(MistSettings);
    CurrentFogType = ELight_FogType::Light;
}

void ALight_VolumetricFogManager::CreateEveningFog()
{
    FLight_FogSettings EveningSettings;
    EveningSettings.FogDensity = 0.05f;
    EveningSettings.FogHeightFalloff = 0.15f;
    EveningSettings.StartDistance = 2000.0f;
    EveningSettings.FogInscatteringColor = FLinearColor(0.7f, 0.6f, 0.8f, 1.0f);
    EveningSettings.bVolumetricFog = true;
    EveningSettings.VolumetricFogScatteringDistribution = 0.3f;
    EveningSettings.VolumetricFogAlbedo = FLinearColor(0.8f, 0.7f, 0.9f, 1.0f);
    
    ApplyFogSettings(EveningSettings);
    CurrentFogType = ELight_FogType::Medium;
}

void ALight_VolumetricFogManager::CreateStormFog()
{
    FLight_FogSettings StormSettings;
    StormSettings.FogDensity = 0.08f;
    StormSettings.FogHeightFalloff = 0.1f;
    StormSettings.StartDistance = 500.0f;
    StormSettings.FogInscatteringColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    StormSettings.bVolumetricFog = true;
    StormSettings.VolumetricFogScatteringDistribution = 0.5f;
    StormSettings.VolumetricFogAlbedo = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    StormSettings.VolumetricFogExtinctionScale = 1.5f;
    
    ApplyFogSettings(StormSettings);
    CurrentFogType = ELight_FogType::Heavy;
}

void ALight_VolumetricFogManager::ClearAllFog()
{
    FLight_FogSettings ClearSettings;
    ClearSettings.FogDensity = 0.001f;
    ClearSettings.FogHeightFalloff = 1.0f;
    ClearSettings.StartDistance = 50000.0f;
    ClearSettings.bVolumetricFog = false;
    
    ApplyFogSettings(ClearSettings);
    CurrentFogType = ELight_FogType::None;
}

void ALight_VolumetricFogManager::EnableVolumetricLightScattering(bool bEnable)
{
    FogSettings.bVolumetricFog = bEnable;
    ApplyFogSettings(FogSettings);
}

void ALight_VolumetricFogManager::SetAtmosphericPerspective(bool bEnable, float Distance)
{
    bEnableAtmosphericPerspective = bEnable;
    AtmosphericPerspectiveDistance = Distance;
    
    // Apply atmospheric perspective settings if available
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        if (FogComp)
        {
            // Configure atmospheric perspective through fog settings
            if (bEnable)
            {
                FogComp->SetStartDistance(FMath::Min(FogSettings.StartDistance, Distance * 0.1f));
            }
        }
    }
}

void ALight_VolumetricFogManager::InitializeFogComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing ExponentialHeightFog
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
    
    // Find existing VolumetricCloud
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VolumetricCloudActor = Cast<AVolumetricCloud>(FoundActors[0]);
    }
}

void ALight_VolumetricFogManager::UpdateFogTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    // Smooth transition curve
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    
    // Interpolate fog settings
    FLight_FogSettings InterpolatedSettings;
    InterpolatedSettings.FogDensity = FMath::Lerp(StartFogSettings.FogDensity, TargetFogSettings.FogDensity, Alpha);
    InterpolatedSettings.FogHeightFalloff = FMath::Lerp(StartFogSettings.FogHeightFalloff, TargetFogSettings.FogHeightFalloff, Alpha);
    InterpolatedSettings.StartDistance = FMath::Lerp(StartFogSettings.StartDistance, TargetFogSettings.StartDistance, Alpha);
    InterpolatedSettings.FogInscatteringColor = FMath::Lerp(StartFogSettings.FogInscatteringColor, TargetFogSettings.FogInscatteringColor, Alpha);
    InterpolatedSettings.VolumetricFogScatteringDistribution = FMath::Lerp(StartFogSettings.VolumetricFogScatteringDistribution, TargetFogSettings.VolumetricFogScatteringDistribution, Alpha);
    InterpolatedSettings.VolumetricFogAlbedo = FMath::Lerp(StartFogSettings.VolumetricFogAlbedo, TargetFogSettings.VolumetricFogAlbedo, Alpha);
    InterpolatedSettings.VolumetricFogEmissive = FMath::Lerp(StartFogSettings.VolumetricFogEmissive, TargetFogSettings.VolumetricFogEmissive, Alpha);
    InterpolatedSettings.VolumetricFogExtinctionScale = FMath::Lerp(StartFogSettings.VolumetricFogExtinctionScale, TargetFogSettings.VolumetricFogExtinctionScale, Alpha);
    InterpolatedSettings.bVolumetricFog = TargetFogSettings.bVolumetricFog;
    
    ApplyFogSettings(InterpolatedSettings);
    
    // Check if transition is complete
    if (Alpha >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentFogType = TargetFogType;
        FogSettings = TargetFogSettings;
    }
}

void ALight_VolumetricFogManager::ApplyFogTypeSettings(ELight_FogType FogType)
{
    switch (FogType)
    {
        case ELight_FogType::None:
            ClearAllFog();
            break;
        case ELight_FogType::Light:
            CreateMorningMist();
            break;
        case ELight_FogType::Medium:
            CreateEveningFog();
            break;
        case ELight_FogType::Heavy:
            CreateStormFog();
            break;
        case ELight_FogType::Volumetric:
            {
                FLight_FogSettings VolumetricSettings;
                VolumetricSettings.FogDensity = 0.03f;
                VolumetricSettings.FogHeightFalloff = 0.3f;
                VolumetricSettings.StartDistance = 3000.0f;
                VolumetricSettings.bVolumetricFog = true;
                VolumetricSettings.VolumetricFogScatteringDistribution = 0.4f;
                VolumetricSettings.VolumetricFogExtinctionScale = 1.2f;
                ApplyFogSettings(VolumetricSettings);
            }
            break;
    }
}