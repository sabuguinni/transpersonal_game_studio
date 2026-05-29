#include "Eng_BiomeManager.h"
#include "../../TranspersonalGame.h"
#include "../../Characters/TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentBiome = EBiomeType::Savana;
    CurrentTemperature = 25.0f;
    CurrentHumidity = 50.0f;
    CurrentWindIntensity = 20.0f;
    EnvironmentalUpdateTimer = 0.0f;
    PlayerCharacter = nullptr;
    
    // Set actor properties
    SetActorTickEnabled(true);
    bReplicates = false; // Single-player game
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("BiomeManager: Initializing biome system"));
    
    InitializeBiomeData();
    
    // Find player character
    PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (PlayerCharacter)
    {
        UE_LOG(LogTranspersonalGame, Log, TEXT("BiomeManager: Player character found"));
        UpdatePlayerBiome();
    }
    else
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("BiomeManager: Player character not found"));
    }
}

void AEng_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    EnvironmentalUpdateTimer += DeltaTime;
    
    if (EnvironmentalUpdateTimer >= UpdateInterval)
    {
        UpdatePlayerBiome();
        EnvironmentalUpdateTimer = 0.0f;
    }
}

EBiomeType AEng_BiomeManager::GetBiomeAtLocation(FVector WorldLocation)
{
    FVector2D Location2D = FVector2D(WorldLocation.X, WorldLocation.Y);
    
    // Check Pantano (Swamp) - Southwest
    if (Location2D.X >= PantanoBounds_Min.X && Location2D.X <= PantanoBounds_Max.X &&
        Location2D.Y >= PantanoBounds_Min.Y && Location2D.Y <= PantanoBounds_Max.Y)
    {
        return EBiomeType::Pantano;
    }
    
    // Check Floresta (Forest) - Northwest
    if (Location2D.X >= FlorestaBounds_Min.X && Location2D.X <= FlorestaBounds_Max.X &&
        Location2D.Y >= FlorestaBounds_Min.Y && Location2D.Y <= FlorestaBounds_Max.Y)
    {
        return EBiomeType::Floresta;
    }
    
    // Check Deserto (Desert) - East
    if (Location2D.X >= DesertoBounds_Min.X && Location2D.X <= DesertoBounds_Max.X &&
        Location2D.Y >= DesertoBounds_Min.Y && Location2D.Y <= DesertoBounds_Max.Y)
    {
        return EBiomeType::Deserto;
    }
    
    // Check Montanha Nevada (Snowy Mountain) - Northeast
    if (Location2D.X >= MontanhaBounds_Min.X && Location2D.X <= MontanhaBounds_Max.X &&
        Location2D.Y >= MontanhaBounds_Min.Y && Location2D.Y <= MontanhaBounds_Max.Y)
    {
        return EBiomeType::MontanhaNevada;
    }
    
    // Check Savana (center) - Default if within bounds
    if (Location2D.X >= SavanaBounds_Min.X && Location2D.X <= SavanaBounds_Max.X &&
        Location2D.Y >= SavanaBounds_Min.Y && Location2D.Y <= SavanaBounds_Max.Y)
    {
        return EBiomeType::Savana;
    }
    
    // Default to Savana for any location outside defined bounds
    return EBiomeType::Savana;
}

FVector AEng_BiomeManager::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 0.0f);
        case EBiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 0.0f);
        case EBiomeType::Savana:
            return FVector(0.0f, 0.0f, 0.0f);
        case EBiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 0.0f);
        case EBiomeType::MontanhaNevada:
            return FVector(40000.0f, 50000.0f, 500.0f);
        default:
            return FVector(0.0f, 0.0f, 0.0f);
    }
}

FVector AEng_BiomeManager::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector Center = GetBiomeCenter(BiomeType);
    
    // Generate random offset within 5km radius from center
    float RandomX = FMath::RandRange(-5000.0f, 5000.0f);
    float RandomY = FMath::RandRange(-5000.0f, 5000.0f);
    float RandomZ = 0.0f;
    
    // Adjust Z for mountain biome
    if (BiomeType == EBiomeType::MontanhaNevada)
    {
        RandomZ = FMath::RandRange(0.0f, 1000.0f);
    }
    
    return Center + FVector(RandomX, RandomY, RandomZ);
}

void AEng_BiomeManager::UpdateEnvironmentalParameters(EBiomeType BiomeType)
{
    FBiomeEnvironmentalData BiomeData = GetBiomeEnvironmentalData(BiomeType);
    
    CurrentTemperature = BiomeData.BaseTemperature;
    CurrentHumidity = BiomeData.BaseHumidity;
    CurrentWindIntensity = BiomeData.BaseWindIntensity;
    
    UE_LOG(LogTranspersonalGame, VeryVerbose, TEXT("BiomeManager: Updated environmental parameters for biome %d - Temp: %.1f, Humidity: %.1f, Wind: %.1f"), 
           (int32)BiomeType, CurrentTemperature, CurrentHumidity, CurrentWindIntensity);
}

