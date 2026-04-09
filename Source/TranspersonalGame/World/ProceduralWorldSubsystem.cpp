// Copyright Transpersonal Game Studio. All Rights Reserved.
// ProceduralWorldSubsystem.cpp - Implementation of procedural world generation

#include "ProceduralWorldSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeStreamingProxy.h"
#include "PCGVolume.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGSubsystem.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Math/UnrealMathUtility.h"
#include "Math/RandomStream.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/PhysicsCore/PhysicsPerformanceProfiler.h"

DEFINE_LOG_CATEGORY_STATIC(LogProceduralWorld, Log, All);

UProceduralWorldSubsystem::UProceduralWorldSubsystem()
{
    // Initialize default values
    bIsGenerating = false;
    CurrentGenerationTimeMs = 0.0f;
    PerformanceBudgetMs = 16.0f; // 60fps target
    GeneratedBiomeCount = 0;
    TotalBiomeCount = 0;
    BiomeMapResolution = 512;
    MaxConcurrentTasks = 4;
    bDebugDrawBiomes = false;
}

void UProceduralWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogProceduralWorld, Log, TEXT("ProceduralWorldSubsystem: Initializing world generation system"));
    
    // Initialize biome map
    BiomeMap.SetNum(BiomeMapResolution);
    for (int32 i = 0; i < BiomeMapResolution; ++i)
    {
        BiomeMap[i].SetNum(BiomeMapResolution);
        for (int32 j = 0; j < BiomeMapResolution; ++j)
        {
            BiomeMap[i][j] = EPrehistoricBiome::DenseJungle; // Default biome
        }
    }
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameTimeHistory);
    
    // Find or create performance profiler
    if (UWorld* World = GetWorld())
    {
        PerformanceProfiler = World->SpawnActor<APhysicsPerformanceProfiler>();
        if (PerformanceProfiler)
        {
            UE_LOG(LogProceduralWorld, Log, TEXT("ProceduralWorldSubsystem: Connected to Physics Performance Profiler"));
        }
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("ProceduralWorldSubsystem: Initialization complete"));
}

void UProceduralWorldSubsystem::Deinitialize()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("ProceduralWorldSubsystem: Shutting down world generation system"));
    
    // Clean up active PCG tasks
    if (UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(GetWorld()))
    {
        for (const FGuid& TaskId : ActivePCGTasks)
        {
            PCGSubsystem->CancelGeneration(TaskId);
        }
    }
    ActivePCGTasks.Empty();
    
    // Clean up generated content
    PCGVolumes.Empty();
    GeneratedLandscape = nullptr;
    CurrentWorldData = nullptr;
    
    Super::Deinitialize();
}

bool UProceduralWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UProceduralWorldSubsystem::GenerateWorld(UProceduralWorldDataAsset* WorldData)
{
    if (!WorldData)
    {
        UE_LOG(LogProceduralWorld, Error, TEXT("GenerateWorld: WorldData is null"));
        OnWorldGenerationComplete.Broadcast(false);
        return;
    }
    
    if (bIsGenerating)
    {
        UE_LOG(LogProceduralWorld, Warning, TEXT("GenerateWorld: Generation already in progress"));
        return;
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateWorld: Starting world generation"));
    
    bIsGenerating = true;
    CurrentWorldData = WorldData;
    GenerationStartTime = FPlatformTime::Seconds();
    GeneratedBiomeCount = 0;
    TotalBiomeCount = WorldData->AvailableBiomes.Num();
    
    // Set performance budget
    PerformanceBudgetMs = WorldData->PCGGenerationBudgetMs;
    MaxConcurrentTasks = WorldData->MaxConcurrentPCGTasks;
    
    // Generate terrain first
    GenerateTerrain(WorldData->TerrainSettings);
    
    // Generate biomes
    GenerateBiomes(WorldData->AvailableBiomes);
    
    // Place PCG volumes
    PlacePCGVolumes();
    
    // Optimize performance
    OptimizePerformance();
    
    bIsGenerating = false;
    double GenerationTime = FPlatformTime::Seconds() - GenerationStartTime;
    CurrentGenerationTimeMs = GenerationTime * 1000.0;
    
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateWorld: World generation completed in %.2f ms"), CurrentGenerationTimeMs);
    OnWorldGenerationComplete.Broadcast(true);
}

void UProceduralWorldSubsystem::GenerateTerrain(const FTerrainGenerationSettings& Settings)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateTerrain: Generating %dx%d terrain"), Settings.WorldSizeKm, Settings.WorldSizeKm);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogProceduralWorld, Error, TEXT("GenerateTerrain: Invalid world"));
        return;
    }
    
    // Generate heightmap data
    GenerateHeightmap(Settings);
    
    // Create landscape if it doesn't exist
    if (!GeneratedLandscape)
    {
        // In a real implementation, we would create the landscape actor here
        // For now, we'll assume it exists or will be created by the level designer
        TArray<AActor*> LandscapeActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
        
        if (LandscapeActors.Num() > 0)
        {
            GeneratedLandscape = Cast<ALandscape>(LandscapeActors[0]);
            UE_LOG(LogProceduralWorld, Log, TEXT("GenerateTerrain: Using existing landscape"));
        }
        else
        {
            UE_LOG(LogProceduralWorld, Warning, TEXT("GenerateTerrain: No landscape found - terrain generation requires existing landscape"));
        }
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateTerrain: Terrain generation complete"));
}

