// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ProceduralWorldGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "../Consciousness/ConsciousnessSystem.h"
#include "../Physics/PhysicsOptimizer.h"

// FastNoise implementation for procedural generation
class UFastNoise : public UObject
{
public:
    UFastNoise() : Seed(1337), Frequency(0.01f) {}
    
    void SetSeed(int32 InSeed) { Seed = InSeed; }
    void SetFrequency(float InFrequency) { Frequency = InFrequency; }
    
    float GetNoise(float X, float Y)
    {
        return SimplexNoise(X * Frequency, Y * Frequency);
    }
    
    float GetNoise(float X, float Y, float Z)
    {
        return SimplexNoise(X * Frequency, Y * Frequency, Z * Frequency);
    }

private:
    int32 Seed;
    float Frequency;
    
    // Simplified Simplex Noise implementation
    float SimplexNoise(float X, float Y)
    {
        int32 i = FMath::FloorToInt(X);
        int32 j = FMath::FloorToInt(Y);
        
        float x = X - i;
        float y = Y - j;
        
        float n0 = Grad(Hash(i, j), x, y);
        float n1 = Grad(Hash(i + 1, j), x - 1, y);
        float n2 = Grad(Hash(i, j + 1), x, y - 1);
        float n3 = Grad(Hash(i + 1, j + 1), x - 1, y - 1);
        
        float ix0 = FMath::InterpEaseInOut(n0, n1, x, 2.0f);
        float ix1 = FMath::InterpEaseInOut(n2, n3, x, 2.0f);
        
        return FMath::InterpEaseInOut(ix0, ix1, y, 2.0f);
    }
    
    float SimplexNoise(float X, float Y, float Z)
    {
        return (SimplexNoise(X, Y) + SimplexNoise(Y, Z) + SimplexNoise(X, Z)) / 3.0f;
    }
    
    int32 Hash(int32 X, int32 Y)
    {
        int32 n = X + Y * 57 + Seed;
        n = (n << 13) ^ n;
        return (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    }
    
    float Grad(int32 Hash, float X, float Y)
    {
        int32 h = Hash & 7;
        float u = h < 4 ? X : Y;
        float v = h < 4 ? Y : X;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }
};

AProceduralWorldGenerator::AProceduralWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    ChunkSize = 1000;
    ViewDistance = 3;
    NoiseScale = 0.01f;
    Seed = 1337;
    
    ConsciousnessInfluenceRadius = 2000.0f;
    ConsciousnessGenerationThreshold = 0.3f;
    bUseConsciousnessBasedGeneration = true;
    
    BiomeTransitionSmoothness = 0.5f;
    
    MaxChunksPerFrame = 1;
    ChunkUpdateInterval = 0.1f;
    bUseInstancedMeshes = true;
    
    LastUpdateTime = 0.0f;
    ChunksGeneratedThisFrame = 0;
    
    // Initialize default biomes
    FBiomeData MeditativeBiome;
    MeditativeBiome.BiomeName = TEXT("Meditative");
    MeditativeBiome.ConsciousnessResonance = 1.5f;
    MeditativeBiome.AmbientColor = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f);
    MeditativeBiome.EnergyFlowIntensity = 2.0f;
    
    FBiomeData GroundedBiome;
    GroundedBiome.BiomeName = TEXT("Grounded");
    GroundedBiome.ConsciousnessResonance = 1.0f;
    GroundedBiome.AmbientColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    GroundedBiome.EnergyFlowIntensity = 1.0f;
    
    FBiomeData TranscendentBiome;
    TranscendentBiome.BiomeName = TEXT("Transcendent");
    TranscendentBiome.ConsciousnessResonance = 2.0f;
    TranscendentBiome.AmbientColor = FLinearColor(1.0f, 0.8f, 1.0f, 1.0f);
    TranscendentBiome.EnergyFlowIntensity = 3.0f;
    
    AvailableBiomes.Add(MeditativeBiome);
    AvailableBiomes.Add(GroundedBiome);
    AvailableBiomes.Add(TranscendentBiome);
}

void AProceduralWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    // Find system references
    ConsciousnessSystem = Cast<AConsciousnessSystem>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AConsciousnessSystem::StaticClass())
    );
    
    PhysicsOptimizer = Cast<APhysicsOptimizer>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APhysicsOptimizer::StaticClass())
    );
    
    // Initialize noise generator
    InitializeNoiseGenerator();
    
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldGenerator initialized"));
}

void AProceduralWorldGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateChunkGeneration(DeltaTime);
    
    // Update world around all players
    UWorld* World = GetWorld();
    if (World)
    {
        for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            APlayerController* PC = Iterator->Get();
            if (PC && PC->GetPawn())
            {
                UpdateWorldAroundPlayer(PC->GetPawn()->GetActorLocation());
            }
        }
    }
}

void AProceduralWorldGenerator::InitializeNoiseGenerator()
{
    NoiseGenerator = NewObject<UFastNoise>(this);
    NoiseGenerator->SetSeed(Seed);
    NoiseGenerator->SetFrequency(NoiseScale);
}

void AProceduralWorldGenerator::UpdateChunkGeneration(float DeltaTime)
{
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= ChunkUpdateInterval)
    {
        LastUpdateTime = 0.0f;
        ChunksGeneratedThisFrame = 0;
        ProcessChunkQueue();
    }
}

void AProceduralWorldGenerator::ProcessChunkQueue()
{
    // Generate new chunks
    while (ChunksToGenerate.Num() > 0 && ChunksGeneratedThisFrame < MaxChunksPerFrame)
    {
        FVector2D ChunkCoord = ChunksToGenerate[0];
        ChunksToGenerate.RemoveAt(0);
        
        if (!IsChunkLoaded(ChunkCoord))
        {
            GenerateChunk(ChunkCoord);
            ChunksGeneratedThisFrame++;
        }
    }
    
    // Unload distant chunks
    for (const FVector2D& ChunkCoord : ChunksToUnload)
    {
        UnloadChunk(ChunkCoord);
    }
    ChunksToUnload.Empty();
}

void AProceduralWorldGenerator::GenerateChunk(FVector2D ChunkCoord)
{
    if (IsChunkLoaded(ChunkCoord))
    {
        return;
    }
    
    FChunkData NewChunk;
    NewChunk.ChunkCoordinates = ChunkCoord;
    NewChunk.bIsLoaded = true;
    NewChunk.bIsGenerated = false;
    
    // Calculate consciousness influence at chunk center
    FVector ChunkWorldLocation = ChunkToWorldLocation(ChunkCoord);
    NewChunk.ConsciousnessLevel = CalculateConsciousnessAtLocation(ChunkWorldLocation);
    
    // Get biome for this chunk
    FBiomeData ChunkBiome = GetBiomeAtLocation(ChunkWorldLocation);
    
    // Generate terrain mesh
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    GenerateTerrainMesh(ChunkCoord, Vertices, Triangles);
    
    // Place vegetation based on biome and consciousness
    PlaceVegetation(ChunkCoord, ChunkBiome);
    
    // Spawn consciousness elements if threshold is met
    if (NewChunk.ConsciousnessLevel >= ConsciousnessGenerationThreshold)
    {
        SpawnConsciousnessElements(ChunkCoord, NewChunk.ConsciousnessLevel);
    }
    
    // Optimize performance
    OptimizeChunkPerformance(ChunkCoord);
    
    NewChunk.bIsGenerated = true;
    LoadedChunks.Add(ChunkCoord, NewChunk);
    
    OnChunkGenerated(ChunkCoord);
    
    UE_LOG(LogTemp, Log, TEXT("Generated chunk at (%f, %f) with consciousness level %f"), 
           ChunkCoord.X, ChunkCoord.Y, NewChunk.ConsciousnessLevel);
}

