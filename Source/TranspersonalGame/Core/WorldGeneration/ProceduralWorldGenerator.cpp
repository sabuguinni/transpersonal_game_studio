#include "ProceduralWorldGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGSubsystem.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogProceduralWorld);

void UProceduralWorldGenerator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogProceduralWorld, Log, TEXT("ProceduralWorldGenerator initialized"));
    
    // Initialize default parameters
    CurrentParams = FWorldGenerationParams();
    GenerationProgress = 0.0f;
    bGenerationInProgress = false;
}

void UProceduralWorldGenerator::Deinitialize()
{
    // Clear generation timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(GenerationTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(GenerationTimer);
    }
    
    // Clean up PCG components
    BiomePCGComponents.Empty();
    BiomeConfigurations.Empty();
    RiverSplines.Empty();
    LakeActors.Empty();
    
    UE_LOG(LogProceduralWorld, Log, TEXT("ProceduralWorldGenerator deinitialized"));
    
    Super::Deinitialize();
}

void UProceduralWorldGenerator::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogProceduralWorld, Log, TEXT("World begin play - setting up procedural generation"));
    
    // Setup world partition if available
    SetupWorldPartition();
}

UProceduralWorldGenerator* UProceduralWorldGenerator::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UProceduralWorldGenerator>();
    }
    return nullptr;
}

void UProceduralWorldGenerator::GenerateWorld(const FWorldGenerationParams& Params)
{
    if (bGenerationInProgress)
    {
        UE_LOG(LogProceduralWorld, Warning, TEXT("World generation already in progress"));
        return;
    }
    
    UE_LOG(LogProceduralWorld, Log, TEXT("Starting world generation with size: %.2f x %.2f km"), 
           Params.WorldSizeKm.X, Params.WorldSizeKm.Y);
    
    CurrentParams = Params;
    bGenerationInProgress = true;
    GenerationProgress = 0.0f;
    
    // Generate in phases
    GenerateTerrain(Params);
    GenerationProgress = 0.2f;
    
    GenerateBiomes();
    GenerationProgress = 0.4f;
    
    GenerateRivers(Params.MajorRivers);
    GenerationProgress = 0.6f;
    
    GenerateLakes(Params.Lakes);
    GenerationProgress = 0.7f;
    
    ApplyErosion(Params.ErosionStrength);
    GenerationProgress = 0.8f;
    
    SetupPCGComponents();
    GenerationProgress = 0.9f;
    
    GenerateVegetation();
    GenerateGeologicalFeatures();
    GenerationProgress = 1.0f;
    
    bGenerationInProgress = false;
    
    UE_LOG(LogProceduralWorld, Log, TEXT("World generation completed"));
}

void UProceduralWorldGenerator::GenerateTerrain(const FWorldGenerationParams& Params)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Generating terrain heightmap"));
    
    // Generate heightmap data
    TArray<uint16> HeightData;
    GenerateHeightmapData(HeightData, Params.HeightmapResolution);
    
    // Create landscape actor
    CreateLandscapeActor(HeightData, Params.HeightmapResolution);
    
    // Setup materials
    SetupLandscapeMaterials();
}

void UProceduralWorldGenerator::GenerateBiomes()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Generating biome distribution"));
    
    TArray<uint8> BiomeData;
    GenerateBiomeMap(BiomeData, CurrentParams.HeightmapResolution);
    
    // Setup PCG for each biome type
    for (int32 BiomeIndex = 0; BiomeIndex < (int32)EBiomeType::LagoonSystems + 1; ++BiomeIndex)
    {
        EBiomeType BiomeType = (EBiomeType)BiomeIndex;
        SetupBiomePCG(BiomeType);
    }
}

