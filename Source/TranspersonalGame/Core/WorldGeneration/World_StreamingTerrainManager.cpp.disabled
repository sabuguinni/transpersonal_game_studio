#include "World_StreamingTerrainManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

AWorld_StreamingTerrainManager::AWorld_StreamingTerrainManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    // Initialize default values
    WorldOrigin = FVector::ZeroVector;
    WorldSize = FVector2D(50000.0f, 50000.0f); // 50km x 50km world
    ChunkSize = 2000.0f; // 2km x 2km chunks
    ChunksPerSide = FMath::CeilToInt(WorldSize.X / ChunkSize);
    bUseWorldPartition = true;
    MaxChunksPerFrame = 3;
    PerformanceBudgetMs = 16.0f; // 16ms budget per frame
    
    LastUpdateTime = 0.0f;
    PlayerPawn = nullptr;
    
    // Initialize streaming settings
    StreamingSettings = FWorld_StreamingSettings();
}

void AWorld_StreamingTerrainManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager: Initializing massive terrain streaming system"));
    
    // Initialize the terrain grid
    InitializeTerrainGrid();
    
    // Find the player pawn
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
}

void AWorld_StreamingTerrainManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check if enough time has passed for update
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < StreamingSettings.UpdateFrequency)
    {
        return;
    }
    
    LastUpdateTime = CurrentTime;
    
    // Update streaming for player
    if (PlayerPawn)
    {
        UpdateStreamingForPlayer(PlayerPawn);
    }
    
    // Process pending loads and unloads with performance budget
    float StartTime = FPlatformTime::Seconds() * 1000.0f;
    int32 ProcessedChunks = 0;
    
    // Process pending loads first
    while (PendingLoads.Num() > 0 && ProcessedChunks < MaxChunksPerFrame)
    {
        FVector2D ChunkCoord = PendingLoads[0];
        PendingLoads.RemoveAt(0);
        
        LoadTerrainChunk(ChunkCoord);
        ProcessedChunks++;
        
        float ElapsedTime = (FPlatformTime::Seconds() * 1000.0f) - StartTime;
        if (ElapsedTime > PerformanceBudgetMs)
        {
            break;
        }
    }
    
    // Process pending unloads
    while (PendingUnloads.Num() > 0 && ProcessedChunks < MaxChunksPerFrame)
    {
        FVector2D ChunkCoord = PendingUnloads[0];
        PendingUnloads.RemoveAt(0);
        
        UnloadTerrainChunk(ChunkCoord);
        ProcessedChunks++;
        
        float ElapsedTime = (FPlatformTime::Seconds() * 1000.0f) - StartTime;
        if (ElapsedTime > PerformanceBudgetMs)
        {
            break;
        }
    }
}