void AProceduralWorldGenerator::UnloadChunk(FVector2D ChunkCoord)
{
    FChunkData* ChunkData = LoadedChunks.Find(ChunkCoord);
    if (!ChunkData)
    {
        return;
    }
    
    // Destroy all generated actors
    for (AActor* Actor : ChunkData->GeneratedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    LoadedChunks.Remove(ChunkCoord);
    OnChunkUnloaded(ChunkCoord);
    
    UE_LOG(LogTemp, Log, TEXT("Unloaded chunk at (%f, %f)"), ChunkCoord.X, ChunkCoord.Y);
}

void AProceduralWorldGenerator::UpdateWorldAroundPlayer(FVector PlayerLocation)
{
    FVector2D PlayerChunk = WorldToChunkCoordinates(PlayerLocation);
    
    // Queue chunks for generation
    for (int32 X = -ViewDistance; X <= ViewDistance; X++)
    {
        for (int32 Y = -ViewDistance; Y <= ViewDistance; Y++)
        {
            FVector2D ChunkCoord = PlayerChunk + FVector2D(X, Y);
            
            if (!IsChunkLoaded(ChunkCoord) && !ChunksToGenerate.Contains(ChunkCoord))
            {
                ChunksToGenerate.Add(ChunkCoord);
            }
        }
    }
    
    // Queue distant chunks for unloading
    TArray<FVector2D> ChunksToCheck;
    LoadedChunks.GetKeys(ChunksToCheck);
    
    for (const FVector2D& ChunkCoord : ChunksToCheck)
    {
        float Distance = FVector2D::Distance(PlayerChunk, ChunkCoord);
        if (Distance > ViewDistance + 1)
        {
            ChunksToUnload.AddUnique(ChunkCoord);
        }
    }
}

float AProceduralWorldGenerator::GetHeightAtLocation(FVector2D WorldLocation)
{
    if (!NoiseGenerator)
    {
        return 0.0f;
    }
    
    float BaseHeight = NoiseGenerator->GetNoise(WorldLocation.X, WorldLocation.Y) * 500.0f;
    float DetailHeight = NoiseGenerator->GetNoise(WorldLocation.X * 4, WorldLocation.Y * 4) * 100.0f;
    
    // Apply consciousness influence to terrain height
    FVector WorldLoc3D(WorldLocation.X, WorldLocation.Y, 0.0f);
    float ConsciousnessInfluence = GetConsciousnessInfluenceAtLocation(WorldLoc3D);
    float ConsciousnessModifier = ConsciousnessInfluence * 200.0f;
    
    return BaseHeight + DetailHeight + ConsciousnessModifier;
}

void AProceduralWorldGenerator::GenerateTerrainMesh(FVector2D ChunkCoord, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    const int32 Resolution = 32;
    const float StepSize = ChunkSize / (float)Resolution;
    
    FVector ChunkWorldPos = ChunkToWorldLocation(ChunkCoord);
    
    // Generate vertices
    for (int32 Y = 0; Y <= Resolution; Y++)
    {
        for (int32 X = 0; X <= Resolution; X++)
        {
            FVector2D WorldPos2D = FVector2D(ChunkWorldPos.X + X * StepSize, ChunkWorldPos.Y + Y * StepSize);
            float Height = GetHeightAtLocation(WorldPos2D);
            
            Vertices.Add(FVector(X * StepSize, Y * StepSize, Height));
        }
    }
    
    // Generate triangles
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 i = Y * (Resolution + 1) + X;
            
            // First triangle
            Triangles.Add(i);
            Triangles.Add(i + Resolution + 1);
            Triangles.Add(i + 1);
            
            // Second triangle
            Triangles.Add(i + 1);
            Triangles.Add(i + Resolution + 1);
            Triangles.Add(i + Resolution + 2);
        }
    }
}

