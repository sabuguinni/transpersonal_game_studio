#include "World_CaveSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"

AWorld_CaveSystem::AWorld_CaveSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    MaxCavesPerBiome = 5;
    CaveSpawnRadius = 15000.0f;
    MinCaveDistance = 2000.0f;
    bAutoGenerateCaves = true;
    bConnectBiomes = true;

    SetupCaveComponents();
    InitializeCaveSystem();
}

void AWorld_CaveSystem::SetupCaveComponents()
{
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create cave entrance mesh component
    CaveEntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveEntranceMesh"));
    CaveEntranceMesh->SetupAttachment(RootComponent);

    // Set default cylinder mesh for cave entrance
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        CaveEntranceMesh->SetStaticMesh(CylinderMesh.Object);
        CaveEntranceMesh->SetWorldScale3D(FVector(3.0f, 3.0f, 0.5f));
    }

    // Create collision component
    CaveCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CaveCollision"));
    CaveCollision->SetupAttachment(RootComponent);
    CaveCollision->SetBoxExtent(FVector(300.0f, 300.0f, 100.0f));
}

void AWorld_CaveSystem::InitializeCaveSystem()
{
    // Initialize cave entrance array
    CaveEntrances.Empty();
    UndergroundTunnels.Empty();

    if (bAutoGenerateCaves)
    {
        GenerateCaveEntrances();
    }

    if (bConnectBiomes)
    {
        CreateUndergroundTunnels();
    }
}

void AWorld_CaveSystem::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateCaves && CaveEntrances.Num() == 0)
    {
        GenerateCaveEntrances();
    }

    if (bConnectBiomes && UndergroundTunnels.Num() == 0)
    {
        CreateUndergroundTunnels();
    }
}

