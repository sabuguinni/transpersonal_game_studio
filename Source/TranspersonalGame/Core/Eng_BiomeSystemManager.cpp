#include "Eng_BiomeSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Math/UnrealMathUtility.h"

AEng_BiomeSystemManager::AEng_BiomeSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default parameters
    BiomeTransitionRadius = 5000.0f; // 50m transition zone
    MaxAssetSpawnDistance = 50000.0f; // 500m spawn radius
    TargetActorsPerBiome = 500; // Target from memory requirements
    BiomeUpdateInterval = 2.0f; // Update every 2 seconds
    BiomeUpdateTimer = 0.0f;
    
    CurrentBiome = EBiomeType::Savana;
    LastPlayerLocation = FVector::ZeroVector;
}

void AEng_BiomeSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: BiomeSystemManager BeginPlay - Initializing biome system"));
    
    InitializeBiomeSystem();
}

void AEng_BiomeSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    BiomeUpdateTimer += DeltaTime;
    
    if (BiomeUpdateTimer >= BiomeUpdateInterval)
    {
        BiomeUpdateTimer = 0.0f;
        
        // Get player location and update current biome
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            UpdateCurrentBiome(PlayerLocation);
        }
    }
}

void AEng_BiomeSystemManager::InitializeBiomeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Initializing biome system with 5 biomes"));
    
    SetupDefaultBiomes();
    SetupEnvironmentalParameters();
    
    // Validate system after initialization
    bool bSystemValid = ValidateBiomeSystem();
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Biome system validation: %s"), 
           bSystemValid ? TEXT("PASSED") : TEXT("FAILED"));
}

void AEng_BiomeSystemManager::SetupDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Biome coordinates from memory ID 709
    FBiomeDefinition SavanaBiome;
    SavanaBiome.BiomeType = EBiomeType::Savana;
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaBiome.Radius = 25000.0f; // 250m radius
    SavanaBiome.BiomeName = TEXT("Cretaceous Savanna");
    BiomeDefinitions.Add(SavanaBiome);
    
    FBiomeDefinition PantanoBiome;
    PantanoBiome.BiomeType = EBiomeType::Pantano;
    PantanoBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoBiome.Radius = 25000.0f;
    PantanoBiome.BiomeName = TEXT("Cretaceous Swampland");
    BiomeDefinitions.Add(PantanoBiome);
    
    FBiomeDefinition FlorestaBiome;
    FlorestaBiome.BiomeType = EBiomeType::Floresta;
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaBiome.Radius = 25000.0f;
    FlorestaBiome.BiomeName = TEXT("Cretaceous Forest");
    BiomeDefinitions.Add(FlorestaBiome);
    
    FBiomeDefinition DesertoBiome;
    DesertoBiome.BiomeType = EBiomeType::Deserto;
    DesertoBiome.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoBiome.Radius = 25000.0f;
    DesertoBiome.BiomeName = TEXT("Cretaceous Desert");
    BiomeDefinitions.Add(DesertoBiome);
    
    FBiomeDefinition MontanhaBiome;
    MontanhaBiome.BiomeType = EBiomeType::Montanha;
    MontanhaBiome.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaBiome.Radius = 25000.0f;
    MontanhaBiome.BiomeName = TEXT("Cretaceous Mountains");
    BiomeDefinitions.Add(MontanhaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Setup %d biome definitions"), BiomeDefinitions.Num());
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
    BiomeHumidityLevels.Add(EBiomeType::Montanha, 0.6f);
    
    // Danger levels (0-1) - based on dinosaur populations
    BiomeDangerLevels.Add(EBiomeType::Savana, 0.5f); // Moderate - mixed herbivores/carnivores
    BiomeDangerLevels.Add(EBiomeType::Pantano, 0.8f); // High - crocodilians and predators
    BiomeDangerLevels.Add(EBiomeType::Floresta, 0.7f); // High - dense predator territory
    BiomeDangerLevels.Add(EBiomeType::Deserto, 0.3f); // Low - sparse populations
    BiomeDangerLevels.Add(EBiomeType::Montanha, 0.4f); // Moderate - territorial species
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Setup environmental parameters for %d biomes"), 
           BiomeTemperatureRanges.Num());
}

EBiomeType AEng_BiomeSystemManager::GetBiomeAtLocation(FVector WorldLocation)
{
    float MinDistance = MAX_FLT;
    EBiomeType ClosestBiome = EBiomeType::Savana;
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist(WorldLocation, Biome.CenterLocation);
        
        // If within biome radius, return immediately
        if (Distance <= Biome.Radius)
        {
            return Biome.BiomeType;
        }
        
        // Track closest biome for fallback
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
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
            return FVector::Dist(WorldLocation, Biome.CenterLocation);
        }
    }
    
    return -1.0f; // Biome not found
}

void AEng_BiomeSystemManager::SpawnBiomeAssets(EBiomeType BiomeType, int32 AssetCount)
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Spawning %d assets for biome %d"), 
           AssetCount, (int32)BiomeType);
    
    // Clear existing assets first
    ClearBiomeAssets(BiomeType);
    
    // Find biome definition
    FBiomeDefinition* BiomeDef = nullptr;
    for (FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            BiomeDef = &Biome;
            break;
        }
    }
    
    if (!BiomeDef)
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Biome definition not found for type %d"), (int32)BiomeType);
        return;
    }
    
    TArray<AActor*> NewSpawnedActors;
    
    // Spawn basic placeholder assets (will be replaced with real assets later)
    for (int32 i = 0; i < AssetCount; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocationInBiome(BiomeType);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Spawn basic static mesh actor as placeholder
            AStaticMeshActor* SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator
            );
            
            if (SpawnedActor)
            {
                SpawnedActor->SetActorLabel(FString::Printf(TEXT("BiomeAsset_%s_%d"), 
                    *UEnum::GetValueAsString(BiomeType), i));
                NewSpawnedActors.Add(SpawnedActor);
            }
        }
    }
    
    SpawnedBiomeActors.Add(BiomeType, NewSpawnedActors);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Successfully spawned %d assets for biome %s"), 
           NewSpawnedActors.Num(), *UEnum::GetValueAsString(BiomeType));
}

