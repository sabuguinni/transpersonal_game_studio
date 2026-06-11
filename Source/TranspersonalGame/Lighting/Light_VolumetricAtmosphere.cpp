#include "Light_VolumetricAtmosphere.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/PostProcessComponent.h"

ULight_VolumetricAtmosphere::ULight_VolumetricAtmosphere()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULight_VolumetricAtmosphere::BeginPlay()
{
    Super::BeginPlay();
    
    FindAtmosphereActors();
    ApplyCretaceousAtmosphere();
}

void ULight_VolumetricAtmosphere::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULight_VolumetricAtmosphere::ApplyCretaceousAtmosphere()
{
    ConfigureDirectionalLight();
    ConfigureFogComponent();
    ConfigureSkyAtmosphere();
    ConfigurePostProcess();
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous atmosphere applied successfully"));
}

void ULight_VolumetricAtmosphere::ConfigureVolumetricFog()
{
    if (!ExponentialHeightFog)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComponent = ExponentialHeightFog->FindComponentByClass<UExponentialHeightFogComponent>();
    if (FogComponent)
    {
        FogComponent->FogDensity = AtmosphereSettings.FogDensity;
        FogComponent->FogHeightFalloff = AtmosphereSettings.FogHeightFalloff;
        FogComponent->FogInscatteringColor = AtmosphereSettings.FogInscatteringColor;
        FogComponent->bEnableVolumetricFog = AtmosphereSettings.bVolumetricFog;
        FogComponent->VolumetricFogScatteringDistribution = AtmosphereSettings.VolumetricScatteringDistribution;
        FogComponent->VolumetricFogAlbedo = AtmosphereSettings.VolumetricAlbedo;
        
        FogComponent->MarkRenderStateDirty();
    }
}

void ULight_VolumetricAtmosphere::SetupLumenGlobalIllumination()
{
    if (!PostProcessVolume)
    {
        return;
    }

    UPostProcessComponent* PPComponent = PostProcessVolume->FindComponentByClass<UPostProcessComponent>();
    if (PPComponent)
    {
        PPComponent->Settings.bOverride_DynamicGlobalIlluminationMethod = true;
        PPComponent->Settings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
        
        PPComponent->Settings.bOverride_ReflectionMethod = true;
        PPComponent->Settings.ReflectionMethod = EReflectionMethod::Lumen;
        
        PPComponent->Settings.bOverride_LumenSceneLightingQuality = true;
        PPComponent->Settings.LumenSceneLightingQuality = 1.0f;
        
        PPComponent->Settings.bOverride_LumenSceneDetail = true;
        PPComponent->Settings.LumenSceneDetail = 1.0f;
        
        PPComponent->MarkRenderStateDirty();
    }
}

void ULight_VolumetricAtmosphere::UpdateAtmosphereSettings(const FLight_AtmosphereSettings& NewSettings)
{
    AtmosphereSettings = NewSettings;
    ApplyCretaceousAtmosphere();
}

void ULight_VolumetricAtmosphere::FindAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsWithTag(World, FName("ExponentialHeightFog"), FoundActors);
    if (FoundActors.Num() > 0)
    {
        ExponentialHeightFog = FoundActors[0];
    }

    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsWithTag(World, FName("SkyAtmosphere"), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = FoundActors[0];
    }

    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void ULight_VolumetricAtmosphere::ApplyAtmosphereInEditor()
{
    FindAtmosphereActors();
    ApplyCretaceousAtmosphere();
}

void ULight_VolumetricAtmosphere::ConfigureDirectionalLight()
{
    if (!SunLight)
    {
        return;
    }

    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    if (LightComponent)
    {
        LightComponent->SetIntensity(AtmosphereSettings.SunIntensity);
        LightComponent->SetLightColor(AtmosphereSettings.SunColor);
        LightComponent->SetCastShadows(true);
        LightComponent->SetCastVolumetricShadow(true);
        LightComponent->MarkRenderStateDirty();
    }
}

void ULight_VolumetricAtmosphere::ConfigureFogComponent()
{
    ConfigureVolumetricFog();
}

void ULight_VolumetricAtmosphere::ConfigureSkyAtmosphere()
{
    if (!SkyAtmosphere)
    {
        return;
    }

    USkyAtmosphereComponent* AtmosphereComponent = SkyAtmosphere->FindComponentByClass<USkyAtmosphereComponent>();
    if (AtmosphereComponent)
    {
        AtmosphereComponent->MarkRenderStateDirty();
    }
}

void ULight_VolumetricAtmosphere::ConfigurePostProcess()
{
    SetupLumenGlobalIllumination();
}