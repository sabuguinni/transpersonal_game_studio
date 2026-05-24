#include "World_StreamingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UWorld_StreamingManager::UWorld_StreamingManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 times per second
    
    StreamingGridSize = 20;           // 20x20 grid
    StreamingCellSize = 100000.0f;    // 1km cells
    StreamingRadius = 500000.0f;      // 5km streaming radius
    MaxDrawCallsPerFrame = 2000;
    MaxMemoryUsageMB = 4096.0f;
    UpdateFrequency = 0.1f;
    
    LastPlayerLocation = FVector::ZeroVector;
    TimeSinceLastUpdate = 0.0f;
}

void UWorld_StreamingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize streaming grid
    InitializeStreamingGrid(StreamingGridSize, StreamingCellSize);
    
    UE_LOG(LogTemp, Warning, TEXT("World_StreamingManager initialized with %dx%d grid"), 
           StreamingGridSize, StreamingGridSize);
}

void UWorld_StreamingManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        // Get player location
        UWorld* World = GetWorld();
        if (World && World->GetFirstPlayerController())
        {
            APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn)
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                UpdatePlayerLocation(PlayerLocation);
            }
        }
        
        // Update performance metrics
        UpdateRenderingBudget();
        OptimizeMemoryUsage();
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void UWorld_StreamingManager::InitializeStreamingGrid(int32 GridSize, float CellSize)
{
    StreamingGridSize = GridSize;
    StreamingCellSize = CellSize;
    StreamingCells.Empty();
    
    // Create grid of streaming cells
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FIntPoint CellCoords(X, Y);
            FWorld_StreamingCell NewCell;
            
            NewCell.CellLocation = CellCoordsToWorldLocation(CellCoords);
            NewCell.CellSize = CellSize;
            NewCell.bIsLoaded = false;
            NewCell.bIsVisible = false;
            
            // Assign biomes based on position
            if (X < GridSize / 3)
            {
                NewCell.BiomeType = EBiomeType::Forest;
            }
            else if (X < 2 * GridSize / 3)
            {
                if (Y < GridSize / 2)
                {
                    NewCell.BiomeType = EBiomeType::Swamp;
                }
                else
                {
                    NewCell.BiomeType = EBiomeType::Savanna;
                }
            }
            else
            {
                if (Y < GridSize / 2)
                {
                    NewCell.BiomeType = EBiomeType::Desert;
                }
                else
                {
                    NewCell.BiomeType = EBiomeType::Mountain;
                }
            }
            
            StreamingCells.Add(CellCoords, NewCell);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Streaming grid initialized: %d cells"), StreamingCells.Num());
}

void UWorld_StreamingManager::UpdatePlayerLocation(const FVector& PlayerLocation)
{
    FVector LocationDelta = PlayerLocation - LastPlayerLocation;
    float MovementDistance = LocationDelta.Size();
    
    // Only update if player moved significantly
    if (MovementDistance > StreamingCellSize * 0.1f) // 10% of cell size
    {
        LastPlayerLocation = PlayerLocation;
        
        // Update cell loading/unloading based on distance
        for (auto& CellPair : StreamingCells)
        {
            FIntPoint CellCoords = CellPair.Key;
            FWorld_StreamingCell& Cell = CellPair.Value;
            
            float DistanceToPlayer = FVector::Dist(Cell.CellLocation, PlayerLocation);
            
            // Load/unload cells based on streaming radius
            if (DistanceToPlayer <= StreamingRadius)
            {
                if (!Cell.bIsLoaded)
                {
                    LoadCell(CellCoords.X, CellCoords.Y);
                }
                
                // Update LOD based on distance
                UpdateCellLOD(CellCoords.X, CellCoords.Y, DistanceToPlayer);
            }
            else if (Cell.bIsLoaded && DistanceToPlayer > StreamingRadius * 1.2f) // Hysteresis
            {
                UnloadCell(CellCoords.X, CellCoords.Y);
            }
        }
    }
}

void UWorld_StreamingManager::LoadCell(int32 CellX, int32 CellY)
{
    FIntPoint CellCoords(CellX, CellY);
    FWorld_StreamingCell* Cell = StreamingCells.Find(CellCoords);
    
    if (Cell && !Cell->bIsLoaded)
    {
        CreateCellActors(*Cell);
        Cell->bIsLoaded = true;
        Cell->bIsVisible = true;
        
        UE_LOG(LogTemp, Log, TEXT("Loaded streaming cell (%d, %d) - Biome: %d"), 
               CellX, CellY, (int32)Cell->BiomeType);
    }
}

void UWorld_StreamingManager::UnloadCell(int32 CellX, int32 CellY)
{
    FIntPoint CellCoords(CellX, CellY);
    FWorld_StreamingCell* Cell = StreamingCells.Find(CellCoords);
    
    if (Cell && Cell->bIsLoaded)
    {
        DestroyCellActors(*Cell);
        Cell->bIsLoaded = false;
        Cell->bIsVisible = false;
        
        UE_LOG(LogTemp, Log, TEXT("Unloaded streaming cell (%d, %d)"), CellX, CellY);
    }
}

