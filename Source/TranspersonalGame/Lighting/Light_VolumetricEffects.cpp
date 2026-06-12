#include "Light_VolumetricEffects.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

ALight_VolumetricEffects::ALight_VolumetricEffects()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create volume mesh component for volumetric bounds
    VolumeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolumeMesh"));
    VolumeMesh->SetupAttachment(RootComponent);
    VolumeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VolumeMesh->SetCastShadow(false);

    // Create volumetric light component
    VolumetricLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("VolumetricLight"));
    VolumetricLight->SetupAttachment(RootComponent);
    VolumetricLight->SetIntensity(1000.0f);
    VolumetricLight->SetLightColor(FLinearColor::White);
    VolumetricLight->SetCastVolumetricShadow(true);
    VolumetricLight->SetVolumetricScatteringIntensity(1.0f);

    // Create particle effect component
    ParticleEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleEffect"));
    ParticleEffect->SetupAttachment(RootComponent);

    // Initialize default settings
    VolumetricType = ELight_VolumetricType::Sunbeams;
    bAnimateIntensity = false;
    AnimationSpeed = 1.0f;
    IntensityVariation = 0.2f;
    BaseIntensity = 1000.0f;
    AnimationTime = 0.0f;

    // Configure default volumetric settings
    VolumetricSettings.Intensity = 1.0f;
    VolumetricSettings.Color = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    VolumetricSettings.Density = 0.5f;
    VolumetricSettings.ScatteringDistribution = 0.2f;
    VolumetricSettings.bCastVolumetricShadows = true;
}

void ALight_VolumetricEffects::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateVolumetricEffect();
    BaseIntensity = VolumetricLight->Intensity;
}

void ALight_VolumetricEffects::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAnimateIntensity)
    {
        AnimateVolumetricIntensity(DeltaTime);
    }
}

void ALight_VolumetricEffects::SetVolumetricType(ELight_VolumetricType NewType)
{
    VolumetricType = NewType;
    UpdateVolumetricEffect();
}

void ALight_VolumetricEffects::ApplyVolumetricSettings(const FLight_VolumetricSettings& Settings)
{
    VolumetricSettings = Settings;

    if (VolumetricLight)
    {
        VolumetricLight->SetIntensity(Settings.Intensity * 1000.0f);
        VolumetricLight->SetLightColor(Settings.Color);
        VolumetricLight->SetVolumetricScatteringIntensity(Settings.Density);
        VolumetricLight->SetCastVolumetricShadow(Settings.bCastVolumetricShadows);
    }

    BaseIntensity = Settings.Intensity * 1000.0f;
}

void ALight_VolumetricEffects::CreateSunbeamEffect(FVector SunDirection, float BeamLength)
{
    VolumetricType = ELight_VolumetricType::Sunbeams;
    
    // Configure for sunbeam volumetrics
    VolumetricSettings.Intensity = 2.0f;
    VolumetricSettings.Color = FLinearColor(1.0f, 0.95f, 0.7f, 1.0f);
    VolumetricSettings.Density = 0.3f;
    VolumetricSettings.ScatteringDistribution = 0.8f; // Strong forward scattering for sunbeams
    
    ApplyVolumetricSettings(VolumetricSettings);
    
    // Orient light along sun direction
    FRotator SunRotation = SunDirection.Rotation();
    SetActorRotation(SunRotation);
    
    // Extend light range for sunbeam effect
    if (VolumetricLight)
    {
        VolumetricLight->SetAttenuationRadius(BeamLength);
        VolumetricLight->SetSourceRadius(50.0f); // Larger source for softer shadows
    }
}

void ALight_VolumetricEffects::CreateFirelightEffect(float FlickerIntensity)
{
    VolumetricType = ELight_VolumetricType::Firelight;
    
    // Configure for firelight volumetrics
    VolumetricSettings.Intensity = 1.5f;
    VolumetricSettings.Color = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    VolumetricSettings.Density = 0.8f;
    VolumetricSettings.ScatteringDistribution = 0.1f; // More isotropic scattering
    
    ApplyVolumetricSettings(VolumetricSettings);
    
    // Enable flickering animation
    bAnimateIntensity = true;
    IntensityVariation = FlickerIntensity;
    AnimationSpeed = 3.0f; // Fast flickering for fire
    
    if (VolumetricLight)
    {
        VolumetricLight->SetAttenuationRadius(500.0f);
        VolumetricLight->SetSourceRadius(20.0f);
    }
}

