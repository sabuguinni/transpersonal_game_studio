#include "Light_VolumetricAtmosphereSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULight_VolumetricAtmosphereSystem::ULight_VolumetricAtmosphereSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    DirectionalLightActor = nullptr;
    SkyAtmosphereActor = nullptr;
    VolumetricCloudActor = nullptr;
    ExponentialHeightFogActor = nullptr;
}

void ULight_VolumetricAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindAtmosphereActors();
    ApplyAtmosphereSettings(GetPresetSettings(CurrentPreset));
}

void ULight_VolumetricAtmosphereSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        InterpolateSettings(Alpha);
        
        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
            AtmosphereSettings = TargetSettings;
        }
    }
}

void ULight_VolumetricAtmosphereSystem::SetAtmospherePreset(ELight_AtmospherePreset NewPreset)
{
    CurrentPreset = NewPreset;
    ApplyAtmosphereSettings(GetPresetSettings(NewPreset));
}

void ULight_VolumetricAtmosphereSystem::ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    AtmosphereSettings = Settings;
    
    UpdateDirectionalLight();
    UpdateSkyAtmosphere();
    UpdateVolumetricFog();
}

void ULight_VolumetricAtmosphereSystem::TransitionToPreset(ELight_AtmospherePreset TargetPreset, float TransitionDuration)
{
    if (bIsTransitioning)
    {
        return;
    }
    
    TargetSettings = GetPresetSettings(TargetPreset);
    this->TransitionDuration = TransitionDuration;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
    CurrentPreset = TargetPreset;
}

FLight_AtmosphereSettings ULight_VolumetricAtmosphereSystem::GetPresetSettings(ELight_AtmospherePreset Preset)
{
    FLight_AtmosphereSettings Settings;
    
    switch (Preset)
    {
        case ELight_AtmospherePreset::CretaceousClear:
            Settings.SunIntensity = 5.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
            Settings.FogDensity = 0.01f;
            Settings.FogHeightFalloff = 0.2f;
            Settings.FogInscatteringColor = FLinearColor(0.45f, 0.55f, 0.6f, 1.0f);
            Settings.CloudCoverage = 0.2f;
            Settings.VolumetricScatteringIntensity = 1.0f;
            break;
            
        case ELight_AtmospherePreset::CretaceousOvercast:
            Settings.SunIntensity = 3.0f;
            Settings.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
            Settings.FogDensity = 0.03f;
            Settings.FogHeightFalloff = 0.15f;
            Settings.FogInscatteringColor = FLinearColor(0.5f, 0.55f, 0.6f, 1.0f);
            Settings.CloudCoverage = 0.7f;
            Settings.VolumetricScatteringIntensity = 0.7f;
            break;
            
        case ELight_AtmospherePreset::CretaceousStorm:
            Settings.SunIntensity = 1.5f;
            Settings.SunColor = FLinearColor(0.6f, 0.65f, 0.75f, 1.0f);
            Settings.FogDensity = 0.05f;
            Settings.FogHeightFalloff = 0.1f;
            Settings.FogInscatteringColor = FLinearColor(0.4f, 0.45f, 0.55f, 1.0f);
            Settings.CloudCoverage = 0.9f;
            Settings.VolumetricScatteringIntensity = 0.5f;
            break;
            
        case ELight_AtmospherePreset::CretaceousFog:
            Settings.SunIntensity = 2.5f;
            Settings.SunColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);
            Settings.FogDensity = 0.08f;
            Settings.FogHeightFalloff = 0.05f;
            Settings.FogInscatteringColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
            Settings.CloudCoverage = 0.5f;
            Settings.VolumetricScatteringIntensity = 1.2f;
            break;
            
        case ELight_AtmospherePreset::CretaceousSunset:
            Settings.SunIntensity = 4.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            Settings.FogDensity = 0.02f;
            Settings.FogHeightFalloff = 0.25f;
            Settings.FogInscatteringColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
            Settings.CloudCoverage = 0.4f;
            Settings.VolumetricScatteringIntensity = 1.5f;
            break;
            
        case ELight_AtmospherePreset::CretaceousNight:
            Settings.SunIntensity = 0.5f;
            Settings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
            Settings.FogDensity = 0.015f;
            Settings.FogHeightFalloff = 0.3f;
            Settings.FogInscatteringColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
            Settings.CloudCoverage = 0.3f;
            Settings.VolumetricScatteringIntensity = 0.8f;
            break;
    }
    
    return Settings;
}