void UProceduralWorldGenerator::GenerateRivers(int32 NumRivers)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Generating %d river systems"), NumRivers);
    
    RiverSplines.Empty();
    
    for (int32 i = 0; i < NumRivers; ++i)
    {
        // Generate random start point in mountains
        FVector StartPoint = FVector(
            FMath::RandRange(-CurrentParams.WorldSizeKm.X * 500.0f, CurrentParams.WorldSizeKm.X * 500.0f),
            FMath::RandRange(-CurrentParams.WorldSizeKm.Y * 500.0f, CurrentParams.WorldSizeKm.Y * 500.0f),
            CurrentParams.MaxElevation * 0.8f
        );
        
        // Generate end point near sea level
        FVector EndPoint = FVector(
            FMath::RandRange(-CurrentParams.WorldSizeKm.X * 500.0f, CurrentParams.WorldSizeKm.X * 500.0f),
            FMath::RandRange(-CurrentParams.WorldSizeKm.Y * 500.0f, CurrentParams.WorldSizeKm.Y * 500.0f),
            CurrentParams.SeaLevel + 10.0f
        );
        
        GenerateRiverSpline(StartPoint, EndPoint);
    }
}

void UProceduralWorldGenerator::GenerateLakes(int32 NumLakes)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Generating %d lakes"), NumLakes);
    
    LakeActors.Empty();
    
    for (int32 i = 0; i < NumLakes; ++i)
    {
        FVector LakePosition = FVector(
            FMath::RandRange(-CurrentParams.WorldSizeKm.X * 400.0f, CurrentParams.WorldSizeKm.X * 400.0f),
            FMath::RandRange(-CurrentParams.WorldSizeKm.Y * 400.0f, CurrentParams.WorldSizeKm.Y * 400.0f),
            FMath::RandRange(CurrentParams.SeaLevel + 50.0f, CurrentParams.MaxElevation * 0.3f)
        );
        
        float LakeRadius = FMath::RandRange(200.0f, 800.0f);
        PlaceLake(LakePosition, LakeRadius);
    }
}

void UProceduralWorldGenerator::ApplyErosion(float Strength, int32 Iterations)
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Applying erosion with strength %.2f over %d iterations"), Strength, Iterations);
    
    if (!GeneratedLandscape)
    {
        UE_LOG(LogProceduralWorld, Warning, TEXT("No landscape found for erosion"));
        return;
    }
    
    // Get heightmap data
    TArray<uint16> HeightData;
    // Note: In a real implementation, we'd extract heightmap data from the landscape
    // For now, we'll simulate the erosion process
    
    for (int32 i = 0; i < Iterations; ++i)
    {
        ApplyHydraulicErosion(HeightData, CurrentParams.HeightmapResolution, Strength);
    }
}

void UProceduralWorldGenerator::SetupWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UWorldPartitionSubsystem* WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>();
    if (WorldPartitionSubsystem)
    {
        UE_LOG(LogProceduralWorld, Log, TEXT("Setting up World Partition for large world streaming"));
        // Configure world partition settings for large world
    }
}

void UProceduralWorldGenerator::SetupPCGComponents()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Setting up PCG components for procedural content"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Setup PCG subsystem
    if (UPCGSubsystem* PCGSubsystem = World->GetSubsystem<UPCGSubsystem>())
    {
        // Configure PCG for each biome
        for (auto& BiomePair : BiomePCGComponents)
        {
            if (BiomePair.Value)
            {
                // Configure PCG component settings
                BiomePair.Value->SetGenerateOnLoad(true);
                BiomePair.Value->SetManagedByRuntimeGeneration(true);
            }
        }
    }
}

void UProceduralWorldGenerator::GenerateVegetation()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Generating vegetation using PCG"));
    
    // Trigger vegetation generation for each biome
    for (auto& BiomePair : BiomePCGComponents)
    {
        if (BiomePair.Value)
        {
            BiomePair.Value->Generate();
        }
    }
}

void UProceduralWorldGenerator::GenerateGeologicalFeatures()
{
    UE_LOG(LogProceduralWorld, Log, TEXT("Generating geological features"));
    
    // Generate caves, rock formations, etc.
    // This would be expanded with specific geological feature generation
}

