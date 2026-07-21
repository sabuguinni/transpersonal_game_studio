#include "EnvArt_CretaceousAtmosphericEnhancer.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogCretaceousAtmosphere, Log, All);

AEnvArt_CretaceousAtmosphericEnhancer::AEnvArt_CretaceousAtmosphericEnhancer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
    
    // Initialize atmospheric settings
    GoldenHourIntensity = 3.5f;
    GoldenHourColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    SunAnglePitch = -25.0f;
    SunAngleYaw = 45.0f;
    
    FogDensity = 0.02f;
    FogHeightFalloff = 0.2f;
    FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    VolumetricFogScatteringDistribution = 0.2f;
    VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
    
    bAtmosphereEnhanced = false;
    bAutoEnhanceOnBeginPlay = true;
}

void AEnvArt_CretaceousAtmosphericEnhancer::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoEnhanceOnBeginPlay)
    {
        EnhanceAtmosphere();
    }
}

void AEnvArt_CretaceousAtmosphericEnhancer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor atmospheric conditions and adjust if needed
    if (bAtmosphereEnhanced)
    {
        ValidateAtmosphericSettings();
    }
}

void AEnvArt_CretaceousAtmosphericEnhancer::EnhanceAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCretaceousAtmosphere, Warning, TEXT("No valid world found"));
        return;
    }
    
    // Enhance directional lighting
    EnhanceDirectionalLighting();
    
    // Enhance volumetric fog
    EnhanceVolumetricFog();
    
    bAtmosphereEnhanced = true;
    UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Cretaceous atmospheric enhancement complete"));
}

void AEnvArt_CretaceousAtmosphericEnhancer::EnhanceDirectionalLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light in the world
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirectionalLight = *ActorItr;
        if (DirectionalLight)
        {
            UDirectionalLightComponent* LightComponent = DirectionalLight->GetLightComponent();
            if (LightComponent)
            {
                // Apply golden hour settings
                LightComponent->SetIntensity(GoldenHourIntensity);
                LightComponent->SetLightColor(GoldenHourColor);
                
                // Set sun angle for atmospheric lighting
                FRotator NewRotation(SunAnglePitch, SunAngleYaw, 0.0f);
                DirectionalLight->SetActorRotation(NewRotation);
                
                UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Enhanced directional light with golden hour settings"));
                break;
            }
        }
    }
}

void AEnvArt_CretaceousAtmosphericEnhancer::EnhanceVolumetricFog()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find exponential height fog in the world
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        AExponentialHeightFog* FogActor = *ActorItr;
        if (FogActor)
        {
            UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
            if (FogComponent)
            {
                // Apply enhanced fog settings
                FogComponent->FogDensity = FogDensity;
                FogComponent->FogHeightFalloff = FogHeightFalloff;
                FogComponent->FogInscatteringColor = FogInscatteringColor;
                FogComponent->bEnableVolumetricFog = true;
                FogComponent->VolumetricFogScatteringDistribution = VolumetricFogScatteringDistribution;
                FogComponent->VolumetricFogAlbedo = VolumetricFogAlbedo;
                
                UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Enhanced volumetric fog for atmospheric depth"));
                break;
            }
        }
    }
}

void AEnvArt_CretaceousAtmosphericEnhancer::ValidateAtmosphericSettings()
{
    // Validate that atmospheric settings are still applied
    // This can be expanded to monitor and auto-correct atmospheric drift
}

void AEnvArt_CretaceousAtmosphericEnhancer::SetGoldenHourSettings(float Intensity, FLinearColor Color, float Pitch, float Yaw)
{
    GoldenHourIntensity = Intensity;
    GoldenHourColor = Color;
    SunAnglePitch = Pitch;
    SunAngleYaw = Yaw;
    
    if (bAtmosphereEnhanced)
    {
        EnhanceDirectionalLighting();
    }
}

void AEnvArt_CretaceousAtmosphericEnhancer::SetFogSettings(float Density, float HeightFalloff, FLinearColor InscatteringColor)
{
    FogDensity = Density;
    FogHeightFalloff = HeightFalloff;
    FogInscatteringColor = InscatteringColor;
    
    if (bAtmosphereEnhanced)
    {
        EnhanceVolumetricFog();
    }
}

bool AEnvArt_CretaceousAtmosphericEnhancer::IsAtmosphereEnhanced() const
{
    return bAtmosphereEnhanced;
}