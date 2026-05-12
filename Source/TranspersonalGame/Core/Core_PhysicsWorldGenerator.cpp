#include "Core_PhysicsWorldGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeInfo.h"
#include "EditorLevelLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCore_PhysicsWorldGenerator::UCore_PhysicsWorldGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default world settings
    WorldSize = FVector(10000.0f, 10000.0f, 2000.0f);
    ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);
    MaxChunksX = 10;
    MaxChunksY = 10;
    BiomeTransitionDistance = 500.0f;
    VegetationDensityMultiplier = 1.0f;

    // Initialize runtime state
    bIsGenerating = false;
    GenerationProgress = 0.0f;

    // Initialize physics materials to null - will be created in BeginPlay
    GrassPhysicsMaterial = nullptr;
    StonePhysicsMaterial = nullptr;
    MudPhysicsMaterial = nullptr;
    SandPhysicsMaterial = nullptr;
    WaterPhysicsMaterial = nullptr;
}

void UCore_PhysicsWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldGenerator: Initializing physics world generator"));
    
    // Setup physics materials
    SetupPhysicsMaterials();
    
    // Initialize terrain properties
    DefaultTerrainProperties = FCore_PhysicsTerrainProperties();
}

void UCore_PhysicsWorldGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsGenerating)
    {
        UpdateTerrainPhysics(DeltaTime);
    }
}

void UCore_PhysicsWorldGenerator::GenerateTerrainChunk(const FVector& ChunkCenter, ECore_TerrainType TerrainType, ECore_BiomeType BiomeType)
{
    UE_LOG(LogTemp, Log, TEXT("Generating terrain chunk at %s"), *ChunkCenter.ToString());
    
    // Create new terrain chunk data
    FCore_TerrainChunk NewChunk;
    NewChunk.ChunkLocation = ChunkCenter;
    NewChunk.ChunkSize = ChunkSize;
    NewChunk.TerrainType = TerrainType;
    NewChunk.BiomeType = BiomeType;
    
    // Set terrain-specific properties
    switch (TerrainType)
    {
        case ECore_TerrainType::Plains:
            NewChunk.ElevationVariance = 50.0f;
            NewChunk.VegetationDensity = 0.7f;
            break;
        case ECore_TerrainType::Hills:
            NewChunk.ElevationVariance = 200.0f;
            NewChunk.VegetationDensity = 0.5f;
            break;
        case ECore_TerrainType::Mountains:
            NewChunk.ElevationVariance = 800.0f;
            NewChunk.VegetationDensity = 0.3f;
            break;
        case ECore_TerrainType::Valleys:
            NewChunk.ElevationVariance = 100.0f;
            NewChunk.VegetationDensity = 0.8f;
            NewChunk.bHasWaterFeatures = true;
            break;
        case ECore_TerrainType::Canyons:
            NewChunk.ElevationVariance = 600.0f;
            NewChunk.VegetationDensity = 0.2f;
            break;
        case ECore_TerrainType::Plateaus:
            NewChunk.ElevationVariance = 300.0f;
            NewChunk.VegetationDensity = 0.4f;
            break;
    }
    
    // Generate heightmap for this chunk
    GenerateHeightmap(ChunkCenter, ChunkSize, TerrainType);
    
    // Apply biome-specific vegetation
    ApplyBiomeVegetation(ChunkCenter, BiomeType, NewChunk.VegetationDensity);
    
    // Add water features if needed
    if (NewChunk.bHasWaterFeatures)
    {
        CreateWaterBody(ChunkCenter, FVector(ChunkSize.X * 0.3f, ChunkSize.Y * 0.3f, 50.0f), 100.0f);
    }
    
    // Store the generated chunk
    GeneratedChunks.Add(NewChunk);
    
    UE_LOG(LogTemp, Log, TEXT("Terrain chunk generated successfully"));
}

