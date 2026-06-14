#include "World_TerrainLODSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

UWorld_TerrainLODSystem::UWorld_TerrainLODSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update
    
    UpdateFrequency = 0.1f;
    MaxActiveChunks = 64;
    bEnableDebugVisualization = false;
    PerformanceBudgetMS = 2.0f; // 2ms budget for terrain LOD
    MaxActorsPerFrame = 10;
    
    LastUpdateTime = 0.0f;
    LastPlayerLocation = FVector::ZeroVector;
    ProcessedChunksThisFrame = 0;
}

void UWorld_TerrainLODSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainLOD();
    
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainLODSystem: Initialized with %d chunks"), TerrainChunks.Num());
}

void UWorld_TerrainLODSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    ProcessedChunksThisFrame = 0;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (Player)
        {
            FVector PlayerLocation = Player->GetActorLocation();
            UpdateTerrainLOD(PlayerLocation);
            LastPlayerLocation = PlayerLocation;
        }
        
        LastUpdateTime = 0.0f;
    }
    
    if (bEnableDebugVisualization)
    {
        DebugDrawChunks();
    }
}

void UWorld_TerrainLODSystem::InitializeTerrainLOD()
{
    TerrainChunks.Empty();
    
    // Create initial terrain chunks in a grid pattern
    const int32 GridSize = 8; // 8x8 grid
    const int32 ChunkSize = 1000;
    const int32 HalfGrid = GridSize / 2;
    
    for (int32 X = -HalfGrid; X < HalfGrid; X++)
    {
        for (int32 Y = -HalfGrid; Y < HalfGrid; Y++)
        {
            FVector ChunkCenter(X * ChunkSize, Y * ChunkSize, 0);
            
            // Determine biome based on location
            EBiomeType BiomeType = EBiomeType::Forest;
            float DistanceFromOrigin = FVector::Dist2D(ChunkCenter, FVector::ZeroVector);
            
            if (DistanceFromOrigin < 1500.0f)
                BiomeType = EBiomeType::Forest;
            else if (ChunkCenter.X > 2000.0f && ChunkCenter.Y < 0.0f)
                BiomeType = EBiomeType::Canyon;
            else if (ChunkCenter.X < -1000.0f && ChunkCenter.Y > 1000.0f)
                BiomeType = EBiomeType::Swamp;
            else if (ChunkCenter.X > 2000.0f && ChunkCenter.Y > -1000.0f)
                BiomeType = EBiomeType::Plains;
            else if (DistanceFromOrigin > 5000.0f)
                BiomeType = EBiomeType::Volcanic;
            else
                BiomeType = EBiomeType::Tundra;
            
            CreateTerrainChunk(ChunkCenter, BiomeType, ChunkSize);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainLODSystem: Created %d terrain chunks"), TerrainChunks.Num());
}

void UWorld_TerrainLODSystem::UpdateTerrainLOD(const FVector& PlayerLocation)
{
    if (TerrainChunks.Num() == 0) return;
    
    double StartTime = FPlatformTime::Seconds();
    
    // Update distances and LOD levels for all chunks
    for (FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        Chunk.DistanceToPlayer = FVector::Dist(Chunk.ChunkCenter, PlayerLocation);
        
        int32 NewLODLevel = CalculateLODLevel(Chunk.DistanceToPlayer);
        
        if (NewLODLevel != Chunk.CurrentLOD)
        {
            ProcessChunkLOD(Chunk, NewLODLevel);
            ProcessedChunksThisFrame++;
            
            // Respect performance budget
            double ElapsedTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
            if (ElapsedTime > PerformanceBudgetMS || ProcessedChunksThisFrame >= MaxActorsPerFrame)
            {
                break;
            }
        }
    }
    
    // Cull distant chunks
    CullDistantChunks(PlayerLocation);
}

void UWorld_TerrainLODSystem::RegisterTerrainChunk(const FWorld_TerrainChunk& Chunk)
{
    TerrainChunks.Add(Chunk);
    UE_LOG(LogTemp, Log, TEXT("Registered terrain chunk at %s"), *Chunk.ChunkCenter.ToString());
}

void UWorld_TerrainLODSystem::UnregisterTerrainChunk(const FVector& ChunkCenter)
{
    for (int32 i = TerrainChunks.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(TerrainChunks[i].ChunkCenter, ChunkCenter) < 100.0f)
        {
            ClearChunkActors(TerrainChunks[i]);
            TerrainChunks.RemoveAt(i);
            break;
        }
    }
}

int32 UWorld_TerrainLODSystem::CalculateLODLevel(float Distance) const
{
    if (Distance <= LODSettings.LOD0Distance) return 0;
    if (Distance <= LODSettings.LOD1Distance) return 1;
    if (Distance <= LODSettings.LOD2Distance) return 2;
    if (Distance <= LODSettings.LOD3Distance) return 3;
    if (Distance <= LODSettings.CullDistance) return 4;
    return 5; // Culled
}

void UWorld_TerrainLODSystem::CreateTerrainChunk(const FVector& Center, EBiomeType BiomeType, int32 Size)
{
    FWorld_TerrainChunk NewChunk;
    NewChunk.ChunkCenter = Center;
    NewChunk.ChunkSize = Size;
    NewChunk.BiomeType = BiomeType;
    NewChunk.CurrentLOD = -1; // Force initial population
    NewChunk.bIsActive = true;
    
    RegisterTerrainChunk(NewChunk);
}

void UWorld_TerrainLODSystem::DestroyTerrainChunk(const FVector& ChunkCenter)
{
    UnregisterTerrainChunk(ChunkCenter);
}

TArray<FWorld_TerrainChunk> UWorld_TerrainLODSystem::GetActiveChunks() const
{
    TArray<FWorld_TerrainChunk> ActiveChunks;
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.bIsActive && Chunk.CurrentLOD < 5)
        {
            ActiveChunks.Add(Chunk);
        }
    }
    return ActiveChunks;
}

