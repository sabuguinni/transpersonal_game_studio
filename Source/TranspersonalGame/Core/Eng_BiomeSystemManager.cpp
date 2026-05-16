#include "Eng_BiomeSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

AEng_BiomeSystemManager::AEng_BiomeSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default parameters
    BiomeTransitionRadius = 5000.0f; // 50m transition zone
    MaxAssetSpawnDistance = 50000.0f; // 500m max spawn distance
    TargetActorsPerBiome = 500; // Target from memory requirements
    BiomeUpdateInterval = 2.0f; // Update every 2 seconds
    BiomeUpdateTimer = 0.0f;
    CurrentBiome = EBiomeType::Savana;
    LastPlayerLocation = FVector::ZeroVector;
}

void AEng_BiomeSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - BiomeSystemManager initializing..."));
    
    InitializeBiomeSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager initialized with %d biomes"), BiomeDefinitions.Num());
}

void AEng_BiomeSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    BiomeUpdateTimer += DeltaTime;
    
    if (BiomeUpdateTimer >= BiomeUpdateInterval)
    {
        // Get player location
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            UpdateCurrentBiome(PlayerLocation);
            LastPlayerLocation = PlayerLocation;
        }
        
        BiomeUpdateTimer = 0.0f;
    }
}

void AEng_BiomeSystemManager::InitializeBiomeSystem()
{
    SetupDefaultBiomes();
    SetupEnvironmentalParameters();
    
    UE_LOG(LogTemp, Warning, TEXT("Biome system initialized with %d biome definitions"), BiomeDefinitions.Num());
}

void AEng_BiomeSystemManager::SetupDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana (Center) - coordinates from memory ID 709
    FBiomeDefinition SavanaBiome;
    SavanaBiome.BiomeType = EBiomeType::Savana;
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaBiome.Radius = 25000.0f; // 250m radius
    SavanaBiome.Description = TEXT("Central grassland with scattered acacia trees");
    BiomeDefinitions.Add(SavanaBiome);
    
    // Pantano (Southwest) - coordinates from memory ID 709
    FBiomeDefinition PantanoBiome;
    PantanoBiome.BiomeType = EBiomeType::Pantano;
    PantanoBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoBiome.Radius = 25000.0f;
    PantanoBiome.Description = TEXT("Wetland swamp with dense vegetation");
    BiomeDefinitions.Add(PantanoBiome);
    
    // Floresta (Northwest) - coordinates from memory ID 709
    FBiomeDefinition FlorestaBiome;
    FlorestaBiome.BiomeType = EBiomeType::Floresta;
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaBiome.Radius = 25000.0f;
    FlorestaBiome.Description = TEXT("Dense prehistoric forest");
    BiomeDefinitions.Add(FlorestaBiome);
    
    // Deserto (East) - coordinates from memory ID 709
    FBiomeDefinition DesertoBiome;
    DesertoBiome.BiomeType = EBiomeType::Deserto;
    DesertoBiome.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoBiome.Radius = 25000.0f;
    DesertoBiome.Description = TEXT("Arid desert with rock formations");
    BiomeDefinitions.Add(DesertoBiome);
    
    // Montanha (Northeast) - coordinates from memory ID 709
    FBiomeDefinition MontanhaBiome;
    MontanhaBiome.BiomeType = EBiomeType::Montanha;
    MontanhaBiome.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaBiome.Radius = 25000.0f;
    MontanhaBiome.Description = TEXT("Rocky mountain terrain");
    BiomeDefinitions.Add(MontanhaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Setup %d default biomes"), BiomeDefinitions.Num());
}

