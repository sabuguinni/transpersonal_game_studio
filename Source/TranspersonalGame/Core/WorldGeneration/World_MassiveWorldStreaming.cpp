#include "World_MassiveWorldStreaming.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "HAL/PlatformFilemanager.h"

AWorld_MassiveWorldStreaming::AWorld_MassiveWorldStreaming()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    PlayerPosition = FVector::ZeroVector;
    TimeSinceLastUpdate = 0.0f;
    CurrentLoadedChunks = 0;
    
    PlayerController = nullptr;
    PlayerPawn = nullptr;
}

void AWorld_MassiveWorldStreaming::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player controller and pawn
    PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        PlayerPawn = PlayerController->GetPawn();
    }
    
    // Initialize world chunks
    InitializeWorldChunks();
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldStreaming: Initialized with %d chunks"), WorldChunks.Num());
}

void AWorld_MassiveWorldStreaming::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= (1.0f / StreamingSettings.UpdateFrequency))
    {
        // Update player position
        if (PlayerPawn)
        {
            UpdatePlayerPosition(PlayerPawn->GetActorLocation());
        }
        
        // Update streaming state
        UpdateStreamingState();
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void AWorld_MassiveWorldStreaming::InitializeWorldChunks()
{
    WorldChunks.Empty();
    
    // Create 20x20 grid of chunks for 200km2 world
    // Each chunk is 10km x 10km (1,000,000 UU)
    const int32 ChunksPerSide = 20;
    const float ChunkSize = 1000000.0f; // 10km
    const float WorldSize = ChunksPerSide * ChunkSize; // 200km
    const float StartOffset = -WorldSize * 0.5f; // Center the world
    
    for (int32 X = 0; X < ChunksPerSide; X++)
    {
        for (int32 Y = 0; Y < ChunksPerSide; Y++)
        {
            FWorld_StreamingChunk NewChunk;
            NewChunk.ChunkCenter = FVector(
                StartOffset + (X * ChunkSize) + (ChunkSize * 0.5f),
                StartOffset + (Y * ChunkSize) + (ChunkSize * 0.5f),
                0.0f
            );
            NewChunk.ChunkSize = ChunkSize;
            
            // Determine biome type based on position
            FVector ChunkPos = NewChunk.ChunkCenter;
            if (ChunkPos.X < -5000000.0f && ChunkPos.Y < -3000000.0f)
            {
                NewChunk.BiomeType = EWorld_BiomeType::Swamp;
            }
            else if (ChunkPos.X < -3000000.0f && ChunkPos.Y > 3000000.0f)
            {
                NewChunk.BiomeType = EWorld_BiomeType::Forest;
            }
            else if (ChunkPos.X > 5000000.0f)
            {
                if (ChunkPos.Y > 4000000.0f)
                {
                    NewChunk.BiomeType = EWorld_BiomeType::Mountain;
                }
                else
                {
                    NewChunk.BiomeType = EWorld_BiomeType::Desert;
                }
            }
            else
            {
                NewChunk.BiomeType = EWorld_BiomeType::Savanna;
            }
            
            NewChunk.bIsLoaded = false;
            NewChunk.bIsVisible = false;
            NewChunk.DistanceToPlayer = 0.0f;
            NewChunk.ChunkPriority = 0;
            
            WorldChunks.Add(NewChunk);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldStreaming: Created %d world chunks"), WorldChunks.Num());
}

void AWorld_MassiveWorldStreaming::UpdatePlayerPosition(const FVector& NewPosition)
{
    PlayerPosition = NewPosition;
}

void AWorld_MassiveWorldStreaming::UpdateStreamingState()
{
    if (WorldChunks.Num() == 0)
    {
        return;
    }
    
    // Calculate distances to all chunks
    CalculateChunkDistances();
    
    // Prioritize chunks based on distance and biome importance
    PrioritizeChunks();
    
    // Process loading and unloading
    ProcessLoadQueue();
    ProcessUnloadQueue();
    
    // Update chunk visibility
    UpdateChunkVisibility();
    
    // Clean up unused resources if needed
    if (CurrentLoadedChunks > StreamingSettings.MaxLoadedChunks)
    {
        OptimizeMemoryUsage();
    }
}

void AWorld_MassiveWorldStreaming::CalculateChunkDistances()
{
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        FWorld_StreamingChunk& Chunk = WorldChunks[i];
        Chunk.DistanceToPlayer = FVector::Dist(PlayerPosition, Chunk.ChunkCenter);
    }
}

void AWorld_MassiveWorldStreaming::PrioritizeChunks()
{
    LoadQueue.Empty();
    UnloadQueue.Empty();
    
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        FWorld_StreamingChunk& Chunk = WorldChunks[i];
        
        // Calculate priority based on distance and biome type
        int32 BiomePriority = 1;
        switch (Chunk.BiomeType)
        {
            case EWorld_BiomeType::Savanna:
                BiomePriority = 5; // Highest priority (starting area)
                break;
            case EWorld_BiomeType::Forest:
                BiomePriority = 4;
                break;
            case EWorld_BiomeType::Swamp:
                BiomePriority = 3;
                break;
            case EWorld_BiomeType::Desert:
                BiomePriority = 2;
                break;
            case EWorld_BiomeType::Mountain:
                BiomePriority = 1;
                break;
        }
        
        // Distance-based priority (closer = higher priority)
        int32 DistancePriority = FMath::Max(1, 10 - (int32)(Chunk.DistanceToPlayer / 1000000.0f));
        
        Chunk.ChunkPriority = BiomePriority + DistancePriority;
        
        // Determine if chunk should be loaded/unloaded
        if (Chunk.DistanceToPlayer <= StreamingSettings.LoadDistance && !Chunk.bIsLoaded)
        {
            LoadQueue.Add(i);
        }
        else if (Chunk.DistanceToPlayer >= StreamingSettings.UnloadDistance && Chunk.bIsLoaded)
        {
            UnloadQueue.Add(i);
        }
    }
    
    // Sort queues by priority
    LoadQueue.Sort([this](const int32& A, const int32& B) {
        return WorldChunks[A].ChunkPriority > WorldChunks[B].ChunkPriority;
    });
    
    UnloadQueue.Sort([this](const int32& A, const int32& B) {
        return WorldChunks[A].ChunkPriority < WorldChunks[B].ChunkPriority;
    });
}

void AWorld_MassiveWorldStreaming::ProcessLoadQueue()
{
    int32 ChunksToLoad = FMath::Min(3, LoadQueue.Num()); // Load max 3 chunks per update
    
    for (int32 i = 0; i < ChunksToLoad; i++)
    {
        if (CurrentLoadedChunks >= StreamingSettings.MaxLoadedChunks)
        {
            break;
        }
        
        int32 ChunkIndex = LoadQueue[i];
        LoadChunk(ChunkIndex);
    }
}

void AWorld_MassiveWorldStreaming::ProcessUnloadQueue()
{
    int32 ChunksToUnload = FMath::Min(2, UnloadQueue.Num()); // Unload max 2 chunks per update
    
    for (int32 i = 0; i < ChunksToUnload; i++)
    {
        int32 ChunkIndex = UnloadQueue[i];
        UnloadChunk(ChunkIndex);
    }
}

void AWorld_MassiveWorldStreaming::LoadChunk(int32 ChunkIndex)
{
    if (ChunkIndex < 0 || ChunkIndex >= WorldChunks.Num())
    {
        return;
    }
    
    FWorld_StreamingChunk& Chunk = WorldChunks[ChunkIndex];
    
    if (Chunk.bIsLoaded)
    {
        return;
    }
    
    // Mark as loaded
    Chunk.bIsLoaded = true;
    CurrentLoadedChunks++;
    
    UE_LOG(LogTemp, Log, TEXT("World_MassiveWorldStreaming: Loaded chunk %d (%s biome) at %s"), 
           ChunkIndex, 
           *UEnum::GetValueAsString(Chunk.BiomeType),
           *Chunk.ChunkCenter.ToString());
}

void AWorld_MassiveWorldStreaming::UnloadChunk(int32 ChunkIndex)
{
    if (ChunkIndex < 0 || ChunkIndex >= WorldChunks.Num())
    {
        return;
    }
    
    FWorld_StreamingChunk& Chunk = WorldChunks[ChunkIndex];
    
    if (!Chunk.bIsLoaded)
    {
        return;
    }
    
    // Mark as unloaded
    Chunk.bIsLoaded = false;
    Chunk.bIsVisible = false;
    CurrentLoadedChunks--;
    
    UE_LOG(LogTemp, Log, TEXT("World_MassiveWorldStreaming: Unloaded chunk %d (%s biome)"), 
           ChunkIndex, 
           *UEnum::GetValueAsString(Chunk.BiomeType));
}

void AWorld_MassiveWorldStreaming::UpdateChunkVisibility()
{
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        FWorld_StreamingChunk& Chunk = WorldChunks[i];
        
        if (Chunk.bIsLoaded)
        {
            bool bShouldBeVisible = Chunk.DistanceToPlayer <= StreamingSettings.VisibilityDistance;
            
            if (bShouldBeVisible != Chunk.bIsVisible)
            {
                SetChunkVisibility(i, bShouldBeVisible);
            }
        }
    }
}

