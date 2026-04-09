#include "JurassicMaterialSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UJurassicMaterialSystem::UJurassicMaterialSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    MaterialNoiseScale = 0.005f;
    MaterialBlendSoftness = 0.3f;
    
    // Initialize terrain material properties
    InitializeTerrainMaterials();
    
    // Initialize storytelling decals
    InitializeStorytellingDecals();
}

void UJurassicMaterialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: BeginPlay - Initializing material system"));
    
    // Load environment parameter collection if set
    if (EnvironmentParameters)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Environment parameters collection loaded"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: No environment parameters collection set"));
    }
}

void UJurassicMaterialSystem::InitializeTerrainMaterials()
{
    // Rich Soil Material
    FTerrainMaterialProperties RichSoil;
    RichSoil.TilingScale = 2.0f;
    RichSoil.BlendWeight = 1.0f;
    RichSoil.PreferredMoisture = 0.7f;
    RichSoil.PreferredSlope = 0.2f;
    RichSoil.PreferredExposure = 0.3f;
    TerrainMaterials.Add(ETerrainMaterialType::RichSoil, RichSoil);
    
    // Muddy Ground Material
    FTerrainMaterialProperties MuddyGround;
    MuddyGround.TilingScale = 1.5f;
    MuddyGround.BlendWeight = 1.0f;
    MuddyGround.PreferredMoisture = 0.9f;
    MuddyGround.PreferredSlope = 0.1f;
    MuddyGround.PreferredExposure = 0.2f;
    TerrainMaterials.Add(ETerrainMaterialType::MuddyGround, MuddyGround);
    
    // Rocky Outcrop Material
    FTerrainMaterialProperties RockyOutcrop;
    RockyOutcrop.TilingScale = 3.0f;
    RockyOutcrop.BlendWeight = 1.0f;
    RockyOutcrop.PreferredMoisture = 0.2f;
    RockyOutcrop.PreferredSlope = 0.8f;
    RockyOutcrop.PreferredExposure = 0.9f;
    TerrainMaterials.Add(ETerrainMaterialType::RockyOutcrop, RockyOutcrop);
    
    // Sandy Riverbed Material
    FTerrainMaterialProperties SandyRiverbed;
    SandyRiverbed.TilingScale = 1.0f;
    SandyRiverbed.BlendWeight = 1.0f;
    SandyRiverbed.PreferredMoisture = 0.6f;
    SandyRiverbed.PreferredSlope = 0.0f;
    SandyRiverbed.PreferredExposure = 0.5f;
    TerrainMaterials.Add(ETerrainMaterialType::SandyRiverbed, SandyRiverbed);
    
    // Fallen Leaves Material
    FTerrainMaterialProperties FallenLeaves;
    FallenLeaves.TilingScale = 1.2f;
    FallenLeaves.BlendWeight = 1.0f;
    FallenLeaves.PreferredMoisture = 0.5f;
    FallenLeaves.PreferredSlope = 0.3f;
    FallenLeaves.PreferredExposure = 0.4f;
    TerrainMaterials.Add(ETerrainMaterialType::FallenLeaves, FallenLeaves);
    
    // Dinosaur Tracks Material
    FTerrainMaterialProperties DinosaurTracks;
    DinosaurTracks.TilingScale = 4.0f;
    DinosaurTracks.BlendWeight = 1.0f;
    DinosaurTracks.PreferredMoisture = 0.4f;
    DinosaurTracks.PreferredSlope = 0.1f;
    DinosaurTracks.PreferredExposure = 0.6f;
    TerrainMaterials.Add(ETerrainMaterialType::DinosaurTracks, DinosaurTracks);
    
    // Burnt Earth Material
    FTerrainMaterialProperties BurntEarth;
    BurntEarth.TilingScale = 2.5f;
    BurntEarth.BlendWeight = 1.0f;
    BurntEarth.PreferredMoisture = 0.1f;
    BurntEarth.PreferredSlope = 0.4f;
    BurntEarth.PreferredExposure = 0.8f;
    TerrainMaterials.Add(ETerrainMaterialType::BurntEarth, BurntEarth);
    
    // Crystal Formations Material
    FTerrainMaterialProperties CrystalFormations;
    CrystalFormations.TilingScale = 5.0f;
    CrystalFormations.BlendWeight = 1.0f;
    CrystalFormations.PreferredMoisture = 0.3f;
    CrystalFormations.PreferredSlope = 0.6f;
    CrystalFormations.PreferredExposure = 0.7f;
    TerrainMaterials.Add(ETerrainMaterialType::CrystalFormations, CrystalFormations);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Initialized %d terrain material types"), TerrainMaterials.Num());
}