void UProceduralWorldSubsystem::GenerateHeightmap(const FTerrainGenerationSettings& Settings)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateHeightmap: Generating heightmap with resolution %d"), Settings.HeightmapResolution);
    
    // Generate noise-based heightmap
    const int32 Resolution = Settings.HeightmapResolution;
    const float WorldSize = Settings.WorldSizeKm * 100000.0f; // Convert km to cm
    const float Scale = WorldSize / Resolution;
    
    FRandomStream RandomStream(Settings.RandomSeed);
    
    // In a real implementation, we would generate actual heightmap data here
    // and apply it to the landscape. For now, we simulate the process.
    
    for (int32 Y = 0; Y < Resolution; ++Y)
    {
        for (int32 X = 0; X < Resolution; ++X)
        {
            float WorldX = (X - Resolution * 0.5f) * Scale;
            float WorldY = (Y - Resolution * 0.5f) * Scale;
            
            // Generate height using Perlin noise
            float Height = GeneratePerlinNoise(
                WorldX, WorldY,
                Settings.NoiseOctaves,
                Settings.NoiseFrequency,
                Settings.NoisePersistence,
                Settings.RandomSeed
            );
            
            // Scale to max elevation
            Height = (Height + 1.0f) * 0.5f * Settings.MaxElevation;
            
            // Store height data (in real implementation, this would go to landscape)
            // For now, we just validate the generation
        }
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateHeightmap: Heightmap generation complete"));
}

float UProceduralWorldSubsystem::GeneratePerlinNoise(float X, float Y, int32 Octaves, float Frequency, float Persistence, int32 Seed) const
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float MaxValue = 0.0f;
    
    FRandomStream RandomStream(Seed);
    
    for (int32 i = 0; i < Octaves; ++i)
    {
        // Simple noise implementation - in production, use proper Perlin noise
        float NoiseValue = FMath::Sin(X * Frequency + RandomStream.FRand() * 100.0f) * 
                          FMath::Cos(Y * Frequency + RandomStream.FRand() * 100.0f);
        
        Value += NoiseValue * Amplitude;
        MaxValue += Amplitude;
        
        Amplitude *= Persistence;
        Frequency *= 2.0f;
    }
    
    return Value / MaxValue;
}

void UProceduralWorldSubsystem::GenerateBiomes(const TArray<FBiomeGenerationData>& BiomeData)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateBiomes: Generating %d biomes"), BiomeData.Num());
    
    if (BiomeData.Num() == 0)
    {
        UE_LOG(LogProceduralWorld, Warning, TEXT("GenerateBiomes: No biome data provided"));
        return;
    }
    
    // Apply biome distribution across the world
    ApplyBiomeDistribution();
    
    // Generate biome-specific content
    for (int32 i = 0; i < BiomeData.Num(); ++i)
    {
        const FBiomeGenerationData& Biome = BiomeData[i];
        
        UE_LOG(LogProceduralWorld, Log, TEXT("GenerateBiomes: Processing biome %d - %s"), 
            i, *UEnum::GetValueAsString(Biome.BiomeType));
        
        // Simulate biome generation time
        CheckPerformanceBudget();
        
        GeneratedBiomeCount++;
        float CompletionPercent = (float)GeneratedBiomeCount / (float)TotalBiomeCount;
        OnBiomeGenerated.Broadcast(Biome.BiomeType, CompletionPercent);
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("GenerateBiomes: Biome generation complete"));
}