FWorld_TerrainChunk* UWorld_TerrainLODSystem::GetChunkAtLocation(const FVector& Location)
{
    for (FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        float Distance = FVector::Dist2D(Chunk.ChunkCenter, Location);
        if (Distance <= Chunk.ChunkSize * 0.5f)
        {
            return &Chunk;
        }
    }
    return nullptr;
}

void UWorld_TerrainLODSystem::ApplyBiomeLOD(const FWorld_TerrainChunk& Chunk, int32 LODLevel)
{
    // Apply biome-specific LOD settings
    switch (Chunk.BiomeType)
    {
    case EBiomeType::Forest:
        // Dense vegetation at close range, sparse at distance
        break;
    case EBiomeType::Plains:
        // Grass and scattered trees
        break;
    case EBiomeType::Swamp:
        // Water effects and hanging moss
        break;
    case EBiomeType::Canyon:
        // Rock formations and sparse vegetation
        break;
    case EBiomeType::Volcanic:
        // Lava effects and volcanic rocks
        break;
    case EBiomeType::Tundra:
        // Snow and ice effects
        break;
    }
}

void UWorld_TerrainLODSystem::PopulateChunkWithBiome(FWorld_TerrainChunk& Chunk)
{
    if (!GetWorld()) return;
    
    // Clear existing actors
    ClearChunkActors(Chunk);
    
    // Spawn biome-appropriate content based on LOD level
    int32 VegetationCount = FMath::Max(1, LODSettings.MaxVegetationPerChunk / (Chunk.CurrentLOD + 1));
    int32 RockCount = FMath::Max(1, LODSettings.MaxRocksPerChunk / (Chunk.CurrentLOD + 1));
    
    SpawnVegetationForChunk(Chunk, Chunk.CurrentLOD);
    SpawnRocksForChunk(Chunk, Chunk.CurrentLOD);
}

void UWorld_TerrainLODSystem::CullDistantChunks(const FVector& PlayerLocation)
{
    for (FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.DistanceToPlayer > LODSettings.CullDistance)
        {
            if (Chunk.bIsActive)
            {
                ClearChunkActors(Chunk);
                Chunk.bIsActive = false;
                Chunk.CurrentLOD = 5; // Culled
            }
        }
        else if (!Chunk.bIsActive && Chunk.DistanceToPlayer <= LODSettings.CullDistance)
        {
            Chunk.bIsActive = true;
            Chunk.CurrentLOD = -1; // Force repopulation
        }
    }
}

void UWorld_TerrainLODSystem::GetLODStatistics(int32& ActiveChunks, int32& TotalActors, float& AverageDistance) const
{
    ActiveChunks = 0;
    TotalActors = 0;
    float TotalDistance = 0.0f;
    
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.bIsActive)
        {
            ActiveChunks++;
            TotalActors += Chunk.TerrainActors.Num();
            TotalDistance += Chunk.DistanceToPlayer;
        }
    }
    
    AverageDistance = ActiveChunks > 0 ? TotalDistance / ActiveChunks : 0.0f;
}

void UWorld_TerrainLODSystem::SetLODSettings(const FWorld_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Warning, TEXT("Updated LOD settings - Cull distance: %.1f"), LODSettings.CullDistance);
}

FWorld_LODSettings UWorld_TerrainLODSystem::GetLODSettings() const
{
    return LODSettings;
}

