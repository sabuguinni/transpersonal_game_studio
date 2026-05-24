#include "World_MassiveWorldController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AWorld_MassiveWorldController::AWorld_MassiveWorldController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for streaming updates

    // Initialize default settings
    WorldSize = 1000000.0f; // 10km x 10km world
    ChunkSize = 10000; // 100m x 100m chunks
    
    StreamingSettings.LoadRadius = 50000.0f;
    StreamingSettings.UnloadRadius = 75000.0f;
    StreamingSettings.MaxActiveChunks = 25;
    StreamingSettings.UpdateFrequency = 0.5f;
    StreamingSettings.bEnableAsyncLoading = true;

    // Initialize performance metrics
    ActiveChunksCount = 0;
    LoadedChunksCount = 0;
    StreamingPerformanceScore = 1.0f;
    LastUpdateTime = 0.0f;
    StreamingUpdateTimer = 0.0f;
    FramesSinceLastUpdate = 0;

    // Initialize component references
    BiomeManager = nullptr;
    TerrainManager = nullptr;
    StreamingManager = nullptr;
}

void AWorld_MassiveWorldController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldController: BeginPlay started"));
    
    // Initialize the massive world system
    InitializeMassiveWorld();
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldController: Massive world initialized"));
}

void AWorld_MassiveWorldController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all loaded chunks
    LoadedChunks.Empty();
    PendingLoadChunks.Empty();
    PendingUnloadChunks.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void AWorld_MassiveWorldController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    StreamingUpdateTimer += DeltaTime;
    FramesSinceLastUpdate++;
    
    // Update streaming based on frequency setting
    if (StreamingUpdateTimer >= StreamingSettings.UpdateFrequency)
    {
        // Get player location for streaming updates
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            UpdateWorldStreaming(PlayerLocation);
        }
        
        // Process pending chunk operations
        ProcessPendingChunkOperations();
        
        // Update performance metrics
        UpdatePerformanceMetrics();
        
        StreamingUpdateTimer = 0.0f;
        FramesSinceLastUpdate = 0;
    }
}

void AWorld_MassiveWorldController::InitializeMassiveWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing massive world system..."));
    
    // Create initial chunk grid around origin
    FVector OriginLocation = FVector::ZeroVector;
    int32 InitialChunkRadius = 2; // 5x5 grid of initial chunks
    
    for (int32 X = -InitialChunkRadius; X <= InitialChunkRadius; X++)
    {
        for (int32 Y = -InitialChunkRadius; Y <= InitialChunkRadius; Y++)
        {
            FIntVector ChunkCoord(X, Y, 0);
            FVector ChunkWorldLocation = ChunkCoordinateToWorldLocation(ChunkCoord);
            
            FWorld_WorldChunkData ChunkData;
            ChunkData.ChunkLocation = ChunkWorldLocation;
            ChunkData.ChunkSize = ChunkSize;
            ChunkData.BiomeType = EWorld_BiomeType::Temperate; // Default biome
            ChunkData.bIsLoaded = true;
            ChunkData.bIsActive = true;
            ChunkData.LoadPriority = 1.0f;
            
            LoadedChunks.Add(ChunkCoord, ChunkData);
        }
    }
    
    ActiveChunksCount = LoadedChunks.Num();
    LoadedChunksCount = LoadedChunks.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Massive world initialized with %d initial chunks"), LoadedChunks.Num());
}

