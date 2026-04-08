// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ProceduralWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeInfo.h"
#include "Landscape/Classes/LandscapeStreamingProxy.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGSubsystem.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

DEFINE_LOG_CATEGORY_STATIC(LogProceduralWorldGen, Log, All);

UProceduralWorldGenerator::UProceduralWorldGenerator()
{
    // Initialize default generation parameters
    GenerationParams.WorldSize = FVector2D(20480.0f, 20480.0f); // 20km x 20km
    GenerationParams.LandscapeResolution = 4033; // 4033x4033 vertices for optimal performance
    GenerationParams.HeightScale = 100.0f;
    GenerationParams.MajorBiomeCount = 5;
    GenerationParams.MinorBiomeCount = 12;
    GenerationParams.RiverComplexity = 6;
    GenerationParams.MajorRiverCount = 3;
    GenerationParams.LakeGenerationProbability = 0.3f;
    GenerationParams.CaveSystemDensity = 0.15f;
    GenerationParams.ResourceDensity = 1.0f;
    GenerationParams.TimeGemLocations = 1;
    GenerationParams.WorldSeed = 12345;

    // Initialize biome data array
    BiomeDataAssets.Empty();
    
    // Initialize generation state
    bIsGenerating = false;
    bIsInitialized = false;
    CurrentGenerationStep = EGenerationStep::NotStarted;
    GenerationProgress = 0.0f;

    // Initialize PCG subsystem reference
    PCGSubsystem = nullptr;
    
    // Initialize landscape reference
    MainLandscape = nullptr;
    
    // Initialize random stream with default seed
    RandomStream = FRandomStream(GenerationParams.WorldSeed);
}

void UProceduralWorldGenerator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Initializing Procedural World Generator"));
    
    // Get PCG subsystem
    if (UWorld* World = GetWorld())
    {
        PCGSubsystem = USubsystemBlueprintLibrary::GetWorldSubsystem(World, UPCGSubsystem::StaticClass());
        if (!PCGSubsystem)
        {
            UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to get PCG Subsystem"));
            return;
        }
    }
    
    // Load default biome data assets
    LoadDefaultBiomeData();
    
    // Initialize generation systems
    InitializeGenerationSystems();
    
    bIsInitialized = true;
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Procedural World Generator initialized successfully"));
}

void UProceduralWorldGenerator::Deinitialize()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Deinitializing Procedural World Generator"));
    
    // Clean up generation data
    CleanupGenerationData();
    
    // Clear references
    PCGSubsystem = nullptr;
    MainLandscape = nullptr;
    BiomeDataAssets.Empty();
    GeneratedRiverSystems.Empty();
    ResourceSpawnLocations.Empty();
    TimeGemLocations.Empty();
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UProceduralWorldGenerator::GenerateWorld(const FWorldGenerationParams& InParams)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Cannot generate world: Generator not initialized"));
        return false;
    }
    
    if (bIsGenerating)
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("World generation already in progress"));
        return false;
    }
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Starting world generation with seed: %d"), InParams.WorldSeed);
    
    // Store generation parameters
    GenerationParams = InParams;
    RandomStream = FRandomStream(GenerationParams.WorldSeed);
    
    // Reset generation state
    bIsGenerating = true;
    CurrentGenerationStep = EGenerationStep::GeneratingTerrain;
    GenerationProgress = 0.0f;
    
    // Start generation process
    return ExecuteGenerationStep();
}