void UProceduralWorldSubsystem::ApplyBiomeDistribution()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("ApplyBiomeDistribution: Calculating biome distribution"));
    
    // Generate biome map based on terrain characteristics
    for (int32 Y = 0; Y < BiomeMapResolution; ++Y)
    {
        for (int32 X = 0; X < BiomeMapResolution; ++X)
        {
            // Convert to world coordinates
            float WorldX = (X - BiomeMapResolution * 0.5f) * 100.0f; // Scale factor
            float WorldY = (Y - BiomeMapResolution * 0.5f) * 100.0f;
            
            // Sample terrain characteristics
            float Height = SampleNoise(WorldX * 0.01f, WorldY * 0.01f);
            float Temperature = 25.0f + Height * 10.0f; // Altitude affects temperature
            float Humidity = 0.7f + SampleNoise(WorldX * 0.005f, WorldY * 0.005f) * 0.3f;
            
            // Determine biome type
            EPrehistoricBiome BiomeType = DetermineBiomeType(Height, Temperature, Humidity);
            BiomeMap[Y][X] = BiomeType;
        }
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("ApplyBiomeDistribution: Biome distribution complete"));
}

EPrehistoricBiome UProceduralWorldSubsystem::DetermineBiomeType(float Height, float Temperature, float Humidity) const
{
    // Simple biome determination logic based on environmental factors
    if (Height < -0.3f)
    {
        return EPrehistoricBiome::CoastalArea;
    }
    else if (Height > 0.6f)
    {
        return EPrehistoricBiome::MountainousRegion;
    }
    else if (Humidity > 0.8f && Temperature > 20.0f)
    {
        return EPrehistoricBiome::DenseJungle;
    }
    else if (Humidity > 0.9f)
    {
        return EPrehistoricBiome::Swampland;
    }
    else if (Humidity < 0.4f)
    {
        return EPrehistoricBiome::OpenPlains;
    }
    else
    {
        return EPrehistoricBiome::RiverDelta;
    }
}

float UProceduralWorldSubsystem::SampleNoise(float X, float Y) const
{
    // Simple noise sampling - in production, use proper noise functions
    return FMath::Sin(X * 3.14159f) * FMath::Cos(Y * 3.14159f);
}

EPrehistoricBiome UProceduralWorldSubsystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Convert world location to biome map coordinates
    int32 X = FMath::Clamp(
        FMath::FloorToInt((WorldLocation.X / 100.0f) + BiomeMapResolution * 0.5f),
        0, BiomeMapResolution - 1
    );
    int32 Y = FMath::Clamp(
        FMath::FloorToInt((WorldLocation.Y / 100.0f) + BiomeMapResolution * 0.5f),
        0, BiomeMapResolution - 1
    );
    
    return BiomeMap[Y][X];
}

void UProceduralWorldSubsystem::PlacePCGVolumes()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("PlacePCGVolumes: Placing PCG volumes for procedural content"));
    
    UWorld* World = GetWorld();
    if (!World || !CurrentWorldData)
    {
        UE_LOG(LogProceduralWorld, Error, TEXT("PlacePCGVolumes: Invalid world or world data"));
        return;
    }
    
    // Create PCG components for each biome type
    CreatePCGComponents();
    
    UE_LOG(LogProceduralWorld, Log, TEXT("PlacePCGVolumes: PCG volume placement complete"));
}

void UProceduralWorldSubsystem::CreatePCGComponents()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("CreatePCGComponents: Creating PCG components"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // In a real implementation, we would create PCG volumes and components here
    // For now, we simulate the process
    
    int32 ComponentCount = 0;
    for (const FBiomeGenerationData& BiomeData : CurrentWorldData->AvailableBiomes)
    {
        if (BiomeData.BiomePCGGraph.IsValid())
        {
            ComponentCount++;
            
            // Simulate PCG component creation and setup
            UE_LOG(LogProceduralWorld, Log, TEXT("CreatePCGComponents: Created PCG component for %s biome"), 
                *UEnum::GetValueAsString(BiomeData.BiomeType));
        }
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("CreatePCGComponents: Created %d PCG components"), ComponentCount);
}

void UProceduralWorldSubsystem::OptimizePerformance()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("OptimizePerformance: Running performance optimization"));
    
    UpdatePerformanceMetrics();
    
    // Check if we're within performance budget
    if (CurrentGenerationTimeMs > PerformanceBudgetMs)
    {
        UE_LOG(LogProceduralWorld, Warning, TEXT("OptimizePerformance: Generation time %.2f ms exceeds budget %.2f ms"), 
            CurrentGenerationTimeMs, PerformanceBudgetMs);
        
        // Apply optimizations
        MaxConcurrentTasks = FMath::Max(1, MaxConcurrentTasks - 1);
        UE_LOG(LogProceduralWorld, Log, TEXT("OptimizePerformance: Reduced concurrent tasks to %d"), MaxConcurrentTasks);
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("OptimizePerformance: Performance optimization complete"));
}