void UJurassicMaterialSystem::InitializeStorytellingDecals()
{
    // Dinosaur Footprint Decal
    FStorytellingDecal DinosaurFootprint;
    DinosaurFootprint.DecalSize = FVector(200.0f, 200.0f, 50.0f);
    DinosaurFootprint.SpawnProbability = 0.15f;
    DinosaurFootprint.MinSpacing = 300.0f;
    DinosaurFootprint.StoryContext = TEXT("Large predator passed through this area recently");
    StorytellingDecals.Add(DinosaurFootprint);
    
    // Blood Stain Decal
    FStorytellingDecal BloodStain;
    BloodStain.DecalSize = FVector(150.0f, 150.0f, 30.0f);
    BloodStain.SpawnProbability = 0.08f;
    BloodStain.MinSpacing = 500.0f;
    BloodStain.StoryContext = TEXT("Evidence of a struggle or hunt");
    StorytellingDecals.Add(BloodStain);
    
    // Claw Marks Decal
    FStorytellingDecal ClawMarks;
    ClawMarks.DecalSize = FVector(100.0f, 300.0f, 20.0f);
    ClawMarks.SpawnProbability = 0.12f;
    ClawMarks.MinSpacing = 400.0f;
    ClawMarks.StoryContext = TEXT("Deep scratches from a territorial marking");
    StorytellingDecals.Add(ClawMarks);
    
    // Burnt Ground Decal
    FStorytellingDecal BurntGround;
    BurntGround.DecalSize = FVector(400.0f, 400.0f, 40.0f);
    BurntGround.SpawnProbability = 0.05f;
    BurntGround.MinSpacing = 1000.0f;
    BurntGround.StoryContext = TEXT("Remains of an ancient fire");
    StorytellingDecals.Add(BurntGround);
    
    // Mysterious Crystal Pattern Decal
    FStorytellingDecal CrystalPattern;
    CrystalPattern.DecalSize = FVector(250.0f, 250.0f, 35.0f);
    CrystalPattern.SpawnProbability = 0.03f;
    CrystalPattern.MinSpacing = 800.0f;
    CrystalPattern.StoryContext = TEXT("Strange crystalline formations hint at the time gem's power");
    StorytellingDecals.Add(CrystalPattern);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Initialized %d storytelling decal types"), StorytellingDecals.Num());
}

void UJurassicMaterialSystem::ApplyTerrainMaterials(ALandscape* TargetLandscape)
{
    if (!TargetLandscape)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicMaterialSystem: No target landscape provided"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Applying terrain materials to landscape: %s"), *TargetLandscape->GetName());
    
    // Get landscape bounds
    FBox LandscapeBounds = TargetLandscape->GetComponentsBoundingBox();
    FVector LandscapeCenter = LandscapeBounds.GetCenter();
    FVector LandscapeExtent = LandscapeBounds.GetExtent();
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Landscape bounds - Center: %s, Extent: %s"), 
           *LandscapeCenter.ToString(), *LandscapeExtent.ToString());
    
    // This is a simplified version - in a full implementation, this would:
    // 1. Sample landscape heightmap and normal data
    // 2. Calculate environmental factors (moisture, slope, exposure)
    // 3. Use landscape painting API to blend materials procedurally
    // 4. Apply noise patterns for natural variation
    
    // For now, we'll apply a basic material setup
    ApplyBaseMaterialBlending(TargetLandscape, LandscapeCenter, LandscapeExtent);
}

void UJurassicMaterialSystem::ApplyBaseMaterialBlending(ALandscape* Landscape, FVector Center, FVector Extent)
{
    // This is a placeholder for landscape material painting
    // In a full implementation, this would use the Landscape API
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Applying base material blending"));
    
    // Example of how material blending would work:
    // 1. Sample heightmap to get elevation data
    // 2. Calculate slope from normal vectors
    // 3. Use noise functions to determine moisture levels
    // 4. Blend materials based on environmental factors
    
    // For demonstration, we'll just log the process
    for (const auto& MaterialPair : TerrainMaterials)
    {
        ETerrainMaterialType MaterialType = MaterialPair.Key;
        const FTerrainMaterialProperties& Properties = MaterialPair.Value;
        
        UE_LOG(LogTemp, Log, TEXT("JurassicMaterialSystem: Material %d - Tiling: %f, Moisture: %f, Slope: %f"), 
               (int32)MaterialType, Properties.TilingScale, Properties.PreferredMoisture, Properties.PreferredSlope);
    }
}