void AWorld_MassiveWorldController::UpdateWorldStreaming(FVector PlayerLocation)
{
    if (!StreamingSettings.bEnableAsyncLoading)
    {
        return;
    }
    
    FIntVector PlayerChunk = WorldLocationToChunkCoordinate(PlayerLocation);
    
    // Calculate streaming radius in chunks
    int32 LoadRadiusChunks = FMath::CeilToInt(StreamingSettings.LoadRadius / ChunkSize);
    int32 UnloadRadiusChunks = FMath::CeilToInt(StreamingSettings.UnloadRadius / ChunkSize);
    
    // Check for chunks to load
    for (int32 X = PlayerChunk.X - LoadRadiusChunks; X <= PlayerChunk.X + LoadRadiusChunks; X++)
    {
        for (int32 Y = PlayerChunk.Y - LoadRadiusChunks; Y <= PlayerChunk.Y + LoadRadiusChunks; Y++)
        {
            FIntVector ChunkCoord(X, Y, 0);
            
            if (!LoadedChunks.Contains(ChunkCoord))
            {
                float Distance = FVector::Dist(ChunkCoordinateToWorldLocation(ChunkCoord), PlayerLocation);
                if (Distance <= StreamingSettings.LoadRadius && !PendingLoadChunks.Contains(ChunkCoord))
                {
                    PendingLoadChunks.Add(ChunkCoord);
                }
            }
        }
    }
    
    // Check for chunks to unload
    TArray<FIntVector> ChunksToUnload;
    for (auto& ChunkPair : LoadedChunks)
    {
        FIntVector ChunkCoord = ChunkPair.Key;
        float Distance = FVector::Dist(ChunkCoordinateToWorldLocation(ChunkCoord), PlayerLocation);
        
        if (Distance > StreamingSettings.UnloadRadius)
        {
            ChunksToUnload.Add(ChunkCoord);
        }
    }
    
    // Add to pending unload
    for (FIntVector ChunkCoord : ChunksToUnload)
    {
        if (!PendingUnloadChunks.Contains(ChunkCoord))
        {
            PendingUnloadChunks.Add(ChunkCoord);
        }
    }
}

void AWorld_MassiveWorldController::LoadChunkAtLocation(FVector Location)
{
    FIntVector ChunkCoord = WorldLocationToChunkCoordinate(Location);
    
    if (!LoadedChunks.Contains(ChunkCoord))
    {
        FWorld_WorldChunkData ChunkData;
        ChunkData.ChunkLocation = ChunkCoordinateToWorldLocation(ChunkCoord);
        ChunkData.ChunkSize = ChunkSize;
        ChunkData.BiomeType = EWorld_BiomeType::Temperate; // TODO: Calculate based on world rules
        ChunkData.bIsLoaded = true;
        ChunkData.bIsActive = true;
        ChunkData.LoadPriority = 1.0f;
        
        LoadedChunks.Add(ChunkCoord, ChunkData);
        LoadedChunksCount = LoadedChunks.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Loaded chunk at %s"), *ChunkCoord.ToString());
    }
}

void AWorld_MassiveWorldController::UnloadChunkAtLocation(FVector Location)
{
    FIntVector ChunkCoord = WorldLocationToChunkCoordinate(Location);
    
    if (LoadedChunks.Contains(ChunkCoord))
    {
        LoadedChunks.Remove(ChunkCoord);
        LoadedChunksCount = LoadedChunks.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Unloaded chunk at %s"), *ChunkCoord.ToString());
    }
}

FWorld_WorldChunkData AWorld_MassiveWorldController::GetChunkDataAtLocation(FVector Location)
{
    FIntVector ChunkCoord = WorldLocationToChunkCoordinate(Location);
    
    if (LoadedChunks.Contains(ChunkCoord))
    {
        return LoadedChunks[ChunkCoord];
    }
    
    return FWorld_WorldChunkData(); // Return default chunk data
}

TArray<FWorld_WorldChunkData> AWorld_MassiveWorldController::GetActiveChunks()
{
    TArray<FWorld_WorldChunkData> ActiveChunks;
    
    for (auto& ChunkPair : LoadedChunks)
    {
        if (ChunkPair.Value.bIsActive)
        {
            ActiveChunks.Add(ChunkPair.Value);
        }
    }
    
    return ActiveChunks;
}

