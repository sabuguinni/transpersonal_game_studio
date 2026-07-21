#include "World_TerrainManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

AWorld_TerrainManager::AWorld_TerrainManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxChunksPerBiome = 8;
    ChunkSpacing = 15000.0f;
    
    // Initialize default terrain chunks for each biome
    TerrainChunks.Empty();
    
    // Savanna chunks (0, 0)
    for (int32 i = 0; i < 4; i++)
    {
        FWorld_TerrainChunk SavannaChunk;
        SavannaChunk.ChunkLocation = FVector(i * 7500.0f - 11250.0f, 0.0f, 0.0f);
        SavannaChunk.BiomeType = EBiomeType::Savanna;
        SavannaChunk.HeightVariation = 300.0f;
        SavannaChunk.bHasWater = (i == 1); // One chunk with water
        TerrainChunks.Add(SavannaChunk);
    }
    
    // Swamp chunks (-50000, -45000)
    for (int32 i = 0; i < 4; i++)
    {
        FWorld_TerrainChunk SwampChunk;
        SwampChunk.ChunkLocation = FVector(-50000.0f + i * 7500.0f - 11250.0f, -45000.0f, 0.0f);
        SwampChunk.BiomeType = EBiomeType::Swamp;
        SwampChunk.HeightVariation = 150.0f;
        SwampChunk.bHasWater = true; // All swamp chunks have water
        TerrainChunks.Add(SwampChunk);
    }
    
    // Forest chunks (-45000, 40000)
    for (int32 i = 0; i < 4; i++)
    {
        FWorld_TerrainChunk ForestChunk;
        ForestChunk.ChunkLocation = FVector(-45000.0f + i * 7500.0f - 11250.0f, 40000.0f, 0.0f);
        ForestChunk.BiomeType = EBiomeType::Forest;
        ForestChunk.HeightVariation = 600.0f;
        ForestChunk.bHasWater = (i == 2); // One chunk with river
        TerrainChunks.Add(ForestChunk);
    }
    
    // Desert chunks (55000, 0)
    for (int32 i = 0; i < 4; i++)
    {
        FWorld_TerrainChunk DesertChunk;
        DesertChunk.ChunkLocation = FVector(55000.0f + i * 7500.0f - 11250.0f, 0.0f, 0.0f);
        DesertChunk.BiomeType = EBiomeType::Desert;
        DesertChunk.HeightVariation = 800.0f;
        DesertChunk.bHasWater = false; // No water in desert
        TerrainChunks.Add(DesertChunk);
    }
    
    // Mountain chunks (40000, 50000)
    for (int32 i = 0; i < 4; i++)
    {
        FWorld_TerrainChunk MountainChunk;
        MountainChunk.ChunkLocation = FVector(40000.0f + i * 7500.0f - 11250.0f, 50000.0f, 0.0f);
        MountainChunk.BiomeType = EBiomeType::Mountain;
        MountainChunk.HeightVariation = 1200.0f;
        MountainChunk.bHasWater = (i == 0); // Mountain lake
        TerrainChunks.Add(MountainChunk);
    }
}

void AWorld_TerrainManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Clear any excess actors first
    ClearExcessActors();
    
    // Generate terrain for all chunks
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        SpawnTerrainChunk(Chunk);
    }
}

void AWorld_TerrainManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_TerrainManager::GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter)
{
    // Find chunks for this biome
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.BiomeType == BiomeType)
        {
            SpawnTerrainChunk(Chunk);
        }
    }
}

void AWorld_TerrainManager::CreateWaterBodies(EBiomeType BiomeType, FVector BiomeCenter)
{
    if (!WaterPlane || !WaterMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterPlane or WaterMaterial not set"));
        return;
    }
    
    // Find chunks with water for this biome
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.BiomeType == BiomeType && Chunk.bHasWater)
        {
            // Spawn water plane
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            FVector WaterLocation = Chunk.ChunkLocation;
            WaterLocation.Z = 50.0f; // Slightly above ground
            
            AStaticMeshActor* WaterActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                WaterLocation,
                FRotator::ZeroRotator,
                SpawnParams
            );
            
            if (WaterActor)
            {
                UStaticMeshComponent* WaterMesh = WaterActor->GetStaticMeshComponent();
                if (WaterMesh)
                {
                    WaterMesh->SetStaticMesh(WaterPlane);
                    WaterMesh->SetMaterial(0, WaterMaterial);
                    
                    // Scale water based on biome type
                    FVector WaterScale = FVector(5.0f, 5.0f, 1.0f);
                    if (BiomeType == EBiomeType::Swamp)
                    {
                        WaterScale = FVector(8.0f, 8.0f, 1.0f); // Larger water areas in swamp
                    }
                    WaterActor->SetActorScale3D(WaterScale);
                    
                    SpawnedTerrainActors.Add(WaterActor);
                }
            }
        }
    }
}

void AWorld_TerrainManager::ClearExcessActors()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() > 20000)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainManager: %d actors exceed 20k limit, clearing excess"), AllActors.Num());
        
        // Remove oldest spawned terrain actors first
        int32 ActorsToRemove = AllActors.Num() - 18000; // Leave buffer
        for (int32 i = 0; i < FMath::Min(ActorsToRemove, SpawnedTerrainActors.Num()); i++)
        {
            if (SpawnedTerrainActors[i] && IsValid(SpawnedTerrainActors[i]))
            {
                SpawnedTerrainActors[i]->Destroy();
            }
        }
        
        // Clean up the array
        SpawnedTerrainActors.RemoveAll([](AActor* Actor) {
            return !IsValid(Actor);
        });
    }
}

UMaterialInterface* AWorld_TerrainManager::GetMaterialForBiome(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return SavannaMaterial;
        case EBiomeType::Swamp:
            return SwampMaterial;
        case EBiomeType::Forest:
            return ForestMaterial;
        case EBiomeType::Desert:
            return DesertMaterial;
        case EBiomeType::Mountain:
            return MountainMaterial;
        default:
            return SavannaMaterial;
    }
}

void AWorld_TerrainManager::SpawnTerrainChunk(const FWorld_TerrainChunk& Chunk)
{
    if (!BaseTerrain)
    {
        UE_LOG(LogTemp, Warning, TEXT("BaseTerrain mesh not set"));
        return;
    }
    
    // Spawn terrain mesh
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* TerrainActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Chunk.ChunkLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (TerrainActor)
    {
        UStaticMeshComponent* TerrainMesh = TerrainActor->GetStaticMeshComponent();
        if (TerrainMesh)
        {
            TerrainMesh->SetStaticMesh(BaseTerrain);
            ApplyBiomeMaterial(TerrainMesh, Chunk.BiomeType);
            
            // Scale terrain based on chunk size and height variation
            FVector TerrainScale = FVector(
                Chunk.ChunkSize.X / 1000.0f,
                Chunk.ChunkSize.Y / 1000.0f,
                Chunk.HeightVariation / 100.0f
            );
            TerrainActor->SetActorScale3D(TerrainScale);
            
            SpawnedTerrainActors.Add(TerrainActor);
        }
    }
    
    // Create water bodies if needed
    if (Chunk.bHasWater)
    {
        CreateWaterBodies(Chunk.BiomeType, Chunk.ChunkLocation);
    }
}

void AWorld_TerrainManager::ApplyBiomeMaterial(UStaticMeshComponent* MeshComp, EBiomeType BiomeType)
{
    if (!MeshComp)
    {
        return;
    }
    
    UMaterialInterface* Material = GetMaterialForBiome(BiomeType);
    if (Material)
    {
        MeshComp->SetMaterial(0, Material);
    }
}