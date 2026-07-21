#include "World_TerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_TerrainGenerator::AWorld_TerrainGenerator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Default terrain parameters
    NoiseScale = 0.01f;
    HeightMultiplier = 1000.0f;
    NoiseOctaves = 4;
    ChunkSize = 512;
    ChunkLoadDistance = 2000.0f;
}

void AWorld_TerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateInitialTerrain();
}

void AWorld_TerrainGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update terrain around player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        UpdateTerrainAroundPlayer(PlayerPawn->GetActorLocation());
    }
}

void AWorld_TerrainGenerator::GenerateInitialTerrain()
{
    // Generate initial 3x3 grid of terrain chunks around origin
    for (int32 X = -1; X <= 1; X++)
    {
        for (int32 Y = -1; Y <= 1; Y++)
        {
            FVector ChunkLocation = FVector(X * ChunkSize * 100.0f, Y * ChunkSize * 100.0f, 0.0f);
            GenerateTerrainChunk(ChunkLocation);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Generated initial terrain with %d chunks"), GeneratedChunks.Num());
}

void AWorld_TerrainGenerator::GenerateTerrainChunk(const FVector& ChunkLocation)
{
    FVector ChunkKey = WorldToChunkCoordinate(ChunkLocation);
    
    if (GeneratedChunks.Contains(ChunkKey))
    {
        return; // Chunk already exists
    }

    FWorld_TerrainChunk NewChunk;
    NewChunk.ChunkLocation = ChunkLocation;
    NewChunk.ChunkSize = ChunkSize;
    NewChunk.bIsGenerated = true;

    // Generate height data using Perlin noise
    NewChunk.HeightData.Reserve(ChunkSize * ChunkSize);
    for (int32 Y = 0; Y < ChunkSize; Y++)
    {
        for (int32 X = 0; X < ChunkSize; X++)
        {
            float WorldX = ChunkLocation.X + (X * 100.0f);
            float WorldY = ChunkLocation.Y + (Y * 100.0f);
            float Height = GenerateNoiseValue(WorldX, WorldY) * HeightMultiplier;
            NewChunk.HeightData.Add(Height);
        }
    }

    GeneratedChunks.Add(ChunkKey, NewChunk);
    CreateTerrainMesh(NewChunk);
}

void AWorld_TerrainGenerator::UpdateTerrainAroundPlayer(const FVector& PlayerLocation)
{
    // Simple distance-based chunk loading
    FVector PlayerChunk = WorldToChunkCoordinate(PlayerLocation);
    
    // Check if we need to generate new chunks around player
    for (int32 X = -2; X <= 2; X++)
    {
        for (int32 Y = -2; Y <= 2; Y++)
        {
            FVector ChunkOffset = FVector(X * ChunkSize * 100.0f, Y * ChunkSize * 100.0f, 0.0f);
            FVector ChunkLocation = PlayerChunk + ChunkOffset;
            
            float Distance = FVector::Dist(PlayerLocation, ChunkLocation);
            if (Distance < ChunkLoadDistance)
            {
                GenerateTerrainChunk(ChunkLocation);
            }
        }
    }

    UpdateChunkVisibility(PlayerLocation);
}

float AWorld_TerrainGenerator::GetHeightAtLocation(const FVector& WorldLocation)
{
    FVector ChunkKey = WorldToChunkCoordinate(WorldLocation);
    
    if (!GeneratedChunks.Contains(ChunkKey))
    {
        return GenerateNoiseValue(WorldLocation.X, WorldLocation.Y) * HeightMultiplier;
    }

    const FWorld_TerrainChunk& Chunk = GeneratedChunks[ChunkKey];
    
    // Simple bilinear interpolation for height lookup
    float LocalX = FMath::Fmod(WorldLocation.X, ChunkSize * 100.0f) / 100.0f;
    float LocalY = FMath::Fmod(WorldLocation.Y, ChunkSize * 100.0f) / 100.0f;
    
    int32 X0 = FMath::FloorToInt(LocalX);
    int32 Y0 = FMath::FloorToInt(LocalY);
    int32 X1 = FMath::Min(X0 + 1, ChunkSize - 1);
    int32 Y1 = FMath::Min(Y0 + 1, ChunkSize - 1);
    
    if (X0 >= 0 && Y0 >= 0 && X1 < ChunkSize && Y1 < ChunkSize)
    {
        float H00 = Chunk.HeightData[Y0 * ChunkSize + X0];
        float H10 = Chunk.HeightData[Y0 * ChunkSize + X1];
        float H01 = Chunk.HeightData[Y1 * ChunkSize + X0];
        float H11 = Chunk.HeightData[Y1 * ChunkSize + X1];
        
        float FracX = LocalX - X0;
        float FracY = LocalY - Y0;
        
        float H0 = FMath::Lerp(H00, H10, FracX);
        float H1 = FMath::Lerp(H01, H11, FracX);
        
        return FMath::Lerp(H0, H1, FracY);
    }
    
    return 0.0f;
}

void AWorld_TerrainGenerator::SetTerrainParameters(float InNoiseScale, float InHeightMultiplier, int32 InOctaves)
{
    NoiseScale = InNoiseScale;
    HeightMultiplier = InHeightMultiplier;
    NoiseOctaves = InOctaves;
    
    // Regenerate all terrain with new parameters
    ClearAllTerrain();
    GenerateInitialTerrain();
}

void AWorld_TerrainGenerator::ClearAllTerrain()
{
    // Destroy all terrain mesh components
    for (UStaticMeshComponent* MeshComp : TerrainMeshComponents)
    {
        if (IsValid(MeshComp))
        {
            MeshComp->DestroyComponent();
        }
    }
    
    TerrainMeshComponents.Empty();
    GeneratedChunks.Empty();
}

float AWorld_TerrainGenerator::GenerateNoiseValue(float X, float Y)
{
    float NoiseValue = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = NoiseScale;
    
    for (int32 Octave = 0; Octave < NoiseOctaves; Octave++)
    {
        NoiseValue += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return FMath::Clamp(NoiseValue, -1.0f, 1.0f);
}

void AWorld_TerrainGenerator::CreateTerrainMesh(const FWorld_TerrainChunk& Chunk)
{
    // Create a static mesh component for this chunk
    UStaticMeshComponent* ChunkMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("TerrainChunk_%s"), *Chunk.ChunkLocation.ToString()));
    
    if (ChunkMesh)
    {
        ChunkMesh->SetupAttachment(RootComponent);
        ChunkMesh->SetWorldLocation(Chunk.ChunkLocation);
        
        // Use a simple cube mesh as placeholder for now
        // In a full implementation, this would generate a proper procedural mesh
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            ChunkMesh->SetStaticMesh(CubeMesh);
            ChunkMesh->SetWorldScale3D(FVector(ChunkSize / 100.0f, ChunkSize / 100.0f, 0.1f));
        }
        
        TerrainMeshComponents.Add(ChunkMesh);
    }
}

void AWorld_TerrainGenerator::UpdateChunkVisibility(const FVector& PlayerLocation)
{
    // Simple distance-based visibility culling
    for (UStaticMeshComponent* MeshComp : TerrainMeshComponents)
    {
        if (IsValid(MeshComp))
        {
            float Distance = FVector::Dist(PlayerLocation, MeshComp->GetComponentLocation());
            bool bShouldBeVisible = Distance < ChunkLoadDistance * 1.5f;
            MeshComp->SetVisibility(bShouldBeVisible);
        }
    }
}

FVector AWorld_TerrainGenerator::WorldToChunkCoordinate(const FVector& WorldLocation)
{
    float ChunkWorldSize = ChunkSize * 100.0f;
    int32 ChunkX = FMath::FloorToInt(WorldLocation.X / ChunkWorldSize);
    int32 ChunkY = FMath::FloorToInt(WorldLocation.Y / ChunkWorldSize);
    return FVector(ChunkX * ChunkWorldSize, ChunkY * ChunkWorldSize, 0.0f);
}