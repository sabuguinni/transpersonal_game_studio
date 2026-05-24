#include "Eng_BiomeSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AEng_BiomeSystemManager::AEng_BiomeSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    BiomeTransitionRadius = 5000.0f;
    MaxAssetSpawnDistance = 10000.0f;
    TargetActorsPerBiome = 500;
    CurrentBiome = EBiomeType::Savana;
    LastPlayerLocation = FVector::ZeroVector;
    BiomeUpdateTimer = 0.0f;
    BiomeUpdateInterval = 2.0f; // Update every 2 seconds
    
    // Setup default biomes and environmental parameters
    SetupDefaultBiomes();
    SetupEnvironmentalParameters();
}

void AEng_BiomeSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager::BeginPlay - Initializing biome system"));
    InitializeBiomeSystem();
}

void AEng_BiomeSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    BiomeUpdateTimer += DeltaTime;
    
    // Update current biome based on player location every few seconds
    if (BiomeUpdateTimer >= BiomeUpdateInterval)
    {
        BiomeUpdateTimer = 0.0f;
        
        // Get player location
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            UpdateCurrentBiome(PlayerLocation);
        }
    }
}

void AEng_BiomeSystemManager::SetupDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana (Center) - Coordinates: (0, 0)
    FBiomeDefinition SavanaBiome;
    SavanaBiome.BiomeType = EBiomeType::Savana;
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaBiome.Radius = 25000.0f;
    SavanaBiome.BiomeName = TEXT("Cretaceous Savana");
    SavanaBiome.Description = TEXT("Open grasslands with scattered acacia trees and rocky outcrops");
    BiomeDefinitions.Add(SavanaBiome);
    
    // Pantano (Southwest) - Coordinates: (-50000, -45000)
    FBiomeDefinition PantanoBiome;
    PantanoBiome.BiomeType = EBiomeType::Pantano;
    PantanoBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoBiome.Radius = 20000.0f;
    PantanoBiome.BiomeName = TEXT("Prehistoric Wetlands");
    PantanoBiome.Description = TEXT("Swampy marshlands with dense vegetation and standing water");
    BiomeDefinitions.Add(PantanoBiome);
    
    // Floresta (Northwest) - Coordinates: (-45000, 40000)
    FBiomeDefinition FlorestaBiome;
    FlorestaBiome.BiomeType = EBiomeType::Floresta;
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaBiome.Radius = 22000.0f;
    FlorestaBiome.BiomeName = TEXT("Cretaceous Forest");
    FlorestaBiome.Description = TEXT("Dense coniferous and fern forests with towering trees");
    BiomeDefinitions.Add(FlorestaBiome);
    
    // Deserto (East) - Coordinates: (55000, 0)
    FBiomeDefinition DesertoBiome;
    DesertoBiome.BiomeType = EBiomeType::Deserto;
    DesertoBiome.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoBiome.Radius = 18000.0f;
    DesertoBiome.BiomeName = TEXT("Arid Badlands");
    DesertoBiome.Description = TEXT("Rocky desert with sparse vegetation and extreme temperatures");
    BiomeDefinitions.Add(DesertoBiome);
    
    // Montanha (Northeast) - Coordinates: (40000, 50000)
    FBiomeDefinition MontanhaBiome;
    MontanhaBiome.BiomeType = EBiomeType::Montanha;
    MontanhaBiome.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaBiome.Radius = 15000.0f;
    MontanhaBiome.BiomeName = TEXT("Volcanic Highlands");
    MontanhaBiome.Description = TEXT("Mountainous terrain with volcanic activity and rocky peaks");
    BiomeDefinitions.Add(MontanhaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager: Setup %d default biomes"), BiomeDefinitions.Num());
}