void AWorld_MassiveWorldStreaming::SetChunkVisibility(int32 ChunkIndex, bool bVisible)
{
    if (ChunkIndex < 0 || ChunkIndex >= WorldChunks.Num())
    {
        return;
    }
    
    FWorld_StreamingChunk& Chunk = WorldChunks[ChunkIndex];
    Chunk.bIsVisible = bVisible;
    
    // In a full implementation, this would show/hide actual level streaming volumes
    // For now, just log the visibility change
    UE_LOG(LogTemp, VeryVerbose, TEXT("World_MassiveWorldStreaming: Chunk %d visibility set to %s"), 
           ChunkIndex, bVisible ? TEXT("true") : TEXT("false"));
}

TArray<int32> AWorld_MassiveWorldStreaming::GetNearbyChunks(const FVector& Position, float Radius)
{
    TArray<int32> NearbyChunks;
    
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        const FWorld_StreamingChunk& Chunk = WorldChunks[i];
        float Distance = FVector::Dist(Position, Chunk.ChunkCenter);
        
        if (Distance <= Radius)
        {
            NearbyChunks.Add(i);
        }
    }
    
    return NearbyChunks;
}

void AWorld_MassiveWorldStreaming::OptimizeMemoryUsage()
{
    // Force unload the furthest chunks if we're over the limit
    TArray<int32> LoadedChunks;
    
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        if (WorldChunks[i].bIsLoaded)
        {
            LoadedChunks.Add(i);
        }
    }
    
    // Sort by distance (furthest first)
    LoadedChunks.Sort([this](const int32& A, const int32& B) {
        return WorldChunks[A].DistanceToPlayer > WorldChunks[B].DistanceToPlayer;
    });
    
    // Unload excess chunks
    int32 ChunksToUnload = CurrentLoadedChunks - StreamingSettings.MaxLoadedChunks;
    for (int32 i = 0; i < ChunksToUnload && i < LoadedChunks.Num(); i++)
    {
        UnloadChunk(LoadedChunks[i]);
    }
    
    // Force garbage collection
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldStreaming: Optimized memory usage, unloaded %d chunks"), ChunksToUnload);
}

