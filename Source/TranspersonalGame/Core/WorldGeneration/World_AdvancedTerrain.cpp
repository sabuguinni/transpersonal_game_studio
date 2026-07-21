#include "World_AdvancedTerrain.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Landscape/Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

UWorld_AdvancedTerrain::UWorld_AdvancedTerrain()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize default values
    TerrainResolution = 1024;
    WorldSize = 100000.0f;
    NoiseFrequency = 0.01f;
    NoiseAmplitude = 500.0f;
    NoiseOctaves = 4;
    
    GeneratedLandscape = nullptr;
    
    // Initialize default biomes
    InitializeDefaultBiomes();
    InitializeTerrainLayers();
}

void UWorld_AdvancedTerrain::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: BeginPlay - Initializing advanced terrain system"));
    
    // Auto-generate terrain if none exists
    if (!GeneratedLandscape && BiomeDefinitions.Num() > 0)
    {
        GenerateAdvancedTerrain();
    }
}

void UWorld_AdvancedTerrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update terrain streaming or LOD if needed
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer > 5.0f) // Update every 5 seconds
    {
        UpdateTimer = 0.0f;
        
        // Check if landscape needs updates
        if (GeneratedLandscape && !GeneratedLandscape->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Landscape reference became invalid, regenerating"));
            GeneratedLandscape = nullptr;
        }
    }
}

void UWorld_AdvancedTerrain::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana biome
    FWorld_BiomeDefinition Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector2D(0.0f, 0.0f);
    Savana.Radius = 20000.0f;
    Savana.BaseHeight = 100.0f;
    Savana.HeightVariation = 200.0f;
    Savana.BiomeColor = FLinearColor(0.8f, 0.6f, 0.3f, 1.0f);
    Savana.Temperature = 28.0f;
    Savana.Humidity = 0.3f;
    Savana.VegetationTypes.Add(TEXT("Acacia"));
    Savana.VegetationTypes.Add(TEXT("Grass"));
    BiomeDefinitions.Add(Savana);
    
    // Forest biome
    FWorld_BiomeDefinition Forest;
    Forest.BiomeName = TEXT("Forest");
    Forest.CenterLocation = FVector2D(-45000.0f, 40000.0f);
    Forest.Radius = 25000.0f;
    Forest.BaseHeight = 200.0f;
    Forest.HeightVariation = 300.0f;
    Forest.BiomeColor = FLinearColor(0.2f, 0.6f, 0.2f, 1.0f);
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.8f;
    Forest.VegetationTypes.Add(TEXT("Oak"));
    Forest.VegetationTypes.Add(TEXT("Pine"));
    Forest.VegetationTypes.Add(TEXT("Fern"));
    BiomeDefinitions.Add(Forest);
    
    // Desert biome
    FWorld_BiomeDefinition Desert;
    Desert.BiomeName = TEXT("Desert");
    Desert.CenterLocation = FVector2D(50000.0f, -40000.0f);
    Desert.Radius = 18000.0f;
    Desert.BaseHeight = 50.0f;
    Desert.HeightVariation = 150.0f;
    Desert.BiomeColor = FLinearColor(0.9f, 0.8f, 0.4f, 1.0f);
    Desert.Temperature = 35.0f;
    Desert.Humidity = 0.1f;
    Desert.VegetationTypes.Add(TEXT("Cactus"));
    Desert.VegetationTypes.Add(TEXT("SagebrushGrass"));
    BiomeDefinitions.Add(Desert);
    
    // Mountain biome
    FWorld_BiomeDefinition Mountain;
    Mountain.BiomeName = TEXT("Mountain");
    Mountain.CenterLocation = FVector2D(-20000.0f, -50000.0f);
    Mountain.Radius = 15000.0f;
    Mountain.BaseHeight = 800.0f;
    Mountain.HeightVariation = 600.0f;
    Mountain.BiomeColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    Mountain.Temperature = 10.0f;
    Mountain.Humidity = 0.6f;
    Mountain.VegetationTypes.Add(TEXT("Pine"));
    Mountain.VegetationTypes.Add(TEXT("Rock"));
    BiomeDefinitions.Add(Mountain);
    
    // Swamp biome
    FWorld_BiomeDefinition Swamp;
    Swamp.BiomeName = TEXT("Swamp");
    Swamp.CenterLocation = FVector2D(30000.0f, 60000.0f);
    Swamp.Radius = 12000.0f;
    Swamp.BaseHeight = 80.0f;
    Swamp.HeightVariation = 100.0f;
    Swamp.BiomeColor = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);
    Swamp.Temperature = 25.0f;
    Swamp.Humidity = 0.9f;
    Swamp.VegetationTypes.Add(TEXT("Willow"));
    Swamp.VegetationTypes.Add(TEXT("Moss"));
    Swamp.VegetationTypes.Add(TEXT("Reeds"));
    BiomeDefinitions.Add(Swamp);
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Initialized %d default biomes"), BiomeDefinitions.Num());
}