void AEng_BiomeSystemManager::SetupEnvironmentalParameters()
{
    // Temperature ranges (Celsius)
    BiomeTemperatureRanges.Add(EBiomeType::Savana, FVector2D(25.0f, 35.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Pantano, FVector2D(22.0f, 30.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Floresta, FVector2D(18.0f, 28.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Deserto, FVector2D(30.0f, 45.0f));
    BiomeTemperatureRanges.Add(EBiomeType::Montanha, FVector2D(10.0f, 20.0f));
    
    // Humidity levels (0-1)
    BiomeHumidityLevels.Add(EBiomeType::Savana, 0.4f);
    BiomeHumidityLevels.Add(EBiomeType::Pantano, 0.9f);
    BiomeHumidityLevels.Add(EBiomeType::Floresta, 0.8f);
    BiomeHumidityLevels.Add(EBiomeType::Deserto, 0.1f);
    BiomeHumidityLevels.Add(EBiomeType::Montanha, 0.6f);
    
    // Danger levels (0-1)
    BiomeDangerLevels.Add(EBiomeType::Savana, 0.5f);
    BiomeDangerLevels.Add(EBiomeType::Pantano, 0.8f);
    BiomeDangerLevels.Add(EBiomeType::Floresta, 0.7f);
    BiomeDangerLevels.Add(EBiomeType::Deserto, 0.9f);
    BiomeDangerLevels.Add(EBiomeType::Montanha, 0.6f);
}

void AEng_BiomeSystemManager::InitializeBiomeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager::InitializeBiomeSystem - Starting initialization"));
    
    // Clear existing spawned actors
    for (auto& BiomeActorPair : SpawnedBiomeActors)
    {
        for (AActor* Actor : BiomeActorPair.Value)
        {
            if (IsValid(Actor))
            {
                Actor->Destroy();
            }
        }
    }
    SpawnedBiomeActors.Empty();
    
    // Initialize spawned actor arrays for each biome
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        SpawnedBiomeActors.Add(Biome.BiomeType, TArray<AActor*>());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager: Biome system initialized with %d biomes"), BiomeDefinitions.Num());
}

EBiomeType AEng_BiomeSystemManager::GetBiomeAtLocation(FVector WorldLocation)
{
    float MinDistance = MAX_FLT;
    EBiomeType ClosestBiome = EBiomeType::Savana;
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
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
            return FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        }
    }
    return -1.0f; // Biome not found
}

void AEng_BiomeSystemManager::SpawnBiomeAssets(EBiomeType BiomeType, int32 AssetCount)
{
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager::SpawnBiomeAssets - Spawning %d assets for biome %d"), AssetCount, (int32)BiomeType);
    
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
        UE_LOG(LogTemp, Error, TEXT("AEng_BiomeSystemManager::SpawnBiomeAssets - Biome definition not found for type %d"), (int32)BiomeType);
        return;
    }
    
    // Clear existing assets for this biome
    ClearBiomeAssets(BiomeType);
    
    // Get or create spawned actors array for this biome
    if (!SpawnedBiomeActors.Contains(BiomeType))
    {
        SpawnedBiomeActors.Add(BiomeType, TArray<AActor*>());
    }
    
    TArray<AActor*>& BiomeActors = SpawnedBiomeActors[BiomeType];
    
    // Spawn placeholder assets (basic shapes for now)
    for (int32 i = 0; i < AssetCount; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocationInBiome(BiomeType);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Create a basic static mesh actor as placeholder
            AStaticMeshActor* SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            
            if (SpawnedActor)
            {
                // Set a basic cube mesh as placeholder
                UStaticMeshComponent* MeshComp = SpawnedActor->GetStaticMeshComponent();
                if (MeshComp)
                {
                    // Try to load a basic cube mesh
                    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
                    if (CubeMesh)
                    {
                        MeshComp->SetStaticMesh(CubeMesh);
                        
                        // Scale based on biome type
                        float Scale = 1.0f;
                        switch (BiomeType)
                        {
                            case EBiomeType::Savana:
                                Scale = FMath::RandRange(0.5f, 2.0f); // Rocks and small trees
                                break;
                            case EBiomeType::Floresta:
                                Scale = FMath::RandRange(1.0f, 4.0f); // Large trees
                                break;
                            case EBiomeType::Pantano:
                                Scale = FMath::RandRange(0.3f, 1.5f); // Swamp vegetation
                                break;
                            case EBiomeType::Deserto:
                                Scale = FMath::RandRange(0.2f, 1.0f); // Sparse rocks
                                break;
                            case EBiomeType::Montanha:
                                Scale = FMath::RandRange(1.5f, 5.0f); // Large rocks and boulders
                                break;
                        }
                        
                        SpawnedActor->SetActorScale3D(FVector(Scale));
                    }
                }
                
                // Set actor label for identification
                FString ActorLabel = FString::Printf(TEXT("%s_Asset_%d"), *UEnum::GetValueAsString(BiomeType), i);
                SpawnedActor->SetActorLabel(ActorLabel);
                
                BiomeActors.Add(SpawnedActor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager: Spawned %d assets for biome %s"), BiomeActors.Num(), *UEnum::GetValueAsString(BiomeType));
}

void AEng_BiomeSystemManager::ClearBiomeAssets(EBiomeType BiomeType)
{
    if (SpawnedBiomeActors.Contains(BiomeType))
    {
        TArray<AActor*>& BiomeActors = SpawnedBiomeActors[BiomeType];
        
        for (AActor* Actor : BiomeActors)
        {
            if (IsValid(Actor))
            {
                Actor->Destroy();
            }
        }
        
        BiomeActors.Empty();
        UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager: Cleared assets for biome %s"), *UEnum::GetValueAsString(BiomeType));
    }
}

FBiomeEnvironmentalData AEng_BiomeSystemManager::GetBiomeEnvironmentalData(EBiomeType BiomeType)
{
    FBiomeEnvironmentalData Data;
    
    // Get temperature range
    if (BiomeTemperatureRanges.Contains(BiomeType))
    {
        FVector2D TempRange = BiomeTemperatureRanges[BiomeType];
        Data.Temperature = FMath::RandRange(TempRange.X, TempRange.Y);
    }
    
    // Get humidity
    if (BiomeHumidityLevels.Contains(BiomeType))
    {
        Data.Humidity = BiomeHumidityLevels[BiomeType];
    }
    
    // Get danger level
    if (BiomeDangerLevels.Contains(BiomeType))
    {
        Data.DangerLevel = BiomeDangerLevels[BiomeType];
    }
    
    Data.BiomeType = BiomeType;
    
    return Data;
}

void AEng_BiomeSystemManager::UpdateCurrentBiome(FVector PlayerLocation)
{
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager: Player entered biome: %s"), *UEnum::GetValueAsString(CurrentBiome));
        
        // Trigger biome change events here if needed
    }
    
    LastPlayerLocation = PlayerLocation;
}

bool AEng_BiomeSystemManager::ValidateBiomeSystem()
{
    bool bIsValid = true;
    
    // Check if all biomes have definitions
    if (BiomeDefinitions.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("AEng_BiomeSystemManager: Expected 5 biomes, found %d"), BiomeDefinitions.Num());
        bIsValid = false;
    }
    
    // Check if all environmental parameters are set
    if (BiomeTemperatureRanges.Num() != 5 || BiomeHumidityLevels.Num() != 5 || BiomeDangerLevels.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("AEng_BiomeSystemManager: Missing environmental parameters"));
        bIsValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AEng_BiomeSystemManager: Validation %s"), bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bIsValid;
}

