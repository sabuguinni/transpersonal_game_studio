#include "World_PerformanceTerrainController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/LandscapeStreamingProxy.h"
#include "Landscape/LandscapeInfo.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

AWorld_PerformanceTerrainController::AWorld_PerformanceTerrainController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds for performance

    // Initialize default settings
    PerformanceSettings = FWorld_TerrainPerformanceSettings();
    
    // Initialize frame time tracking
    for (int32 i = 0; i < 60; ++i)
    {
        FrameTimeHistory[i] = 16.67f; // Default to 60 FPS
    }
    
    SetActorTickEnabled(true);
}

void AWorld_PerformanceTerrainController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: BeginPlay - Initializing terrain system"));
    
    InitializeTerrainSystem();
    
    // Start performance monitoring
    LastPerformanceCheck = GetWorld()->GetTimeSeconds();
}

void AWorld_PerformanceTerrainController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update frame time history
    FrameTimeHistory[FrameTimeIndex] = DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimeIndex = (FrameTimeIndex + 1) % 60;
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (int32 i = 0; i < 60; ++i)
    {
        TotalFrameTime += FrameTimeHistory[i];
    }
    AverageFrameTime = TotalFrameTime / 60.0f;
    
    // Get player location for terrain streaming
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Update terrain streaming
        StreamTerrainChunks(PlayerLocation);
        
        // Update LOD based on player position
        UpdateTerrainLOD(PlayerLocation);
    }
    
    // Performance check every 2 seconds
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPerformanceCheck > 2.0f)
    {
        CheckTerrainPerformance();
        LastPerformanceCheck = CurrentTime;
    }
}

void AWorld_PerformanceTerrainController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up active landscapes
    ActiveLandscapes.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void AWorld_PerformanceTerrainController::InitializeTerrainSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Initializing terrain system"));
    
    // Clear existing data
    TerrainChunks.Empty();
    ActiveLandscapes.Empty();
    ActiveChunkCount = 0;
    
    // Create initial terrain chunks in a grid pattern
    const int32 GridSize = 8; // 8x8 grid of chunks
    const float ChunkSize = 2000.0f; // 2km per chunk
    
    for (int32 X = 0; X < GridSize; ++X)
    {
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            FWorld_TerrainChunk NewChunk;
            NewChunk.ChunkLocation = FVector(
                X * ChunkSize - (GridSize * ChunkSize * 0.5f),
                Y * ChunkSize - (GridSize * ChunkSize * 0.5f),
                0.0f
            );
            NewChunk.ChunkSize = FVector(ChunkSize, ChunkSize, 1000.0f);
            
            // Assign biome types based on position
            if (X < 3 && Y < 3)
                NewChunk.BiomeType = EBiomeType::Temperate_Forest;
            else if (X > 5 || Y > 5)
                NewChunk.BiomeType = EBiomeType::Arid_Desert;
            else
                NewChunk.BiomeType = EBiomeType::Grassland_Plains;
            
            NewChunk.TerrainComplexity = FMath::RandRange(0.3f, 0.8f);
            NewChunk.bIsActive = false;
            NewChunk.bNeedsGeneration = true;
            
            TerrainChunks.Add(NewChunk);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Created %d terrain chunks"), TerrainChunks.Num());
}

void AWorld_PerformanceTerrainController::GenerateTerrainChunk(const FVector& Location, EBiomeType BiomeType)
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Generating terrain chunk at %s"), *Location.ToString());
    
    // Check if we're at the maximum number of active landscapes
    if (ActiveLandscapes.Num() >= PerformanceSettings.MaxLandscapeActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Max landscapes reached, skipping generation"));
        return;
    }
    
    // Create landscape actor
    CreateLandscapeActor(Location, FVector(1.0f, 1.0f, 1.0f));
}

void AWorld_PerformanceTerrainController::CreateLandscapeActor(const FVector& Location, const FVector& Scale)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("PerformanceTerrainController: No world available for landscape creation"));
        return;
    }
    
    // Spawn landscape proxy for performance
    ALandscapeProxy* NewLandscape = GetWorld()->SpawnActor<ALandscapeProxy>(
        ALandscapeProxy::StaticClass(),
        Location,
        FRotator::ZeroRotator
    );
    
    if (NewLandscape)
    {
        NewLandscape->SetActorScale3D(Scale);
        ActiveLandscapes.Add(NewLandscape);
        ActiveChunkCount++;
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Created landscape at %s"), *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PerformanceTerrainController: Failed to create landscape at %s"), *Location.ToString());
    }
}