FWorld_StreamingChunk AWorld_MassiveWorldStreaming::GetChunkAtPosition(const FVector& Position)
{
    float ClosestDistance = FLT_MAX;
    int32 ClosestChunkIndex = -1;
    
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        float Distance = FVector::Dist(Position, WorldChunks[i].ChunkCenter);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestChunkIndex = i;
        }
    }
    
    if (ClosestChunkIndex >= 0)
    {
        return WorldChunks[ClosestChunkIndex];
    }
    
    return FWorld_StreamingChunk();
}

void AWorld_MassiveWorldStreaming::ForceLoadBiome(EWorld_BiomeType BiomeType)
{
    for (int32 i = 0; i < WorldChunks.Num(); i++)
    {
        if (WorldChunks[i].BiomeType == BiomeType && !WorldChunks[i].bIsLoaded)
        {
            LoadChunk(i);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldStreaming: Force loaded all chunks of biome %s"), 
           *UEnum::GetValueAsString(BiomeType));
}

void AWorld_MassiveWorldStreaming::PreloadPlayerPath(const TArray<FVector>& PathPoints)
{
    const float PreloadRadius = StreamingSettings.LoadDistance * 0.5f;
    
    for (const FVector& Point : PathPoints)
    {
        TArray<int32> NearbyChunks = GetNearbyChunks(Point, PreloadRadius);
        
        for (int32 ChunkIndex : NearbyChunks)
        {
            if (!WorldChunks[ChunkIndex].bIsLoaded)
            {
                LoadChunk(ChunkIndex);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldStreaming: Preloaded chunks along player path (%d points)"), PathPoints.Num());
}

void AWorld_MassiveWorldStreaming::CleanupUnusedResources()
{
    // Implementation for cleaning up unused resources
    // This would involve more complex memory management in a full implementation
}