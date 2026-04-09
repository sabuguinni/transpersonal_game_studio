#include "EnvironmentArtManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UEnvironmentArtManager::UEnvironmentArtManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    VegetationDensity = 0.7f;
    RockDensity = 0.3f;
    PropDensity = 0.2f;
    MaterialBlendRadius = 1000.0f;
    NoiseScale = 0.001f;
    DetailLevel = EEnvironmentDetailLevel::High;
    
    // Initialize biome settings
    InitializeBiomeSettings();
}

void UEnvironmentArtManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: BeginPlay - Initializing environment art system"));
    
    // Find landscape in world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundLandscapes;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundLandscapes);
        
        if (FoundLandscapes.Num() > 0)
        {
            TargetLandscape = Cast<ALandscape>(FoundLandscapes[0]);
            UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Found landscape: %s"), *TargetLandscape->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("EnvironmentArtManager: No landscape found in world"));
        }
    }
}

void UEnvironmentArtManager::InitializeBiomeSettings()
{
    // Dense Forest Biome
    FBiomeArtSettings DenseForest;
    DenseForest.BiomeName = TEXT("Dense Forest");
    DenseForest.VegetationDensity = 0.9f;
    DenseForest.RockDensity = 0.2f;
    DenseForest.PropDensity = 0.3f;
    DenseForest.CanopyDensity = 0.8f;
    DenseForest.UnderbrushDensity = 0.7f;
    DenseForest.DominantMaterialType = ETerrainMaterialType::RichSoil;
    DenseForest.SecondaryMaterialType = ETerrainMaterialType::FallenLeaves;
    BiomeSettings.Add(EBiomeType::DenseForest, DenseForest);
    
    // River Valley Biome
    FBiomeArtSettings RiverValley;
    RiverValley.BiomeName = TEXT("River Valley");
    RiverValley.VegetationDensity = 0.6f;
    RiverValley.RockDensity = 0.4f;
    RiverValley.PropDensity = 0.5f;
    RiverValley.CanopyDensity = 0.4f;
    RiverValley.UnderbrushDensity = 0.8f;
    RiverValley.DominantMaterialType = ETerrainMaterialType::SandyRiverbed;
    RiverValley.SecondaryMaterialType = ETerrainMaterialType::MuddyGround;
    BiomeSettings.Add(EBiomeType::RiverValley, RiverValley);
    
    // Rocky Highlands Biome
    FBiomeArtSettings RockyHighlands;
    RockyHighlands.BiomeName = TEXT("Rocky Highlands");
    RockyHighlands.VegetationDensity = 0.3f;
    RockyHighlands.RockDensity = 0.8f;
    RockyHighlands.PropDensity = 0.1f;
    RockyHighlands.CanopyDensity = 0.2f;
    RockyHighlands.UnderbrushDensity = 0.3f;
    RockyHighlands.DominantMaterialType = ETerrainMaterialType::RockyOutcrop;
    RockyHighlands.SecondaryMaterialType = ETerrainMaterialType::BurntEarth;
    BiomeSettings.Add(EBiomeType::RockyHighlands, RockyHighlands);
    
    // Crystal Caves Biome
    FBiomeArtSettings CrystalCaves;
    CrystalCaves.BiomeName = TEXT("Crystal Caves");
    CrystalCaves.VegetationDensity = 0.1f;
    CrystalCaves.RockDensity = 0.9f;
    CrystalCaves.PropDensity = 0.6f;
    CrystalCaves.CanopyDensity = 0.0f;
    CrystalCaves.UnderbrushDensity = 0.1f;
    CrystalCaves.DominantMaterialType = ETerrainMaterialType::CrystalFormations;
    CrystalCaves.SecondaryMaterialType = ETerrainMaterialType::RockyOutcrop;
    BiomeSettings.Add(EBiomeType::CrystalCaves, CrystalCaves);
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Initialized %d biome settings"), BiomeSettings.Num());
}

void UEnvironmentArtManager::PopulateEnvironmentInArea(FVector Center, float Radius, EBiomeType BiomeType)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("EnvironmentArtManager: No valid world found"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Populating area at %s with radius %f, biome: %d"), 
           *Center.ToString(), Radius, (int32)BiomeType);
    
    // Get biome settings
    if (!BiomeSettings.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Error, TEXT("EnvironmentArtManager: Biome type %d not found in settings"), (int32)BiomeType);
        return;
    }
    
    const FBiomeArtSettings& CurrentBiome = BiomeSettings[BiomeType];
    
    // Calculate number of elements to place based on density and area
    float Area = PI * Radius * Radius;
    int32 VegetationCount = FMath::RoundToInt(Area * CurrentBiome.VegetationDensity * 0.0001f); // Scale factor
    int32 RockCount = FMath::RoundToInt(Area * CurrentBiome.RockDensity * 0.00005f);
    int32 PropCount = FMath::RoundToInt(Area * CurrentBiome.PropDensity * 0.00003f);
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Placing %d vegetation, %d rocks, %d props"), 
           VegetationCount, RockCount, PropCount);
    
    // Place vegetation
    PlaceVegetationCluster(Center, Radius, VegetationCount, CurrentBiome);
    
    // Place rocks
    PlaceRockFormations(Center, Radius, RockCount, CurrentBiome);
    
    // Place environmental props
    PlaceEnvironmentalProps(Center, Radius, PropCount, CurrentBiome);
    
    // Apply terrain materials
    ApplyBiomeMaterials(Center, Radius, CurrentBiome);
}