EBiomeType UProceduralWorldGenerator::GetBiomeAtPosition(const FVector& WorldPosition) const
{
    // Convert world position to heightmap coordinates
    FVector2D HeightmapCoords = WorldToHeightmapCoords(WorldPosition);
    
    // Sample biome data (simplified)
    float NoiseValue = GeneratePerlinNoise(HeightmapCoords.X * 0.001f, HeightmapCoords.Y * 0.001f, 1.0f);
    
    if (NoiseValue < -0.3f)
        return EBiomeType::CoastalWetlands;
    else if (NoiseValue < -0.1f)
        return EBiomeType::FloodPlains;
    else if (NoiseValue < 0.1f)
        return EBiomeType::TropicalRainforest;
    else if (NoiseValue < 0.3f)
        return EBiomeType::UplandForests;
    else if (NoiseValue < 0.5f)
        return EBiomeType::OpenWoodlands;
    else if (NoiseValue < 0.7f)
        return EBiomeType::VolcanicRegions;
    else
        return EBiomeType::RiverDeltas;
}

float UProceduralWorldGenerator::GetElevationAtPosition(const FVector& WorldPosition) const
{
    if (!GeneratedLandscape)
    {
        return CurrentParams.SeaLevel;
    }
    
    // Sample landscape height at position
    return GeneratedLandscape->GetHeightAtLocation(WorldPosition);
}

bool UProceduralWorldGenerator::IsNearWater(const FVector& WorldPosition, float Range) const
{
    // Check distance to rivers
    for (const auto& RiverSpline : RiverSplines)
    {
        if (RiverSpline)
        {
            FVector ClosestPoint = RiverSpline->FindLocationClosestToWorldLocation(WorldPosition, ESplineCoordinateSpace::World);
            if (FVector::Dist(WorldPosition, ClosestPoint) <= Range)
            {
                return true;
            }
        }
    }
    
    // Check distance to lakes
    for (const auto& LakeActor : LakeActors)
    {
        if (LakeActor)
        {
            if (FVector::Dist(WorldPosition, LakeActor->GetActorLocation()) <= Range)
            {
                return true;
            }
        }
    }
    
    return false;
}

float UProceduralWorldGenerator::GetGenerationProgress() const
{
    return GenerationProgress;
}

// Private implementation methods

void UProceduralWorldGenerator::GenerateHeightmapData(TArray<uint16>& HeightData, int32 Resolution)
{
    HeightData.SetNum(Resolution * Resolution);
    
    const float Scale = 0.001f;
    const int32 Seed = CurrentParams.Seed;
    
    for (int32 Y = 0; Y < Resolution; ++Y)
    {
        for (int32 X = 0; X < Resolution; ++X)
        {
            float WorldX = (X - Resolution * 0.5f) * CurrentParams.WorldSizeKm.X * 1000.0f / Resolution;
            float WorldY = (Y - Resolution * 0.5f) * CurrentParams.WorldSizeKm.Y * 1000.0f / Resolution;
            
            // Generate base terrain
            float Height = GeneratePerlinNoise(WorldX * Scale, WorldY * Scale, 1.0f, 6);
            Height += GenerateRidgedNoise(WorldX * Scale * 2.0f, WorldY * Scale * 2.0f, 0.5f) * 0.3f;
            Height += GenerateVoronoiNoise(WorldX * Scale * 0.5f, WorldY * Scale * 0.5f, 2.0f) * 0.2f;
            
            // Normalize and scale
            Height = (Height + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
            Height = FMath::Clamp(Height, 0.0f, 1.0f);
            
            // Convert to uint16 heightmap format
            uint16 HeightValue = (uint16)(Height * 65535.0f * CurrentParams.MaxElevation / 512.0f);
            HeightData[Y * Resolution + X] = HeightValue;
        }
    }
}

void UProceduralWorldGenerator::GenerateBiomeMap(TArray<uint8>& BiomeData, int32 Resolution)
{
    BiomeData.SetNum(Resolution * Resolution);
    
    for (int32 Y = 0; Y < Resolution; ++Y)
    {
        for (int32 X = 0; X < Resolution; ++X)
        {
            float WorldX = (X - Resolution * 0.5f) * CurrentParams.WorldSizeKm.X * 1000.0f / Resolution;
            float WorldY = (Y - Resolution * 0.5f) * CurrentParams.WorldSizeKm.Y * 1000.0f / Resolution;
            
            EBiomeType BiomeType = GetBiomeAtPosition(FVector(WorldX, WorldY, 0.0f));
            BiomeData[Y * Resolution + X] = (uint8)BiomeType;
        }
    }
}

void UProceduralWorldGenerator::CreateLandscapeActor(const TArray<uint16>& HeightData, int32 Resolution)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create landscape actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("GeneratedLandscape");
    
    GeneratedLandscape = World->SpawnActor<ALandscape>(SpawnParams);
    if (GeneratedLandscape)
    {
        // Configure landscape
        GeneratedLandscape->SetActorLocation(FVector::ZeroVector);
        GeneratedLandscape->SetActorScale3D(FVector::OneVector);
        
        UE_LOG(LogProceduralWorld, Log, TEXT("Created landscape actor"));
    }
}

