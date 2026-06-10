#include "Light_VolumetricController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

ULight_VolumetricController::ULight_VolumetricController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize Cretaceous default settings (humid, warm atmosphere)
    CretaceousSettings.FogDensity = 0.02f;
    CretaceousSettings.FogHeightFalloff = 0.2f;
    CretaceousSettings.FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f);
    CretaceousSettings.VolumetricScatteringDistribution = 0.2f;
    CretaceousSettings.VolumetricFogDistance = 6000.0f;
    CretaceousSettings.bEnableVolumetricFog = true;
    
    // Storm settings (dense, dramatic fog)
    StormSettings.FogDensity = 0.08f;
    StormSettings.FogHeightFalloff = 0.15f;
    StormSettings.FogInscatteringColor = FLinearColor(0.6f, 0.65f, 0.7f);
    StormSettings.VolumetricScatteringDistribution = 0.4f;
    StormSettings.VolumetricFogDistance = 3000.0f;
    StormSettings.bEnableVolumetricFog = true;
    
    // Clear settings (minimal atmospheric haze)
    ClearSettings.FogDensity = 0.005f;
    ClearSettings.FogHeightFalloff = 0.3f;
    ClearSettings.FogInscatteringColor = FLinearColor(0.95f, 0.9f, 0.85f);
    ClearSettings.VolumetricScatteringDistribution = 0.1f;
    ClearSettings.VolumetricFogDistance = 10000.0f;
    ClearSettings.bEnableVolumetricFog = true;
    
    CurrentSettings = CretaceousSettings;
    TargetSettings = CretaceousSettings;
}

void ULight_VolumetricController::BeginPlay()
{
    Super::BeginPlay();
    
    FindOrCreateFogActor();
    InitializeDefaultSettings();
    ApplyVolumetricSettings(CretaceousSettings);
}

void ULight_VolumetricController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bTransitioning)
    {
        UpdateVolumetricTransition(DeltaTime);
    }
}

void ULight_VolumetricController::FindOrCreateFogActor()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        FogActor = *ActorItr;
        FogComponent = FogActor->GetComponent();
        break;
    }
    
    // Create if doesn't exist
    if (!FogActor)
    {
        FogActor = World->SpawnActor<AExponentialHeightFog>();
        if (FogActor)
        {
            FogComponent = FogActor->GetComponent();
        }
    }
}

void ULight_VolumetricController::InitializeDefaultSettings()
{
    if (!FogComponent) return;
    
    // Enable volumetric fog for Cretaceous atmosphere
    FogComponent->SetVolumetricFog(true);
    FogComponent->SetVolumetricFogScatteringDistribution(0.2f);
    FogComponent->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
    FogComponent->SetVolumetricFogEmissive(FLinearColor(0.0f, 0.0f, 0.0f));
    FogComponent->SetVolumetricFogExtinctionScale(1.0f);
}

void ULight_VolumetricController::SetWeatherType(EWeatherType NewWeatherType)
{
    if (CurrentWeatherType == NewWeatherType) return;
    
    CurrentWeatherType = NewWeatherType;
    
    FLight_VolumetricSettings NewSettings;
    switch (NewWeatherType)
    {
        case EWeatherType::Clear:
            NewSettings = ClearSettings;
            break;
        case EWeatherType::Storm:
            NewSettings = StormSettings;
            break;
        case EWeatherType::Overcast:
        default:
            NewSettings = CretaceousSettings;
            break;
    }
    
    TransitionToSettings(NewSettings, 3.0f);
}

void ULight_VolumetricController::ApplyVolumetricSettings(const FLight_VolumetricSettings& Settings)
{
    if (!FogComponent) return;
    
    CurrentSettings = Settings;
    
    FogComponent->SetFogDensity(Settings.FogDensity);
    FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
    FogComponent->SetFogInscatteringColor(Settings.FogInscatteringColor);
    FogComponent->SetVolumetricFogScatteringDistribution(Settings.VolumetricScatteringDistribution);
    FogComponent->SetVolumetricFogDistance(Settings.VolumetricFogDistance);
    FogComponent->SetVolumetricFog(Settings.bEnableVolumetricFog);
    
    UE_LOG(LogTemp, Warning, TEXT("Applied volumetric settings - Density: %f, Weather: %d"), 
           Settings.FogDensity, (int32)CurrentWeatherType);
}

void ULight_VolumetricController::TransitionToSettings(const FLight_VolumetricSettings& NewSettings, float Duration)
{
    TargetSettings = NewSettings;
    TransitionSpeed = (Duration > 0.0f) ? 1.0f / Duration : 10.0f;
    TransitionProgress = 0.0f;
    bTransitioning = true;
}

void ULight_VolumetricController::ApplyCretaceousVolumetrics()
{
    ApplyVolumetricSettings(CretaceousSettings);
}

void ULight_VolumetricController::UpdateVolumetricTransition(float DeltaTime)
{
    TransitionProgress += TransitionSpeed * DeltaTime;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bTransitioning = false;
    }
    
    FLight_VolumetricSettings LerpedSettings = LerpSettings(CurrentSettings, TargetSettings, TransitionProgress);
    ApplyVolumetricSettings(LerpedSettings);
    
    if (!bTransitioning)
    {
        CurrentSettings = TargetSettings;
    }
}

FLight_VolumetricSettings ULight_VolumetricController::LerpSettings(const FLight_VolumetricSettings& A, const FLight_VolumetricSettings& B, float Alpha)
{
    FLight_VolumetricSettings Result;
    
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.VolumetricScatteringDistribution = FMath::Lerp(A.VolumetricScatteringDistribution, B.VolumetricScatteringDistribution, Alpha);
    Result.VolumetricFogDistance = FMath::Lerp(A.VolumetricFogDistance, B.VolumetricFogDistance, Alpha);
    Result.bEnableVolumetricFog = (Alpha < 0.5f) ? A.bEnableVolumetricFog : B.bEnableVolumetricFog;
    
    return Result;
}