void UCore_PhysicsWorldGenerator::GenerateFullTerrain(int32 ChunkCountX, int32 ChunkCountY)
{
    UE_LOG(LogTemp, Log, TEXT("Generating full terrain: %dx%d chunks"), ChunkCountX, ChunkCountY);
    
    bIsGenerating = true;
    GenerationProgress = 0.0f;
    
    // Clear existing terrain
    ClearGeneratedTerrain();
    
    float TotalChunks = ChunkCountX * ChunkCountY;
    int32 CurrentChunk = 0;
    
    for (int32 X = 0; X < ChunkCountX; X++)
    {
        for (int32 Y = 0; Y < ChunkCountY; Y++)
        {
            // Calculate chunk center
            FVector ChunkCenter = FVector(
                (X - ChunkCountX / 2) * ChunkSize.X,
                (Y - ChunkCountY / 2) * ChunkSize.Y,
                0.0f
            );
            
            // Determine terrain and biome types based on position
            ECore_TerrainType TerrainType = ECore_TerrainType::Plains;
            ECore_BiomeType BiomeType = ECore_BiomeType::Temperate;
            
            // Simple biome distribution logic
            float DistanceFromCenter = FVector::Dist2D(ChunkCenter, FVector::ZeroVector);
            
            if (DistanceFromCenter < 2000.0f)
            {
                TerrainType = ECore_TerrainType::Plains;
                BiomeType = ECore_BiomeType::Tropical;
            }
            else if (DistanceFromCenter < 4000.0f)
            {
                TerrainType = ECore_TerrainType::Hills;
                BiomeType = ECore_BiomeType::Temperate;
            }
            else
            {
                TerrainType = ECore_TerrainType::Mountains;
                BiomeType = ECore_BiomeType::Savanna;
            }
            
            // Generate the chunk
            GenerateTerrainChunk(ChunkCenter, TerrainType, BiomeType);
            
            // Update progress
            CurrentChunk++;
            GenerationProgress = CurrentChunk / TotalChunks;
        }
    }
    
    bIsGenerating = false;
    GenerationProgress = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Full terrain generation complete"));
}

void UCore_PhysicsWorldGenerator::ApplyPhysicsPropertiesToTerrain(const FCore_PhysicsTerrainProperties& Properties)
{
    UE_LOG(LogTemp, Log, TEXT("Applying physics properties to terrain"));
    
    DefaultTerrainProperties = Properties;
    
    // Apply properties to all existing terrain actors
    for (AActor* TerrainActor : TerrainActors)
    {
        if (TerrainActor)
        {
            SetupTerrainCollision(TerrainActor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics properties applied to %d terrain actors"), TerrainActors.Num());
}

void UCore_PhysicsWorldGenerator::GenerateTropicalForest(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("Generating tropical forest at %s with radius %f"), *Location.ToString(), Radius);
    
    // Generate dense vegetation
    int32 TreeCount = FMath::RandRange(50, 100);
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = Location + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        if (IsLocationSuitableForVegetation(TreeLocation, ECore_BiomeType::Tropical))
        {
            // Create tree placeholder - in full implementation would spawn actual tree meshes
            UE_LOG(LogTemp, VeryVerbose, TEXT("Placing tropical tree at %s"), *TreeLocation.ToString());
        }
    }
    
    // Generate undergrowth
    int32 BushCount = FMath::RandRange(100, 200);
    for (int32 i = 0; i < BushCount; i++)
    {
        FVector BushLocation = Location + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        if (IsLocationSuitableForVegetation(BushLocation, ECore_BiomeType::Tropical))
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Placing undergrowth at %s"), *BushLocation.ToString());
        }
    }
}

void UCore_PhysicsWorldGenerator::GenerateSavanna(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("Generating savanna at %s with radius %f"), *Location.ToString(), Radius);
    
    // Generate sparse trees
    int32 TreeCount = FMath::RandRange(10, 25);
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = Location + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        if (IsLocationSuitableForVegetation(TreeLocation, ECore_BiomeType::Savanna))
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Placing savanna tree at %s"), *TreeLocation.ToString());
        }
    }
    
    // Generate grass patches
    int32 GrassCount = FMath::RandRange(30, 60);
    for (int32 i = 0; i < GrassCount; i++)
    {
        FVector GrassLocation = Location + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Placing grass patch at %s"), *GrassLocation.ToString());
    }
}

void UCore_PhysicsWorldGenerator::GenerateWetlands(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("Generating wetlands at %s with radius %f"), *Location.ToString(), Radius);
    
    // Create water body
    CreateWaterBody(Location, FVector(Radius * 0.8f, Radius * 0.8f, 20.0f), 50.0f);
    
    // Generate wetland vegetation
    int32 ReedCount = FMath::RandRange(40, 80);
    for (int32 i = 0; i < ReedCount; i++)
    {
        FVector ReedLocation = Location + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Placing wetland reeds at %s"), *ReedLocation.ToString());
    }
}

