#include "EnvArt_WaterFeatureSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AEnvArt_WaterFeatureSystem::AEnvArt_WaterFeatureSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create water surface mesh
    WaterSurfaceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterSurfaceMesh"));
    WaterSurfaceMesh->SetupAttachment(RootComponent);
    WaterSurfaceMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WaterSurfaceMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    WaterSurfaceMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // Create water bed mesh
    WaterBedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterBedMesh"));
    WaterBedMesh->SetupAttachment(RootComponent);
    WaterBedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WaterBedMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create mist particle system
    MistParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MistParticles"));
    MistParticles->SetupAttachment(RootComponent);
    MistParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    MistParticles->bAutoActivate = true;

    // Create ripple particle system
    RippleParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RippleParticles"));
    RippleParticles->SetupAttachment(WaterSurfaceMesh);
    RippleParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 5.0f));
    RippleParticles->bAutoActivate = true;

    // Create ambient water sound
    AmbientWaterSound = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientWaterSound"));
    AmbientWaterSound->SetupAttachment(RootComponent);
    AmbientWaterSound->bAutoActivate = true;
    AmbientWaterSound->SetVolumeMultiplier(0.5f);

    // Initialize default configuration
    WaterConfig = FEnvArt_WaterFeatureConfig();
}

void AEnvArt_WaterFeatureSystem::BeginPlay()
{
    Super::BeginPlay();

    // Setup initial water feature configuration
    UpdateWaterFeatureVisuals();
    SetupWaterMaterials();
    ConfigureMistParticles();
    ConfigureRippleParticles();
    ConfigureAmbientSound();

    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Water feature initialized with type %d"), 
           static_cast<int32>(WaterConfig.FeatureType));
}

void AEnvArt_WaterFeatureSystem::ConfigureWaterFeature(const FEnvArt_WaterFeatureConfig& NewConfig)
{
    WaterConfig = NewConfig;
    UpdateWaterFeatureVisuals();
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Configured water feature type %d"), 
           static_cast<int32>(WaterConfig.FeatureType));
}

void AEnvArt_WaterFeatureSystem::SetWaterLevel(float NewLevel)
{
    WaterConfig.WaterLevel = NewLevel;
    
    if (WaterSurfaceMesh)
    {
        FVector CurrentLocation = WaterSurfaceMesh->GetRelativeLocation();
        WaterSurfaceMesh->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewLevel));
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Set water level to %f"), NewLevel);
}

void AEnvArt_WaterFeatureSystem::SetFlowSpeed(float NewSpeed)
{
    WaterConfig.FlowSpeed = FMath::Clamp(NewSpeed, 0.0f, 10.0f);
    
    // Update material parameters for flow animation
    if (WaterSurfaceMesh && WaterSurfaceMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = WaterSurfaceMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("FlowSpeed"), WaterConfig.FlowSpeed);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Set flow speed to %f"), NewSpeed);
}