void AWorld_PerformanceTerrainController::UpdateTerrainLOD(const FVector& PlayerLocation)
{
    for (ALandscapeProxy* Landscape : ActiveLandscapes)
    {
        if (!IsValid(Landscape))
            continue;
        
        float Distance = FVector::Dist(Landscape->GetActorLocation(), PlayerLocation);
        
        // Adjust LOD based on distance
        if (Distance > PerformanceSettings.TerrainLODDistance * 2.0f)
        {
            // Far LOD - reduce quality
            Landscape->SetActorHiddenInGame(false);
            // Additional LOD settings would go here
        }
        else if (Distance > PerformanceSettings.TerrainLODDistance)
        {
            // Medium LOD
            Landscape->SetActorHiddenInGame(false);
        }
        else
        {
            // Near LOD - full quality
            Landscape->SetActorHiddenInGame(false);
        }
        
        // Cull distant terrain if enabled
        if (PerformanceSettings.bEnableTerrainCulling && Distance > PerformanceSettings.CullingDistance)
        {
            Landscape->SetActorHiddenInGame(true);
        }
    }
}

void AWorld_PerformanceTerrainController::OptimizeTerrainPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Optimizing terrain performance"));
    
    if (AverageFrameTime > 20.0f) // Below 50 FPS
    {
        // Reduce terrain quality
        PerformanceSettings.TerrainLODDistance *= 0.9f;
        PerformanceSettings.CullingDistance *= 0.9f;
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Reducing terrain quality due to low FPS"));
    }
    else if (AverageFrameTime < 14.0f) // Above 70 FPS
    {
        // Increase terrain quality
        PerformanceSettings.TerrainLODDistance *= 1.05f;
        PerformanceSettings.CullingDistance *= 1.05f;
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Increasing terrain quality due to good FPS"));
    }
    
    ApplyPerformanceOptimizations();
}

void AWorld_PerformanceTerrainController::CheckTerrainPerformance()
{
    if (bPerformanceOptimizationEnabled)
    {
        OptimizeTerrainPerformance();
    }
    
    // Log performance stats
    UE_LOG(LogTemp, Log, TEXT("PerformanceTerrainController: Avg Frame Time: %.2fms, Active Chunks: %d"), 
           AverageFrameTime, ActiveChunkCount);
}

void AWorld_PerformanceTerrainController::AdjustTerrainQuality(float TargetFrameTime)
{
    float CurrentFrameTime = AverageFrameTime;
    
    if (CurrentFrameTime > TargetFrameTime * 1.2f)
    {
        // Performance is poor, reduce quality
        PerformanceSettings.MaxLandscapeActors = FMath::Max(4, PerformanceSettings.MaxLandscapeActors - 2);
        PerformanceSettings.TerrainResolution = FMath::Max(512, PerformanceSettings.TerrainResolution / 2);
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f)
    {
        // Performance is good, increase quality
        PerformanceSettings.MaxLandscapeActors = FMath::Min(32, PerformanceSettings.MaxLandscapeActors + 1);
        PerformanceSettings.TerrainResolution = FMath::Min(2048, PerformanceSettings.TerrainResolution * 2);
    }
}

float AWorld_PerformanceTerrainController::GetCurrentTerrainLoad() const
{
    return static_cast<float>(ActiveChunkCount) / static_cast<float>(PerformanceSettings.MaxLandscapeActors);
}

void AWorld_PerformanceTerrainController::ActivateTerrainChunk(int32 ChunkIndex)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
        return;
    
    FWorld_TerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    
    if (!Chunk.bIsActive && ActiveChunkCount < PerformanceSettings.MaxLandscapeActors)
    {
        if (Chunk.bNeedsGeneration)
        {
            GenerateTerrainChunk(Chunk.ChunkLocation, Chunk.BiomeType);
            Chunk.bNeedsGeneration = false;
        }
        
        Chunk.bIsActive = true;
        ActiveChunkCount++;
    }
}

void AWorld_PerformanceTerrainController::DeactivateTerrainChunk(int32 ChunkIndex)
{
    if (!TerrainChunks.IsValidIndex(ChunkIndex))
        return;
    
    FWorld_TerrainChunk& Chunk = TerrainChunks[ChunkIndex];
    
    if (Chunk.bIsActive)
    {
        Chunk.bIsActive = false;
        ActiveChunkCount = FMath::Max(0, ActiveChunkCount - 1);
    }
}

void AWorld_PerformanceTerrainController::StreamTerrainChunks(const FVector& PlayerLocation)
{
    const float StreamingRange = PerformanceSettings.TerrainLODDistance * 1.5f;
    
    for (int32 i = 0; i < TerrainChunks.Num(); ++i)
    {
        const FWorld_TerrainChunk& Chunk = TerrainChunks[i];
        float Distance = FVector::Dist(Chunk.ChunkLocation, PlayerLocation);
        
        if (Distance <= StreamingRange && !Chunk.bIsActive)
        {
            ActivateTerrainChunk(i);
        }
        else if (Distance > StreamingRange * 1.2f && Chunk.bIsActive)
        {
            DeactivateTerrainChunk(i);
        }
    }
}