void UCore_PhysicsWorldGenerator::GenerateRiver(const TArray<FVector>& RiverPath, float RiverWidth)
{
    UE_LOG(LogTemp, Log, TEXT("Generating river with %d path points, width %f"), RiverPath.Num(), RiverWidth);
    
    for (int32 i = 0; i < RiverPath.Num() - 1; i++)
    {
        FVector StartPoint = RiverPath[i];
        FVector EndPoint = RiverPath[i + 1];
        FVector MidPoint = (StartPoint + EndPoint) * 0.5f;
        
        // Create water segment
        CreateWaterBody(MidPoint, FVector(RiverWidth, FVector::Dist(StartPoint, EndPoint), 30.0f), 100.0f);
    }
}

void UCore_PhysicsWorldGenerator::GenerateLake(const FVector& Center, float Radius, float Depth)
{
    UE_LOG(LogTemp, Log, TEXT("Generating lake at %s with radius %f, depth %f"), *Center.ToString(), Radius, Depth);
    
    CreateWaterBody(Center, FVector(Radius * 2.0f, Radius * 2.0f, 50.0f), Depth);
    
    // Add shoreline vegetation
    int32 ShorelineVegCount = FMath::RandRange(20, 40);
    for (int32 i = 0; i < ShorelineVegCount; i++)
    {
        float Angle = (i / float(ShorelineVegCount)) * 2.0f * PI;
        FVector VegLocation = Center + FVector(
            FMath::Cos(Angle) * (Radius + FMath::RandRange(50.0f, 150.0f)),
            FMath::Sin(Angle) * (Radius + FMath::RandRange(50.0f, 150.0f)),
            0.0f
        );
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Placing shoreline vegetation at %s"), *VegLocation.ToString());
    }
}

void UCore_PhysicsWorldGenerator::GenerateWaterfall(const FVector& TopLocation, const FVector& BottomLocation)
{
    UE_LOG(LogTemp, Log, TEXT("Generating waterfall from %s to %s"), *TopLocation.ToString(), *BottomLocation.ToString());
    
    // Create water pool at bottom
    CreateWaterBody(BottomLocation, FVector(200.0f, 200.0f, 50.0f), 150.0f);
    
    // In full implementation, would create particle effects and sound
}

void UCore_PhysicsWorldGenerator::PlaceResourceNodes(ECore_BiomeType BiomeType, int32 NodeCount)
{
    UE_LOG(LogTemp, Log, TEXT("Placing %d resource nodes for biome type"), NodeCount);
    
    for (int32 i = 0; i < NodeCount; i++)
    {
        FVector NodeLocation = FVector(
            FMath::RandRange(-WorldSize.X * 0.5f, WorldSize.X * 0.5f),
            FMath::RandRange(-WorldSize.Y * 0.5f, WorldSize.Y * 0.5f),
            GetElevationAtLocation(FVector::ZeroVector)
        );
        
        // Add to resource nodes for appropriate chunk
        for (FCore_TerrainChunk& Chunk : GeneratedChunks)
        {
            if (FVector::Dist(Chunk.ChunkLocation, NodeLocation) < ChunkSize.X * 0.5f)
            {
                Chunk.ResourceNodes.Add(NodeLocation);
                break;
            }
        }
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Placed resource node at %s"), *NodeLocation.ToString());
    }
}

void UCore_PhysicsWorldGenerator::GenerateStoneOutcrops(const FVector& Location, int32 Count)
{
    UE_LOG(LogTemp, Log, TEXT("Generating %d stone outcrops at %s"), Count, *Location.ToString());
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector OutcropLocation = Location + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Placing stone outcrop at %s"), *OutcropLocation.ToString());
    }
}

void UCore_PhysicsWorldGenerator::GenerateWoodResources(const FVector& Location, int32 TreeCount)
{
    UE_LOG(LogTemp, Log, TEXT("Generating %d wood resource trees at %s"), TreeCount, *Location.ToString());
    
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = Location + FVector(
            FMath::RandRange(-300.0f, 300.0f),
            FMath::RandRange(-300.0f, 300.0f),
            0.0f
        );
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Placing resource tree at %s"), *TreeLocation.ToString());
    }
}

void UCore_PhysicsWorldGenerator::SetupTerrainCollision(AActor* TerrainActor)
{
    if (!TerrainActor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = TerrainActor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Apply appropriate physics material based on terrain type
        if (GrassPhysicsMaterial)
        {
            MeshComp->SetPhysMaterialOverride(GrassPhysicsMaterial);
        }
        
        // Enable collision
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        
        UE_LOG(LogTemp, Log, TEXT("Setup collision for terrain actor: %s"), *TerrainActor->GetName());
    }
}

