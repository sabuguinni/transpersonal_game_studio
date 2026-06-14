#include "EnvArt_FallenLogAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

AEnvArt_FallenLogAsset::AEnvArt_FallenLogAsset()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default properties
    DecayState = EEnvArt_LogDecayState::Weathered;
    LogLength = 800.0f;
    LogDiameter = 120.0f;
    bHasHollowSection = false;
    bCanHidePlayer = true;
    bAdaptToTerrain = true;
    TerrainAdaptionStrength = 0.7f;
    bSpawnMossParticles = true;
    MossParticleIntensity = 0.5f;
    bCanBeClimbed = true;
    bCanProvideWood = true;
    WoodResourceAmount = 25;
    bCanSpawnInsects = true;

    // Initialize internal state
    CurrentWeatheringLevel = 0.3f;
    MossGrowthTimer = 0.0f;
    bMaterialsInitialized = false;
    OriginalScale = FVector(1.0f);

    // Set default mesh scale based on log dimensions
    if (GetStaticMeshComponent())
    {
        GetStaticMeshComponent()->SetWorldScale3D(FVector(LogLength / 100.0f, LogDiameter / 100.0f, LogDiameter / 100.0f));
    }
}

void AEnvArt_FallenLogAsset::BeginPlay()
{
    Super::BeginPlay();

    InitializeMaterials();
    
    if (bAdaptToTerrain)
    {
        AdaptToTerrainSurface();
    }

    UpdateDecayVisuals();
    
    if (bSpawnMossParticles)
    {
        SpawnMossEffects();
    }

    OriginalScale = GetActorScale3D();
}

void AEnvArt_FallenLogAsset::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update moss growth over time
    if (DecayState == EEnvArt_LogDecayState::Mossy || DecayState == EEnvArt_LogDecayState::Decomposing)
    {
        MossGrowthTimer += DeltaTime;
        
        if (MossGrowthTimer >= 5.0f) // Update every 5 seconds
        {
            MaterialData.MossIntensity = FMath::Clamp(MaterialData.MossIntensity + 0.01f, 0.0f, 1.0f);
            UpdateMaterialProperties();
            MossGrowthTimer = 0.0f;
        }
    }

    // Handle environmental effects
    if (bSpawnMossParticles && MossParticleIntensity > 0.0f)
    {
        SpawnEnvironmentalEffects();
    }
}

void AEnvArt_FallenLogAsset::SetDecayState(EEnvArt_LogDecayState NewDecayState)
{
    DecayState = NewDecayState;
    
    // Update properties based on decay state
    switch (DecayState)
    {
        case EEnvArt_LogDecayState::Fresh:
            MaterialData.WeatheringAmount = 0.1f;
            MaterialData.MossIntensity = 0.0f;
            WoodResourceAmount = 35;
            break;
            
        case EEnvArt_LogDecayState::Weathered:
            MaterialData.WeatheringAmount = 0.3f;
            MaterialData.MossIntensity = 0.2f;
            WoodResourceAmount = 25;
            break;
            
        case EEnvArt_LogDecayState::Mossy:
            MaterialData.WeatheringAmount = 0.5f;
            MaterialData.MossIntensity = 0.6f;
            WoodResourceAmount = 15;
            bHasHollowSection = true;
            break;
            
        case EEnvArt_LogDecayState::Decomposing:
            MaterialData.WeatheringAmount = 0.8f;
            MaterialData.MossIntensity = 0.9f;
            WoodResourceAmount = 5;
            bHasHollowSection = true;
            bCanSpawnInsects = true;
            break;
            
        case EEnvArt_LogDecayState::Fossilized:
            MaterialData.WeatheringAmount = 0.2f;
            MaterialData.MossIntensity = 0.1f;
            WoodResourceAmount = 0;
            bCanProvideWood = false;
            break;
    }
    
    UpdateDecayVisuals();
    UpdateMaterialProperties();
}