void ULight_VolumetricAtmosphereSystem::UpdateVolumetricFog()
{
    if (!bEnableVolumetricFog)
    {
        return;
    }
    
    if (ExponentialHeightFogActor)
    {
        UExponentialHeightFogComponent* FogComponent = ExponentialHeightFogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(AtmosphereSettings.FogDensity);
            FogComponent->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
            FogComponent->SetFogInscatteringColor(AtmosphereSettings.FogInscatteringColor);
            FogComponent->SetVolumetricFogScatteringDistribution(AtmosphereSettings.VolumetricScatteringIntensity);
        }
    }
}

void ULight_VolumetricAtmosphereSystem::UpdateDirectionalLight()
{
    if (DirectionalLightActor)
    {
        UDirectionalLightComponent* LightComponent = DirectionalLightActor->FindComponentByClass<UDirectionalLightComponent>();
        if (LightComponent)
        {
            LightComponent->SetIntensity(AtmosphereSettings.SunIntensity);
            LightComponent->SetLightColor(AtmosphereSettings.SunColor);
            LightComponent->SetCastVolumetricShadow(true);
            LightComponent->SetVolumetricScatteringIntensity(AtmosphereSettings.VolumetricScatteringIntensity);
        }
    }
}

void ULight_VolumetricAtmosphereSystem::UpdateSkyAtmosphere()
{
    if (SkyAtmosphereActor)
    {
        USkyAtmosphereComponent* SkyComponent = SkyAtmosphereActor->FindComponentByClass<USkyAtmosphereComponent>();
        if (SkyComponent)
        {
            // Configure sky atmosphere for Cretaceous period
            SkyComponent->SetAtmosphereHeight(60.0f);
            SkyComponent->SetAerialPespectiveViewDistanceScale(1.0f);
            SkyComponent->SetRayleighScatteringScale(0.331f);
            SkyComponent->SetMieScatteringScale(0.003996f);
        }
    }
}

void ULight_VolumetricAtmosphereSystem::FindAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find DirectionalLight
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        DirectionalLightActor = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find SkyAtmosphere
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    for (AActor* Actor : AllActors)
    {
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphereActor = Actor;
            break;
        }
    }
    
    // Find ExponentialHeightFog
    for (AActor* Actor : AllActors)
    {
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            ExponentialHeightFogActor = Actor;
            break;
        }
    }
}

void ULight_VolumetricAtmosphereSystem::InterpolateSettings(float Alpha)
{
    FLight_AtmosphereSettings CurrentSettings = AtmosphereSettings;
    
    AtmosphereSettings.SunIntensity = FMath::Lerp(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, Alpha);
    AtmosphereSettings.SunColor = FMath::Lerp(CurrentSettings.SunColor, TargetSettings.SunColor, Alpha);
    AtmosphereSettings.FogDensity = FMath::Lerp(CurrentSettings.FogDensity, TargetSettings.FogDensity, Alpha);
    AtmosphereSettings.FogHeightFalloff = FMath::Lerp(CurrentSettings.FogHeightFalloff, TargetSettings.FogHeightFalloff, Alpha);
    AtmosphereSettings.FogInscatteringColor = FMath::Lerp(CurrentSettings.FogInscatteringColor, TargetSettings.FogInscatteringColor, Alpha);
    AtmosphereSettings.CloudCoverage = FMath::Lerp(CurrentSettings.CloudCoverage, TargetSettings.CloudCoverage, Alpha);
    AtmosphereSettings.VolumetricScatteringIntensity = FMath::Lerp(CurrentSettings.VolumetricScatteringIntensity, TargetSettings.VolumetricScatteringIntensity, Alpha);
    
    UpdateDirectionalLight();
    UpdateSkyAtmosphere();
    UpdateVolumetricFog();
}

FLight_AtmosphereSettings ULight_VolumetricAtmosphereSystem::GetCurrentInterpolatedSettings()
{
    return AtmosphereSettings;
}