FString AEng_BiomeSystemManager::GetBiomeSystemStatus()
{
    FString Status = TEXT("=== BIOME SYSTEM STATUS ===\n");
    
    Status += FString::Printf(TEXT("Total Biomes: %d\n"), BiomeDefinitions.Num());
    Status += FString::Printf(TEXT("Current Biome: %s\n"), *UEnum::GetValueAsString(CurrentBiome));
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        int32 ActorCount = 0;
        if (SpawnedBiomeActors.Contains(Biome.BiomeType))
        {
            ActorCount = SpawnedBiomeActors[Biome.BiomeType].Num();
        }
        
        Status += FString::Printf(TEXT("%s: %d actors at (%.0f, %.0f)\n"), 
            *UEnum::GetValueAsString(Biome.BiomeType), 
            ActorCount,
            Biome.CenterLocation.X, 
            Biome.CenterLocation.Y);
    }
    
    return Status;
}

TMap<EBiomeType, int32> AEng_BiomeSystemManager::GetSpawnedActorCounts()
{
    TMap<EBiomeType, int32> Counts;
    
    for (const auto& BiomeActorPair : SpawnedBiomeActors)
    {
        Counts.Add(BiomeActorPair.Key, BiomeActorPair.Value.Num());
    }
    
    return Counts;
}

FVector AEng_BiomeSystemManager::GetRandomSpawnLocationInBiome(EBiomeType BiomeType)
{
    // Find biome definition
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            // Generate random point within biome radius
            float RandomAngle = FMath::RandRange(0.0f, 360.0f);
            float RandomDistance = FMath::RandRange(0.0f, Biome.Radius * 0.8f); // Stay within 80% of radius
            
            FVector Offset = FVector(
                FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                0.0f
            );
            
            return Biome.CenterLocation + Offset;
        }
    }
    
    return FVector::ZeroVector;
}

bool AEng_BiomeSystemManager::IsValidSpawnLocation(FVector Location)
{
    // Basic validation - check if location is not too close to existing actors
    // This is a simplified implementation
    
    if (!GetWorld())
    {
        return false;
    }
    
    // Check for overlapping actors in a small radius
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Location,
        100.0f, // 1 meter radius
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AStaticMeshActor::StaticClass(),
        TArray<AActor*>(),
        OverlappingActors
    );
    
    return OverlappingActors.Num() == 0;
}