void AEnvArt_FallenLogAsset::UpdateMaterialProperties()
{
    if (!DynamicMaterial || !bMaterialsInitialized)
    {
        InitializeMaterials();
        return;
    }

    // Update material parameters
    DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), MaterialData.WeatheringAmount);
    DynamicMaterial->SetScalarParameterValue(TEXT("MossIntensity"), MaterialData.MossIntensity);
    
    if (MaterialData.BarkTexture)
    {
        DynamicMaterial->SetTextureParameterValue(TEXT("BarkTexture"), MaterialData.BarkTexture);
    }
    
    if (MaterialData.MossTexture)
    {
        DynamicMaterial->SetTextureParameterValue(TEXT("MossTexture"), MaterialData.MossTexture);
    }

    // Update weathering level for internal tracking
    CurrentWeatheringLevel = MaterialData.WeatheringAmount;
}

void AEnvArt_FallenLogAsset::AdaptToTerrainSurface()
{
    if (!GetWorld())
    {
        return;
    }

    FVector StartLocation = GetActorLocation() + FVector(0, 0, 100);
    FVector EndLocation = GetActorLocation() - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Adjust position to terrain surface
        FVector NewLocation = HitResult.Location;
        NewLocation.Z += LogDiameter * 0.3f; // Partially embed in terrain
        SetActorLocation(NewLocation);
        
        // Adjust rotation to match terrain normal
        if (TerrainAdaptionStrength > 0.0f)
        {
            FVector TerrainNormal = HitResult.Normal;
            FVector ForwardVector = GetActorForwardVector();
            FVector RightVector = FVector::CrossProduct(ForwardVector, TerrainNormal).GetSafeNormal();
            FVector AdjustedForward = FVector::CrossProduct(TerrainNormal, RightVector).GetSafeNormal();
            
            FRotator NewRotation = FRotationMatrix::MakeFromXZ(AdjustedForward, TerrainNormal).Rotator();
            FRotator CurrentRotation = GetActorRotation();
            FRotator BlendedRotation = FMath::Lerp(CurrentRotation, NewRotation, TerrainAdaptionStrength);
            
            SetActorRotation(BlendedRotation);
        }
    }
}

void AEnvArt_FallenLogAsset::SpawnMossEffects()
{
    if (DecayState == EEnvArt_LogDecayState::Fresh || DecayState == EEnvArt_LogDecayState::Fossilized)
    {
        return;
    }

    // Create moss particle effects (placeholder implementation)
    if (GetWorld() && MossParticleIntensity > 0.0f)
    {
        FVector EffectLocation = GetActorLocation() + FVector(0, 0, LogDiameter * 0.5f);
        
        // Log moss effect spawn for debugging
        UE_LOG(LogTemp, Log, TEXT("Spawning moss effects at %s with intensity %f"), 
               *EffectLocation.ToString(), MossParticleIntensity);
    }
}

void AEnvArt_FallenLogAsset::SetBiomeSpecificProperties(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            MaterialData.MossIntensity = FMath::Clamp(MaterialData.MossIntensity + 0.3f, 0.0f, 1.0f);
            bSpawnMossParticles = true;
            MossParticleIntensity = 0.8f;
            break;
            
        case EBiomeType::Swamp:
            DecayState = EEnvArt_LogDecayState::Decomposing;
            MaterialData.MossIntensity = 0.9f;
            MaterialData.WeatheringAmount = 0.8f;
            bHasHollowSection = true;
            break;
            
        case EBiomeType::Plains:
            MaterialData.WeatheringAmount = FMath::Clamp(MaterialData.WeatheringAmount + 0.2f, 0.0f, 1.0f);
            MaterialData.MossIntensity = FMath::Max(0.0f, MaterialData.MossIntensity - 0.3f);
            bSpawnMossParticles = false;
            break;
            
        case EBiomeType::Canyon:
            DecayState = EEnvArt_LogDecayState::Fossilized;
            MaterialData.WeatheringAmount = 0.1f;
            MaterialData.MossIntensity = 0.0f;
            bCanProvideWood = false;
            break;
            
        case EBiomeType::Volcanic:
            MaterialData.WeatheringAmount = 0.9f;
            MaterialData.MossIntensity = 0.0f;
            WoodResourceAmount = 0;
            bCanProvideWood = false;
            break;
            
        case EBiomeType::Tundra:
            DecayState = EEnvArt_LogDecayState::Fossilized;
            MaterialData.MossIntensity = 0.1f;
            bSpawnMossParticles = false;
            break;
    }
    
    UpdateMaterialProperties();
    UpdateDecayVisuals();
}