void AWorld_StreamingTerrainManager::InitializeTerrainGrid()
{
    UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager: Initializing %dx%d terrain grid"), ChunksPerSide, ChunksPerSide);
    
    TerrainChunks.Empty();
    TerrainChunks.Reserve(ChunksPerSide * ChunksPerSide);
    
    // Create terrain chunk data for the entire world
    for (int32 X = 0; X < ChunksPerSide; X++)
    {
        for (int32 Y = 0; Y < ChunksPerSide; Y++)
        {
            FWorld_TerrainChunk NewChunk;
            NewChunk.ChunkLocation = FVector(
                WorldOrigin.X + (X * ChunkSize),
                WorldOrigin.Y + (Y * ChunkSize),
                WorldOrigin.Z
            );
            NewChunk.ChunkSize = FVector2D(ChunkSize, ChunkSize);
            NewChunk.LODLevel = 0;
            NewChunk.bIsLoaded = false;
            NewChunk.bIsVisible = false;
            
            // Assign biome types based on location (simple zoning)
            float DistanceFromCenter = FVector2D(NewChunk.ChunkLocation.X, NewChunk.ChunkLocation.Y).Size();
            if (DistanceFromCenter < 5000.0f)
            {
                NewChunk.BiomeType = EBiomeType::Temperate;
            }
            else if (DistanceFromCenter < 15000.0f)
            {
                NewChunk.BiomeType = EBiomeType::Desert;
            }
            else
            {
                NewChunk.BiomeType = EBiomeType::Tundra;
            }
            
            TerrainChunks.Add(NewChunk);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager: Created %d terrain chunks"), TerrainChunks.Num());
}

void AWorld_StreamingTerrainManager::UpdateStreamingForPlayer(APawn* InPlayerPawn)
{
    if (!InPlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = InPlayerPawn->GetActorLocation();
    FVector2D PlayerChunkCoord = WorldLocationToChunkCoord(PlayerLocation);
    
    // Get chunks that should be loaded
    TArray<FVector2D> VisibleChunks = GetVisibleChunks(PlayerLocation, StreamingSettings.LoadDistance);
    
    // Check which chunks need to be loaded
    for (const FVector2D& ChunkCoord : VisibleChunks)
    {
        if (!IsChunkLoaded(ChunkCoord) && !PendingLoads.Contains(ChunkCoord))
        {
            PendingLoads.Add(ChunkCoord);
        }
    }
    
    // Check which loaded chunks should be unloaded
    TArray<FVector2D> ChunksToUnload;
    for (const auto& LoadedChunk : LoadedChunks)
    {
        FVector2D ChunkCoord = LoadedChunk.Key;
        FVector ChunkWorldLocation = ChunkCoordToWorldLocation(ChunkCoord);
        float DistanceToPlayer = FVector::Dist(PlayerLocation, ChunkWorldLocation);
        
        if (DistanceToPlayer > StreamingSettings.UnloadDistance)
        {
            ChunksToUnload.Add(ChunkCoord);
        }
    }
    
    // Add chunks to unload queue
    for (const FVector2D& ChunkCoord : ChunksToUnload)
    {
        if (!PendingUnloads.Contains(ChunkCoord))
        {
            PendingUnloads.Add(ChunkCoord);
        }
    }
    
    // Update LOD levels for loaded chunks
    for (const auto& LoadedChunk : LoadedChunks)
    {
        FVector2D ChunkCoord = LoadedChunk.Key;
        FVector ChunkWorldLocation = ChunkCoordToWorldLocation(ChunkCoord);
        float DistanceToPlayer = FVector::Dist(PlayerLocation, ChunkWorldLocation);
        
        int32 NewLODLevel = 0;
        if (DistanceToPlayer > StreamingSettings.LODDistance2)
        {
            NewLODLevel = 2;
        }
        else if (DistanceToPlayer > StreamingSettings.LODDistance1)
        {
            NewLODLevel = 1;
        }
        
        UpdateChunkLOD(ChunkCoord, NewLODLevel);
    }
}

void AWorld_StreamingTerrainManager::LoadTerrainChunk(const FVector2D& ChunkCoord)
{
    if (IsChunkLoaded(ChunkCoord))
    {
        return;
    }
    
    // Check if we've reached the maximum number of loaded chunks
    if (LoadedChunks.Num() >= StreamingSettings.MaxLoadedChunks)
    {
        UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager: Maximum loaded chunks reached, cannot load chunk at %s"), *ChunkCoord.ToString());
        return;
    }
    
    FVector ChunkWorldLocation = ChunkCoordToWorldLocation(ChunkCoord);
    
    // Create a landscape proxy for this chunk
    if (UWorld* World = GetWorld())
    {
        // For now, create a simple static mesh as a placeholder
        // In a full implementation, this would create actual landscape geometry
        ALandscapeProxy* NewLandscape = World->SpawnActor<ALandscapeProxy>();
        if (NewLandscape)
        {
            NewLandscape->SetActorLocation(ChunkWorldLocation);
            NewLandscape->SetActorLabel(FString::Printf(TEXT("TerrainChunk_%d_%d"), (int32)ChunkCoord.X, (int32)ChunkCoord.Y));
            
            LoadedChunks.Add(ChunkCoord, NewLandscape);
            
            UE_LOG(LogTemp, Log, TEXT("StreamingTerrainManager: Loaded terrain chunk at %s"), *ChunkCoord.ToString());
        }
    }
}

void AWorld_StreamingTerrainManager::UnloadTerrainChunk(const FVector2D& ChunkCoord)
{
    if (ALandscapeProxy** FoundChunk = LoadedChunks.Find(ChunkCoord))
    {
        if (*FoundChunk && IsValid(*FoundChunk))
        {
            (*FoundChunk)->Destroy();
        }
        
        LoadedChunks.Remove(ChunkCoord);
        
        UE_LOG(LogTemp, Log, TEXT("StreamingTerrainManager: Unloaded terrain chunk at %s"), *ChunkCoord.ToString());
    }
}

void AWorld_StreamingTerrainManager::UpdateChunkLOD(const FVector2D& ChunkCoord, int32 NewLODLevel)
{
    if (ALandscapeProxy** FoundChunk = LoadedChunks.Find(ChunkCoord))
    {
        if (*FoundChunk && IsValid(*FoundChunk))
        {
            // Update LOD level for the landscape
            // This would involve updating the landscape component LOD settings
            // For now, we just log the change
            UE_LOG(LogTemp, VeryVerbose, TEXT("StreamingTerrainManager: Updated chunk %s to LOD %d"), *ChunkCoord.ToString(), NewLODLevel);
        }
    }
}

FVector2D AWorld_StreamingTerrainManager::WorldLocationToChunkCoord(const FVector& WorldLocation) const
{
    FVector RelativeLocation = WorldLocation - WorldOrigin;
    return FVector2D(
        FMath::FloorToInt(RelativeLocation.X / ChunkSize),
        FMath::FloorToInt(RelativeLocation.Y / ChunkSize)
    );
}

FVector AWorld_StreamingTerrainManager::ChunkCoordToWorldLocation(const FVector2D& ChunkCoord) const
{
    return FVector(
        WorldOrigin.X + (ChunkCoord.X * ChunkSize),
        WorldOrigin.Y + (ChunkCoord.Y * ChunkSize),
        WorldOrigin.Z
    );
}

bool AWorld_StreamingTerrainManager::IsChunkLoaded(const FVector2D& ChunkCoord) const
{
    return LoadedChunks.Contains(ChunkCoord);
}

void AWorld_StreamingTerrainManager::SetStreamingSettings(const FWorld_StreamingSettings& NewSettings)
{
    StreamingSettings = NewSettings;
    UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager: Updated streaming settings"));
}

TArray<FVector2D> AWorld_StreamingTerrainManager::GetVisibleChunks(const FVector& ViewLocation, float ViewDistance) const
{
    TArray<FVector2D> VisibleChunks;
    
    FVector2D CenterChunk = WorldLocationToChunkCoord(ViewLocation);
    int32 ChunkRadius = FMath::CeilToInt(ViewDistance / ChunkSize);
    
    for (int32 X = -ChunkRadius; X <= ChunkRadius; X++)
    {
        for (int32 Y = -ChunkRadius; Y <= ChunkRadius; Y++)
        {
            FVector2D ChunkCoord = CenterChunk + FVector2D(X, Y);
            
            // Check if chunk is within world bounds
            if (ChunkCoord.X >= 0 && ChunkCoord.X < ChunksPerSide &&
                ChunkCoord.Y >= 0 && ChunkCoord.Y < ChunksPerSide)
            {
                FVector ChunkWorldLocation = ChunkCoordToWorldLocation(ChunkCoord);
                float Distance = FVector::Dist(ViewLocation, ChunkWorldLocation);
                
                if (Distance <= ViewDistance)
                {
                    VisibleChunks.Add(ChunkCoord);
                }
            }
        }
    }
    
    return VisibleChunks;
}

void AWorld_StreamingTerrainManager::OptimizeMemoryUsage()
{
    // Force garbage collection
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager: Optimized memory usage, %d chunks loaded"), LoadedChunks.Num());
}

void AWorld_StreamingTerrainManager::AnalyzeStreamingPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STREAMING TERRAIN PERFORMANCE ANALYSIS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Loaded Chunks: %d / %d"), LoadedChunks.Num(), StreamingSettings.MaxLoadedChunks);
    UE_LOG(LogTemp, Warning, TEXT("Pending Loads: %d"), PendingLoads.Num());
    UE_LOG(LogTemp, Warning, TEXT("Pending Unloads: %d"), PendingUnloads.Num());
    UE_LOG(LogTemp, Warning, TEXT("World Size: %.0fx%.0f"), WorldSize.X, WorldSize.Y);
    UE_LOG(LogTemp, Warning, TEXT("Chunk Size: %.0f"), ChunkSize);
    UE_LOG(LogTemp, Warning, TEXT("Total Chunks: %d"), ChunksPerSide * ChunksPerSide);
    UE_LOG(LogTemp, Warning, TEXT("Performance Budget: %.2fms"), PerformanceBudgetMs);
}

void AWorld_StreamingTerrainManager::DebugDrawChunkGrid()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw chunk grid for debugging
    for (int32 X = 0; X <= ChunksPerSide; X++)
    {
        FVector Start = FVector(WorldOrigin.X + (X * ChunkSize), WorldOrigin.Y, WorldOrigin.Z + 100.0f);
        FVector End = FVector(WorldOrigin.X + (X * ChunkSize), WorldOrigin.Y + WorldSize.Y, WorldOrigin.Z + 100.0f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 10.0f, 0, 5.0f);
    }
    
    for (int32 Y = 0; Y <= ChunksPerSide; Y++)
    {
        FVector Start = FVector(WorldOrigin.X, WorldOrigin.Y + (Y * ChunkSize), WorldOrigin.Z + 100.0f);
        FVector End = FVector(WorldOrigin.X + WorldSize.X, WorldOrigin.Y + (Y * ChunkSize), WorldOrigin.Z + 100.0f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 10.0f, 0, 5.0f);
    }
    
    // Draw loaded chunks in green
    for (const auto& LoadedChunk : LoadedChunks)
    {
        FVector ChunkCenter = ChunkCoordToWorldLocation(LoadedChunk.Key) + FVector(ChunkSize * 0.5f, ChunkSize * 0.5f, 100.0f);
        DrawDebugBox(GetWorld(), ChunkCenter, FVector(ChunkSize * 0.4f, ChunkSize * 0.4f, 50.0f), FColor::Green, false, 10.0f, 0, 10.0f);
    }
}

void AWorld_StreamingTerrainManager::LogStreamingStats()
{
    UE_LOG(LogTemp, Warning, TEXT("StreamingTerrainManager Stats:"));
    UE_LOG(LogTemp, Warning, TEXT("- Loaded Chunks: %d"), LoadedChunks.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Pending Loads: %d"), PendingLoads.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Pending Unloads: %d"), PendingUnloads.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Total Terrain Chunks: %d"), TerrainChunks.Num());
}