void UWorld_StreamingManager::UpdateCellLOD(int32 CellX, int32 CellY, float DistanceToPlayer)
{
    FIntPoint CellCoords(CellX, CellY);
    FWorld_StreamingCell* Cell = StreamingCells.Find(CellCoords);
    
    if (Cell && Cell->bIsLoaded)
    {
        for (AActor* Actor : Cell->CellActors)
        {
            if (Actor)
            {
                UpdateActorLOD(Actor, DistanceToPlayer);
            }
        }
    }
}

void UWorld_StreamingManager::SetCellBiome(int32 CellX, int32 CellY, EBiomeType BiomeType)
{
    FIntPoint CellCoords(CellX, CellY);
    FWorld_StreamingCell* Cell = StreamingCells.Find(CellCoords);
    
    if (Cell)
    {
        Cell->BiomeType = BiomeType;
        
        // Reload cell if it's currently loaded
        if (Cell->bIsLoaded)
        {
            UnloadCell(CellX, CellY);
            LoadCell(CellX, CellY);
        }
    }
}

EBiomeType UWorld_StreamingManager::GetCellBiome(int32 CellX, int32 CellY) const
{
    FIntPoint CellCoords(CellX, CellY);
    const FWorld_StreamingCell* Cell = StreamingCells.Find(CellCoords);
    
    return Cell ? Cell->BiomeType : EBiomeType::Forest;
}

void UWorld_StreamingManager::OptimizeMemoryUsage()
{
    float CurrentMemoryMB = GetMemoryUsageMB();
    
    if (CurrentMemoryMB > MaxMemoryUsageMB)
    {
        // Find furthest loaded cells and unload them
        TArray<TPair<float, FIntPoint>> CellDistances;
        
        for (const auto& CellPair : StreamingCells)
        {
            const FWorld_StreamingCell& Cell = CellPair.Value;
            if (Cell.bIsLoaded)
            {
                float Distance = FVector::Dist(Cell.CellLocation, LastPlayerLocation);
                CellDistances.Add(TPair<float, FIntPoint>(Distance, CellPair.Key));
            }
        }
        
        // Sort by distance (furthest first)
        CellDistances.Sort([](const TPair<float, FIntPoint>& A, const TPair<float, FIntPoint>& B) {
            return A.Key > B.Key;
        });
        
        // Unload furthest cells until memory is under budget
        int32 CellsUnloaded = 0;
        for (const auto& CellDistance : CellDistances)
        {
            if (GetMemoryUsageMB() <= MaxMemoryUsageMB * 0.9f) // 90% of budget
                break;
                
            UnloadCell(CellDistance.Value.X, CellDistance.Value.Y);
            CellsUnloaded++;
        }
        
        if (CellsUnloaded > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Memory optimization: unloaded %d cells"), CellsUnloaded);
        }
    }
}

void UWorld_StreamingManager::UpdateRenderingBudget()
{
    int32 CurrentDrawCalls = GetActiveDrawCalls();
    
    if (CurrentDrawCalls > MaxDrawCallsPerFrame)
    {
        // Reduce LOD for distant objects
        for (auto& CellPair : StreamingCells)
        {
            FWorld_StreamingCell& Cell = CellPair.Value;
            if (Cell.bIsLoaded)
            {
                float Distance = FVector::Dist(Cell.CellLocation, LastPlayerLocation);
                if (Distance > LODSettings.MediumDetailDistance)
                {
                    UpdateCellLOD(CellPair.Key.X, CellPair.Key.Y, Distance * 1.5f); // Force lower LOD
                }
            }
        }
    }
}

int32 UWorld_StreamingManager::GetActiveDrawCalls() const
{
    // Simplified draw call estimation
    int32 DrawCalls = 0;
    
    for (const auto& CellPair : StreamingCells)
    {
        const FWorld_StreamingCell& Cell = CellPair.Value;
        if (Cell.bIsLoaded && Cell.bIsVisible)
        {
            DrawCalls += Cell.CellActors.Num() * 2; // Estimate 2 draw calls per actor
        }
    }
    
    return DrawCalls;
}

float UWorld_StreamingManager::GetMemoryUsageMB() const
{
    // Simplified memory usage estimation
    float MemoryMB = 0.0f;
    
    for (const auto& CellPair : StreamingCells)
    {
        const FWorld_StreamingCell& Cell = CellPair.Value;
        if (Cell.bIsLoaded)
        {
            MemoryMB += Cell.CellActors.Num() * 5.0f; // Estimate 5MB per actor
        }
    }
    
    return MemoryMB;
}

void UWorld_StreamingManager::DebugDrawStreamingGrid()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (const auto& CellPair : StreamingCells)
    {
        const FWorld_StreamingCell& Cell = CellPair.Value;
        FColor CellColor = Cell.bIsLoaded ? FColor::Green : FColor::Red;
        
        FVector CellMin = Cell.CellLocation - FVector(Cell.CellSize * 0.5f, Cell.CellSize * 0.5f, 0);
        FVector CellMax = Cell.CellLocation + FVector(Cell.CellSize * 0.5f, Cell.CellSize * 0.5f, 1000);
        
        DrawDebugBox(World, Cell.CellLocation, FVector(Cell.CellSize * 0.5f, Cell.CellSize * 0.5f, 500), 
                     CellColor, false, 1.0f, 0, 100.0f);
    }
}

