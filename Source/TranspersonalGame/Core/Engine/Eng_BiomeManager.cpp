#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    BiomeRadius = 15000.0f; // 15km radius per biome
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeBiomeCenters();
    UE_LOG(LogTemp, Warning, TEXT("Eng_BiomeManager initialized with 5 biomes"));
}

void UEng_BiomeManager::Deinitialize()
{
    BiomeCenters.Empty();
    BiomeWeights.Empty();
    Super::Deinitialize();
}

void UEng_BiomeManager::InitializeBiomeCenters()
{
    // Initialize the 5 biome centers based on memory coordinates
    BiomeCenters.Empty();
    BiomeCenters.Add(EBiomeType::Savana, FVector(0.0f, 0.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Pantano, FVector(-50000.0f, -45000.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Floresta, FVector(-45000.0f, 40000.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Deserto, FVector(55000.0f, 0.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Montanha, FVector(40000.0f, 50000.0f, 0.0f));

    // Set equal weights for distribution
    BiomeWeights.Empty();
    BiomeWeights.Add(EBiomeType::Savana, 0.2f);
    BiomeWeights.Add(EBiomeType::Pantano, 0.2f);
    BiomeWeights.Add(EBiomeType::Floresta, 0.2f);
    BiomeWeights.Add(EBiomeType::Deserto, 0.2f);
    BiomeWeights.Add(EBiomeType::Montanha, 0.2f);
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savana;

    for (const auto& BiomePair : BiomeCenters)
    {
        float Distance = FVector::Dist2D(WorldLocation, BiomePair.Value);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}

FVector UEng_BiomeManager::GetBiomeCenter(EBiomeType BiomeType) const
{
    if (const FVector* Center = BiomeCenters.Find(BiomeType))
    {
        return *Center;
    }
    return FVector::ZeroVector;
}

TArray<FVector> UEng_BiomeManager::GetSpawnPointsInBiome(EBiomeType BiomeType, int32 NumPoints) const
{
    TArray<FVector> SpawnPoints;
    FVector BiomeCenter = GetBiomeCenter(BiomeType);

    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector RandomPoint = GetRandomPointInBiome(BiomeType);
        if (IsValidSpawnLocation(RandomPoint))
        {
            SpawnPoints.Add(RandomPoint);
        }
    }

    return SpawnPoints;
}

void UEng_BiomeManager::DistributeActorsAcrossBiomes(TArray<AActor*> ActorsToDistribute)
{
    if (ActorsToDistribute.Num() == 0)
    {
        return;
    }

    int32 ActorsPerBiome = ActorsToDistribute.Num() / 5;
    int32 ActorIndex = 0;

    // Distribute actors equally across biomes
    for (const auto& BiomePair : BiomeCenters)
    {
        EBiomeType CurrentBiome = BiomePair.Key;
        TArray<FVector> SpawnPoints = GetSpawnPointsInBiome(CurrentBiome, ActorsPerBiome);

        for (int32 i = 0; i < ActorsPerBiome && ActorIndex < ActorsToDistribute.Num(); i++)
        {
            if (SpawnPoints.IsValidIndex(i) && ActorsToDistribute.IsValidIndex(ActorIndex))
            {
                AActor* Actor = ActorsToDistribute[ActorIndex];
                if (Actor)
                {
                    Actor->SetActorLocation(SpawnPoints[i]);
                    UE_LOG(LogTemp, Log, TEXT("Moved actor %s to biome %d at location %s"), 
                           *Actor->GetName(), (int32)CurrentBiome, *SpawnPoints[i].ToString());
                }
                ActorIndex++;
            }
        }
    }
}

bool UEng_BiomeManager::IsLocationInBiome(FVector Location, EBiomeType BiomeType) const
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float Distance = FVector::Dist2D(Location, BiomeCenter);
    return Distance <= BiomeRadius;
}

void UEng_BiomeManager::SetBiomeRadius(float NewRadius)
{
    BiomeRadius = FMath::Max(1000.0f, NewRadius); // Minimum 1km radius
}

int32 UEng_BiomeManager::GetActorCountInBiome(EBiomeType BiomeType) const
{
    if (!GetWorld())
    {
        return 0;
    }

    int32 Count = 0;
    FVector BiomeCenter = GetBiomeCenter(BiomeType);

    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && IsLocationInBiome(Actor->GetActorLocation(), BiomeType))
        {
            Count++;
        }
    }

    return Count;
}

void UEng_BiomeManager::ValidateBiomeDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME DISTRIBUTION VALIDATION ==="));
    
    for (const auto& BiomePair : BiomeCenters)
    {
        EBiomeType BiomeType = BiomePair.Key;
        int32 ActorCount = GetActorCountInBiome(BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("Biome %d: %d actors"), (int32)BiomeType, ActorCount);
    }
}

FVector UEng_BiomeManager::GetRandomPointInBiome(EBiomeType BiomeType) const
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    // Generate random point within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, BiomeRadius);
    
    float X = BiomeCenter.X + RandomDistance * FMath::Cos(RandomAngle);
    float Y = BiomeCenter.Y + RandomDistance * FMath::Sin(RandomAngle);
    float Z = BiomeCenter.Z + 100.0f; // Spawn slightly above ground
    
    return FVector(X, Y, Z);
}

bool UEng_BiomeManager::IsValidSpawnLocation(FVector Location) const
{
    // Basic validation - can be extended with terrain checks
    return true;
}