void UProceduralWorldGenerator::SetupLandscapeMaterials()
{
    if (!GeneratedLandscape)
    {
        return;
    }
    
    // Load or create landscape material
    // This would reference the actual landscape material asset
    UE_LOG(LogProceduralWorld, Log, TEXT("Setting up landscape materials"));
}

void UProceduralWorldGenerator::GenerateRiverSpline(const FVector& StartPoint, const FVector& EndPoint)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create spline component for river
    AActor* RiverActor = World->SpawnActor<AActor>();
    if (RiverActor)
    {
        USplineComponent* SplineComp = NewObject<USplineComponent>(RiverActor);
        RiverActor->SetRootComponent(SplineComp);
        
        // Setup spline points
        SplineComp->ClearSplinePoints();
        SplineComp->AddSplinePoint(StartPoint, ESplineCoordinateSpace::World);
        
        // Add intermediate points for natural river flow
        int32 NumSegments = 8;
        for (int32 i = 1; i < NumSegments; ++i)
        {
            float Alpha = (float)i / (float)NumSegments;
            FVector IntermediatePoint = FMath::Lerp(StartPoint, EndPoint, Alpha);
            
            // Add some randomness for natural curves
            IntermediatePoint.X += FMath::RandRange(-500.0f, 500.0f);
            IntermediatePoint.Y += FMath::RandRange(-500.0f, 500.0f);
            
            SplineComp->AddSplinePoint(IntermediatePoint, ESplineCoordinateSpace::World);
        }
        
        SplineComp->AddSplinePoint(EndPoint, ESplineCoordinateSpace::World);
        SplineComp->UpdateSpline();
        
        RiverSplines.Add(SplineComp);
    }
}

void UProceduralWorldGenerator::PlaceLake(const FVector& Position, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create lake actor (simplified - would use proper water system)
    AActor* LakeActor = World->SpawnActor<AActor>();
    if (LakeActor)
    {
        LakeActor->SetActorLocation(Position);
        LakeActor->SetActorScale3D(FVector(Radius / 100.0f));
        
        LakeActors.Add(LakeActor);
    }
}

void UProceduralWorldGenerator::ApplyHydraulicErosion(TArray<uint16>& HeightData, int32 Resolution, float Strength)
{
    // Simplified hydraulic erosion simulation
    // In a full implementation, this would use proper fluid simulation
    
    for (int32 Y = 1; Y < Resolution - 1; ++Y)
    {
        for (int32 X = 1; X < Resolution - 1; ++X)
        {
            int32 Index = Y * Resolution + X;
            uint16 CurrentHeight = HeightData[Index];
            
            // Find steepest descent
            uint16 MinHeight = CurrentHeight;
            int32 MinIndex = Index;
            
            // Check neighbors
            for (int32 DY = -1; DY <= 1; ++DY)
            {
                for (int32 DX = -1; DX <= 1; ++DX)
                {
                    if (DX == 0 && DY == 0) continue;
                    
                    int32 NeighborIndex = (Y + DY) * Resolution + (X + DX);
                    if (HeightData[NeighborIndex] < MinHeight)
                    {
                        MinHeight = HeightData[NeighborIndex];
                        MinIndex = NeighborIndex;
                    }
                }
            }
            
            // Apply erosion
            if (MinIndex != Index)
            {
                uint16 ErosionAmount = (uint16)((CurrentHeight - MinHeight) * Strength * 0.1f);
                HeightData[Index] = FMath::Max((int32)HeightData[Index] - (int32)ErosionAmount, 0);
            }
        }
    }
}