void AEng_BiomeSystemManager::SetupEnvironmentalParameters()
{
    // Temperature ranges (Celsius) - Cretaceous period was warmer
    BiomeTemperatureRanges.Add(EBiomeType::Savana, FVector2D(25.0f, 35.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Pantano, FVector2D(28.0f, 38.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Floresta, FVector2D(22.0f, 32.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Deserto, FVector2D(30.0f, 45.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Montanha, FVector2D(15.0f, 25.0f));
    
    // Humidity levels (0-1)
    BiomeHumidityLevels.Add(EBiomeType::Savana, 0.4f);
    BiomeHumidityLevels.Add(EBiomeType::Pantano, 0.9f);
    BiomeHumidityLevels.Add(EBiomeType::Floresta, 0.8f);
    BiomeHumidityLevels.Add(EBiomeType::Deserto, 0.1f);
    BiomeHumidityLevels.Add(EBiomeType::Montanha, 0.5f);
    
    // Danger levels (0-1) - based on predator density
    BiomeDangerLevels.Add(EBiomeType::Savana, 0.6f);
    BiomeDangerLevels.Add(EBiomeType::Pantano, 0.8f);
    BiomeDangerLevels.Add(EBiomeType::Floresta, 0.7f);
    BiomeDangerLevels.Add(EBiomeType::Deserto, 0.4f);
    BiomeDangerLevels.Add(EBiomeType::Montanha, 0.5f);
}

EBiomeType AEng_BiomeSystemManager::GetBiomeAtLocation(FVector WorldLocation)
{
    float ClosestDistance = MAX_FLT;
    EBiomeType ClosestBiome = EBiomeType::Savana;
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        
        if (Distance <= Biome.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

float AEng_BiomeSystemManager::GetDistanceToBiomeCenter(FVector WorldLocation, EBiomeType BiomeType)
{
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        }
    }
    
    return -1.0f; // Biome not found
}

void AEng_BiomeSystemManager::SpawnBiomeAssets(EBiomeType BiomeType, int32 AssetCount)
{
    // Clear existing assets first
    ClearBiomeAssets(BiomeType);
    
    // Find biome definition
    FBiomeDefinition* TargetBiome = nullptr;
    for (FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            TargetBiome = &Biome;
            break;
        }
    }
    
    if (!TargetBiome)
    {
        UE_LOG(LogTemp, Error, TEXT("Biome type %d not found"), (int32)BiomeType);
        return;
    }
    
    // Get or create asset array for this biome
    TArray<AActor*>& BiomeActors = SpawnedBiomeActors.FindOrAdd(BiomeType);
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning %d assets for biome %d at location %s"), 
           AssetCount, (int32)BiomeType, *TargetBiome->CenterLocation.ToString());
    
    // Spawn basic placeholder assets (will be replaced with real assets later)
    for (int32 i = 0; i < AssetCount; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocationInBiome(BiomeType);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Create basic static mesh actor as placeholder
            AStaticMeshActor* SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator
            );
            
            if (SpawnedActor)
            {
                // Set actor label for identification
                FString ActorLabel = FString::Printf(TEXT("BiomeAsset_%s_%d"), 
                                                   *UEnum::GetValueAsString(BiomeType), i);
                SpawnedActor->SetActorLabel(ActorLabel);
                
                BiomeActors.Add(SpawnedActor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d actors for biome %d"), BiomeActors.Num(), (int32)BiomeType);
}

void AEng_BiomeSystemManager::ClearBiomeAssets(EBiomeType BiomeType)
{
    TArray<AActor*>* BiomeActors = SpawnedBiomeActors.Find(BiomeType);
    
    if (BiomeActors)
    {
        int32 ClearedCount = 0;
        for (AActor* Actor : *BiomeActors)
        {
            if (IsValid(Actor))
            {
                Actor->Destroy();
                ClearedCount++;
            }
        }
        
        BiomeActors->Empty();
        UE_LOG(LogTemp, Warning, TEXT("Cleared %d assets from biome %d"), ClearedCount, (int32)BiomeType);
    }
}

FBiomeEnvironmentalData AEng_BiomeSystemManager::GetBiomeEnvironmentalData(EBiomeType BiomeType)
{
    FBiomeEnvironmentalData Data;
    Data.BiomeType = BiomeType;
    
    // Get temperature range
    if (FVector2D* TempRange = BiomeTemperatureRanges.Find(BiomeType))
    {
        Data.TemperatureRange = *TempRange;
    }
    
    // Get humidity
    if (float* Humidity = BiomeHumidityLevels.Find(BiomeType))
    {
        Data.HumidityLevel = *Humidity;
    }
    
    // Get danger level
    if (float* Danger = BiomeDangerLevels.Find(BiomeType))
    {
        Data.DangerLevel = *Danger;
    }
    
    return Data;
}

void AEng_BiomeSystemManager::UpdateCurrentBiome(FVector PlayerLocation)
{
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        UE_LOG(LogTemp, Warning, TEXT("Player entered biome: %s"), *UEnum::GetValueAsString(CurrentBiome));
        
        // Trigger biome change events here
        // This could notify other systems about biome changes
    }
}

bool AEng_BiomeSystemManager::ValidateBiomeSystem()
{
    bool bIsValid = true;
    
    // Check biome definitions
    if (BiomeDefinitions.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("Expected 5 biomes, found %d"), BiomeDefinitions.Num());
        bIsValid = false;
    }
    
    // Check environmental parameters
    if (BiomeTemperatureRanges.Num() != 5 || 
        BiomeHumidityLevels.Num() != 5 || 
        BiomeDangerLevels.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing environmental parameters"));
        bIsValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome system validation: %s"), bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    return bIsValid;
}

FString AEng_BiomeSystemManager::GetBiomeSystemStatus()
{
    FString Status = TEXT("=== BIOME SYSTEM STATUS ===\n");
    
    Status += FString::Printf(TEXT("Total Biomes: %d\n"), BiomeDefinitions.Num());
    Status += FString::Printf(TEXT("Current Biome: %s\n"), *UEnum::GetValueAsString(CurrentBiome));
    
    // Count spawned actors per biome
    TMap<EBiomeType, int32> ActorCounts = GetSpawnedActorCounts();
    
    for (const auto& Pair : ActorCounts)
    {
        Status += FString::Printf(TEXT("Biome %s: %d actors\n"), 
                                *UEnum::GetValueAsString(Pair.Key), Pair.Value);
    }
    
    return Status;
}

TMap<EBiomeType, int32> AEng_BiomeSystemManager::GetSpawnedActorCounts()
{
    TMap<EBiomeType, int32> Counts;
    
    for (const auto& Pair : SpawnedBiomeActors)
    {
        int32 ValidActorCount = 0;
        for (AActor* Actor : Pair.Value)
        {
            if (IsValid(Actor))
            {
                ValidActorCount++;
            }
        }
        Counts.Add(Pair.Key, ValidActorCount);
    }
    
    return Counts;
}

FVector AEng_BiomeSystemManager::GetRandomSpawnLocationInBiome(EBiomeType BiomeType)
{
    // Find biome center
    FVector BiomeCenter = FVector::ZeroVector;
    float BiomeRadius = 25000.0f;
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            BiomeCenter = Biome.CenterLocation;
            BiomeRadius = Biome.Radius;
            break;
        }
    }
    
    // Generate random location within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, BiomeRadius * 0.8f); // Stay within 80% of radius
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return BiomeCenter + RandomOffset;
}

bool AEng_BiomeSystemManager::IsValidSpawnLocation(FVector Location)
{
    // Basic validation - check if location is not overlapping with existing actors
    // This is a simplified check - in production would use more sophisticated collision detection
    
    TArray<AActor*> AllActors = SpawnedBiomeActors.FindRef(CurrentBiome);
    
    for (AActor* Actor : AllActors)
    {
        if (IsValid(Actor))
        {
            float Distance = FVector::Dist(Location, Actor->GetActorLocation());
            if (Distance < 500.0f) // Minimum 5m separation
            {
                return false;
            }
        }
    }
    
    return true;
}