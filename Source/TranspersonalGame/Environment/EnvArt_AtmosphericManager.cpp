#include "EnvArt_AtmosphericManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default atmospheric settings
    AtmosphericSettings.SunIntensity = 8.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AtmosphericSettings.SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    AtmosphericSettings.VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    AtmosphericSettings.VolumetricFogScatteringDistribution = 0.6f;
    
    DirectionalLightRef = nullptr;
    FogActorRef = nullptr;
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and cache atmospheric actors in the world
    FindAndCacheAtmosphericActors();
    
    // Apply initial atmospheric settings
    ApplyAtmosphericSettings();
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnvArt_AtmosphericManager::FindAndCacheAtmosphericActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        DirectionalLightRef = *ActorItr;
        break; // Use first found
    }
    
    // Find exponential height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        FogActorRef = *ActorItr;
        break; // Use first found
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericManager: Found DirectionalLight=%s, Fog=%s"), 
           DirectionalLightRef ? TEXT("Yes") : TEXT("No"),
           FogActorRef ? TEXT("Yes") : TEXT("No"));
}

void AEnvArt_AtmosphericManager::ApplyAtmosphericSettings()
{
    UpdateDirectionalLight();
    UpdateFogSettings();
}

void AEnvArt_AtmosphericManager::SetGoldenHourLighting()
{
    AtmosphericSettings.SunIntensity = 8.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AtmosphericSettings.SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
    ApplyAtmosphericSettings();
}

void AEnvArt_AtmosphericManager::SetMidDayLighting()
{
    AtmosphericSettings.SunIntensity = 12.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    AtmosphericSettings.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    ApplyAtmosphericSettings();
}

void AEnvArt_AtmosphericManager::SetDuskLighting()
{
    AtmosphericSettings.SunIntensity = 4.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    AtmosphericSettings.SunRotation = FRotator(-15.0f, 60.0f, 0.0f);
    ApplyAtmosphericSettings();
}

void AEnvArt_AtmosphericManager::EnhanceVolumetricFog()
{
    AtmosphericSettings.FogDensity = 0.025f;
    AtmosphericSettings.VolumetricFogScatteringDistribution = 0.8f;
    UpdateFogSettings();
}

void AEnvArt_AtmosphericManager::UpdateDirectionalLight()
{
    if (!DirectionalLightRef)
    {
        return;
    }
    
    // Update light transform
    DirectionalLightRef->SetActorRotation(AtmosphericSettings.SunRotation);
    
    // Update light component properties
    UDirectionalLightComponent* LightComp = DirectionalLightRef->GetDirectionalLightComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(AtmosphericSettings.SunIntensity);
        LightComp->SetLightColor(AtmosphericSettings.SunColor);
        LightComp->SetCastVolumetricFog(true);
        LightComp->SetVolumetricScatteringIntensity(1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Updated DirectionalLight: Intensity=%.1f, Color=(%.2f,%.2f,%.2f)"),
               AtmosphericSettings.SunIntensity,
               AtmosphericSettings.SunColor.R,
               AtmosphericSettings.SunColor.G,
               AtmosphericSettings.SunColor.B);
    }
}

void AEnvArt_AtmosphericManager::UpdateFogSettings()
{
    if (!FogActorRef)
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComp = FogActorRef->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(AtmosphericSettings.FogDensity);
        FogComp->SetFogHeightFalloff(AtmosphericSettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(AtmosphericSettings.FogInscatteringColor);
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(AtmosphericSettings.VolumetricFogScatteringDistribution);
        FogComp->SetVolumetricFogAlbedo(AtmosphericSettings.VolumetricFogAlbedo);
        FogComp->SetVolumetricFogEmissive(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
        FogComp->SetVolumetricFogExtinctionScale(1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Updated Fog: Density=%.3f, Falloff=%.2f, Volumetric=True"),
               AtmosphericSettings.FogDensity,
               AtmosphericSettings.FogHeightFalloff);
    }
}