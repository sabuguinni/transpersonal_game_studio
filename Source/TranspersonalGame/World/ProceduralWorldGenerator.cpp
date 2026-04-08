// ProceduralWorldGenerator.cpp
// Implementação do sistema de geração procedural de mundo
#include "ProceduralWorldGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AProceduralWorldGenerator::AProceduralWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Criar componente de mesh procedural
    TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
    TerrainMesh->SetupAttachment(RootComponent);

    // Configurações padrão
    ChunkSize = 1000;
    ViewDistance = 3;
    NoiseScale = 0.01f;
    HeightMultiplier = 500.0f;
    Seed = 12345;
    LastPlayerPosition = FVector::ZeroVector;
    CachedPlayer = nullptr;

    // Inicializar biomas padrão
    FBiomeData GrasslandBiome;
    GrasslandBiome.BiomeName = TEXT("Grassland");
    GrasslandBiome.Temperature = 20.0f;
    GrasslandBiome.Humidity = 0.6f;
    GrasslandBiome.Elevation = 100.0f;
    AvailableBiomes.Add(GrasslandBiome);

    FBiomeData ForestBiome;
    ForestBiome.BiomeName = TEXT("Forest");
    ForestBiome.Temperature = 15.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.Elevation = 200.0f;
    AvailableBiomes.Add(ForestBiome);

    FBiomeData MountainBiome;
    MountainBiome.BiomeName = TEXT("Mountain");
    MountainBiome.Temperature = 5.0f;
    MountainBiome.Humidity = 0.4f;
    MountainBiome.Elevation = 800.0f;
    AvailableBiomes.Add(MountainBiome);
}

void AProceduralWorldGenerator::BeginPlay()
{
    Super::BeginPlay();

    // Inicializar gerador de números aleatórios
    FMath::RandInit(Seed);

    // Encontrar player
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            CachedPlayer = PC->GetPawn();
        }
    }

    // Gerar chunks iniciais
    UpdateChunks();
}

void AProceduralWorldGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Verificar se o player se moveu significativamente
    if (CachedPlayer)
    {
        FVector CurrentPlayerPosition = CachedPlayer->GetActorLocation();
        float DistanceMoved = FVector::Dist(CurrentPlayerPosition, LastPlayerPosition);
        
        if (DistanceMoved > ChunkSize * 0.5f) // Atualizar quando player mover meio chunk
        {
            LastPlayerPosition = CurrentPlayerPosition;
            UpdateChunks();
        }
    }
}

void AProceduralWorldGenerator::UpdateChunks()
{
    if (!CachedPlayer) return;

    FVector PlayerLocation = CachedPlayer->GetActorLocation();
    FVector2D PlayerChunk = FVector2D(
        FMath::FloorToInt(PlayerLocation.X / ChunkSize),
        FMath::FloorToInt(PlayerLocation.Y / ChunkSize)
    );

    ChunksToGenerate.Empty();
    ChunksToUnload.Empty();

    // Determinar chunks que devem estar carregados
    TArray<FVector2D> RequiredChunks;
    for (int32 X = -ViewDistance; X <= ViewDistance; X++)
    {
        for (int32 Y = -ViewDistance; Y <= ViewDistance; Y++)
        {
            FVector2D ChunkCoord = PlayerChunk + FVector2D(X, Y);
            RequiredChunks.Add(ChunkCoord);

            // Verificar se chunk precisa ser gerado
            if (!LoadedChunks.Contains(ChunkCoord))
            {
                ChunksToGenerate.Add(ChunkCoord);
            }
        }
    }

    // Determinar chunks para descarregar
    for (auto& ChunkPair : LoadedChunks)
    {
        if (!RequiredChunks.Contains(ChunkPair.Key))
        {
            ChunksToUnload.Add(ChunkPair.Key);
        }
    }

    // Processar chunks (limitar para evitar lag)
    int32 ChunksProcessedThisFrame = 0;
    const int32 MaxChunksPerFrame = 2;

    // Gerar novos chunks
    for (const FVector2D& ChunkCoord : ChunksToGenerate)
    {
        if (ChunksProcessedThisFrame >= MaxChunksPerFrame) break;
        GenerateChunk(ChunkCoord);
        ChunksProcessedThisFrame++;
    }

    // Descarregar chunks distantes
    for (const FVector2D& ChunkCoord : ChunksToUnload)
    {
        UnloadChunk(ChunkCoord);
    }
}