void AEnvArt_WaterFeatureSystem::ToggleMist(bool bEnabled)
{
    WaterConfig.bHasMist = bEnabled;
    
    if (MistParticles)
    {
        if (bEnabled)
        {
            MistParticles->Activate();
        }
        else
        {
            MistParticles->Deactivate();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Mist %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AEnvArt_WaterFeatureSystem::ToggleRipples(bool bEnabled)
{
    WaterConfig.bHasRipples = bEnabled;
    
    if (RippleParticles)
    {
        if (bEnabled)
        {
            RippleParticles->Activate();
        }
        else
        {
            RippleParticles->Deactivate();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Ripples %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AEnvArt_WaterFeatureSystem::ToggleAmbientSound(bool bEnabled)
{
    WaterConfig.bHasAmbientSound = bEnabled;
    
    if (AmbientWaterSound)
    {
        if (bEnabled)
        {
            AmbientWaterSound->Play();
        }
        else
        {
            AmbientWaterSound->Stop();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Ambient sound %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AEnvArt_WaterFeatureSystem::CreatePondConfiguration()
{
    WaterConfig.FeatureType = EEnvArt_WaterFeatureType::SmallPond;
    WaterConfig.WaterLevel = 0.0f;
    WaterConfig.FlowSpeed = 0.1f;
    WaterConfig.bHasMist = true;
    WaterConfig.bHasRipples = true;
    WaterConfig.bHasAmbientSound = true;
    
    UpdateWaterFeatureVisuals();
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Created pond configuration"));
}

void AEnvArt_WaterFeatureSystem::CreateStreamConfiguration()
{
    WaterConfig.FeatureType = EEnvArt_WaterFeatureType::Stream;
    WaterConfig.WaterLevel = -10.0f;
    WaterConfig.FlowSpeed = 2.0f;
    WaterConfig.bHasMist = false;
    WaterConfig.bHasRipples = true;
    WaterConfig.bHasAmbientSound = true;
    
    UpdateWaterFeatureVisuals();
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Created stream configuration"));
}

void AEnvArt_WaterFeatureSystem::CreateWaterfallConfiguration()
{
    WaterConfig.FeatureType = EEnvArt_WaterFeatureType::Waterfall;
    WaterConfig.WaterLevel = 100.0f;
    WaterConfig.FlowSpeed = 5.0f;
    WaterConfig.bHasMist = true;
    WaterConfig.bHasRipples = false;
    WaterConfig.bHasAmbientSound = true;
    
    UpdateWaterFeatureVisuals();
    UE_LOG(LogTemp, Log, TEXT("EnvArt_WaterFeatureSystem: Created waterfall configuration"));
}

void AEnvArt_WaterFeatureSystem::UpdateWaterFeatureVisuals()
{
    if (!WaterSurfaceMesh || !WaterBedMesh)
        return;

    // Configure based on water feature type
    switch (WaterConfig.FeatureType)
    {
        case EEnvArt_WaterFeatureType::SmallPond:
            WaterSurfaceMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 1.0f));
            WaterBedMesh->SetRelativeScale3D(FVector(2.2f, 2.2f, 0.5f));
            break;
            
        case EEnvArt_WaterFeatureType::LargePond:
            WaterSurfaceMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 1.0f));
            WaterBedMesh->SetRelativeScale3D(FVector(4.2f, 4.2f, 0.5f));
            break;
            
        case EEnvArt_WaterFeatureType::Stream:
            WaterSurfaceMesh->SetRelativeScale3D(FVector(8.0f, 1.0f, 1.0f));
            WaterBedMesh->SetRelativeScale3D(FVector(8.2f, 1.2f, 0.3f));
            break;
            
        case EEnvArt_WaterFeatureType::Waterfall:
            WaterSurfaceMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 6.0f));
            WaterBedMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.2f));
            break;
            
        case EEnvArt_WaterFeatureType::Marsh:
            WaterSurfaceMesh->SetRelativeScale3D(FVector(6.0f, 6.0f, 1.0f));
            WaterBedMesh->SetRelativeScale3D(FVector(6.2f, 6.2f, 0.8f));
            break;
            
        case EEnvArt_WaterFeatureType::RiverBend:
            WaterSurfaceMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 1.0f));
            WaterBedMesh->SetRelativeScale3D(FVector(3.2f, 3.2f, 0.4f));
            break;
    }

    // Apply water level
    SetWaterLevel(WaterConfig.WaterLevel);
    
    // Apply flow speed
    SetFlowSpeed(WaterConfig.FlowSpeed);
    
    // Configure effects
    ToggleMist(WaterConfig.bHasMist);
    ToggleRipples(WaterConfig.bHasRipples);
    ToggleAmbientSound(WaterConfig.bHasAmbientSound);
}