void UJurassicMaterialSystem::PaintMaterialInArea(FVector Center, float Radius, ETerrainMaterialType MaterialType, float Intensity)
{
    if (!TerrainMaterials.Contains(MaterialType))
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicMaterialSystem: Material type %d not found"), (int32)MaterialType);
        return;
    }
    
    const FTerrainMaterialProperties& MaterialProperties = TerrainMaterials[MaterialType];
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Painting material %d in area at %s with radius %f, intensity %f"), 
           (int32)MaterialType, *Center.ToString(), Radius, Intensity);
    
    // This is a placeholder for landscape painting
    // In a full implementation, this would:
    // 1. Find the landscape component(s) that overlap the area
    // 2. Convert world coordinates to landscape texture coordinates
    // 3. Use the landscape painting API to modify weightmaps
    // 4. Apply the material with the specified intensity
    
    // For now, we'll simulate the painting process
    float Area = PI * Radius * Radius;
    int32 AffectedTexels = FMath::RoundToInt(Area / (MaterialProperties.TilingScale * MaterialProperties.TilingScale));
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMaterialSystem: Would affect approximately %d texels"), AffectedTexels);
}

void UJurassicMaterialSystem::PlaceStorytellingDecals(FVector Center, float Radius, int32 MaxDecals)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicMaterialSystem: No valid world found"));
        return;
    }
    
    if (StorytellingDecals.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: No storytelling decals configured"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Placing storytelling decals in area at %s with radius %f"), 
           *Center.ToString(), Radius);
    
    int32 DecalsPlaced = 0;
    int32 Attempts = 0;
    const int32 MaxAttempts = MaxDecals * 3; // Prevent infinite loops
    
    while (DecalsPlaced < MaxDecals && Attempts < MaxAttempts)
    {
        Attempts++;
        
        // Select random decal type
        int32 DecalIndex = FMath::RandRange(0, StorytellingDecals.Num() - 1);
        const FStorytellingDecal& DecalData = StorytellingDecals[DecalIndex];
        
        // Check spawn probability
        if (FMath::FRand() > DecalData.SpawnProbability)
            continue;
        
        // Generate random position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius) * FMath::Sqrt(FMath::FRand());
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector SpawnLocation = Center + Offset;
        
        // Check if location is suitable
        if (IsLocationSuitableForDecal(SpawnLocation, DecalData))
        {
            PlaceStorytellingDecal(DecalData, SpawnLocation);
            DecalsPlaced++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Placed %d storytelling decals after %d attempts"), 
           DecalsPlaced, Attempts);
}

void UJurassicMaterialSystem::PlaceStorytellingDecal(const FStorytellingDecal& DecalData, FVector Location)
{
    if (!GetWorld() || !DecalData.DecalMaterial)
    {
        return;
    }
    
    // Trace to find ground
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        FVector GroundLocation = HitResult.Location;
        FVector Normal = HitResult.Normal;
        
        // Create decal actor
        ADecalActor* DecalActor = GetWorld()->SpawnActor<ADecalActor>();
        if (DecalActor)
        {
            // Set decal properties
            DecalActor->SetDecalMaterial(DecalData.DecalMaterial);
            DecalActor->GetDecal()->DecalSize = DecalData.DecalSize;
            
            // Position and orient the decal
            DecalActor->SetActorLocation(GroundLocation + Normal * 10.0f); // Slight offset from surface
            
            // Orient decal to surface normal
            FRotator DecalRotation = FRotationMatrix::MakeFromZ(-Normal).Rotator();
            DecalActor->SetActorRotation(DecalRotation);
            
            // Add random rotation around the normal for variation
            float RandomYaw = FMath::RandRange(0.0f, 360.0f);
            DecalActor->AddActorLocalRotation(FRotator(0, RandomYaw, 0));
            
            UE_LOG(LogTemp, Log, TEXT("JurassicMaterialSystem: Placed storytelling decal at %s - %s"), 
                   *GroundLocation.ToString(), *DecalData.StoryContext);
        }
    }
}

bool UJurassicMaterialSystem::IsLocationSuitableForDecal(FVector Location, const FStorytellingDecal& DecalData)
{
    if (!GetWorld())
        return false;
    
    // Check minimum spacing from other decals
    TArray<AActor*> FoundDecals;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADecalActor::StaticClass(), FoundDecals);
    
    for (AActor* DecalActor : FoundDecals)
    {
        float Distance = FVector::Dist(DecalActor->GetActorLocation(), Location);
        if (Distance < DecalData.MinSpacing)
        {
            return false; // Too close to existing decal
        }
    }
    
    // Check if location has valid ground
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (!GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return false; // No ground found
    }
    
    // Check slope (don't place on steep slopes)
    FVector Normal = HitResult.Normal;
    float SlopeDot = FVector::DotProduct(Normal, FVector::UpVector);
    if (SlopeDot < 0.7f) // Too steep
    {
        return false;
    }
    
    return true;
}