void AWorld_MassiveWorldController::OptimizeStreamingPerformance()
{
    // Limit active chunks to max setting
    if (LoadedChunks.Num() > StreamingSettings.MaxActiveChunks)
    {
        // Find chunks with lowest priority and deactivate them
        TArray<TPair<FIntVector, float>> ChunkPriorities;
        
        for (auto& ChunkPair : LoadedChunks)
        {
            ChunkPriorities.Add(TPair<FIntVector, float>(ChunkPair.Key, ChunkPair.Value.LoadPriority));
        }
        
        // Sort by priority (lowest first)
        ChunkPriorities.Sort([](const TPair<FIntVector, float>& A, const TPair<FIntVector, float>& B) {
            return A.Value < B.Value;
        });
        
        // Deactivate lowest priority chunks
        int32 ChunksToDeactivate = LoadedChunks.Num() - StreamingSettings.MaxActiveChunks;
        for (int32 i = 0; i < ChunksToDeactivate && i < ChunkPriorities.Num(); i++)
        {
            FIntVector ChunkCoord = ChunkPriorities[i].Key;
            if (LoadedChunks.Contains(ChunkCoord))
            {
                LoadedChunks[ChunkCoord].bIsActive = false;
            }
        }
    }
    
    // Update active chunk count
    ActiveChunksCount = 0;
    for (auto& ChunkPair : LoadedChunks)
    {
        if (ChunkPair.Value.bIsActive)
        {
            ActiveChunksCount++;
        }
    }
}

void AWorld_MassiveWorldController::SetStreamingSettings(const FWorld_StreamingSettings& NewSettings)
{
    StreamingSettings = NewSettings;
    UE_LOG(LogTemp, Warning, TEXT("Updated streaming settings - Load Radius: %.0f, Max Chunks: %d"), 
           StreamingSettings.LoadRadius, StreamingSettings.MaxActiveChunks);
}

float AWorld_MassiveWorldController::CalculateStreamingPerformance()
{
    float Performance = 1.0f;
    
    // Factor in chunk count vs max
    if (StreamingSettings.MaxActiveChunks > 0)
    {
        float ChunkRatio = (float)ActiveChunksCount / (float)StreamingSettings.MaxActiveChunks;
        Performance *= FMath::Clamp(2.0f - ChunkRatio, 0.1f, 1.0f);
    }
    
    // Factor in update frequency
    if (FramesSinceLastUpdate > 0)
    {
        float FrameRatio = StreamingSettings.UpdateFrequency * 60.0f / FramesSinceLastUpdate;
        Performance *= FMath::Clamp(FrameRatio, 0.1f, 1.0f);
    }
    
    return FMath::Clamp(Performance, 0.0f, 1.0f);
}