TArray<FString> UWorld_StreamingManager::GetStreamingStats() const
{
    TArray<FString> Stats;
    
    int32 LoadedCells = 0;
    int32 VisibleCells = 0;
    
    for (const auto& CellPair : StreamingCells)
    {
        const FWorld_StreamingCell& Cell = CellPair.Value;
        if (Cell.bIsLoaded) LoadedCells++;
        if (Cell.bIsVisible) VisibleCells++;
    }
    
    Stats.Add(FString::Printf(TEXT("Total Cells: %d"), StreamingCells.Num()));
    Stats.Add(FString::Printf(TEXT("Loaded Cells: %d"), LoadedCells));
    Stats.Add(FString::Printf(TEXT("Visible Cells: %d"), VisibleCells));
    Stats.Add(FString::Printf(TEXT("Draw Calls: %d"), GetActiveDrawCalls()));
    Stats.Add(FString::Printf(TEXT("Memory Usage: %.1f MB"), GetMemoryUsageMB()));
    
    return Stats;
}

FIntPoint UWorld_StreamingManager::WorldLocationToCellCoords(const FVector& WorldLocation) const
{
    int32 CellX = FMath::FloorToInt(WorldLocation.X / StreamingCellSize);
    int32 CellY = FMath::FloorToInt(WorldLocation.Y / StreamingCellSize);
    
    // Clamp to grid bounds
    CellX = FMath::Clamp(CellX, 0, StreamingGridSize - 1);
    CellY = FMath::Clamp(CellY, 0, StreamingGridSize - 1);
    
    return FIntPoint(CellX, CellY);
}

FVector UWorld_StreamingManager::CellCoordsToWorldLocation(const FIntPoint& CellCoords) const
{
    float WorldX = CellCoords.X * StreamingCellSize + (StreamingCellSize * 0.5f);
    float WorldY = CellCoords.Y * StreamingCellSize + (StreamingCellSize * 0.5f);
    
    return FVector(WorldX, WorldY, 0);
}

bool UWorld_StreamingManager::IsCellInRange(const FIntPoint& CellCoords, const FVector& PlayerLocation) const
{
    FVector CellLocation = CellCoordsToWorldLocation(CellCoords);
    float Distance = FVector::Dist(CellLocation, PlayerLocation);
    
    return Distance <= StreamingRadius;
}

void UWorld_StreamingManager::CreateCellActors(FWorld_StreamingCell& Cell)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    Cell.CellActors.Empty();
    
    // Create biome-appropriate actors
    int32 ActorCount = 0;
    FVector BaseLocation = Cell.CellLocation;
    
    switch (Cell.BiomeType)
    {
        case EBiomeType::Forest:
            ActorCount = 15; // Dense vegetation
            break;
        case EBiomeType::Swamp:
            ActorCount = 8;  // Sparse, wet terrain
            break;
        case EBiomeType::Savanna:
            ActorCount = 10; // Scattered trees
            break;
        case EBiomeType::Desert:
            ActorCount = 3;  // Very sparse
            break;
        case EBiomeType::Mountain:
            ActorCount = 5;  // Rocky terrain
            break;
        default:
            ActorCount = 8;
            break;
    }
    
    // Spawn placeholder actors (cubes for now)
    for (int32 i = 0; i < ActorCount; i++)
    {
        FVector SpawnLocation = BaseLocation + FVector(
            FMath::RandRange(-Cell.CellSize * 0.4f, Cell.CellSize * 0.4f),
            FMath::RandRange(-Cell.CellSize * 0.4f, Cell.CellSize * 0.4f),
            FMath::RandRange(0, 500)
        );
        
        AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (NewActor)
        {
            Cell.CellActors.Add(NewActor);
        }
    }
}

void UWorld_StreamingManager::DestroyCellActors(FWorld_StreamingCell& Cell)
{
    for (AActor* Actor : Cell.CellActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    Cell.CellActors.Empty();
}

void UWorld_StreamingManager::UpdateActorLOD(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor) return;
    
    // Determine LOD level based on distance
    int32 LODLevel = 0;
    
    if (DistanceToPlayer > LODSettings.CullDistance)
    {
        Actor->SetActorHiddenInGame(true);
        return;
    }
    else if (DistanceToPlayer > LODSettings.LowDetailDistance)
    {
        LODLevel = 3;
    }
    else if (DistanceToPlayer > LODSettings.MediumDetailDistance)
    {
        LODLevel = 2;
    }
    else if (DistanceToPlayer > LODSettings.HighDetailDistance)
    {
        LODLevel = 1;
    }
    
    Actor->SetActorHiddenInGame(false);
    
    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetForcedLodModel(LODLevel + 1);
        }
    }
}