void UWorld_AdvancedTerrain::InitializeTerrainLayers()
{
    TerrainLayers.Empty();
    
    // Base layer
    FWorld_TerrainLayer BaseLayer;
    BaseLayer.LayerName = TEXT("Base");
    BaseLayer.BlendWeight = 1.0f;
    BaseLayer.NoiseScale = 1.0f;
    BaseLayer.HeightThreshold = 0.0f;
    TerrainLayers.Add(BaseLayer);
    
    // Rock layer
    FWorld_TerrainLayer RockLayer;
    RockLayer.LayerName = TEXT("Rock");
    RockLayer.BlendWeight = 0.8f;
    RockLayer.NoiseScale = 0.5f;
    RockLayer.HeightThreshold = 400.0f;
    TerrainLayers.Add(RockLayer);
    
    // Grass layer
    FWorld_TerrainLayer GrassLayer;
    GrassLayer.LayerName = TEXT("Grass");
    GrassLayer.BlendWeight = 0.6f;
    GrassLayer.NoiseScale = 2.0f;
    GrassLayer.HeightThreshold = 100.0f;
    TerrainLayers.Add(GrassLayer);
}

void UWorld_AdvancedTerrain::GenerateAdvancedTerrain()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World_AdvancedTerrain: No valid world for terrain generation"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Starting advanced terrain generation"));
    
    // Clear existing terrain
    ClearGeneratedTerrain();
    
    // Create biome markers first
    CreateBiomeMarkers();
    
    // Generate the main terrain mesh
    CreateTerrainMesh();
    
    // Apply terrain layers and materials
    ApplyTerrainLayers();
    
    // Generate environmental features
    PlaceEnvironmentalFeatures();
    
    // Generate water bodies
    GenerateRivers();
    
    // Setup collision
    SetupTerrainCollision();
    
    // Log generation stats
    LogTerrainGenerationStats();
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Advanced terrain generation completed"));
}

void UWorld_AdvancedTerrain::CreateBiomeMarkers()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Clear existing markers
    for (AStaticMeshActor* Marker : BiomeMarkers)
    {
        if (Marker && IsValid(Marker))
        {
            Marker->Destroy();
        }
    }
    BiomeMarkers.Empty();
    
    // Load cube mesh for markers
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if (!CubeMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("World_AdvancedTerrain: Failed to load cube mesh for biome markers"));
        return;
    }
    
    // Create marker for each biome
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        FVector SpawnLocation(Biome.CenterLocation.X, Biome.CenterLocation.Y, Biome.BaseHeight);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        AStaticMeshActor* BiomeMarker = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation);
        if (BiomeMarker)
        {
            // Set the mesh
            UStaticMeshComponent* MeshComp = BiomeMarker->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                
                // Scale the marker based on biome radius
                float ScaleFactor = Biome.Radius / 10000.0f;
                FVector MarkerScale(ScaleFactor, ScaleFactor, Biome.HeightVariation / 100.0f);
                BiomeMarker->SetActorScale3D(MarkerScale);
                
                // Set label
                FString MarkerName = FString::Printf(TEXT("BiomeMarker_%s"), *Biome.BiomeName);
                BiomeMarker->SetActorLabel(MarkerName);
                
                BiomeMarkers.Add(BiomeMarker);
                
                UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Created biome marker for %s at %s"), 
                       *Biome.BiomeName, *SpawnLocation.ToString());
            }
        }
    }
}

FWorld_BiomeDefinition UWorld_AdvancedTerrain::GetBiomeAtLocation(FVector WorldLocation)
{
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    float MaxInfluence = 0.0f;
    FWorld_BiomeDefinition DominantBiome;
    
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        float Influence = CalculateBiomeInfluence(Location2D, Biome);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Biome;
        }
    }
    
    return DominantBiome;
}

float UWorld_AdvancedTerrain::GetHeightAtLocation(FVector2D WorldLocation)
{
    float BaseHeight = 0.0f;
    float TotalWeight = 0.0f;
    
    // Blend heights from all biomes based on influence
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        float Influence = CalculateBiomeInfluence(WorldLocation, Biome);
        if (Influence > 0.0f)
        {
            float NoiseValue = CalculatePerlinNoise(WorldLocation.X, WorldLocation.Y, NoiseOctaves, NoiseFrequency, 1.0f);
            float BiomeHeight = Biome.BaseHeight + (NoiseValue * Biome.HeightVariation);
            
            BaseHeight += BiomeHeight * Influence;
            TotalWeight += Influence;
        }
    }
    
    if (TotalWeight > 0.0f)
    {
        BaseHeight /= TotalWeight;
    }
    
    return BaseHeight;
}