void AProceduralWorldGenerator::GenerateChunk(const FVector2D& ChunkCoord)
{
    FChunkData NewChunk;
    NewChunk.ChunkCoordinates = ChunkCoord;
    NewChunk.bIsLoaded = true;
    NewChunk.bIsGenerated = false;

    // Gerar terreno
    GenerateTerrain(NewChunk);

    // Gerar vegetação
    GenerateVegetation(NewChunk);

    // Gerar estruturas
    GenerateStructures(NewChunk);

    NewChunk.bIsGenerated = true;
    LoadedChunks.Add(ChunkCoord, NewChunk);

    // Notificar Blueprint
    OnChunkGenerated(ChunkCoord);

    UE_LOG(LogTemp, Log, TEXT("Generated chunk at (%f, %f)"), ChunkCoord.X, ChunkCoord.Y);
}

void AProceduralWorldGenerator::UnloadChunk(const FVector2D& ChunkCoord)
{
    if (LoadedChunks.Contains(ChunkCoord))
    {
        LoadedChunks.Remove(ChunkCoord);
        OnChunkUnloaded(ChunkCoord);
        UE_LOG(LogTemp, Log, TEXT("Unloaded chunk at (%f, %f)"), ChunkCoord.X, ChunkCoord.Y);
    }
}

void AProceduralWorldGenerator::GenerateTerrain(FChunkData& ChunkData)
{
    const int32 VerticesPerSide = 33; // 32x32 quads
    const float VertexSpacing = ChunkSize / (VerticesPerSide - 1);

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FVector> Normals;

    // Gerar vértices
    for (int32 Y = 0; Y < VerticesPerSide; Y++)
    {
        for (int32 X = 0; X < VerticesPerSide; X++)
        {
            float WorldX = ChunkData.ChunkCoordinates.X * ChunkSize + X * VertexSpacing;
            float WorldY = ChunkData.ChunkCoordinates.Y * ChunkSize + Y * VertexSpacing;
            float Height = GenerateHeight(WorldX, WorldY);

            Vertices.Add(FVector(X * VertexSpacing, Y * VertexSpacing, Height));
            UVs.Add(FVector2D(X / (float)(VerticesPerSide - 1), Y / (float)(VerticesPerSide - 1)));
            ChunkData.HeightmapData.Add(FVector(WorldX, WorldY, Height));

            // Calcular normal (aproximada)
            FVector Normal = FVector(0, 0, 1); // Placeholder - calcular baseado em vizinhos
            Normals.Add(Normal);
        }
    }

    // Gerar triângulos
    for (int32 Y = 0; Y < VerticesPerSide - 1; Y++)
    {
        for (int32 X = 0; X < VerticesPerSide - 1; X++)
        {
            int32 BottomLeft = Y * VerticesPerSide + X;
            int32 BottomRight = BottomLeft + 1;
            int32 TopLeft = (Y + 1) * VerticesPerSide + X;
            int32 TopRight = TopLeft + 1;

            // Primeiro triângulo
            Triangles.Add(BottomLeft);
            Triangles.Add(TopLeft);
            Triangles.Add(BottomRight);

            // Segundo triângulo
            Triangles.Add(BottomRight);
            Triangles.Add(TopLeft);
            Triangles.Add(TopRight);
        }
    }

    // Determinar bioma para este chunk
    float CenterX = ChunkData.ChunkCoordinates.X * ChunkSize + ChunkSize * 0.5f;
    float CenterY = ChunkData.ChunkCoordinates.Y * ChunkSize + ChunkSize * 0.5f;
    float CenterHeight = GenerateHeight(CenterX, CenterY);
    ChunkData.BiomeData = DetermineBiome(CenterX, CenterY, CenterHeight);

    // Criar mesh procedural
    if (TerrainMesh)
    {
        TerrainMesh->CreateMeshSection(
            LoadedChunks.Num(), // Section ID baseado no número de chunks
            Vertices,
            Triangles,
            Normals,
            UVs,
            TArray<FColor>(),
            TArray<FProcMeshTangent>(),
            true
        );

        // Aplicar material do bioma se disponível
        if (ChunkData.BiomeData.TerrainMaterial)
        {
            TerrainMesh->SetMaterial(LoadedChunks.Num(), ChunkData.BiomeData.TerrainMaterial);
        }
    }
}