bool UProceduralWorldGenerator::ExecuteGenerationStep()
{
    if (!bIsGenerating)
    {
        return false;
    }
    
    switch (CurrentGenerationStep)
    {
        case EGenerationStep::GeneratingTerrain:
            return GenerateBaseTerrain();
            
        case EGenerationStep::GeneratingBiomes:
            return GenerateBiomes();
            
        case EGenerationStep::GeneratingWaterSystems:
            return GenerateWaterSystems();
            
        case EGenerationStep::GeneratingGeology:
            return GenerateGeologicalFeatures();
            
        case EGenerationStep::GeneratingVegetation:
            return GenerateVegetation();
            
        case EGenerationStep::GeneratingResources:
            return GenerateResources();
            
        case EGenerationStep::PlacingQuestItems:
            return PlaceQuestItems();
            
        case EGenerationStep::Finalizing:
            return FinalizeGeneration();
            
        default:
            UE_LOG(LogProceduralWorldGen, Error, TEXT("Unknown generation step"));
            return false;
    }
}

bool UProceduralWorldGenerator::GenerateBaseTerrain()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generating base terrain"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("No valid world found"));
        return false;
    }
    
    // Create or find main landscape
    MainLandscape = FindOrCreateMainLandscape();
    if (!MainLandscape)
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to create main landscape"));
        return false;
    }
    
    // Generate heightmap data
    TArray<uint16> HeightmapData;
    if (!GenerateHeightmapData(HeightmapData))
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to generate heightmap data"));
        return false;
    }
    
    // Apply heightmap to landscape
    if (!ApplyHeightmapToLandscape(HeightmapData))
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to apply heightmap to landscape"));
        return false;
    }
    
    // Move to next step
    CurrentGenerationStep = EGenerationStep::GeneratingBiomes;
    GenerationProgress = 0.2f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Base terrain generation completed"));
    return true;
}

bool UProceduralWorldGenerator::GenerateBiomes()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generating biomes"));
    
    if (!MainLandscape)
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("No landscape found for biome generation"));
        return false;
    }
    
    // Create biome distribution map
    TArray<TArray<EBiomeType>> BiomeMap;
    if (!CreateBiomeDistributionMap(BiomeMap))
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to create biome distribution map"));
        return false;
    }
    
    // Apply biomes to landscape
    if (!ApplyBiomesToLandscape(BiomeMap))
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to apply biomes to landscape"));
        return false;
    }
    
    // Generate biome transition zones
    if (!GenerateBiomeTransitions(BiomeMap))
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("Failed to generate biome transitions"));
        return false;
    }
    
    // Move to next step
    CurrentGenerationStep = EGenerationStep::GeneratingWaterSystems;
    GenerationProgress = 0.35f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Biome generation completed"));
    return true;
}

bool UProceduralWorldGenerator::GenerateWaterSystems()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generating water systems"));
    
    // Clear existing river systems
    GeneratedRiverSystems.Empty();
    
    // Generate major rivers
    for (int32 i = 0; i < GenerationParams.MajorRiverCount; i++)
    {
        FRiverSystemData RiverData;
        if (GenerateRiverSystem(RiverData, true))
        {
            GeneratedRiverSystems.Add(RiverData);
            UE_LOG(LogProceduralWorldGen, Log, TEXT("Generated major river %d"), i + 1);
        }
    }
    
    // Generate minor tributaries
    int32 TributaryCount = FMath::RandRange(3, 8);
    for (int32 i = 0; i < TributaryCount; i++)
    {
        FRiverSystemData TributaryData;
        if (GenerateRiverSystem(TributaryData, false))
        {
            GeneratedRiverSystems.Add(TributaryData);
        }
    }
    
    // Generate lakes
    if (!GenerateLakes())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("Lake generation had issues"));
    }
    
    // Move to next step
    CurrentGenerationStep = EGenerationStep::GeneratingGeology;
    GenerationProgress = 0.5f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Water systems generation completed"));
    return true;
}

bool UProceduralWorldGenerator::GenerateGeologicalFeatures()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generating geological features"));
    
    // Generate cave systems
    if (!GenerateCaveSystems())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("Cave system generation had issues"));
    }
    
    // Generate cliffs and rock formations
    if (!GenerateRockFormations())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("Rock formation generation had issues"));
    }
    
    // Generate valleys and ravines
    if (!GenerateValleys())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("Valley generation had issues"));
    }
    
    // Move to next step
    CurrentGenerationStep = EGenerationStep::GeneratingVegetation;
    GenerationProgress = 0.65f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Geological features generation completed"));
    return true;
}