void UWorld_AdvancedTerrain::ApplyTerrainLayers()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Applying terrain layers (placeholder implementation)"));
    
    // This would typically involve creating landscape layer info objects
    // and applying them to the landscape, but for now we'll log the intent
    for (const FWorld_TerrainLayer& Layer : TerrainLayers)
    {
        UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Would apply layer %s with weight %f"), 
               *Layer.LayerName, Layer.BlendWeight);
    }
}

void UWorld_AdvancedTerrain::GenerateRivers()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Generating rivers and water bodies"));
    
    // Create simple water planes for rivers
    UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
    if (!PlaneMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("World_AdvancedTerrain: Failed to load plane mesh for water bodies"));
        return;
    }
    
    // Create a river between Forest and Swamp biomes
    FVector RiverStart(-45000.0f, 40000.0f, 150.0f); // Forest center
    FVector RiverEnd(30000.0f, 60000.0f, 70.0f);     // Swamp center
    FVector RiverMid = (RiverStart + RiverEnd) * 0.5f;
    RiverMid.Z = 100.0f;
    
    // Create river segments
    TArray<FVector> RiverPoints = {RiverStart, RiverMid, RiverEnd};
    
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        FVector SegmentStart = RiverPoints[i];
        FVector SegmentEnd = RiverPoints[i + 1];
        FVector SegmentCenter = (SegmentStart + SegmentEnd) * 0.5f;
        
        AStaticMeshActor* WaterSegment = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SegmentCenter, FRotator::ZeroRotator);
        if (WaterSegment)
        {
            UStaticMeshComponent* MeshComp = WaterSegment->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(PlaneMesh);
                
                float SegmentLength = FVector::Dist(SegmentStart, SegmentEnd);
                FVector SegmentScale(SegmentLength / 100.0f, 500.0f / 100.0f, 1.0f); // 500cm wide river
                WaterSegment->SetActorScale3D(SegmentScale);
                
                // Rotate to align with river direction
                FVector Direction = (SegmentEnd - SegmentStart).GetSafeNormal();
                FRotator WaterRotation = Direction.Rotation();
                WaterSegment->SetActorRotation(WaterRotation);
                
                FString WaterName = FString::Printf(TEXT("RiverSegment_%d"), i);
                WaterSegment->SetActorLabel(WaterName);
                
                UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Created river segment %d at %s"), 
                       i, *SegmentCenter.ToString());
            }
        }
    }
    
    // Create a lake in the Swamp biome
    FVector LakeLocation(30000.0f, 60000.0f, 75.0f);
    AStaticMeshActor* Lake = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), LakeLocation, FRotator::ZeroRotator);
    if (Lake)
    {
        UStaticMeshComponent* MeshComp = Lake->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(PlaneMesh);
            FVector LakeScale(50.0f, 50.0f, 1.0f); // Large circular lake
            Lake->SetActorScale3D(LakeScale);
            Lake->SetActorLabel(TEXT("SwampLake"));
            
            UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Created swamp lake at %s"), *LakeLocation.ToString());
        }
    }
}

void UWorld_AdvancedTerrain::PlaceEnvironmentalFeatures()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Placing environmental features"));
    
    // This would place rocks, vegetation, and other environmental details
    // For now, we'll create some basic rock formations
    if (!GetWorld())
    {
        return;
    }
    
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if (!CubeMesh)
    {
        return;
    }
    
    // Place rock formations in Mountain biome
    FVector2D MountainCenter(-20000.0f, -50000.0f);
    for (int32 i = 0; i < 10; i++)
    {
        float Angle = (i / 10.0f) * 2.0f * PI;
        float Distance = FMath::RandRange(5000.0f, 12000.0f);
        
        FVector RockLocation;
        RockLocation.X = MountainCenter.X + FMath::Cos(Angle) * Distance;
        RockLocation.Y = MountainCenter.Y + FMath::Sin(Angle) * Distance;
        RockLocation.Z = GetHeightAtLocation(FVector2D(RockLocation.X, RockLocation.Y)) + 100.0f;
        
        AStaticMeshActor* Rock = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), RockLocation, FRotator::ZeroRotator);
        if (Rock)
        {
            UStaticMeshComponent* MeshComp = Rock->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                
                float RockScale = FMath::RandRange(2.0f, 8.0f);
                Rock->SetActorScale3D(FVector(RockScale, RockScale, RockScale * 1.5f));
                
                FString RockName = FString::Printf(TEXT("MountainRock_%d"), i);
                Rock->SetActorLabel(RockName);
            }
        }
    }
}