void AProceduralWorldGenerator::PlaceVegetation(FVector2D ChunkCoord, const FBiomeData& Biome)
{
    if (Biome.VegetationMeshes.Num() == 0)
    {
        return;
    }
    
    FVector ChunkWorldPos = ChunkToWorldLocation(ChunkCoord);
    const int32 VegetationCount = FMath::RandRange(50, 200);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector2D RandomOffset(
            FMath::RandRange(-ChunkSize * 0.5f, ChunkSize * 0.5f),
            FMath::RandRange(-ChunkSize * 0.5f, ChunkSize * 0.5f)
        );
        
        FVector2D VegWorldPos2D = FVector2D(ChunkWorldPos.X, ChunkWorldPos.Y) + RandomOffset;
        float Height = GetHeightAtLocation(VegWorldPos2D);
        FVector VegWorldPos(VegWorldPos2D.X, VegWorldPos2D.Y, Height);
        
        // Check consciousness influence for vegetation density
        float ConsciousnessInfluence = GetConsciousnessInfluenceAtLocation(VegWorldPos);
        if (FMath::RandRange(0.0f, 1.0f) > ConsciousnessInfluence * Biome.ConsciousnessResonance)
        {
            continue;
        }
        
        UStaticMesh* VegMesh = Biome.VegetationMeshes[FMath::RandRange(0, Biome.VegetationMeshes.Num() - 1)];
        if (!VegMesh)
        {
            continue;
        }
        
        FTransform VegTransform;
        VegTransform.SetLocation(VegWorldPos);
        VegTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
        VegTransform.SetScale3D(FVector(FMath::RandRange(0.8f, 1.2f)));
        
        if (bUseInstancedMeshes)
        {
            TArray<FTransform> Transforms;
            Transforms.Add(VegTransform);
            CreateInstancedMeshForChunk(ChunkCoord, VegMesh, Transforms);
        }
    }
}

float AProceduralWorldGenerator::CalculateConsciousnessAtLocation(FVector WorldLocation)
{
    if (!ConsciousnessSystem || !bUseConsciousnessBasedGeneration)
    {
        return 0.5f; // Default neutral consciousness
    }
    
    // Get consciousness data from the consciousness system
    // This would integrate with the actual consciousness system implementation
    float BaseConsciousness = 0.5f;
    
    // Add noise-based consciousness variation
    if (NoiseGenerator)
    {
        float ConsciousnessNoise = NoiseGenerator->GetNoise(
            WorldLocation.X * 0.001f, 
            WorldLocation.Y * 0.001f, 
            0.0f
        );
        BaseConsciousness += ConsciousnessNoise * 0.3f;
    }
    
    return FMath::Clamp(BaseConsciousness, 0.0f, 1.0f);
}

FBiomeData AProceduralWorldGenerator::GetBiomeAtLocation(FVector WorldLocation)
{
    if (AvailableBiomes.Num() == 0)
    {
        return FBiomeData();
    }
    
    float ConsciousnessLevel = CalculateConsciousnessAtLocation(WorldLocation);
    
    // Select biome based on consciousness level
    if (ConsciousnessLevel < 0.3f)
    {
        return AvailableBiomes[1]; // Grounded
    }
    else if (ConsciousnessLevel < 0.7f)
    {
        return AvailableBiomes[0]; // Meditative
    }
    else
    {
        return AvailableBiomes[2]; // Transcendent
    }
}

float AProceduralWorldGenerator::GetConsciousnessInfluenceAtLocation(FVector WorldLocation)
{
    return CalculateConsciousnessAtLocation(WorldLocation);
}