void AWorld_PerformanceTerrainController::ApplyBiomeToTerrain(ALandscapeProxy* Landscape, EBiomeType BiomeType)
{
    if (!IsValid(Landscape))
        return;
    
    // Apply biome-specific materials and settings
    // This would integrate with the biome system
    UE_LOG(LogTemp, Log, TEXT("PerformanceTerrainController: Applying biome %d to landscape"), static_cast<int32>(BiomeType));
}

void AWorld_PerformanceTerrainController::UpdateTerrainMaterials(ALandscapeProxy* Landscape, EBiomeType BiomeType)
{
    if (!IsValid(Landscape))
        return;
    
    // Update landscape materials based on biome
    // This would load appropriate materials for the biome type
}

FVector AWorld_PerformanceTerrainController::GetNearestChunkLocation(const FVector& WorldLocation) const
{
    float MinDistance = FLT_MAX;
    FVector NearestLocation = FVector::ZeroVector;
    
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        float Distance = FVector::Dist(Chunk.ChunkLocation, WorldLocation);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestLocation = Chunk.ChunkLocation;
        }
    }
    
    return NearestLocation;
}

int32 AWorld_PerformanceTerrainController::GetChunkIndexAtLocation(const FVector& WorldLocation) const
{
    for (int32 i = 0; i < TerrainChunks.Num(); ++i)
    {
        const FWorld_TerrainChunk& Chunk = TerrainChunks[i];
        FVector ChunkMin = Chunk.ChunkLocation - (Chunk.ChunkSize * 0.5f);
        FVector ChunkMax = Chunk.ChunkLocation + (Chunk.ChunkSize * 0.5f);
        
        if (WorldLocation.X >= ChunkMin.X && WorldLocation.X <= ChunkMax.X &&
            WorldLocation.Y >= ChunkMin.Y && WorldLocation.Y <= ChunkMax.Y)
        {
            return i;
        }
    }
    
    return INDEX_NONE;
}

bool AWorld_PerformanceTerrainController::IsChunkInRange(const FVector& ChunkLocation, const FVector& PlayerLocation) const
{
    float Distance = FVector::Dist(ChunkLocation, PlayerLocation);
    return Distance <= PerformanceSettings.TerrainLODDistance * 1.5f;
}

void AWorld_PerformanceTerrainController::EditorGenerateTestTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Generating test terrain in editor"));
    
    InitializeTerrainSystem();
    
    // Generate a few test chunks around the origin
    for (int32 i = 0; i < 4; ++i)
    {
        FVector TestLocation = FVector(i * 2000.0f, 0.0f, 0.0f);
        GenerateTerrainChunk(TestLocation, EBiomeType::Temperate_Forest);
    }
}

void AWorld_PerformanceTerrainController::EditorOptimizeAllTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Optimizing all terrain in editor"));
    
    OptimizeTerrainPerformance();
    ApplyPerformanceOptimizations();
}

void AWorld_PerformanceTerrainController::EditorValidateTerrainSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceTerrainController: Validating terrain system"));
    
    UE_LOG(LogTemp, Warning, TEXT("Total Chunks: %d"), TerrainChunks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Landscapes: %d"), ActiveLandscapes.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Settings: MaxActors=%d, LODDistance=%.1f"), 
           PerformanceSettings.MaxLandscapeActors, PerformanceSettings.TerrainLODDistance);
}

void AWorld_PerformanceTerrainController::UpdateChunkStates(const FVector& PlayerLocation)
{
    // Update chunk activation states based on player proximity
    StreamTerrainChunks(PlayerLocation);
}

void AWorld_PerformanceTerrainController::ManageTerrainMemory()
{
    // Clean up invalid landscape references
    ActiveLandscapes.RemoveAll([](ALandscapeProxy* Landscape)
    {
        return !IsValid(Landscape);
    });
    
    // Update active chunk count
    ActiveChunkCount = ActiveLandscapes.Num();
}

void AWorld_PerformanceTerrainController::ApplyPerformanceOptimizations()
{
    // Apply current performance settings to all active landscapes
    for (ALandscapeProxy* Landscape : ActiveLandscapes)
    {
        if (IsValid(Landscape))
        {
            // Apply LOD and culling settings
            // This would set specific landscape properties for optimization
        }
    }
    
    ManageTerrainMemory();
}