void UWorld_AdvancedTerrain::RegenerateAllTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Regenerating all terrain"));
    GenerateAdvancedTerrain();
}

void UWorld_AdvancedTerrain::ClearGeneratedTerrain()
{
    // Clear biome markers
    for (AStaticMeshActor* Marker : BiomeMarkers)
    {
        if (Marker && IsValid(Marker))
        {
            Marker->Destroy();
        }
    }
    BiomeMarkers.Empty();
    
    // Clear landscape reference
    if (GeneratedLandscape && IsValid(GeneratedLandscape))
    {
        GeneratedLandscape->Destroy();
        GeneratedLandscape = nullptr;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Cleared existing terrain"));
}

float UWorld_AdvancedTerrain::CalculatePerlinNoise(float X, float Y, int32 Octaves, float Frequency, float Amplitude)
{
    float Result = 0.0f;
    float CurrentAmplitude = Amplitude;
    float CurrentFrequency = Frequency;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Result += FMath::PerlinNoise2D(FVector2D(X * CurrentFrequency, Y * CurrentFrequency)) * CurrentAmplitude;
        CurrentAmplitude *= 0.5f;
        CurrentFrequency *= 2.0f;
    }
    
    return Result;
}

float UWorld_AdvancedTerrain::CalculateBiomeInfluence(FVector2D Location, const FWorld_BiomeDefinition& Biome)
{
    float Distance = FVector2D::Distance(Location, Biome.CenterLocation);
    if (Distance > Biome.Radius)
    {
        return 0.0f;
    }
    
    // Linear falloff
    float Influence = 1.0f - (Distance / Biome.Radius);
    return FMath::Clamp(Influence, 0.0f, 1.0f);
}

FLinearColor UWorld_AdvancedTerrain::BlendBiomeColors(FVector2D Location)
{
    FLinearColor BlendedColor = FLinearColor::Black;
    float TotalWeight = 0.0f;
    
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        float Influence = CalculateBiomeInfluence(Location, Biome);
        if (Influence > 0.0f)
        {
            BlendedColor += Biome.BiomeColor * Influence;
            TotalWeight += Influence;
        }
    }
    
    if (TotalWeight > 0.0f)
    {
        BlendedColor /= TotalWeight;
    }
    
    return BlendedColor;
}

void UWorld_AdvancedTerrain::CreateTerrainMesh()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrain: Creating terrain mesh (placeholder - would create actual landscape)"));
    
    // In a full implementation, this would create a proper UE5 Landscape
    // For now, we'll log that this step would happen
    // The actual landscape creation requires more complex setup with heightmaps
}

void UWorld_AdvancedTerrain::ApplyBiomeMaterials()
{
    UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Applying biome materials"));
    
    // This would apply different materials to different parts of the landscape
    // based on biome influence and terrain layers
}

void UWorld_AdvancedTerrain::GenerateVegetationPlacements()
{
    UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Generating vegetation placements"));
    
    // This would place vegetation based on biome types and environmental conditions
}

void UWorld_AdvancedTerrain::CreateWaterBodies()
{
    UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Creating water bodies"));
    
    // This is handled by GenerateRivers() for now
}

void UWorld_AdvancedTerrain::SetupTerrainCollision()
{
    UE_LOG(LogTemp, Log, TEXT("World_AdvancedTerrain: Setting up terrain collision"));
    
    // This would configure collision for the landscape and environmental features
}

bool UWorld_AdvancedTerrain::IsValidTerrainLocation(FVector Location)
{
    // Check if location is within world bounds
    float MaxDistance = WorldSize * 0.5f;
    return FVector2D(Location.X, Location.Y).Size() <= MaxDistance;
}

void UWorld_AdvancedTerrain::LogTerrainGenerationStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== World_AdvancedTerrain Generation Stats ==="));
    UE_LOG(LogTemp, Warning, TEXT("Biomes: %d"), BiomeDefinitions.Num());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Layers: %d"), TerrainLayers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Biome Markers: %d"), BiomeMarkers.Num());
    UE_LOG(LogTemp, Warning, TEXT("World Size: %.0f units"), WorldSize);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Resolution: %d"), TerrainResolution);
    UE_LOG(LogTemp, Warning, TEXT("=== End Stats ==="));
}