void ALight_VolumetricEffects::CreateAtmosphericMist(float MistDensity)
{
    VolumetricType = ELight_VolumetricType::Mist;
    
    // Configure for atmospheric mist
    VolumetricSettings.Intensity = 0.8f;
    VolumetricSettings.Color = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    VolumetricSettings.Density = MistDensity;
    VolumetricSettings.ScatteringDistribution = 0.0f; // Isotropic scattering for mist
    
    ApplyVolumetricSettings(VolumetricSettings);
    
    // Enable gentle animation for mist movement
    bAnimateIntensity = true;
    IntensityVariation = 0.1f;
    AnimationSpeed = 0.5f; // Slow, gentle movement
    
    if (VolumetricLight)
    {
        VolumetricLight->SetAttenuationRadius(2000.0f); // Large radius for atmospheric effect
        VolumetricLight->SetSourceRadius(100.0f);
    }
}

void ALight_VolumetricEffects::UpdateVolumetricEffect()
{
    switch (VolumetricType)
    {
        case ELight_VolumetricType::Sunbeams:
            CreateSunbeamEffect(FVector(1.0f, 0.0f, -0.5f).GetSafeNormal());
            break;
            
        case ELight_VolumetricType::Firelight:
            CreateFirelightEffect(0.3f);
            break;
            
        case ELight_VolumetricType::Moonbeams:
            VolumetricSettings.Intensity = 0.5f;
            VolumetricSettings.Color = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
            VolumetricSettings.Density = 0.2f;
            ApplyVolumetricSettings(VolumetricSettings);
            break;
            
        case ELight_VolumetricType::Mist:
            CreateAtmosphericMist(0.1f);
            break;
            
        case ELight_VolumetricType::Smoke:
            VolumetricSettings.Intensity = 0.3f;
            VolumetricSettings.Color = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
            VolumetricSettings.Density = 1.0f;
            ApplyVolumetricSettings(VolumetricSettings);
            bAnimateIntensity = true;
            AnimationSpeed = 2.0f;
            break;
            
        case ELight_VolumetricType::Dust:
            VolumetricSettings.Intensity = 0.8f;
            VolumetricSettings.Color = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
            VolumetricSettings.Density = 0.3f;
            ApplyVolumetricSettings(VolumetricSettings);
            break;
    }
}

void ALight_VolumetricEffects::AnimateVolumetricIntensity(float DeltaTime)
{
    AnimationTime += DeltaTime * AnimationSpeed;
    
    float IntensityMultiplier = 1.0f;
    
    switch (VolumetricType)
    {
        case ELight_VolumetricType::Firelight:
            // Flickering fire effect with multiple sine waves
            IntensityMultiplier = 1.0f + 
                (FMath::Sin(AnimationTime * 2.0f) * 0.3f +
                 FMath::Sin(AnimationTime * 3.7f) * 0.2f +
                 FMath::Sin(AnimationTime * 5.1f) * 0.1f) * IntensityVariation;
            break;
            
        case ELight_VolumetricType::Mist:
            // Gentle, slow variation for atmospheric mist
            IntensityMultiplier = 1.0f + FMath::Sin(AnimationTime) * IntensityVariation;
            break;
            
        case ELight_VolumetricType::Smoke:
            // Turbulent smoke movement
            IntensityMultiplier = 1.0f + 
                (FMath::Sin(AnimationTime * 1.5f) * 0.4f +
                 FMath::Cos(AnimationTime * 2.3f) * 0.3f) * IntensityVariation;
            break;
            
        default:
            // Simple sine wave for other types
            IntensityMultiplier = 1.0f + FMath::Sin(AnimationTime) * IntensityVariation;
            break;
    }
    
    if (VolumetricLight)
    {
        VolumetricLight->SetIntensity(BaseIntensity * IntensityMultiplier);
    }
}