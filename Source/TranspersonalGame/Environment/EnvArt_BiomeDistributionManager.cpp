#include "EnvArt_BiomeDistributionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UEnvArt_BiomeDistributionManager::UEnvArt_BiomeDistributionManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    DistributionPercentagePerBiome = 20.0f; // 20% per biome (5 biomes = 100%)
    bAutoDistributeOnBeginPlay = false;
    
    SetupDefaultBiomes();
}

void UEnvArt_BiomeDistributionManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeCoordinates();
    
    if (bAutoDistributeOnBeginPlay)
    {
        DistributeDinosaursAcrossBiomes();
    }
}

void UEnvArt_BiomeDistributionManager::InitializeBiomeCoordinates()
{
    SetupDefaultBiomes();
    
    UE_LOG(LogTemp, Warning, TEXT("EnvArt_BiomeDistributionManager: Initialized %d biomes"), BiomeCoordinates.Num());
    
    for (const FEnvArt_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome: %s at location (%f, %f, %f)"), 
               *Biome.BiomeName, 
               Biome.CenterLocation.X, 
               Biome.CenterLocation.Y, 
               Biome.CenterLocation.Z);
    }
}

void UEnvArt_BiomeDistributionManager::SetupDefaultBiomes()
{
    BiomeCoordinates.Empty();
    
    // Default biome coordinates from memory
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Savana"), FVector(0, 0, 100), 15000.0f));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Pantano"), FVector(-50000, -45000, 100), 15000.0f));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Floresta"), FVector(-45000, 40000, 100), 15000.0f));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Deserto"), FVector(55000, 0, 100), 15000.0f));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Montanha"), FVector(40000, 50000, 100), 15000.0f));
}

FVector UEnvArt_BiomeDistributionManager::GetRandomLocationInBiome(const FString& BiomeName) const
{
    for (const FEnvArt_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        if (Biome.BiomeName == BiomeName)
        {
            return GetRandomOffsetInRadius(Biome.CenterLocation, Biome.SpawnRadius);
        }
    }
    
    // Return center of first biome if not found
    if (BiomeCoordinates.Num() > 0)
    {
        return BiomeCoordinates[0].CenterLocation;
    }
    
    return FVector::ZeroVector;
}

FVector UEnvArt_BiomeDistributionManager::GetRandomOffsetInRadius(const FVector& Center, float Radius) const
{
    float RandomX = FMath::RandRange(-Radius, Radius);
    float RandomY = FMath::RandRange(-Radius, Radius);
    
    return FVector(Center.X + RandomX, Center.Y + RandomY, Center.Z);
}

TArray<FVector> UEnvArt_BiomeDistributionManager::GetDistributedLocationsAcrossBiomes(int32 TotalCount) const
{
    TArray<FVector> Locations;
    
    if (BiomeCoordinates.Num() == 0 || TotalCount <= 0)
    {
        return Locations;
    }
    
    int32 CountPerBiome = TotalCount / BiomeCoordinates.Num();
    int32 Remainder = TotalCount % BiomeCoordinates.Num();
    
    for (int32 BiomeIndex = 0; BiomeIndex < BiomeCoordinates.Num(); BiomeIndex++)
    {
        const FEnvArt_BiomeCoordinates& Biome = BiomeCoordinates[BiomeIndex];
        int32 SpawnCount = CountPerBiome + (BiomeIndex < Remainder ? 1 : 0);
        
        for (int32 i = 0; i < SpawnCount; i++)
        {
            FVector SpawnLocation = GetRandomOffsetInRadius(Biome.CenterLocation, Biome.SpawnRadius);
            Locations.Add(SpawnLocation);
        }
    }
    
    return Locations;
}

void UEnvArt_BiomeDistributionManager::DistributeActorsAcrossBiomes(const TArray<AActor*>& ActorsToDistribute)
{
    if (ActorsToDistribute.Num() == 0 || BiomeCoordinates.Num() == 0)
    {
        return;
    }
    
    TArray<FVector> DistributedLocations = GetDistributedLocationsAcrossBiomes(ActorsToDistribute.Num());
    
    for (int32 i = 0; i < ActorsToDistribute.Num() && i < DistributedLocations.Num(); i++)
    {
        if (ActorsToDistribute[i])
        {
            ActorsToDistribute[i]->SetActorLocation(DistributedLocations[i]);
            
            FString BiomeName = GetBiomeNameAtLocation(DistributedLocations[i]);
            FString NewLabel = FString::Printf(TEXT("%s_%s_%d"), 
                                             *ActorsToDistribute[i]->GetClass()->GetName(), 
                                             *BiomeName, 
                                             i + 1);
            ActorsToDistribute[i]->SetActorLabel(NewLabel);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvArt_BiomeDistributionManager: Distributed %d actors across biomes"), ActorsToDistribute.Num());
}

FString UEnvArt_BiomeDistributionManager::GetBiomeNameAtLocation(const FVector& Location) const
{
    for (const FEnvArt_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        if (IsLocationInBiome(Location, Biome))
        {
            return Biome.BiomeName;
        }
    }
    
    return TEXT("Unknown");
}

bool UEnvArt_BiomeDistributionManager::IsLocationInBiome(const FVector& Location, const FEnvArt_BiomeCoordinates& Biome) const
{
    float Distance = FVector::Dist2D(Location, Biome.CenterLocation);
    return Distance <= Biome.SpawnRadius;
}

void UEnvArt_BiomeDistributionManager::DistributeDinosaursAcrossBiomes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn different dinosaur types in appropriate biomes
    SpawnDinosaurInBiome(TEXT("TRex"), TEXT("Savana"), 2);
    SpawnDinosaurInBiome(TEXT("TRex"), TEXT("Floresta"), 1);
    SpawnDinosaurInBiome(TEXT("Velociraptor"), TEXT("Savana"), 3);
    SpawnDinosaurInBiome(TEXT("Velociraptor"), TEXT("Floresta"), 2);
    SpawnDinosaurInBiome(TEXT("Brachiosaurus"), TEXT("Floresta"), 2);
    SpawnDinosaurInBiome(TEXT("Triceratops"), TEXT("Savana"), 2);
    SpawnDinosaurInBiome(TEXT("Triceratops"), TEXT("Pantano"), 1);
}

void UEnvArt_BiomeDistributionManager::SpawnDinosaurInBiome(const FString& DinosaurType, const FString& BiomeName, int32 Count)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeName);
        
        // Create a basic static mesh actor as placeholder
        // In a real implementation, this would spawn the actual dinosaur mesh
        AStaticMeshActor* DinosaurActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
        
        if (DinosaurActor)
        {
            FString ActorLabel = FString::Printf(TEXT("%s_%s_%d"), *DinosaurType, *BiomeName, i + 1);
            DinosaurActor->SetActorLabel(ActorLabel);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned %s in %s at location (%f, %f, %f)"), 
                   *ActorLabel, 
                   *BiomeName,
                   SpawnLocation.X, 
                   SpawnLocation.Y, 
                   SpawnLocation.Z);
        }
    }
}