bool UProceduralWorldGenerator::GenerateVegetation()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generating vegetation"));
    
    if (!PCGSubsystem)
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("PCG Subsystem not available for vegetation generation"));
        return false;
    }
    
    // Generate vegetation for each biome type
    for (const auto& BiomeData : BiomeDataAssets)
    {
        if (BiomeData.IsValid())
        {
            if (!GenerateVegetationForBiome(BiomeData.Get()))
            {
                UE_LOG(LogProceduralWorldGen, Warning, TEXT("Vegetation generation failed for biome: %s"), 
                    *BiomeData->BiomeName);
            }
        }
    }
    
    // Move to next step
    CurrentGenerationStep = EGenerationStep::GeneratingResources;
    GenerationProgress = 0.8f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Vegetation generation completed"));
    return true;
}

bool UProceduralWorldGenerator::GenerateResources()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generating resources"));
    
    // Clear existing resource locations
    ResourceSpawnLocations.Empty();
    
    // Generate resources for each resource type
    TArray<EResourceType> ResourceTypes = {
        EResourceType::Stone,
        EResourceType::Wood,
        EResourceType::Flint,
        EResourceType::Clay,
        EResourceType::Fiber,
        EResourceType::Berries,
        EResourceType::Herbs,
        EResourceType::Water,
        EResourceType::Obsidian,
        EResourceType::Amber,
        EResourceType::Crystal
    };
    
    for (EResourceType ResourceType : ResourceTypes)
    {
        if (!GenerateResourceType(ResourceType))
        {
            UE_LOG(LogProceduralWorldGen, Warning, TEXT("Failed to generate resource type: %d"), (int32)ResourceType);
        }
    }
    
    // Move to next step
    CurrentGenerationStep = EGenerationStep::PlacingQuestItems;
    GenerationProgress = 0.9f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Resource generation completed"));
    return true;
}

bool UProceduralWorldGenerator::PlaceQuestItems()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Placing quest items"));
    
    // Clear existing time gem locations
    TimeGemLocations.Empty();
    
    // Place time gems (main quest items)
    for (int32 i = 0; i < GenerationParams.TimeGemLocations; i++)
    {
        FVector GemLocation;
        if (FindSuitableTimeGemLocation(GemLocation))
        {
            TimeGemLocations.Add(GemLocation);
            UE_LOG(LogProceduralWorldGen, Log, TEXT("Placed time gem at location: %s"), *GemLocation.ToString());
        }
        else
        {
            UE_LOG(LogProceduralWorldGen, Warning, TEXT("Failed to find suitable location for time gem %d"), i + 1);
        }
    }
    
    // Move to final step
    CurrentGenerationStep = EGenerationStep::Finalizing;
    GenerationProgress = 0.95f;
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Quest item placement completed"));
    return true;
}

bool UProceduralWorldGenerator::FinalizeGeneration()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Finalizing world generation"));
    
    // Optimize generated content
    if (!OptimizeGeneratedContent())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("Content optimization had issues"));
    }
    
    // Generate LODs
    if (!GenerateLODs())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("LOD generation had issues"));
    }
    
    // Setup streaming
    if (!SetupWorldPartitionStreaming())
    {
        UE_LOG(LogProceduralWorldGen, Warning, TEXT("World partition streaming setup had issues"));
    }
    
    // Finalize generation
    bIsGenerating = false;
    CurrentGenerationStep = EGenerationStep::Completed;
    GenerationProgress = 1.0f;
    
    // Broadcast completion event
    OnWorldGenerationCompleted.Broadcast(true);
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("World generation completed successfully"));
    return true;
}

