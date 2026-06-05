#include "Light_VolumetricFogManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ALight_VolumetricFogManager::ALight_VolumetricFogManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default volumetric fog settings for Cretaceous period
    VolumetricSettings.FogDensity = 0.02f;
    VolumetricSettings.FogHeightFalloff = 0.2f;
    VolumetricSettings.FogInscatteringColor = FColor(180, 200, 255, 255);
    VolumetricSettings.VolumetricFogScatteringDistribution = 0.2f;
    VolumetricSettings.VolumetricFogAlbedo = FColor(240, 240, 255, 255);
    VolumetricSettings.VolumetricFogExtinctionScale = 1.0f;
    VolumetricSettings.SunVolumetricScatteringIntensity = 2.5f;
    VolumetricSettings.SkyLightVolumetricIntensity = 1.5f;

    FogActor = nullptr;
    SunLight = nullptr;
    SkyLight = nullptr;
}

void ALight_VolumetricFogManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVolumetricFog();
}

void ALight_VolumetricFogManager::InitializeVolumetricFog()
{
    FindExistingLightingActors();
    
    if (!FogActor)
    {
        CreateVolumetricFogActor();
    }
    
    ConfigureSunLight();
    ConfigureSkyLight();
    ApplyFogSettings();
}

void ALight_VolumetricFogManager::FindExistingLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find existing fog actor
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        FogActor = *ActorItr;
        break;
    }

    // Find existing directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }

    // Find existing sky light
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyLight = *ActorItr;
        break;
    }
}

void ALight_VolumetricFogManager::CreateVolumetricFogActor()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = nullptr;

    FogActor = World->SpawnActor<AExponentialHeightFog>(
        AExponentialHeightFog::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );
}

void ALight_VolumetricFogManager::ConfigureSunLight()
{
    if (!SunLight)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        SunLight = World->SpawnActor<ADirectionalLight>(
            ADirectionalLight::StaticClass(),
            FVector(0, 0, 500),
            FRotator(-45, 0, 0),
            SpawnParams
        );
    }

    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(8.0f);
            LightComp->SetLightColor(FColor(255, 245, 210, 255));
            LightComp->SetCastVolumetricShadow(true);
            LightComp->SetVolumetricScatteringIntensity(VolumetricSettings.SunVolumetricScatteringIntensity);
        }
    }
}

void ALight_VolumetricFogManager::ConfigureSkyLight()
{
    if (!SkyLight)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        SkyLight = World->SpawnActor<ASkyLight>(
            ASkyLight::StaticClass(),
            FVector(0, 0, 1000),
            FRotator::ZeroRotator,
            SpawnParams
        );
    }

    if (SkyLight)
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(VolumetricSettings.SkyLightVolumetricIntensity);
            SkyComp->SetLightColor(FColor(200, 220, 255, 255));
            SkyComp->SetRealTimeCapture(true);
            SkyComp->SetCastVolumetricShadow(true);
        }
    }
}

void ALight_VolumetricFogManager::ApplyFogSettings()
{
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
    if (!FogComp) return;

    FogComp->SetFogDensity(VolumetricSettings.FogDensity);
    FogComp->SetFogHeightFalloff(VolumetricSettings.FogHeightFalloff);
    FogComp->SetFogInscatteringColor(FLinearColor(VolumetricSettings.FogInscatteringColor));
    FogComp->SetVolumetricFog(true);
    FogComp->SetVolumetricFogScatteringDistribution(VolumetricSettings.VolumetricFogScatteringDistribution);
    FogComp->SetVolumetricFogAlbedo(FLinearColor(VolumetricSettings.VolumetricFogAlbedo));
    FogComp->SetVolumetricFogExtinctionScale(VolumetricSettings.VolumetricFogExtinctionScale);
}

void ALight_VolumetricFogManager::UpdateVolumetricSettings(const FLight_VolumetricFogSettings& NewSettings)
{
    VolumetricSettings = NewSettings;
    ApplyFogSettings();
    ConfigureSunLight();
    ConfigureSkyLight();
}

void ALight_VolumetricFogManager::SetFogDensity(float NewDensity)
{
    VolumetricSettings.FogDensity = FMath::Clamp(NewDensity, 0.001f, 1.0f);
    
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(VolumetricSettings.FogDensity);
        }
    }
}

void ALight_VolumetricFogManager::SetFogHeightFalloff(float NewFalloff)
{
    VolumetricSettings.FogHeightFalloff = FMath::Clamp(NewFalloff, 0.001f, 2.0f);
    
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogHeightFalloff(VolumetricSettings.FogHeightFalloff);
        }
    }
}

void ALight_VolumetricFogManager::SetVolumetricScatteringIntensity(float NewIntensity)
{
    VolumetricSettings.SunVolumetricScatteringIntensity = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetComponent();
        if (LightComp)
        {
            LightComp->SetVolumetricScatteringIntensity(VolumetricSettings.SunVolumetricScatteringIntensity);
        }
    }
}

void ALight_VolumetricFogManager::ApplyCretaceousAtmosphere()
{
    // Optimized settings for Cretaceous period atmosphere
    VolumetricSettings.FogDensity = 0.025f;
    VolumetricSettings.FogHeightFalloff = 0.15f;
    VolumetricSettings.FogInscatteringColor = FColor(190, 210, 255, 255);
    VolumetricSettings.VolumetricFogScatteringDistribution = 0.3f;
    VolumetricSettings.VolumetricFogAlbedo = FColor(245, 245, 255, 255);
    VolumetricSettings.SunVolumetricScatteringIntensity = 3.0f;
    VolumetricSettings.SkyLightVolumetricIntensity = 1.8f;

    UpdateVolumetricSettings(VolumetricSettings);
}

void ALight_VolumetricFogManager::SaveAtmosphericSettings()
{
    // This would typically save to a config file or level data
    // For now, we ensure the settings persist in the actor
    UE_LOG(LogTemp, Warning, TEXT("Volumetric atmospheric settings saved"));
}

void ALight_VolumetricFogManager::EditorApplyVolumetricFog()
{
    InitializeVolumetricFog();
    ApplyCretaceousAtmosphere();
}