void AProceduralWorldGenerator::UpdateChunkConsciousness(FVector2D ChunkCoord)
{
    FChunkData* ChunkData = LoadedChunks.Find(ChunkCoord);
    if (!ChunkData)
    {
        return;
    }
    
    FVector ChunkWorldLocation = ChunkToWorldLocation(ChunkCoord);
    float NewConsciousnessLevel = CalculateConsciousnessAtLocation(ChunkWorldLocation);
    
    if (FMath::Abs(NewConsciousnessLevel - ChunkData->ConsciousnessLevel) > 0.1f)
    {
        ChunkData->ConsciousnessLevel = NewConsciousnessLevel;
        OnConsciousnessLevelChanged(ChunkCoord, NewConsciousnessLevel);
        
        // Respawn consciousness elements if needed
        if (NewConsciousnessLevel >= ConsciousnessGenerationThreshold)
        {
            SpawnConsciousnessElements(ChunkCoord, NewConsciousnessLevel);
        }
    }
}

void AProceduralWorldGenerator::SpawnConsciousnessElements(FVector2D ChunkCoord, float ConsciousnessLevel)
{
    // This would spawn special consciousness-related elements
    // Integration point with consciousness system
    UE_LOG(LogTemp, Log, TEXT("Spawning consciousness elements at chunk (%f, %f) with level %f"), 
           ChunkCoord.X, ChunkCoord.Y, ConsciousnessLevel);
}

void AProceduralWorldGenerator::OptimizeChunkPerformance(FVector2D ChunkCoord)
{
    if (PhysicsOptimizer)
    {
        // Apply physics optimizations to chunk
        FChunkData* ChunkData = LoadedChunks.Find(ChunkCoord);
        if (ChunkData)
        {
            for (AActor* Actor : ChunkData->GeneratedActors)
            {
                if (IsValid(Actor))
                {
                    // Apply physics optimizations
                    // This would integrate with the PhysicsOptimizer
                }
            }
        }
    }
}

UInstancedStaticMeshComponent* AProceduralWorldGenerator::GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh)
{
    if (!Mesh)
    {
        return nullptr;
    }
    
    UInstancedStaticMeshComponent** Found = InstancedMeshComponents.Find(Mesh);
    if (Found)
    {
        return *Found;
    }
    
    UInstancedStaticMeshComponent* NewComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
        *FString::Printf(TEXT("InstancedMesh_%s"), *Mesh->GetName())
    );
    NewComponent->SetStaticMesh(Mesh);
    NewComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    
    InstancedMeshComponents.Add(Mesh, NewComponent);
    return NewComponent;
}

void AProceduralWorldGenerator::CreateInstancedMeshForChunk(FVector2D ChunkCoord, UStaticMesh* Mesh, const TArray<FTransform>& Transforms)
{
    UInstancedStaticMeshComponent* InstancedComponent = GetOrCreateInstancedMeshComponent(Mesh);
    if (!InstancedComponent)
    {
        return;
    }
    
    for (const FTransform& Transform : Transforms)
    {
        InstancedComponent->AddInstance(Transform);
    }
}

FVector2D AProceduralWorldGenerator::WorldToChunkCoordinates(FVector WorldLocation)
{
    return FVector2D(
        FMath::FloorToFloat(WorldLocation.X / ChunkSize),
        FMath::FloorToFloat(WorldLocation.Y / ChunkSize)
    );
}

FVector AProceduralWorldGenerator::ChunkToWorldLocation(FVector2D ChunkCoord)
{
    return FVector(
        ChunkCoord.X * ChunkSize + ChunkSize * 0.5f,
        ChunkCoord.Y * ChunkSize + ChunkSize * 0.5f,
        0.0f
    );
}

bool AProceduralWorldGenerator::IsChunkLoaded(FVector2D ChunkCoord)
{
    return LoadedChunks.Contains(ChunkCoord);
}

void AProceduralWorldGenerator::ClearAllChunks()
{
    TArray<FVector2D> ChunksToRemove;
    LoadedChunks.GetKeys(ChunksToRemove);
    
    for (const FVector2D& ChunkCoord : ChunksToRemove)
    {
        UnloadChunk(ChunkCoord);
    }
    
    ChunksToGenerate.Empty();
    ChunksToUnload.Empty();
}