#include "Light_VolumetricEffects.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ALight_VolumetricEffects::ALight_VolumetricEffects()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    VolumeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolumeMeshComponent"));
    VolumeMeshComponent->SetupAttachment(RootComponent);
    
    // Set default volumetric settings for Cretaceous atmosphere
    VolumetricSettings.Intensity = 1.2f;
    VolumetricSettings.Density = 0.6f;
    VolumetricSettings.Tint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm atmospheric tint
    VolumetricSettings.ScatteringDistribution = 0.3f;
    VolumetricSettings.ExtinctionScale = 1.1f;
    
    BaseIntensity = VolumetricSettings.Intensity;
}

void ALight_VolumetricEffects::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVolumetricMesh();
    UpdateVolumetricParameters();
    
    // Apply Cretaceous-specific volumetric effects based on type
    switch (VolumetricType)
    {
    case ELight_VolumetricType::Sunbeams:
        CreateCretaceousSunbeams();
        break;
    case ELight_VolumetricType::Mist:
        CreatePrehistoricMist();
        break;
    case ELight_VolumetricType::Dust:
        // Prehistoric dust with higher density due to volcanic activity
        VolumetricSettings.Density = 0.8f;
        VolumetricSettings.Tint = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
        break;
    case ELight_VolumetricType::Pollen:
        // Heavy pollen from primitive plants
        VolumetricSettings.Density = 0.4f;
        VolumetricSettings.Tint = FLinearColor(1.0f, 1.0f, 0.8f, 1.0f);
        break;
    case ELight_VolumetricType::Steam:
        // Volcanic steam effects
        VolumetricSettings.Density = 1.2f;
        VolumetricSettings.Tint = FLinearColor(0.95f, 0.95f, 1.0f, 1.0f);
        break;
    }
    
    UpdateVolumetricParameters();
}

void ALight_VolumetricEffects::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAnimateIntensity)
    {
        AnimateVolumetricEffects(DeltaTime);
    }
}

void ALight_VolumetricEffects::InitializeVolumetricMesh()
{
    if (VolumeMeshComponent)
    {
        // Use a simple cube mesh for volumetric effects
        // In production, this would be replaced with proper volumetric mesh
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            VolumeMeshComponent->SetStaticMesh(CubeMesh);
            VolumeMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            VolumeMeshComponent->SetCastShadow(false);
            
            // Scale for volumetric coverage
            SetActorScale3D(FVector(10.0f, 10.0f, 5.0f));
        }
        
        // Create dynamic material instance for runtime parameter control
        if (VolumetricMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(VolumetricMaterial, this);
            if (DynamicMaterial)
            {
                VolumeMeshComponent->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void ALight_VolumetricEffects::SetVolumetricType(ELight_VolumetricType NewType)
{
    VolumetricType = NewType;
    UpdateVolumetricParameters();
}

void ALight_VolumetricEffects::UpdateVolumetricParameters()
{
    UpdateMaterialParameters();
    
    // Update Material Parameter Collection if available
    if (VolumetricMPC)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), VolumetricMPC, TEXT("VolumetricIntensity"), VolumetricSettings.Intensity);
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), VolumetricMPC, TEXT("VolumetricDensity"), VolumetricSettings.Density);
        UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), VolumetricMPC, TEXT("VolumetricTint"), VolumetricSettings.Tint);
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), VolumetricMPC, TEXT("ScatteringDistribution"), VolumetricSettings.ScatteringDistribution);
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), VolumetricMPC, TEXT("ExtinctionScale"), VolumetricSettings.ExtinctionScale);
    }
}

void ALight_VolumetricEffects::SetIntensity(float NewIntensity)
{
    VolumetricSettings.Intensity = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    BaseIntensity = VolumetricSettings.Intensity;
    UpdateVolumetricParameters();
}

void ALight_VolumetricEffects::SetDensity(float NewDensity)
{
    VolumetricSettings.Density = FMath::Clamp(NewDensity, 0.0f, 5.0f);
    UpdateVolumetricParameters();
}

void ALight_VolumetricEffects::CreateCretaceousSunbeams()
{
    // Configure for dramatic Cretaceous sunbeams through humid atmosphere
    VolumetricSettings.Intensity = 1.5f;
    VolumetricSettings.Density = 0.7f;
    VolumetricSettings.Tint = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f); // Warm golden tint
    VolumetricSettings.ScatteringDistribution = 0.4f; // More forward scattering
    VolumetricSettings.ExtinctionScale = 0.8f; // Less extinction for dramatic effect
    
    bAnimateIntensity = true;
    AnimationSpeed = 0.5f; // Slow, atmospheric animation
    IntensityVariation = 0.2f; // Subtle variation
    
    BaseIntensity = VolumetricSettings.Intensity;
}

void ALight_VolumetricEffects::CreatePrehistoricMist()
{
    // Configure for ground-level mist in prehistoric environment
    VolumetricSettings.Intensity = 0.8f;
    VolumetricSettings.Density = 1.0f;
    VolumetricSettings.Tint = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Cool mist tint
    VolumetricSettings.ScatteringDistribution = 0.1f; // Isotropic scattering
    VolumetricSettings.ExtinctionScale = 1.2f; // Higher extinction for dense mist
    
    bAnimateIntensity = true;
    AnimationSpeed = 0.3f; // Very slow movement
    IntensityVariation = 0.4f; // More variation for natural look
    
    BaseIntensity = VolumetricSettings.Intensity;
    
    // Position lower for ground mist
    FVector CurrentLocation = GetActorLocation();
    SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z - 200.0f));
    SetActorScale3D(FVector(20.0f, 20.0f, 2.0f)); // Wide and low
}

void ALight_VolumetricEffects::AnimateVolumetricEffects(float DeltaTime)
{
    AnimationTime += DeltaTime * AnimationSpeed;
    
    // Create natural breathing effect for volumetric intensity
    float SinWave = FMath::Sin(AnimationTime);
    float CosWave = FMath::Cos(AnimationTime * 0.7f); // Different frequency for complexity
    
    float IntensityModifier = 1.0f + (SinWave * 0.3f + CosWave * 0.2f) * IntensityVariation;
    VolumetricSettings.Intensity = BaseIntensity * IntensityModifier;
    
    // Add subtle density variation for organic feel
    float DensityModifier = 1.0f + (FMath::Sin(AnimationTime * 0.5f) * 0.1f);
    float CurrentDensity = VolumetricSettings.Density * DensityModifier;
    
    UpdateMaterialParameters();
    
    // Update MPC parameters if available
    if (VolumetricMPC)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), VolumetricMPC, TEXT("VolumetricIntensity"), VolumetricSettings.Intensity);
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), VolumetricMPC, TEXT("VolumetricDensity"), CurrentDensity);
    }
}

void ALight_VolumetricEffects::UpdateMaterialParameters()
{
    if (VolumeMeshComponent && VolumeMeshComponent->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(VolumeMeshComponent->GetMaterial(0));
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("Intensity"), VolumetricSettings.Intensity);
            DynamicMaterial->SetScalarParameterValue(TEXT("Density"), VolumetricSettings.Density);
            DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), VolumetricSettings.Tint);
            DynamicMaterial->SetScalarParameterValue(TEXT("ScatteringDistribution"), VolumetricSettings.ScatteringDistribution);
            DynamicMaterial->SetScalarParameterValue(TEXT("ExtinctionScale"), VolumetricSettings.ExtinctionScale);
        }
    }
}