void AEnvArt_WaterFeatureSystem::ConfigureMistParticles()
{
    if (!MistParticles)
        return;

    // Configure mist based on water feature type
    switch (WaterConfig.FeatureType)
    {
        case EEnvArt_WaterFeatureType::SmallPond:
        case EEnvArt_WaterFeatureType::LargePond:
            MistParticles->SetFloatParameter(TEXT("SpawnRate"), 10.0f);
            MistParticles->SetVectorParameter(TEXT("InitialVelocity"), FVector(0, 0, 20));
            break;
            
        case EEnvArt_WaterFeatureType::Waterfall:
            MistParticles->SetFloatParameter(TEXT("SpawnRate"), 50.0f);
            MistParticles->SetVectorParameter(TEXT("InitialVelocity"), FVector(0, 0, 50));
            break;
            
        case EEnvArt_WaterFeatureType::Marsh:
            MistParticles->SetFloatParameter(TEXT("SpawnRate"), 30.0f);
            MistParticles->SetVectorParameter(TEXT("InitialVelocity"), FVector(0, 0, 10));
            break;
            
        default:
            MistParticles->SetFloatParameter(TEXT("SpawnRate"), 5.0f);
            MistParticles->SetVectorParameter(TEXT("InitialVelocity"), FVector(0, 0, 15));
            break;
    }
}

void AEnvArt_WaterFeatureSystem::ConfigureRippleParticles()
{
    if (!RippleParticles)
        return;

    // Configure ripples based on flow speed
    float RippleRate = WaterConfig.FlowSpeed * 5.0f;
    RippleParticles->SetFloatParameter(TEXT("SpawnRate"), RippleRate);
    RippleParticles->SetFloatParameter(TEXT("InitialSize"), 0.5f + WaterConfig.FlowSpeed * 0.2f);
}

void AEnvArt_WaterFeatureSystem::ConfigureAmbientSound()
{
    if (!AmbientWaterSound)
        return;

    // Configure sound based on water feature type and flow speed
    float VolumeMultiplier = 0.3f + (WaterConfig.FlowSpeed * 0.1f);
    float PitchMultiplier = 0.8f + (WaterConfig.FlowSpeed * 0.1f);
    
    AmbientWaterSound->SetVolumeMultiplier(VolumeMultiplier);
    AmbientWaterSound->SetPitchMultiplier(PitchMultiplier);
}

void AEnvArt_WaterFeatureSystem::SetupWaterMaterials()
{
    if (!WaterSurfaceMesh || !WaterBedMesh)
        return;

    // Create dynamic material instances for runtime parameter control
    if (WaterSurfaceMaterial)
    {
        UMaterialInstanceDynamic* DynamicWaterMaterial = WaterSurfaceMesh->CreateDynamicMaterialInstance(0, WaterSurfaceMaterial);
        if (DynamicWaterMaterial)
        {
            DynamicWaterMaterial->SetScalarParameterValue(TEXT("FlowSpeed"), WaterConfig.FlowSpeed);
            DynamicWaterMaterial->SetScalarParameterValue(TEXT("WaterDepth"), FMath::Abs(WaterConfig.WaterLevel) * 0.1f);
        }
    }

    if (WaterBedMaterial)
    {
        UMaterialInstanceDynamic* DynamicBedMaterial = WaterBedMesh->CreateDynamicMaterialInstance(0, WaterBedMaterial);
        if (DynamicBedMaterial)
        {
            // Configure bed material based on feature type
            switch (WaterConfig.FeatureType)
            {
                case EEnvArt_WaterFeatureType::Stream:
                    DynamicBedMaterial->SetScalarParameterValue(TEXT("RockDensity"), 0.8f);
                    break;
                case EEnvArt_WaterFeatureType::Marsh:
                    DynamicBedMaterial->SetScalarParameterValue(TEXT("MudAmount"), 0.9f);
                    break;
                default:
                    DynamicBedMaterial->SetScalarParameterValue(TEXT("SandAmount"), 0.7f);
                    break;
            }
        }
    }
}