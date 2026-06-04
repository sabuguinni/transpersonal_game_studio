#include "World_MilestoneWorldStreamer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"

AWorld_MilestoneWorldStreamer::AWorld_MilestoneWorldStreamer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for streaming updates

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    StreamingSettings = FWorld_StreamingSettings();
    PerformanceBudgetMs = 5.0f; // 5ms budget for streaming operations
    bAdaptiveQuality = true;
    
    PlayerPawn = nullptr;
    LastPlayerLocation = FVector::ZeroVector;
    TimeSinceLastUpdate = 0.0f;
    PerformanceAnalyzer = nullptr;
}

void AWorld_MilestoneWorldStreamer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize streaming system
    InitializeStreaming();
    
    // Find player pawn
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
    
    // Try to find performance analyzer
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AWorld_MilestoneWorldStreamer::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor != this)
            {
                // Look for performance analyzer component
                if (UPerf_MilestonePerformanceAnalyzer* Analyzer = Actor->FindComponentByClass<UPerf_MilestonePerformanceAnalyzer>())
                {
                    IntegrateWithPerformanceAnalyzer(Analyzer);
                    break;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MilestoneWorldStreamer: Initialized with %d chunks"), StreamingChunks.Num());
}

void AWorld_MilestoneWorldStreamer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    // Update streaming based on frequency setting
    if (TimeSinceLastUpdate >= StreamingSettings.UpdateFrequency)
    {
        UpdateStreaming();
        TimeSinceLastUpdate = 0.0f;
    }
}

void AWorld_MilestoneWorldStreamer::InitializeStreaming()
{
    // Create default chunk grid if none exists
    if (StreamingChunks.Num() == 0)
    {
        CreateChunkGrid(3, 1000.0f); // 3x3 grid with 1000 unit chunks
    }
    
    // Initialize all chunks as unloaded
    for (FWorld_StreamingChunk& Chunk : StreamingChunks)
    {
        Chunk.bIsLoaded = false;
        Chunk.bIsVisible = false;
        Chunk.LODLevel = 0;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MilestoneWorldStreamer: Streaming initialized"));
}

void AWorld_MilestoneWorldStreamer::UpdateStreaming()
{
    // Update player location
    UpdatePlayerLocation();
    
    // Update chunk distances
    UpdateChunkDistances();
    
    // Process loading/unloading based on performance budget
    if (IsWithinPerformanceBudget())
    {
        ProcessChunkLoading();
        ProcessChunkUnloading();
    }
    
    // Adaptive quality based on performance
    if (bAdaptiveQuality && PerformanceAnalyzer)
    {
        AdaptQualityForPerformance();
    }
}

void AWorld_MilestoneWorldStreamer::LoadChunk(int32 ChunkIndex)
{
    if (!StreamingChunks.IsValidIndex(ChunkIndex))
    {
        return;
    }
    
    FWorld_StreamingChunk& Chunk = StreamingChunks[ChunkIndex];
    
    if (!Chunk.bIsLoaded)
    {
        // Mark as loaded
        Chunk.bIsLoaded = true;
        Chunk.bIsVisible = true;
        
        // Set appropriate LOD based on distance
        if (Chunk.DistanceToPlayer < 500.0f)
        {
            Chunk.LODLevel = 0; // Highest quality
        }
        else if (Chunk.DistanceToPlayer < 1500.0f)
        {
            Chunk.LODLevel = 1; // Medium quality
        }
        else
        {
            Chunk.LODLevel = 2; // Lowest quality
        }
        
        UE_LOG(LogTemp, Log, TEXT("Loaded chunk %d at distance %.1f with LOD %d"), 
               ChunkIndex, Chunk.DistanceToPlayer, Chunk.LODLevel);
    }
}

void AWorld_MilestoneWorldStreamer::UnloadChunk(int32 ChunkIndex)
{
    if (!StreamingChunks.IsValidIndex(ChunkIndex))
    {
        return;
    }
    
    FWorld_StreamingChunk& Chunk = StreamingChunks[ChunkIndex];
    
    if (Chunk.bIsLoaded)
    {
        // Mark as unloaded
        Chunk.bIsLoaded = false;
        Chunk.bIsVisible = false;
        Chunk.LODLevel = 0;
        
        UE_LOG(LogTemp, Log, TEXT("Unloaded chunk %d"), ChunkIndex);
    }
}

void AWorld_MilestoneWorldStreamer::UpdateChunkLOD(int32 ChunkIndex, int32 NewLODLevel)
{
    if (!StreamingChunks.IsValidIndex(ChunkIndex))
    {
        return;
    }
    
    FWorld_StreamingChunk& Chunk = StreamingChunks[ChunkIndex];
    
    if (Chunk.bIsLoaded && Chunk.LODLevel != NewLODLevel)
    {
        Chunk.LODLevel = FMath::Clamp(NewLODLevel, 0, 3);
        UE_LOG(LogTemp, Log, TEXT("Updated chunk %d LOD to %d"), ChunkIndex, Chunk.LODLevel);
    }
}

void AWorld_MilestoneWorldStreamer::IntegrateWithPerformanceAnalyzer(UPerf_MilestonePerformanceAnalyzer* Analyzer)
{
    PerformanceAnalyzer = Analyzer;
    
    if (PerformanceAnalyzer)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_MilestoneWorldStreamer: Integrated with Performance Analyzer"));
    }
}