void UEnvironmentArtManager::PlaceVegetationCluster(FVector Center, float Radius, int32 Count, const FBiomeArtSettings& BiomeSettings)
{
    if (!GetWorld() || VegetationAssets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: No vegetation assets configured"));
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        // Generate random position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius) * FMath::Sqrt(FMath::FRand()); // Uniform distribution
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector SpawnLocation = Center + Offset;
        
        // Trace to find ground
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = false;
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            FVector GroundLocation = HitResult.Location;
            
            // Check slope (don't place on steep slopes)
            FVector Normal = HitResult.Normal;
            float SlopeDot = FVector::DotProduct(Normal, FVector::UpVector);
            if (SlopeDot < 0.7f) // Too steep
                continue;
            
            // Select random vegetation asset
            int32 AssetIndex = FMath::RandRange(0, VegetationAssets.Num() - 1);
            UStaticMesh* VegetationMesh = VegetationAssets[AssetIndex];
            
            if (VegetationMesh)
            {
                // Spawn vegetation actor
                AStaticMeshActor* VegetationActor = GetWorld()->SpawnActor<AStaticMeshActor>();
                if (VegetationActor)
                {
                    VegetationActor->GetStaticMeshComponent()->SetStaticMesh(VegetationMesh);
                    
                    // Random rotation (only Y-axis for trees/plants)
                    FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                    
                    // Random scale variation
                    float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
                    FVector Scale = FVector(ScaleVariation);
                    
                    VegetationActor->SetActorLocation(GroundLocation);
                    VegetationActor->SetActorRotation(RandomRotation);
                    VegetationActor->SetActorScale3D(Scale);
                    
                    // Enable Nanite if supported
                    VegetationActor->GetStaticMeshComponent()->bEvaluateWorldPositionOffset = true;
                    
                    // Add to spawned actors list
                    SpawnedActors.Add(VegetationActor);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Placed %d vegetation instances"), Count);
}

void UEnvironmentArtManager::PlaceRockFormations(FVector Center, float Radius, int32 Count, const FBiomeArtSettings& BiomeSettings)
{
    if (!GetWorld() || RockAssets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: No rock assets configured"));
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        // Generate random position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(Radius * 0.3f, Radius); // Rocks more towards edges
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector SpawnLocation = Center + Offset;
        
        // Trace to find ground
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = false;
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            FVector GroundLocation = HitResult.Location;
            
            // Select random rock asset
            int32 AssetIndex = FMath::RandRange(0, RockAssets.Num() - 1);
            UStaticMesh* RockMesh = RockAssets[AssetIndex];
            
            if (RockMesh)
            {
                // Spawn rock actor
                AStaticMeshActor* RockActor = GetWorld()->SpawnActor<AStaticMeshActor>();
                if (RockActor)
                {
                    RockActor->GetStaticMeshComponent()->SetStaticMesh(RockMesh);
                    
                    // Random rotation
                    FRotator RandomRotation = FRotator(
                        FMath::RandRange(-15.0f, 15.0f), // Slight pitch variation
                        FMath::RandRange(0.0f, 360.0f),  // Full yaw rotation
                        FMath::RandRange(-10.0f, 10.0f)  // Slight roll variation
                    );
                    
                    // Random scale variation (rocks can vary more)
                    float ScaleVariation = FMath::RandRange(0.5f, 2.0f);
                    FVector Scale = FVector(ScaleVariation);
                    
                    RockActor->SetActorLocation(GroundLocation);
                    RockActor->SetActorRotation(RandomRotation);
                    RockActor->SetActorScale3D(Scale);
                    
                    // Enable collision for rocks
                    RockActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    
                    // Add to spawned actors list
                    SpawnedActors.Add(RockActor);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Placed %d rock formations"), Count);
}

void UEnvironmentArtManager::PlaceEnvironmentalProps(FVector Center, float Radius, int32 Count, const FBiomeArtSettings& BiomeSettings)
{
    if (!GetWorld() || EnvironmentalProps.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: No environmental props configured"));
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        // Generate random position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius) * FMath::Sqrt(FMath::FRand());
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector SpawnLocation = Center + Offset;
        
        // Trace to find ground
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = false;
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            FVector GroundLocation = HitResult.Location;
            
            // Select random prop
            int32 PropIndex = FMath::RandRange(0, EnvironmentalProps.Num() - 1);
            const FEnvironmentalProp& PropData = EnvironmentalProps[PropIndex];
            
            // Check spawn probability
            if (FMath::FRand() > PropData.SpawnProbability)
                continue;
            
            if (PropData.PropMesh)
            {
                // Spawn prop actor
                AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>();
                if (PropActor)
                {
                    PropActor->GetStaticMeshComponent()->SetStaticMesh(PropData.PropMesh);
                    
                    // Apply prop-specific rotation
                    FRotator Rotation = PropData.RandomRotation ? 
                        FRotator(0, FMath::RandRange(0.0f, 360.0f), 0) : 
                        FRotator::ZeroRotator;
                    
                    // Apply scale variation
                    float ScaleVariation = FMath::RandRange(PropData.MinScale, PropData.MaxScale);
                    FVector Scale = FVector(ScaleVariation);
                    
                    PropActor->SetActorLocation(GroundLocation);
                    PropActor->SetActorRotation(Rotation);
                    PropActor->SetActorScale3D(Scale);
                    
                    // Set collision based on prop settings
                    PropActor->GetStaticMeshComponent()->SetCollisionEnabled(
                        PropData.HasCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
                    );
                    
                    // Add to spawned actors list
                    SpawnedActors.Add(PropActor);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Placed %d environmental props"), Count);
}

void UEnvironmentArtManager::ApplyBiomeMaterials(FVector Center, float Radius, const FBiomeArtSettings& BiomeSettings)
{
    if (!TargetLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: No target landscape for material application"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Applying biome materials to landscape"));
    
    // This is a placeholder for landscape material painting
    // In a full implementation, this would use the Landscape API to paint materials
    // based on the biome settings and procedural rules
    
    // For now, we'll just log the intent
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Would apply %s materials in area"), *BiomeSettings.BiomeName);
}

void UEnvironmentArtManager::ClearEnvironmentInArea(FVector Center, float Radius)
{
    if (!GetWorld())
        return;
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Clearing environment in area at %s with radius %f"), 
           *Center.ToString(), Radius);
    
    // Remove spawned actors within radius
    for (int32 i = SpawnedActors.Num() - 1; i >= 0; i--)
    {
        if (SpawnedActors[i] && IsValid(SpawnedActors[i]))
        {
            float Distance = FVector::Dist(SpawnedActors[i]->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                SpawnedActors[i]->Destroy();
                SpawnedActors.RemoveAt(i);
            }
        }
        else
        {
            // Clean up invalid references
            SpawnedActors.RemoveAt(i);
        }
    }
}

void UEnvironmentArtManager::SetDetailLevel(EEnvironmentDetailLevel NewDetailLevel)
{
    DetailLevel = NewDetailLevel;
    
    // Adjust density multipliers based on detail level
    switch (DetailLevel)
    {
        case EEnvironmentDetailLevel::Low:
            VegetationDensity = 0.3f;
            RockDensity = 0.2f;
            PropDensity = 0.1f;
            break;
        case EEnvironmentDetailLevel::Medium:
            VegetationDensity = 0.5f;
            RockDensity = 0.3f;
            PropDensity = 0.15f;
            break;
        case EEnvironmentDetailLevel::High:
            VegetationDensity = 0.7f;
            RockDensity = 0.4f;
            PropDensity = 0.2f;
            break;
        case EEnvironmentDetailLevel::Ultra:
            VegetationDensity = 1.0f;
            RockDensity = 0.5f;
            PropDensity = 0.3f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentArtManager: Detail level set to %d"), (int32)DetailLevel);
}

FBiomeArtSettings UEnvironmentArtManager::GetBiomeSettings(EBiomeType BiomeType) const
{
    if (BiomeSettings.Contains(BiomeType))
    {
        return BiomeSettings[BiomeType];
    }
    
    // Return default settings if biome not found
    FBiomeArtSettings DefaultSettings;
    DefaultSettings.BiomeName = TEXT("Default");
    DefaultSettings.VegetationDensity = 0.5f;
    DefaultSettings.RockDensity = 0.3f;
    DefaultSettings.PropDensity = 0.2f;
    return DefaultSettings;
}

void UEnvironmentArtManager::UpdateEnvironmentLOD(FVector ViewerLocation)
{
    if (!GetWorld())
        return;
    
    // Update LOD for spawned actors based on distance to viewer
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor && IsValid(Actor))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), ViewerLocation);
            
            // Simple LOD system based on distance
            if (Distance > 5000.0f)
            {
                Actor->SetActorHiddenInGame(true);
            }
            else if (Distance > 2000.0f)
            {
                Actor->SetActorHiddenInGame(false);
                // Could reduce detail here
            }
            else
            {
                Actor->SetActorHiddenInGame(false);
                // Full detail
            }
        }
    }
}