void AWorld_MassiveWorldController::LogPerformanceMetrics()
{
    StreamingPerformanceScore = CalculateStreamingPerformance();
    
    UE_LOG(LogTemp, Warning, TEXT("=== MASSIVE WORLD PERFORMANCE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Chunks: %d / %d"), ActiveChunksCount, StreamingSettings.MaxActiveChunks);
    UE_LOG(LogTemp, Warning, TEXT("Loaded Chunks: %d"), LoadedChunksCount);
    UE_LOG(LogTemp, Warning, TEXT("Pending Load: %d"), PendingLoadChunks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Pending Unload: %d"), PendingUnloadChunks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), StreamingPerformanceScore);
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}

void AWorld_MassiveWorldController::DebugShowChunkBounds()
{
    if (!GetWorld())
    {
        return;
    }
    
    for (auto& ChunkPair : LoadedChunks)
    {
        FVector ChunkCenter = ChunkPair.Value.ChunkLocation;
        FVector ChunkExtent = FVector(ChunkSize * 0.5f, ChunkSize * 0.5f, 1000.0f);
        
        FColor ChunkColor = ChunkPair.Value.bIsActive ? FColor::Green : FColor::Yellow;
        
        DrawDebugBox(GetWorld(), ChunkCenter, ChunkExtent, ChunkColor, false, 5.0f, 0, 50.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Debug: Showing bounds for %d chunks"), LoadedChunks.Num());
}

void AWorld_MassiveWorldController::DebugClearAllChunks()
{
    LoadedChunks.Empty();
    PendingLoadChunks.Empty();
    PendingUnloadChunks.Empty();
    
    ActiveChunksCount = 0;
    LoadedChunksCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Debug: Cleared all chunks"));
}

void AWorld_MassiveWorldController::DebugGenerateTestWorld()
{
    DebugClearAllChunks();
    
    // Generate a 7x7 grid of test chunks
    for (int32 X = -3; X <= 3; X++)
    {
        for (int32 Y = -3; Y <= 3; Y++)
        {
            FIntVector ChunkCoord(X, Y, 0);
            FVector ChunkWorldLocation = ChunkCoordinateToWorldLocation(ChunkCoord);
            
            FWorld_WorldChunkData ChunkData;
            ChunkData.ChunkLocation = ChunkWorldLocation;
            ChunkData.ChunkSize = ChunkSize;
            ChunkData.BiomeType = static_cast<EWorld_BiomeType>(FMath::RandRange(0, 4)); // Random biome
            ChunkData.bIsLoaded = true;
            ChunkData.bIsActive = true;
            ChunkData.LoadPriority = FMath::RandRange(0.5f, 1.5f);
            
            LoadedChunks.Add(ChunkCoord, ChunkData);
        }
    }
    
    ActiveChunksCount = LoadedChunks.Num();
    LoadedChunksCount = LoadedChunks.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Debug: Generated test world with %d chunks"), LoadedChunks.Num());
}

FIntVector AWorld_MassiveWorldController::WorldLocationToChunkCoordinate(FVector WorldLocation)
{
    int32 ChunkX = FMath::FloorToInt(WorldLocation.X / ChunkSize);
    int32 ChunkY = FMath::FloorToInt(WorldLocation.Y / ChunkSize);
    return FIntVector(ChunkX, ChunkY, 0);
}

FVector AWorld_MassiveWorldController::ChunkCoordinateToWorldLocation(FIntVector ChunkCoordinate)
{
    float WorldX = ChunkCoordinate.X * ChunkSize + (ChunkSize * 0.5f);
    float WorldY = ChunkCoordinate.Y * ChunkSize + (ChunkSize * 0.5f);
    return FVector(WorldX, WorldY, 0.0f);
}

void AWorld_MassiveWorldController::ProcessPendingChunkOperations()
{
    // Process pending loads (limited per frame for performance)
    int32 LoadsThisFrame = 0;
    int32 MaxLoadsPerFrame = 3;
    
    for (int32 i = PendingLoadChunks.Num() - 1; i >= 0 && LoadsThisFrame < MaxLoadsPerFrame; i--)
    {
        FIntVector ChunkCoord = PendingLoadChunks[i];
        LoadChunkAtLocation(ChunkCoordinateToWorldLocation(ChunkCoord));
        PendingLoadChunks.RemoveAt(i);
        LoadsThisFrame++;
    }
    
    // Process pending unloads (limited per frame for performance)
    int32 UnloadsThisFrame = 0;
    int32 MaxUnloadsPerFrame = 5;
    
    for (int32 i = PendingUnloadChunks.Num() - 1; i >= 0 && UnloadsThisFrame < MaxUnloadsPerFrame; i--)
    {
        FIntVector ChunkCoord = PendingUnloadChunks[i];
        UnloadChunkAtLocation(ChunkCoordinateToWorldLocation(ChunkCoord));
        PendingUnloadChunks.RemoveAt(i);
        UnloadsThisFrame++;
    }
}

void AWorld_MassiveWorldController::UpdatePerformanceMetrics()
{
    // Optimize streaming if performance is degrading
    OptimizeStreamingPerformance();
    
    // Calculate current performance score
    StreamingPerformanceScore = CalculateStreamingPerformance();
    
    // Log performance warnings if needed
    if (StreamingPerformanceScore < 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Massive World: Performance degraded (%.2f) - optimizing..."), StreamingPerformanceScore);
    }
}