void UWorld_TerrainLODSystem::DebugDrawChunks()
{
    if (!GetWorld()) return;
    
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        FColor ChunkColor = FColor::Green;
        
        switch (Chunk.CurrentLOD)
        {
        case 0: ChunkColor = FColor::Green; break;
        case 1: ChunkColor = FColor::Yellow; break;
        case 2: ChunkColor = FColor::Orange; break;
        case 3: ChunkColor = FColor::Red; break;
        case 4: ChunkColor = FColor::Purple; break;
        default: ChunkColor = FColor::Black; break;
        }
        
        if (!Chunk.bIsActive) ChunkColor = FColor::Black;
        
        DrawDebugBox(GetWorld(), Chunk.ChunkCenter, 
                    FVector(Chunk.ChunkSize * 0.5f, Chunk.ChunkSize * 0.5f, 100.0f),
                    ChunkColor, false, 0.1f, 0, 5.0f);
        
        // Draw biome type text
        FString BiomeText = UEnum::GetValueAsString(Chunk.BiomeType);
        DrawDebugString(GetWorld(), Chunk.ChunkCenter + FVector(0, 0, 200), 
                       FString::Printf(TEXT("%s LOD%d"), *BiomeText, Chunk.CurrentLOD),
                       nullptr, ChunkColor, 0.1f);
    }
}

void UWorld_TerrainLODSystem::ValidateChunkIntegrity()
{
    int32 InvalidChunks = 0;
    
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        // Check for invalid actors
        for (AActor* Actor : Chunk.TerrainActors)
        {
            if (!IsValid(Actor))
            {
                InvalidChunks++;
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Chunk validation: %d chunks with invalid actors"), InvalidChunks);
}

void UWorld_TerrainLODSystem::ProcessChunkLOD(FWorld_TerrainChunk& Chunk, int32 NewLODLevel)
{
    if (NewLODLevel == Chunk.CurrentLOD) return;
    
    Chunk.CurrentLOD = NewLODLevel;
    
    if (NewLODLevel >= 5) // Culled
    {
        ClearChunkActors(Chunk);
        Chunk.bIsActive = false;
    }
    else
    {
        Chunk.bIsActive = true;
        PopulateChunkWithBiome(Chunk);
        ApplyBiomeLOD(Chunk, NewLODLevel);
    }
}

void UWorld_TerrainLODSystem::SpawnVegetationForChunk(FWorld_TerrainChunk& Chunk, int32 LODLevel)
{
    if (!GetWorld() || LODLevel >= 4) return;
    
    int32 VegetationCount = FMath::Max(1, LODSettings.MaxVegetationPerChunk / (LODLevel + 1));
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = Chunk.ChunkCenter + FVector(
            FMath::RandRange(-Chunk.ChunkSize * 0.4f, Chunk.ChunkSize * 0.4f),
            FMath::RandRange(-Chunk.ChunkSize * 0.4f, Chunk.ChunkSize * 0.4f),
            0
        );
        
        AStaticMeshActor* VegActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
        if (VegActor)
        {
            VegActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%s_%d"), 
                                   *UEnum::GetValueAsString(Chunk.BiomeType), i));
            Chunk.TerrainActors.Add(VegActor);
        }
    }
}

void UWorld_TerrainLODSystem::SpawnRocksForChunk(FWorld_TerrainChunk& Chunk, int32 LODLevel)
{
    if (!GetWorld() || LODLevel >= 3) return;
    
    int32 RockCount = FMath::Max(1, LODSettings.MaxRocksPerChunk / (LODLevel + 1));
    
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector SpawnLocation = Chunk.ChunkCenter + FVector(
            FMath::RandRange(-Chunk.ChunkSize * 0.3f, Chunk.ChunkSize * 0.3f),
            FMath::RandRange(-Chunk.ChunkSize * 0.3f, Chunk.ChunkSize * 0.3f),
            0
        );
        
        AStaticMeshActor* RockActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
        if (RockActor)
        {
            RockActor->SetActorLabel(FString::Printf(TEXT("Rock_%s_%d"), 
                                    *UEnum::GetValueAsString(Chunk.BiomeType), i));
            Chunk.TerrainActors.Add(RockActor);
        }
    }
}

void UWorld_TerrainLODSystem::ClearChunkActors(FWorld_TerrainChunk& Chunk)
{
    for (AActor* Actor : Chunk.TerrainActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    Chunk.TerrainActors.Empty();
}

bool UWorld_TerrainLODSystem::IsChunkInFrustum(const FWorld_TerrainChunk& Chunk) const
{
    // Simplified frustum check - always return true for now
    // In a full implementation, this would check against camera frustum
    return true;
}

float UWorld_TerrainLODSystem::CalculateChunkPriority(const FWorld_TerrainChunk& Chunk, const FVector& PlayerLocation) const
{
    float Distance = FVector::Dist(Chunk.ChunkCenter, PlayerLocation);
    float Priority = 1.0f / (Distance + 1.0f); // Closer chunks have higher priority
    
    // Boost priority for chunks in player's forward direction
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Player)
    {
        FVector PlayerForward = Player->GetActorForwardVector();
        FVector ToChunk = (Chunk.ChunkCenter - PlayerLocation).GetSafeNormal();
        float DotProduct = FVector::DotProduct(PlayerForward, ToChunk);
        Priority *= (1.0f + DotProduct * 0.5f); // Boost forward chunks by up to 50%
    }
    
    return Priority;
}