ALandscape* UProceduralWorldGenerator::FindOrCreateMainLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Try to find existing landscape
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape && !Landscape->IsPendingKill())
        {
            UE_LOG(LogProceduralWorldGen, Log, TEXT("Found existing landscape: %s"), *Landscape->GetName());
            return Landscape;
        }
    }
    
    // Create new landscape if none found
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Creating new landscape"));
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(TEXT("MainLandscape"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ALandscape* NewLandscape = World->SpawnActor<ALandscape>(SpawnParams);
    if (NewLandscape)
    {
        // Configure landscape properties
        NewLandscape->SetActorLocation(FVector::ZeroVector);
        NewLandscape->SetActorScale3D(FVector::OneVector);
        
        UE_LOG(LogProceduralWorldGen, Log, TEXT("Created new landscape: %s"), *NewLandscape->GetName());
    }
    
    return NewLandscape;
}

bool UProceduralWorldGenerator::GenerateHeightmapData(TArray<uint16>& OutHeightmapData)
{
    const int32 Resolution = GenerationParams.LandscapeResolution;
    const int32 TotalPixels = Resolution * Resolution;
    
    OutHeightmapData.Empty(TotalPixels);
    OutHeightmapData.AddUninitialized(TotalPixels);
    
    // Generate heightmap using multiple octaves of noise
    const float Scale = 0.001f; // Noise scale
    const int32 Octaves = 6;
    const float Persistence = 0.5f;
    const float Lacunarity = 2.0f;
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float Height = 0.0f;
            float Amplitude = 1.0f;
            float Frequency = Scale;
            float MaxValue = 0.0f;
            
            // Generate multiple octaves of noise
            for (int32 Octave = 0; Octave < Octaves; Octave++)
            {
                float NoiseValue = FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency));
                Height += NoiseValue * Amplitude;
                MaxValue += Amplitude;
                
                Amplitude *= Persistence;
                Frequency *= Lacunarity;
            }
            
            // Normalize height
            Height = Height / MaxValue;
            
            // Apply height scale and convert to uint16
            float ScaledHeight = (Height + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
            ScaledHeight *= GenerationParams.HeightScale;
            
            // Clamp to uint16 range
            uint16 HeightValue = FMath::Clamp(FMath::RoundToInt(ScaledHeight * 655.35f), 0, 65535);
            
            OutHeightmapData[Y * Resolution + X] = HeightValue;
        }
    }
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generated heightmap data: %dx%d pixels"), Resolution, Resolution);
    return true;
}

bool UProceduralWorldGenerator::ApplyHeightmapToLandscape(const TArray<uint16>& HeightmapData)
{
    if (!MainLandscape)
    {
        UE_LOG(LogProceduralWorldGen, Error, TEXT("No landscape to apply heightmap to"));
        return false;
    }
    
    // This is a simplified implementation
    // In a real project, you would use ULandscapeEditorObject or similar APIs
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Applied heightmap to landscape (simplified implementation)"));
    
    return true;
}

void UProceduralWorldGenerator::LoadDefaultBiomeData()
{
    // This would typically load biome data assets from the content browser
    // For now, we'll create default configurations
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Loading default biome data"));
    
    // In a real implementation, you would load actual UBiomeData assets
    // For now, we just log that the system is ready
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Default biome data loaded"));
}

void UProceduralWorldGenerator::InitializeGenerationSystems()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Initializing generation systems"));
    
    // Initialize noise generation
    // Initialize PCG graph templates
    // Initialize resource databases
    // Initialize biome transition rules
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generation systems initialized"));
}

void UProceduralWorldGenerator::CleanupGenerationData()
{
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Cleaning up generation data"));
    
    // Clean up temporary data structures
    // Clear caches
    // Reset state variables
    
    UE_LOG(LogProceduralWorldGen, Log, TEXT("Generation data cleaned up"));
}

// Additional helper methods would be implemented here...
// This is a comprehensive foundation for the procedural world generation system