void AWorld_MilestoneWorldStreamer::OptimizeForPerformance()
{
    if (!PerformanceAnalyzer)
    {
        return;
    }
    
    // Reduce active chunks if performance is poor
    int32 LoadedChunks = 0;
    for (const FWorld_StreamingChunk& Chunk : StreamingChunks)
    {
        if (Chunk.bIsLoaded)
        {
            LoadedChunks++;
        }
    }
    
    // If too many chunks loaded, unload distant ones
    if (LoadedChunks > StreamingSettings.MaxActiveChunks)
    {
        // Find most distant loaded chunk
        int32 MostDistantIndex = -1;
        float MaxDistance = 0.0f;
        
        for (int32 i = 0; i < StreamingChunks.Num(); ++i)
        {
            if (StreamingChunks[i].bIsLoaded && StreamingChunks[i].DistanceToPlayer > MaxDistance)
            {
                MaxDistance = StreamingChunks[i].DistanceToPlayer;
                MostDistantIndex = i;
            }
        }
        
        if (MostDistantIndex >= 0)
        {
            UnloadChunk(MostDistantIndex);
        }
    }
}

bool AWorld_MilestoneWorldStreamer::IsWithinPerformanceBudget() const
{
    if (PerformanceAnalyzer)
    {
        // Check if current frame time allows for streaming operations
        return true; // Simplified - would check actual performance metrics
    }
    
    return true;
}

void AWorld_MilestoneWorldStreamer::UpdatePlayerLocation()
{
    if (PlayerPawn)
    {
        LastPlayerLocation = PlayerPawn->GetActorLocation();
    }
    else if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
            if (PlayerPawn)
            {
                LastPlayerLocation = PlayerPawn->GetActorLocation();
            }
        }
    }
}

float AWorld_MilestoneWorldStreamer::GetDistanceToChunk(int32 ChunkIndex) const
{
    if (!StreamingChunks.IsValidIndex(ChunkIndex))
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(LastPlayerLocation, StreamingChunks[ChunkIndex].ChunkLocation);
}

int32 AWorld_MilestoneWorldStreamer::GetNearestChunkIndex() const
{
    int32 NearestIndex = -1;
    float MinDistance = FLT_MAX;
    
    for (int32 i = 0; i < StreamingChunks.Num(); ++i)
    {
        float Distance = GetDistanceToChunk(i);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestIndex = i;
        }
    }
    
    return NearestIndex;
}

TArray<int32> AWorld_MilestoneWorldStreamer::GetChunksInRange(float Range) const
{
    TArray<int32> ChunksInRange;
    
    for (int32 i = 0; i < StreamingChunks.Num(); ++i)
    {
        if (GetDistanceToChunk(i) <= Range)
        {
            ChunksInRange.Add(i);
        }
    }
    
    return ChunksInRange;
}