float AProceduralWorldGenerator::GenerateHeight(float X, float Y) const
{
    // Combinar múltiplas octavas de noise para terreno mais interessante
    float Height = 0.0f;
    
    // Base terrain
    Height += FractalNoise(X * NoiseScale, Y * NoiseScale, 4) * HeightMultiplier;
    
    // Montanhas
    Height += RidgedNoise(X * NoiseScale * 0.5f, Y * NoiseScale * 0.5f) * HeightMultiplier * 2.0f;
    
    // Detalhes finos
    Height += PerlinNoise(X * NoiseScale * 4.0f, Y * NoiseScale * 4.0f) * HeightMultiplier * 0.1f;

    return FMath::Max(0.0f, Height);
}

FBiomeData AProceduralWorldGenerator::DetermineBiome(float X, float Y, float Height) const
{
    if (AvailableBiomes.Num() == 0)
    {
        return FBiomeData(); // Retornar bioma vazio se não há biomas definidos
    }

    // Determinar bioma baseado em altura e noise de temperatura/humidade
    float Temperature = PerlinNoise(X * 0.001f, Y * 0.001f) * 30.0f + 10.0f; // -20 a 40 graus
    float Humidity = PerlinNoise(X * 0.002f + 1000.0f, Y * 0.002f + 1000.0f) * 0.5f + 0.5f; // 0 a 1

    // Ajustar temperatura baseado na altura
    Temperature -= Height * 0.01f; // Mais frio em altitudes maiores

    // Encontrar bioma mais adequado
    int32 BestBiomeIndex = 0;
    float BestScore = FLT_MAX;

    for (int32 i = 0; i < AvailableBiomes.Num(); i++)
    {
        const FBiomeData& Biome = AvailableBiomes[i];
        
        float TempDiff = FMath::Abs(Temperature - Biome.Temperature);
        float HumidityDiff = FMath::Abs(Humidity - Biome.Humidity);
        float ElevationDiff = FMath::Abs(Height - Biome.Elevation) * 0.001f; // Menor peso para elevação
        
        float Score = TempDiff + HumidityDiff * 50.0f + ElevationDiff;
        
        if (Score < BestScore)
        {
            BestScore = Score;
            BestBiomeIndex = i;
        }
    }

    return AvailableBiomes[BestBiomeIndex];
}

void AProceduralWorldGenerator::GenerateVegetation(const FChunkData& ChunkData)
{
    if (ChunkData.BiomeData.VegetationMeshes.Num() == 0) return;

    const int32 VegetationSamples = 100; // Número de tentativas de colocação por chunk
    
    for (int32 i = 0; i < VegetationSamples; i++)
    {
        float LocalX = FMath::RandRange(0.0f, ChunkSize);
        float LocalY = FMath::RandRange(0.0f, ChunkSize);
        float WorldX = ChunkData.ChunkCoordinates.X * ChunkSize + LocalX;
        float WorldY = ChunkData.ChunkCoordinates.Y * ChunkSize + LocalY;
        float Height = GenerateHeight(WorldX, WorldY);

        if (ShouldPlaceVegetation(WorldX, WorldY, Height, ChunkData.BiomeData))
        {
            // Escolher mesh aleatória da vegetação do bioma
            int32 MeshIndex = FMath::RandRange(0, ChunkData.BiomeData.VegetationMeshes.Num() - 1);
            UStaticMesh* VegMesh = ChunkData.BiomeData.VegetationMeshes[MeshIndex];

            if (VegMesh && GetWorld())
            {
                // Spawnar actor de vegetação
                FVector SpawnLocation = FVector(WorldX, WorldY, Height);
                FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                
                AStaticMeshActor* VegActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                    AStaticMeshActor::StaticClass(),
                    SpawnLocation,
                    SpawnRotation
                );

                if (VegActor)
                {
                    VegActor->GetStaticMeshComponent()->SetStaticMesh(VegMesh);
                    
                    // Variação de escala
                    float Scale = FMath::RandRange(0.8f, 1.2f);
                    VegActor->SetActorScale3D(FVector(Scale));
                }
            }
        }
    }
}