void UJurassicMaterialSystem::UpdateEnvironmentalParameters(float TimeOfDay, float WeatherIntensity, float Moisture)
{
    if (!EnvironmentParameters)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: No environment parameters collection set"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMaterialSystem: Updating environmental parameters - Time: %f, Weather: %f, Moisture: %f"), 
           TimeOfDay, WeatherIntensity, Moisture);
    
    // Get the parameter collection instance
    UMaterialParameterCollectionInstance* ParameterInstance = GetWorld()->GetParameterCollectionInstance(EnvironmentParameters);
    if (ParameterInstance)
    {
        // Update time of day (0-1, where 0.5 is noon)
        ParameterInstance->SetScalarParameterValue(TEXT("TimeOfDay"), TimeOfDay);
        
        // Update weather intensity (0-1, where 1 is storm)
        ParameterInstance->SetScalarParameterValue(TEXT("WeatherIntensity"), WeatherIntensity);
        
        // Update moisture level (0-1, where 1 is very wet)
        ParameterInstance->SetScalarParameterValue(TEXT("MoistureLevel"), Moisture);
        
        // Calculate derived parameters
        float SunIntensity = FMath::Clamp(1.0f - FMath::Abs(TimeOfDay - 0.5f) * 2.0f, 0.1f, 1.0f);
        ParameterInstance->SetScalarParameterValue(TEXT("SunIntensity"), SunIntensity);
        
        float ShadowIntensity = FMath::Lerp(0.3f, 0.8f, 1.0f - SunIntensity);
        ParameterInstance->SetScalarParameterValue(TEXT("ShadowIntensity"), ShadowIntensity);
        
        // Wind effect based on weather
        float WindStrength = FMath::Lerp(0.2f, 1.0f, WeatherIntensity);
        ParameterInstance->SetScalarParameterValue(TEXT("WindStrength"), WindStrength);
        
        UE_LOG(LogTemp, Log, TEXT("JurassicMaterialSystem: Environmental parameters updated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicMaterialSystem: Failed to get parameter collection instance"));
    }
}

FTerrainMaterialProperties UJurassicMaterialSystem::GetMaterialProperties(ETerrainMaterialType MaterialType)
{
    if (TerrainMaterials.Contains(MaterialType))
    {
        return TerrainMaterials[MaterialType];
    }
    
    // Return default properties if material type not found
    FTerrainMaterialProperties DefaultProperties;
    UE_LOG(LogTemp, Warning, TEXT("JurassicMaterialSystem: Material type %d not found, returning defaults"), (int32)MaterialType);
    return DefaultProperties;
}

float UJurassicMaterialSystem::CalculateMoistureLevel(FVector Location)
{
    // Simplified moisture calculation based on location
    // In a full implementation, this would consider:
    // - Distance to water sources
    // - Elevation
    // - Vegetation density
    // - Weather patterns
    
    float Noise = FMath::PerlinNoise2D(FVector2D(Location.X * 0.001f, Location.Y * 0.001f));
    return FMath::Clamp((Noise + 1.0f) * 0.5f, 0.0f, 1.0f);
}

float UJurassicMaterialSystem::CalculateSlopeLevel(FVector Location)
{
    // Simplified slope calculation
    // In a full implementation, this would sample the landscape heightmap
    // and calculate the actual slope at the given location
    
    float NoiseX = FMath::PerlinNoise2D(FVector2D(Location.X * 0.002f, Location.Y * 0.002f));
    float NoiseY = FMath::PerlinNoise2D(FVector2D(Location.X * 0.002f + 100.0f, Location.Y * 0.002f + 100.0f));
    
    float Gradient = FMath::Sqrt(NoiseX * NoiseX + NoiseY * NoiseY);
    return FMath::Clamp(Gradient, 0.0f, 1.0f);
}

float UJurassicMaterialSystem::CalculateExposureLevel(FVector Location)
{
    // Simplified exposure calculation
    // In a full implementation, this would consider:
    // - Elevation relative to surroundings
    // - Vegetation cover
    // - Proximity to large structures or cliffs
    
    float Noise = FMath::PerlinNoise2D(FVector2D(Location.X * 0.0005f, Location.Y * 0.0005f));
    return FMath::Clamp((Noise + 1.0f) * 0.5f, 0.0f, 1.0f);
}