void AEng_BiomeSystemManager::ClearBiomeAssets(EBiomeType BiomeType)
{
    if (TArray<AActor*>* ExistingActors = SpawnedBiomeActors.Find(BiomeType))
    {
        int32 ClearedCount = 0;
        for (AActor* Actor : *ExistingActors)
        {
            if (IsValid(Actor))
            {
                Actor->Destroy();
                ClearedCount++;
            }
        }
        
        ExistingActors->Empty();
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Cleared %d assets from biome %s"), 
               ClearedCount, *UEnum::GetValueAsString(BiomeType));
    }
}

FBiomeEnvironmentalData AEng_BiomeSystemManager::GetBiomeEnvironmentalData(EBiomeType BiomeType)
{
    FBiomeEnvironmentalData Data;
    Data.BiomeType = BiomeType;
    
    // Get temperature range
    if (FVector2D* TempRange = BiomeTemperatureRanges.Find(BiomeType))
    {
        Data.Temperature = FMath::RandRange(TempRange->X, TempRange->Y);
    }
    else
    {
        Data.Temperature = 25.0f; // Default
    }
    
    // Get humidity
    if (float* Humidity = BiomeHumidityLevels.Find(BiomeType))
    {
        Data.Humidity = *Humidity;
    }
    else
    {
        Data.Humidity = 0.5f; // Default
    }
    
    // Get danger level
    if (float* Danger = BiomeDangerLevels.Find(BiomeType))
    {
        Data.DangerLevel = *Danger;
    }
    else
    {
        Data.DangerLevel = 0.5f; // Default
    }
    
    return Data;
}

void AEng_BiomeSystemManager::UpdateCurrentBiome(FVector PlayerLocation)
{
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Player entered biome: %s"), 
               *UEnum::GetValueAsString(CurrentBiome));
        
        // Trigger biome transition events here if needed
    }
    
    LastPlayerLocation = PlayerLocation;
}

bool AEng_BiomeSystemManager::ValidateBiomeSystem()
{
    bool bIsValid = true;
    
    // Check biome definitions
    if (BiomeDefinitions.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Invalid biome count: %d (expected 5)"), 
               BiomeDefinitions.Num());
        bIsValid = false;
    }
    
    // Check environmental parameters
    if (BiomeTemperatureRanges.Num() != 5 || BiomeHumidityLevels.Num() != 5 || BiomeDangerLevels.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Incomplete environmental parameters"));
        bIsValid = false;
    }
    
    // Validate each biome has valid coordinates
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.Radius <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Invalid radius for biome %s"), 
                   *UEnum::GetValueAsString(Biome.BiomeType));
            bIsValid = false;
        }
    }
    
    return bIsValid;
}

FString AEng_BiomeSystemManager::GetBiomeSystemStatus()
{
    FString Status = TEXT("=== BIOME SYSTEM STATUS ===\n");
    
    Status += FString::Printf(TEXT("Current Biome: %s\n"), *UEnum::GetValueAsString(CurrentBiome));
    Status += FString::Printf(TEXT("Biome Definitions: %d/5\n"), BiomeDefinitions.Num());
    Status += FString::Printf(TEXT("Environmental Parameters: %d/5\n"), BiomeTemperatureRanges.Num());
    
    // Asset counts per biome
    Status += TEXT("\nAsset Counts:\n");
    for (const auto& BiomeActors : SpawnedBiomeActors)
    {
        Status += FString::Printf(TEXT("  %s: %d actors\n"), 
            *UEnum::GetValueAsString(BiomeActors.Key), 
            BiomeActors.Value.Num());
    }
    
    Status += FString::Printf(TEXT("\nSystem Valid: %s\n"), 
        ValidateBiomeSystem() ? TEXT("YES") : TEXT("NO"));
    
    return Status;
}

TMap<EBiomeType, int32> AEng_BiomeSystemManager::GetSpawnedActorCounts()
{
    TMap<EBiomeType, int32> Counts;
    
    for (const auto& BiomeActors : SpawnedBiomeActors)
    {
        int32 ValidCount = 0;
        for (AActor* Actor : BiomeActors.Value)
        {
            if (IsValid(Actor))
            {
                ValidCount++;
            }
        }
        Counts.Add(BiomeActors.Key, ValidCount);
    }
    
    return Counts;
}

FVector AEng_BiomeSystemManager::GetRandomSpawnLocationInBiome(EBiomeType BiomeType)
{
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            // Generate random point within biome radius
            float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
            float RandomRadius = FMath::RandRange(0.0f, Biome.Radius * 0.8f); // Stay within 80% of radius
            
            FVector Offset = FVector(
                FMath::Cos(RandomAngle) * RandomRadius,
                FMath::Sin(RandomAngle) * RandomRadius,
                0.0f
            );
            
            return Biome.CenterLocation + Offset;
        }
    }
    
    return FVector::ZeroVector;
}

bool AEng_BiomeSystemManager::IsValidSpawnLocation(FVector Location)
{
    // Basic validation - can be expanded with terrain checks
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for overlapping actors (simple radius check)
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        World,
        Location,
        500.0f, // 5m radius
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        OverlappingActors
    );
    
    // Allow spawn if few overlapping actors
    return OverlappingActors.Num() < 3;
}