bool AEng_BiomeManager::IsValidSpawnLocation(FVector Location, EActorSpawnType SpawnType)
{
    EBiomeType LocationBiome = GetBiomeAtLocation(Location);
    
    switch (SpawnType)
    {
        case EActorSpawnType::Vegetation:
            // Vegetation spawns in all biomes except desert
            return LocationBiome != EBiomeType::Deserto;
            
        case EActorSpawnType::Water:
            // Water features mainly in swamp and forest
            return LocationBiome == EBiomeType::Pantano || LocationBiome == EBiomeType::Floresta;
            
        case EActorSpawnType::Rock:
            // Rocks spawn everywhere but more common in mountains and desert
            return true;
            
        case EActorSpawnType::Dinosaur:
            // Dinosaurs can spawn in all biomes
            return true;
            
        case EActorSpawnType::Structure:
            // Structures avoid swamp areas
            return LocationBiome != EBiomeType::Pantano;
            
        default:
            return true;
    }
}

FBiomeEnvironmentalData AEng_BiomeManager::GetBiomeEnvironmentalData(EBiomeType BiomeType)
{
    FBiomeEnvironmentalData Data;
    
    switch (BiomeType)
    {
        case EBiomeType::Pantano:
            Data.BaseTemperature = 28.0f;
            Data.BaseHumidity = 85.0f;
            Data.BaseWindIntensity = 10.0f;
            Data.FogDensity = 0.8f;
            Data.LightIntensity = 0.6f;
            break;
            
        case EBiomeType::Floresta:
            Data.BaseTemperature = 22.0f;
            Data.BaseHumidity = 75.0f;
            Data.BaseWindIntensity = 15.0f;
            Data.FogDensity = 0.3f;
            Data.LightIntensity = 0.4f;
            break;
            
        case EBiomeType::Savana:
            Data.BaseTemperature = 30.0f;
            Data.BaseHumidity = 45.0f;
            Data.BaseWindIntensity = 25.0f;
            Data.FogDensity = 0.1f;
            Data.LightIntensity = 1.0f;
            break;
            
        case EBiomeType::Deserto:
            Data.BaseTemperature = 42.0f;
            Data.BaseHumidity = 15.0f;
            Data.BaseWindIntensity = 35.0f;
            Data.FogDensity = 0.0f;
            Data.LightIntensity = 1.2f;
            break;
            
        case EBiomeType::MontanhaNevada:
            Data.BaseTemperature = -5.0f;
            Data.BaseHumidity = 60.0f;
            Data.BaseWindIntensity = 45.0f;
            Data.FogDensity = 0.5f;
            Data.LightIntensity = 0.9f;
            break;
            
        default:
            // Default to Savana values
            Data.BaseTemperature = 25.0f;
            Data.BaseHumidity = 50.0f;
            Data.BaseWindIntensity = 20.0f;
            Data.FogDensity = 0.2f;
            Data.LightIntensity = 1.0f;
            break;
    }
    
    return Data;
}

void AEng_BiomeManager::InitializeBiomeData()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("BiomeManager: Initializing biome boundaries"));
    UE_LOG(LogTranspersonalGame, Log, TEXT("Pantano: (%.0f,%.0f) to (%.0f,%.0f)"), 
           PantanoBounds_Min.X, PantanoBounds_Min.Y, PantanoBounds_Max.X, PantanoBounds_Max.Y);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Floresta: (%.0f,%.0f) to (%.0f,%.0f)"), 
           FlorestaBounds_Min.X, FlorestaBounds_Min.Y, FlorestaBounds_Max.X, FlorestaBounds_Max.Y);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Savana: (%.0f,%.0f) to (%.0f,%.0f)"), 
           SavanaBounds_Min.X, SavanaBounds_Min.Y, SavanaBounds_Max.X, SavanaBounds_Max.Y);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Deserto: (%.0f,%.0f) to (%.0f,%.0f)"), 
           DesertoBounds_Min.X, DesertoBounds_Min.Y, DesertoBounds_Max.X, DesertoBounds_Max.Y);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Montanha: (%.0f,%.0f) to (%.0f,%.0f)"), 
           MontanhaBounds_Min.X, MontanhaBounds_Min.Y, MontanhaBounds_Max.X, MontanhaBounds_Max.Y);
}

void AEng_BiomeManager::UpdatePlayerBiome()
{
    if (!PlayerCharacter)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        UpdateEnvironmentalParameters(CurrentBiome);
        
        UE_LOG(LogTranspersonalGame, Log, TEXT("BiomeManager: Player entered biome %d at location (%.0f, %.0f, %.0f)"), 
               (int32)CurrentBiome, PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z);
    }
}