bool AProceduralWorldGenerator::ShouldPlaceVegetation(float X, float Y, float Height, const FBiomeData& Biome) const
{
    // Verificar inclinação do terreno
    float SlopeThreshold = 0.7f; // Não colocar vegetação em encostas muito íngremes
    
    // Usar noise para distribuição natural
    float VegetationDensity = PerlinNoise(X * 0.05f, Y * 0.05f) * 0.5f + 0.5f;
    
    // Diferentes densidades para diferentes biomas
    float BiomeDensity = 0.3f; // Padrão
    if (Biome.BiomeName == TEXT("Forest"))
    {
        BiomeDensity = 0.7f;
    }
    else if (Biome.BiomeName == TEXT("Grassland"))
    {
        BiomeDensity = 0.4f;
    }
    else if (Biome.BiomeName == TEXT("Mountain"))
    {
        BiomeDensity = 0.1f;
    }

    return VegetationDensity < BiomeDensity && Height > 10.0f && Height < 1000.0f;
}

void AProceduralWorldGenerator::GenerateStructures(const FChunkData& ChunkData)
{
    // Estruturas são mais raras que vegetação
    const int32 StructureSamples = 5;
    
    for (int32 i = 0; i < StructureSamples; i++)
    {
        float LocalX = FMath::RandRange(0.0f, ChunkSize);
        float LocalY = FMath::RandRange(0.0f, ChunkSize);
        float WorldX = ChunkData.ChunkCoordinates.X * ChunkSize + LocalX;
        float WorldY = ChunkData.ChunkCoordinates.Y * ChunkSize + LocalY;
        float Height = GenerateHeight(WorldX, WorldY);

        if (ShouldPlaceStructure(WorldX, WorldY, Height))
        {
            // Placeholder para estruturas - implementar com assets específicos
            UE_LOG(LogTemp, Log, TEXT("Would place structure at (%f, %f, %f)"), WorldX, WorldY, Height);
        }
    }
}

bool AProceduralWorldGenerator::ShouldPlaceStructure(float X, float Y, float Height) const
{
    // Estruturas são muito raras
    float StructureNoise = PerlinNoise(X * 0.001f, Y * 0.001f);
    return StructureNoise > 0.95f && Height > 50.0f && Height < 500.0f;
}

float AProceduralWorldGenerator::PerlinNoise(float X, float Y) const
{
    // Implementação simplificada de Perlin noise
    return FMath::PerlinNoise2D(FVector2D(X, Y));
}

float AProceduralWorldGenerator::FractalNoise(float X, float Y, int32 Octaves) const
{
    float Result = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    float MaxValue = 0.0f;

    for (int32 i = 0; i < Octaves; i++)
    {
        Result += PerlinNoise(X * Frequency, Y * Frequency) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }

    return Result / MaxValue;
}

float AProceduralWorldGenerator::RidgedNoise(float X, float Y) const
{
    float Noise = PerlinNoise(X, Y);
    return 1.0f - FMath::Abs(Noise);
}

// Implementações das funções públicas
void AProceduralWorldGenerator::RegenerateWorld()
{
    // Limpar chunks existentes
    LoadedChunks.Empty();
    
    // Limpar mesh
    if (TerrainMesh)
    {
        TerrainMesh->ClearAllMeshSections();
    }
    
    // Regenerar
    UpdateChunks();
}

void AProceduralWorldGenerator::SetSeed(int32 NewSeed)
{
    Seed = NewSeed;
    FMath::RandInit(Seed);
    RegenerateWorld();
}

FBiomeData AProceduralWorldGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float Height = GenerateHeight(WorldLocation.X, WorldLocation.Y);
    return DetermineBiome(WorldLocation.X, WorldLocation.Y, Height);
}

float AProceduralWorldGenerator::GetHeightAtLocation(const FVector& WorldLocation) const
{
    return GenerateHeight(WorldLocation.X, WorldLocation.Y);
}