bool AEnvArt_FallenLogAsset::CanPlayerHide() const
{
    return bCanHidePlayer && bHasHollowSection && LogDiameter >= 100.0f;
}

int32 AEnvArt_FallenLogAsset::HarvestWood()
{
    if (!bCanProvideWood || WoodResourceAmount <= 0)
    {
        return 0;
    }

    int32 HarvestedAmount = FMath::Min(5, WoodResourceAmount);
    WoodResourceAmount -= HarvestedAmount;
    
    // Update decay state if wood is depleted
    if (WoodResourceAmount <= 0)
    {
        bCanProvideWood = false;
        if (DecayState != EEnvArt_LogDecayState::Fossilized)
        {
            SetDecayState(EEnvArt_LogDecayState::Decomposing);
        }
    }
    
    return HarvestedAmount;
}

float AEnvArt_FallenLogAsset::GetLogVolume() const
{
    float Radius = LogDiameter * 0.5f;
    return PI * Radius * Radius * LogLength;
}

bool AEnvArt_FallenLogAsset::IsFullyDecomposed() const
{
    return DecayState == EEnvArt_LogDecayState::Decomposing && WoodResourceAmount <= 0;
}

void AEnvArt_FallenLogAsset::InitializeMaterials()
{
    if (!GetStaticMeshComponent() || !MaterialData.BaseMaterial)
    {
        return;
    }

    // Create dynamic material instance
    DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialData.BaseMaterial, this);
    
    if (DynamicMaterial)
    {
        GetStaticMeshComponent()->SetMaterial(0, DynamicMaterial);
        bMaterialsInitialized = true;
        
        // Set initial material parameters
        UpdateMaterialProperties();
    }
}

void AEnvArt_FallenLogAsset::UpdateDecayVisuals()
{
    if (!GetStaticMeshComponent())
    {
        return;
    }

    // Adjust scale based on decay state
    FVector ScaleModifier = OriginalScale;
    
    switch (DecayState)
    {
        case EEnvArt_LogDecayState::Fresh:
            ScaleModifier *= 1.0f;
            break;
            
        case EEnvArt_LogDecayState::Weathered:
            ScaleModifier *= 0.95f;
            break;
            
        case EEnvArt_LogDecayState::Mossy:
            ScaleModifier *= 0.9f;
            ScaleModifier.Z *= 0.8f; // Flatten slightly
            break;
            
        case EEnvArt_LogDecayState::Decomposing:
            ScaleModifier *= 0.8f;
            ScaleModifier.Z *= 0.6f; // More flattened
            break;
            
        case EEnvArt_LogDecayState::Fossilized:
            ScaleModifier *= 0.9f;
            break;
    }
    
    GetStaticMeshComponent()->SetWorldScale3D(ScaleModifier);
}

void AEnvArt_FallenLogAsset::HandleTerrainAdaption()
{
    if (bAdaptToTerrain)
    {
        AdaptToTerrainSurface();
    }
}

void AEnvArt_FallenLogAsset::SpawnEnvironmentalEffects()
{
    // Placeholder for particle system spawning
    // This would integrate with UE5's Niagara particle system
    if (GetWorld() && bSpawnMossParticles)
    {
        // Log environmental effect activity
        static float LastLogTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastLogTime > 10.0f) // Log every 10 seconds
        {
            UE_LOG(LogTemp, Log, TEXT("Environmental effects active on log %s"), *GetName());
            LastLogTime = CurrentTime;
        }
    }
}