void AWorld_MilestoneWorldStreamer::CreateChunkGrid(int32 GridSize, float ChunkSize)
{
    StreamingChunks.Empty();
    
    int32 HalfGrid = GridSize / 2;
    
    for (int32 X = -HalfGrid; X <= HalfGrid; ++X)
    {
        for (int32 Y = -HalfGrid; Y <= HalfGrid; ++Y)
        {
            FWorld_StreamingChunk NewChunk;
            NewChunk.ChunkLocation = FVector(X * ChunkSize, Y * ChunkSize, 0.0f);
            NewChunk.ChunkSize = ChunkSize;
            
            StreamingChunks.Add(NewChunk);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created %dx%d chunk grid (%d total chunks)"), 
           GridSize, GridSize, StreamingChunks.Num());
}

void AWorld_MilestoneWorldStreamer::ValidateStreamingSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Streaming System Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total chunks: %d"), StreamingChunks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Load distance: %.1f"), StreamingSettings.LoadDistance);
    UE_LOG(LogTemp, Warning, TEXT("Max active chunks: %d"), StreamingSettings.MaxActiveChunks);
    UE_LOG(LogTemp, Warning, TEXT("Performance budget: %.1fms"), PerformanceBudgetMs);
    UE_LOG(LogTemp, Warning, TEXT("Player location: %s"), *LastPlayerLocation.ToString());
    
    int32 LoadedCount = 0;
    for (const FWorld_StreamingChunk& Chunk : StreamingChunks)
    {
        if (Chunk.bIsLoaded)
        {
            LoadedCount++;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Currently loaded chunks: %d"), LoadedCount);
}

void AWorld_MilestoneWorldStreamer::DebugDrawChunks()
{
    if (UWorld* World = GetWorld())
    {
        for (int32 i = 0; i < StreamingChunks.Num(); ++i)
        {
            const FWorld_StreamingChunk& Chunk = StreamingChunks[i];
            
            FColor ChunkColor = Chunk.bIsLoaded ? FColor::Green : FColor::Red;
            if (Chunk.bIsVisible && !Chunk.bIsLoaded)
            {
                ChunkColor = FColor::Yellow;
            }
            
            DrawDebugBox(World, Chunk.ChunkLocation, 
                        FVector(Chunk.ChunkSize * 0.5f), 
                        ChunkColor, false, 1.0f, 0, 5.0f);
            
            // Draw LOD level
            if (Chunk.bIsLoaded)
            {
                DrawDebugString(World, Chunk.ChunkLocation + FVector(0, 0, 100), 
                               FString::Printf(TEXT("LOD%d"), Chunk.LODLevel), 
                               nullptr, FColor::White, 1.0f);
            }
        }
    }
}

FString AWorld_MilestoneWorldStreamer::GetStreamingStatus() const
{
    int32 LoadedCount = 0;
    int32 VisibleCount = 0;
    
    for (const FWorld_StreamingChunk& Chunk : StreamingChunks)
    {
        if (Chunk.bIsLoaded) LoadedCount++;
        if (Chunk.bIsVisible) VisibleCount++;
    }
    
    return FString::Printf(TEXT("Chunks: %d total, %d loaded, %d visible"), 
                          StreamingChunks.Num(), LoadedCount, VisibleCount);
}

void AWorld_MilestoneWorldStreamer::UpdateChunkDistances()
{
    for (FWorld_StreamingChunk& Chunk : StreamingChunks)
    {
        Chunk.DistanceToPlayer = FVector::Dist(LastPlayerLocation, Chunk.ChunkLocation);
    }
}

void AWorld_MilestoneWorldStreamer::ProcessChunkLoading()
{
    TArray<int32> ChunksToLoad = GetChunksInRange(StreamingSettings.LoadDistance);
    
    int32 LoadedCount = 0;
    for (const FWorld_StreamingChunk& Chunk : StreamingChunks)
    {
        if (Chunk.bIsLoaded) LoadedCount++;
    }
    
    for (int32 ChunkIndex : ChunksToLoad)
    {
        if (LoadedCount >= StreamingSettings.MaxActiveChunks)
        {
            break;
        }
        
        if (ShouldLoadChunk(ChunkIndex))
        {
            LoadChunk(ChunkIndex);
            LoadedCount++;
        }
    }
}

void AWorld_MilestoneWorldStreamer::ProcessChunkUnloading()
{
    for (int32 i = 0; i < StreamingChunks.Num(); ++i)
    {
        if (ShouldUnloadChunk(i))
        {
            UnloadChunk(i);
        }
    }
}

void AWorld_MilestoneWorldStreamer::AdaptQualityForPerformance()
{
    if (!PerformanceAnalyzer)
    {
        return;
    }
    
    // Simplified performance adaptation
    // In real implementation, would check actual FPS and adjust LOD accordingly
    for (int32 i = 0; i < StreamingChunks.Num(); ++i)
    {
        FWorld_StreamingChunk& Chunk = StreamingChunks[i];
        
        if (Chunk.bIsLoaded)
        {
            // Adjust LOD based on distance and performance
            int32 NewLOD = 0;
            if (Chunk.DistanceToPlayer > 1000.0f)
            {
                NewLOD = 2;
            }
            else if (Chunk.DistanceToPlayer > 500.0f)
            {
                NewLOD = 1;
            }
            
            UpdateChunkLOD(i, NewLOD);
        }
    }
}

bool AWorld_MilestoneWorldStreamer::ShouldLoadChunk(int32 ChunkIndex) const
{
    if (!StreamingChunks.IsValidIndex(ChunkIndex))
    {
        return false;
    }
    
    const FWorld_StreamingChunk& Chunk = StreamingChunks[ChunkIndex];
    return !Chunk.bIsLoaded && Chunk.DistanceToPlayer <= StreamingSettings.LoadDistance;
}

bool AWorld_MilestoneWorldStreamer::ShouldUnloadChunk(int32 ChunkIndex) const
{
    if (!StreamingChunks.IsValidIndex(ChunkIndex))
    {
        return false;
    }
    
    const FWorld_StreamingChunk& Chunk = StreamingChunks[ChunkIndex];
    return Chunk.bIsLoaded && Chunk.DistanceToPlayer >= StreamingSettings.UnloadDistance;
}