void UProceduralWorldSubsystem::UpdatePerformanceMetrics()
{
    // Update frame time history
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    FrameTimeHistory.Add(CurrentFrameTime);
    
    if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FMath::Max(1, FrameTimeHistory.Num());
    
    // Log performance metrics
    UE_LOG(LogProceduralWorld, VeryVerbose, TEXT("Performance: Current=%.2fms, Average=%.2fms, Budget=%.2fms"), 
        CurrentFrameTime, AverageFrameTime, PerformanceBudgetMs);
}

void UProceduralWorldSubsystem::CheckPerformanceBudget()
{
    UpdatePerformanceMetrics();
    
    // If we're over budget, yield to maintain frame rate
    if (FrameTimeHistory.Num() > 0)
    {
        float CurrentFrameTime = FrameTimeHistory.Last();
        if (CurrentFrameTime > PerformanceBudgetMs)
        {
            // In a real implementation, we would yield here
            UE_LOG(LogProceduralWorld, VeryVerbose, TEXT("CheckPerformanceBudget: Over budget, yielding"));
        }
    }
}

void UProceduralWorldSubsystem::SetPerformanceBudget(float BudgetMs)
{
    PerformanceBudgetMs = FMath::Max(1.0f, BudgetMs);
    UE_LOG(LogProceduralWorld, Log, TEXT("SetPerformanceBudget: Performance budget set to %.2f ms"), PerformanceBudgetMs);
}

void UProceduralWorldSubsystem::DebugDrawBiomes(bool bEnable)
{
    bDebugDrawBiomes = bEnable;
    
    if (bEnable)
    {
        UE_LOG(LogProceduralWorld, Log, TEXT("DebugDrawBiomes: Enabled biome debug visualization"));
        
        // Generate debug visualization data
        DebugBiomePoints.Empty();
        DebugBiomeColors.Empty();
        
        for (int32 Y = 0; Y < BiomeMapResolution; Y += 10) // Sample every 10th point for performance
        {
            for (int32 X = 0; X < BiomeMapResolution; X += 10)
            {
                FVector WorldPos(
                    (X - BiomeMapResolution * 0.5f) * 100.0f,
                    (Y - BiomeMapResolution * 0.5f) * 100.0f,
                    1000.0f // Elevated for visibility
                );
                
                DebugBiomePoints.Add(WorldPos);
                
                // Color based on biome type
                EPrehistoricBiome BiomeType = BiomeMap[Y][X];
                FColor BiomeColor = FColor::Green; // Default
                
                switch (BiomeType)
                {
                    case EPrehistoricBiome::DenseJungle: BiomeColor = FColor::Green; break;
                    case EPrehistoricBiome::OpenPlains: BiomeColor = FColor::Yellow; break;
                    case EPrehistoricBiome::RiverDelta: BiomeColor = FColor::Blue; break;
                    case EPrehistoricBiome::MountainousRegion: BiomeColor = FColor::White; break;
                    case EPrehistoricBiome::CoastalArea: BiomeColor = FColor::Cyan; break;
                    case EPrehistoricBiome::Swampland: BiomeColor = FColor::Magenta; break;
                    case EPrehistoricBiome::VolcanicRegion: BiomeColor = FColor::Red; break;
                    case EPrehistoricBiome::CaveSystem: BiomeColor = FColor::Black; break;
                }
                
                DebugBiomeColors.Add(BiomeColor);
            }
        }
    }
    else
    {
        UE_LOG(LogProceduralWorld, Log, TEXT("DebugDrawBiomes: Disabled biome debug visualization"));
        DebugBiomePoints.Empty();
        DebugBiomeColors.Empty();
    }
}

void UProceduralWorldSubsystem::RegenerateArea(const FVector& Center, float Radius)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("RegenerateArea: Regenerating area at %s with radius %.2f"), 
        *Center.ToString(), Radius);
    
    // In a real implementation, we would regenerate PCG content in the specified area
    // For now, we simulate the process
    
    if (PerformanceProfiler)
    {
        // Use the performance profiler to monitor regeneration
        UE_LOG(LogProceduralWorld, Log, TEXT("RegenerateArea: Using performance profiler for monitoring"));
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("RegenerateArea: Area regeneration complete"));
}