void UCore_PhysicsWorldGenerator::ApplyWeatherEffects(float WindStrength, const FVector& WindDirection)
{
    DefaultTerrainProperties.WindStrength = WindStrength;
    DefaultTerrainProperties.WindDirection = WindDirection.GetSafeNormal();
    
    UE_LOG(LogTemp, Log, TEXT("Applied weather effects: Wind strength %f, direction %s"), 
           WindStrength, *WindDirection.ToString());
}

void UCore_PhysicsWorldGenerator::UpdateTerrainPhysics(float DeltaTime)
{
    // Update physics simulation for terrain elements
    // This would handle dynamic terrain changes, erosion, etc.
    
    if (GenerationProgress < 1.0f)
    {
        // Update generation progress
        GenerationProgress = FMath::Clamp(GenerationProgress + DeltaTime * 0.1f, 0.0f, 1.0f);
    }
}

void UCore_PhysicsWorldGenerator::TestTerrainGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing terrain generation..."));
    
    // Generate a small test area
    GenerateTerrainChunk(FVector::ZeroVector, ECore_TerrainType::Plains, ECore_BiomeType::Tropical);
    GenerateTropicalForest(FVector(500.0f, 0.0f, 0.0f), 200.0f);
    GenerateLake(FVector(-500.0f, 0.0f, 0.0f), 150.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Test terrain generation complete"));
}

void UCore_PhysicsWorldGenerator::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating physics setup..."));
    
    int32 ValidActors = 0;
    for (AActor* Actor : TerrainActors)
    {
        if (Actor && Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            ValidActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics validation: %d/%d terrain actors valid"), ValidActors, TerrainActors.Num());
}

void UCore_PhysicsWorldGenerator::ClearGeneratedTerrain()
{
    UE_LOG(LogTemp, Log, TEXT("Clearing generated terrain..."));
    
    // Clear terrain actors
    for (AActor* Actor : TerrainActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    TerrainActors.Empty();
    
    // Clear vegetation actors
    for (AActor* Actor : VegetationActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    VegetationActors.Empty();
    
    // Clear water actors
    for (AActor* Actor : WaterActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    WaterActors.Empty();
    
    // Clear chunk data
    GeneratedChunks.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Terrain cleared"));
}

// === PRIVATE METHODS ===

void UCore_PhysicsWorldGenerator::GenerateHeightmap(const FVector& ChunkCenter, const FVector& ChunkSize, ECore_TerrainType TerrainType)
{
    // Generate procedural heightmap based on terrain type
    // In full implementation would use noise functions and create actual landscape
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Generating heightmap for chunk at %s"), *ChunkCenter.ToString());
}

void UCore_PhysicsWorldGenerator::ApplyBiomeVegetation(const FVector& Location, ECore_BiomeType BiomeType, float Density)
{
    switch (BiomeType)
    {
        case ECore_BiomeType::Tropical:
            GenerateTropicalForest(Location, ChunkSize.X * 0.4f);
            break;
        case ECore_BiomeType::Savanna:
            GenerateSavanna(Location, ChunkSize.X * 0.4f);
            break;
        case ECore_BiomeType::Wetlands:
            GenerateWetlands(Location, ChunkSize.X * 0.3f);
            break;
        default:
            // Default temperate vegetation
            break;
    }
}

void UCore_PhysicsWorldGenerator::CreateWaterBody(const FVector& Location, const FVector& Size, float Depth)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Creating water body at %s with size %s"), *Location.ToString(), *Size.ToString());
    
    // In full implementation would create actual water plane with physics
}

void UCore_PhysicsWorldGenerator::SetupPhysicsMaterials()
{
    // Create physics materials for different terrain types
    // In full implementation would load or create actual UPhysicalMaterial assets
    
    UE_LOG(LogTemp, Log, TEXT("Setting up physics materials for terrain"));
}

FVector UCore_PhysicsWorldGenerator::CalculateTerrainNormal(const FVector& Location)
{
    // Calculate terrain normal at given location
    return FVector::UpVector;
}

float UCore_PhysicsWorldGenerator::GetElevationAtLocation(const FVector& Location)
{
    // Get terrain elevation at location
    return 0.0f;
}

bool UCore_PhysicsWorldGenerator::IsLocationSuitableForVegetation(const FVector& Location, ECore_BiomeType BiomeType)
{
    // Check if location is suitable for vegetation based on slope, water proximity, etc.
    return true;
}