void UProceduralWorldGenerator::SetupBiomePCG(EBiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create PCG component for biome
    AActor* PCGActor = World->SpawnActor<AActor>();
    if (PCGActor)
    {
        UPCGComponent* PCGComp = NewObject<UPCGComponent>(PCGActor);
        PCGActor->SetRootComponent(PCGComp);
        
        // Configure PCG component for this biome
        // Load appropriate PCG graph asset
        
        BiomePCGComponents.Add(BiomeType, PCGComp);
    }
}

float UProceduralWorldGenerator::GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves) const
{
    float Result = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; ++i)
    {
        Result += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return Result / MaxValue;
}

float UProceduralWorldGenerator::GenerateRidgedNoise(float X, float Y, float Scale) const
{
    float Noise = FMath::Abs(FMath::PerlinNoise2D(FVector2D(X * Scale, Y * Scale)));
    return 1.0f - Noise;
}

float UProceduralWorldGenerator::GenerateVoronoiNoise(float X, float Y, float Scale) const
{
    // Simplified Voronoi noise
    float CellSize = 1.0f / Scale;
    int32 CellX = FMath::FloorToInt(X / CellSize);
    int32 CellY = FMath::FloorToInt(Y / CellSize);
    
    float MinDist = FLT_MAX;
    
    for (int32 DY = -1; DY <= 1; ++DY)
    {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
            int32 SeedX = CellX + DX;
            int32 SeedY = CellY + DY;
            
            // Generate pseudo-random point in cell
            float PointX = (SeedX + 0.5f + FMath::Sin(SeedX * 127.1f + SeedY * 311.7f) * 0.5f) * CellSize;
            float PointY = (SeedY + 0.5f + FMath::Sin(SeedX * 269.5f + SeedY * 183.3f) * 0.5f) * CellSize;
            
            float Dist = FMath::Sqrt((X - PointX) * (X - PointX) + (Y - PointY) * (Y - PointY));
            MinDist = FMath::Min(MinDist, Dist);
        }
    }
    
    return FMath::Clamp(MinDist * Scale, 0.0f, 1.0f);
}

FVector2D UProceduralWorldGenerator::WorldToHeightmapCoords(const FVector& WorldPos) const
{
    float X = (WorldPos.X + CurrentParams.WorldSizeKm.X * 500.0f) / (CurrentParams.WorldSizeKm.X * 1000.0f) * CurrentParams.HeightmapResolution;
    float Y = (WorldPos.Y + CurrentParams.WorldSizeKm.Y * 500.0f) / (CurrentParams.WorldSizeKm.Y * 1000.0f) * CurrentParams.HeightmapResolution;
    
    return FVector2D(X, Y);
}

FVector UProceduralWorldGenerator::HeightmapToWorldCoords(const FVector2D& HeightmapCoords) const
{
    float X = (HeightmapCoords.X / CurrentParams.HeightmapResolution * CurrentParams.WorldSizeKm.X * 1000.0f) - CurrentParams.WorldSizeKm.X * 500.0f;
    float Y = (HeightmapCoords.Y / CurrentParams.HeightmapResolution * CurrentParams.WorldSizeKm.Y * 1000.0f) - CurrentParams.WorldSizeKm.Y * 500.0f;
    
    return FVector(X, Y, 0.0f);
}