void AWorld_CaveSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_CaveSystem::GenerateCaveEntrances()
{
    CaveEntrances.Empty();

    // Define biome centers
    TArray<TPair<EBiomeType, FVector>> BiomeCenters = {
        {EBiomeType::Savanna, FVector(0.0f, 0.0f, 0.0f)},
        {EBiomeType::Swamp, FVector(-50000.0f, -45000.0f, 0.0f)},
        {EBiomeType::Forest, FVector(-45000.0f, 40000.0f, 0.0f)},
        {EBiomeType::Desert, FVector(55000.0f, 0.0f, 0.0f)},
        {EBiomeType::Mountain, FVector(40000.0f, 50000.0f, 0.0f)}
    };

    for (const auto& BiomePair : BiomeCenters)
    {
        EBiomeType BiomeType = BiomePair.Key;
        FVector BiomeCenter = BiomePair.Value;

        for (int32 i = 0; i < MaxCavesPerBiome; i++)
        {
            FVector CaveLocation = GetRandomLocationInBiome(BiomeType);
            if (IsCaveLocationValid(CaveLocation))
            {
                FWorld_CaveEntrance NewCave = CreateCaveAtLocation(CaveLocation, BiomeType);
                CaveEntrances.Add(NewCave);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated %d cave entrances"), CaveEntrances.Num());
}

void AWorld_CaveSystem::CreateUndergroundTunnels()
{
    UndergroundTunnels.Empty();

    // Create tunnels between different biomes
    TArray<EBiomeType> BiomeTypes = {
        EBiomeType::Savanna,
        EBiomeType::Swamp,
        EBiomeType::Forest,
        EBiomeType::Desert,
        EBiomeType::Mountain
    };

    for (int32 i = 0; i < BiomeTypes.Num(); i++)
    {
        for (int32 j = i + 1; j < BiomeTypes.Num(); j++)
        {
            // Create tunnel between biome i and biome j
            TArray<FWorld_CaveEntrance> StartBiomeCaves = GetCavesInBiome(BiomeTypes[i]);
            TArray<FWorld_CaveEntrance> EndBiomeCaves = GetCavesInBiome(BiomeTypes[j]);

            if (StartBiomeCaves.Num() > 0 && EndBiomeCaves.Num() > 0)
            {
                FWorld_UndergroundTunnel NewTunnel;
                NewTunnel.StartLocation = StartBiomeCaves[0].Location;
                NewTunnel.EndLocation = EndBiomeCaves[0].Location;
                NewTunnel.StartBiome = BiomeTypes[i];
                NewTunnel.EndBiome = BiomeTypes[j];
                NewTunnel.TunnelWidth = 400.0f;
                NewTunnel.TunnelHeight = 300.0f;

                UndergroundTunnels.Add(NewTunnel);

                // Mark caves as connected
                for (auto& Cave : CaveEntrances)
                {
                    if ((Cave.Location - NewTunnel.StartLocation).Size() < 100.0f ||
                        (Cave.Location - NewTunnel.EndLocation).Size() < 100.0f)
                    {
                        Cave.bIsConnected = true;
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Created %d underground tunnels"), UndergroundTunnels.Num());
}

void AWorld_CaveSystem::ConnectNearestCaves()
{
    for (int32 i = 0; i < CaveEntrances.Num(); i++)
    {
        if (CaveEntrances[i].bIsConnected) continue;

        float NearestDistance = FLT_MAX;
        int32 NearestIndex = -1;

        for (int32 j = 0; j < CaveEntrances.Num(); j++)
        {
            if (i == j || CaveEntrances[j].BiomeType == CaveEntrances[i].BiomeType) continue;

            float Distance = FVector::Dist(CaveEntrances[i].Location, CaveEntrances[j].Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestIndex = j;
            }
        }

        if (NearestIndex >= 0 && NearestDistance < 50000.0f)
        {
            FWorld_UndergroundTunnel NewTunnel;
            NewTunnel.StartLocation = CaveEntrances[i].Location;
            NewTunnel.EndLocation = CaveEntrances[NearestIndex].Location;
            NewTunnel.StartBiome = CaveEntrances[i].BiomeType;
            NewTunnel.EndBiome = CaveEntrances[NearestIndex].BiomeType;

            UndergroundTunnels.Add(NewTunnel);

            CaveEntrances[i].bIsConnected = true;
            CaveEntrances[NearestIndex].bIsConnected = true;
        }
    }
}

FWorld_CaveEntrance AWorld_CaveSystem::CreateCaveAtLocation(FVector Location, EBiomeType Biome)
{
    FWorld_CaveEntrance NewCave;
    NewCave.Location = Location;
    NewCave.Rotation = FRotator(0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f), 0.0f);
    NewCave.BiomeType = Biome;
    NewCave.Depth = UKismetMathLibrary::RandomFloatInRange(300.0f, 800.0f);
    NewCave.Width = UKismetMathLibrary::RandomFloatInRange(200.0f, 500.0f);
    NewCave.bIsConnected = false;

    return NewCave;
}

bool AWorld_CaveSystem::IsCaveLocationValid(FVector Location)
{
    // Check minimum distance from other caves
    for (const FWorld_CaveEntrance& ExistingCave : CaveEntrances)
    {
        if (FVector::Dist(Location, ExistingCave.Location) < MinCaveDistance)
        {
            return false;
        }
    }

    // Check if location is not too close to water or other obstacles
    return true;
}

TArray<FWorld_CaveEntrance> AWorld_CaveSystem::GetCavesInBiome(EBiomeType Biome)
{
    TArray<FWorld_CaveEntrance> BiomeCaves;

    for (const FWorld_CaveEntrance& Cave : CaveEntrances)
    {
        if (Cave.BiomeType == Biome)
        {
            BiomeCaves.Add(Cave);
        }
    }

    return BiomeCaves;
}

float AWorld_CaveSystem::GetDistanceToCave(FVector Location)
{
    float MinDistance = FLT_MAX;

    for (const FWorld_CaveEntrance& Cave : CaveEntrances)
    {
        float Distance = FVector::Dist(Location, Cave.Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
        }
    }

    return MinDistance;
}

void AWorld_CaveSystem::RegenerateCaveSystem()
{
    ClearAllCaves();
    GenerateCaveEntrances();
    CreateUndergroundTunnels();
    ConnectNearestCaves();
}

void AWorld_CaveSystem::ClearAllCaves()
{
    CaveEntrances.Empty();
    UndergroundTunnels.Empty();
}

FVector AWorld_CaveSystem::GetRandomLocationInBiome(EBiomeType Biome)
{
    FVector BiomeCenter;

    switch (Biome)
    {
    case EBiomeType::Savanna:
        BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
        break;
    case EBiomeType::Swamp:
        BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
        break;
    case EBiomeType::Forest:
        BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
        break;
    case EBiomeType::Desert:
        BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
        break;
    case EBiomeType::Mountain:
        BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
        break;
    default:
        BiomeCenter = FVector::ZeroVector;
        break;
    }

    float RandomX = UKismetMathLibrary::RandomFloatInRange(-CaveSpawnRadius, CaveSpawnRadius);
    float RandomY = UKismetMathLibrary::RandomFloatInRange(-CaveSpawnRadius, CaveSpawnRadius);
    float RandomZ = UKismetMathLibrary::RandomFloatInRange(50.0f, 200.0f);

    return BiomeCenter + FVector(RandomX, RandomY, RandomZ);
}

bool AWorld_CaveSystem::IsLocationUnderground(FVector Location)
{
    return Location.Z < 0.0f;
}