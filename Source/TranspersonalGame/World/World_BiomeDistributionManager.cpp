#include "World_BiomeDistributionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"

UWorld_BiomeDistributionManager::UWorld_BiomeDistributionManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoDistributeOnStart = false;
    MaxTotalActors = 50000;
    MinDistanceBetweenActors = 500.0f;
    
    SetupDefaultBiomes();
}

void UWorld_BiomeDistributionManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoDistributeOnStart)
    {
        InitializeBiomeAreas();
    }
}

void UWorld_BiomeDistributionManager::SetupDefaultBiomes()
{
    BiomeSpawnAreas.Empty();
    
    // Setup the 5 biomes as per memory requirements
    FWorld_BiomeSpawnData Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.SpawnRadius = 15000.0f;
    Savana.MaxActorsPerBiome = 10000;
    BiomeSpawnAreas.Add(Savana);
    
    FWorld_BiomeSpawnData Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    Pantano.SpawnRadius = 15000.0f;
    Pantano.MaxActorsPerBiome = 10000;
    BiomeSpawnAreas.Add(Pantano);
    
    FWorld_BiomeSpawnData Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.SpawnRadius = 15000.0f;
    Floresta.MaxActorsPerBiome = 10000;
    BiomeSpawnAreas.Add(Floresta);
    
    FWorld_BiomeSpawnData Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    Deserto.SpawnRadius = 15000.0f;
    Deserto.MaxActorsPerBiome = 10000;
    BiomeSpawnAreas.Add(Deserto);
    
    FWorld_BiomeSpawnData Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    Montanha.SpawnRadius = 15000.0f;
    Montanha.MaxActorsPerBiome = 10000;
    BiomeSpawnAreas.Add(Montanha);
}

void UWorld_BiomeDistributionManager::InitializeBiomeAreas()
{
    if (BiomeSpawnAreas.Num() == 0)
    {
        SetupDefaultBiomes();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeDistributionManager: Initialized %d biome areas"), BiomeSpawnAreas.Num());
}

void UWorld_BiomeDistributionManager::DistributeActorsAcrossBiomes()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Remove actors that are already distributed
    TArray<AActor*> ActorsToDistribute;
    for (AActor* Actor : AllActors)
    {
        if (Actor && !Actor->IsA<APawn>() && Actor->GetName().Contains(TEXT("StaticMeshActor")))
        {
            ActorsToDistribute.Add(Actor);
        }
    }
    
    if (ActorsToDistribute.Num() == 0) return;
    
    int32 ActorsPerBiome = ActorsToDistribute.Num() / BiomeSpawnAreas.Num();
    int32 ActorIndex = 0;
    
    for (const FWorld_BiomeSpawnData& BiomeData : BiomeSpawnAreas)
    {
        int32 ActorsForThisBiome = FMath::Min(ActorsPerBiome, ActorsToDistribute.Num() - ActorIndex);
        
        for (int32 i = 0; i < ActorsForThisBiome && ActorIndex < ActorsToDistribute.Num(); i++, ActorIndex++)
        {
            AActor* Actor = ActorsToDistribute[ActorIndex];
            if (Actor)
            {
                FVector NewLocation = GetRandomLocationInBiome(BiomeData.BiomeName);
                Actor->SetActorLocation(NewLocation);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeDistributionManager: Distributed %d actors across %d biomes"), 
           ActorIndex, BiomeSpawnAreas.Num());
}

FVector UWorld_BiomeDistributionManager::GetRandomLocationInBiome(const FString& BiomeName)
{
    for (const FWorld_BiomeSpawnData& BiomeData : BiomeSpawnAreas)
    {
        if (BiomeData.BiomeName == BiomeName)
        {
            float RandomX = FMath::RandRange(-BiomeData.SpawnRadius, BiomeData.SpawnRadius);
            float RandomY = FMath::RandRange(-BiomeData.SpawnRadius, BiomeData.SpawnRadius);
            
            return BiomeData.CenterLocation + FVector(RandomX, RandomY, 0.0f);
        }
    }
    
    return FVector::ZeroVector;
}

FString UWorld_BiomeDistributionManager::GetBiomeAtLocation(const FVector& Location)
{
    for (const FWorld_BiomeSpawnData& BiomeData : BiomeSpawnAreas)
    {
        float Distance = FVector::Dist2D(Location, BiomeData.CenterLocation);
        if (Distance <= BiomeData.SpawnRadius)
        {
            return BiomeData.BiomeName;
        }
    }
    
    return TEXT("Unknown");
}

int32 UWorld_BiomeDistributionManager::GetActorCountInBiome(const FString& BiomeName)
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 Count = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && GetBiomeAtLocation(Actor->GetActorLocation()) == BiomeName)
        {
            Count++;
        }
    }
    
    return Count;
}

void UWorld_BiomeDistributionManager::SpawnActorInBiome(const FString& BiomeName, UClass* ActorClass, int32 Count)
{
    if (!ActorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeName);
        if (IsLocationValidForSpawn(SpawnLocation, BiomeName))
        {
            FRotator SpawnRotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
            
            AActor* NewActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParams);
            if (NewActor)
            {
                UE_LOG(LogTemp, Log, TEXT("Spawned %s in biome %s at %s"), 
                       *ActorClass->GetName(), *BiomeName, *SpawnLocation.ToString());
            }
        }
    }
}

void UWorld_BiomeDistributionManager::RebalanceBiomeDistribution()
{
    // Count actors in each biome
    TMap<FString, int32> BiomeCounts;
    for (const FWorld_BiomeSpawnData& BiomeData : BiomeSpawnAreas)
    {
        BiomeCounts.Add(BiomeData.BiomeName, GetActorCountInBiome(BiomeData.BiomeName));
    }
    
    // Find biome with most actors
    FString OverpopulatedBiome;
    int32 MaxCount = 0;
    for (const auto& Pair : BiomeCounts)
    {
        if (Pair.Value > MaxCount)
        {
            MaxCount = Pair.Value;
            OverpopulatedBiome = Pair.Key;
        }
    }
    
    // Move excess actors to less populated biomes
    if (MaxCount > 10000) // Threshold for rebalancing
    {
        DistributeActorsAcrossBiomes();
    }
}

void UWorld_BiomeDistributionManager::DebugPrintBiomeStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME DISTRIBUTION STATS ==="));
    
    int32 TotalActors = 0;
    for (const FWorld_BiomeSpawnData& BiomeData : BiomeSpawnAreas)
    {
        int32 Count = GetActorCountInBiome(BiomeData.BiomeName);
        TotalActors += Count;
        
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors at center %s"), 
               *BiomeData.BiomeName, Count, *BiomeData.CenterLocation.ToString());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total actors across all biomes: %d"), TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("=== END BIOME STATS ==="));
}

bool UWorld_BiomeDistributionManager::IsLocationValidForSpawn(const FVector& Location, const FString& BiomeName)
{
    // Check if location is within biome bounds
    if (GetBiomeAtLocation(Location) != BiomeName)
    {
        return false;
    }
    
    // Check minimum distance from other actors
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Location) < MinDistanceBetweenActors)
        {
            return false;
        }
    }
    
    return true;
}

TArray<AActor*> UWorld_BiomeDistributionManager::GetActorsInBiome(const FString& BiomeName)
{
    TArray<AActor*> BiomeActors;
    UWorld* World = GetWorld();
    if (!World) return BiomeActors;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && GetBiomeAtLocation(Actor->GetActorLocation()) == BiomeName)
        {
            BiomeActors.Add(Actor);
        }
    }
    
    return BiomeActors;
}