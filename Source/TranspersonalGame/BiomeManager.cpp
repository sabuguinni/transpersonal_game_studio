#include "BiomeManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance

    InitializeBiomeDefinitions();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache player reference
    PlayerRef = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: No player character found at BeginPlay"));
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeDefinitions.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastBiomeUpdate += DeltaTime;
    if (TimeSinceLastBiomeUpdate < BiomeUpdateInterval)
    {
        return;
    }
    TimeSinceLastBiomeUpdate = 0.0f;

    // Update player biome
    if (!PlayerRef)
    {
        PlayerRef = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    }

    if (PlayerRef)
    {
        FVector PlayerLoc = PlayerRef->GetActorLocation();
        EEng_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);
        if (NewBiome != CurrentPlayerBiome)
        {
            CurrentPlayerBiome = NewBiome;
            UE_LOG(LogTemp, Log, TEXT("BiomeManager: Player entered biome: %s"), *GetBiomeName(NewBiome));
        }
    }
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Normalize location to [0,1] range
    float NX = (WorldLocation.X + WorldSize * 0.5f) / WorldSize;
    float NY = (WorldLocation.Y + WorldSize * 0.5f) / WorldSize;
    float NZ = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);

    // Sample two noise octaves for biome variation
    float NoiseA = SampleNoise(NX, NY, 1.0f);
    float NoiseB = SampleNoise(NX + 100.0f, NY + 100.0f, 2.0f);
    float BlendedNoise = (NoiseA * 0.7f + NoiseB * 0.3f);

    return ClassifyBiomeFromNoise(BlendedNoise, NZ);
}

FEng_BiomeData ABiomeManager::GetBiomeDataAtLocation(FVector WorldLocation) const
{
    EEng_BiomeType Type = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData* Data = FindBiomeDataConst(Type);
    if (Data)
    {
        return *Data;
    }

    // Return default biome data if not found
    FEng_BiomeData Default;
    Default.BiomeType = EEng_BiomeType::Unknown;
    return Default;
}

FString ABiomeManager::GetBiomeName(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::Jungle:   return TEXT("Jungle");
        case EEng_BiomeType::Savanna:  return TEXT("Savanna");
        case EEng_BiomeType::Swamp:    return TEXT("Swamp");
        case EEng_BiomeType::Volcanic: return TEXT("Volcanic");
        case EEng_BiomeType::Coastal:  return TEXT("Coastal");
        case EEng_BiomeType::Forest:   return TEXT("Forest");
        default:                       return TEXT("Unknown");
    }
}

float ABiomeManager::GetDangerLevelAtLocation(FVector WorldLocation) const
{
    FEng_BiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.DangerLevel;
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    FEng_BiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    // Altitude modifier: -1°C per 100m
    float AltitudeMod = -WorldLocation.Z / 10000.0f;
    return Data.Temperature + AltitudeMod;
}

TArray<FString> ABiomeManager::GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeData* Data = FindBiomeDataConst(BiomeType);
    if (Data)
    {
        return Data->DinosaurSpecies;
    }
    return TArray<FString>();
}

void ABiomeManager::InitializeBiomeDefinitions()
{
    BiomeDefinitions.Empty();

    // --- JUNGLE ---
    FEng_BiomeData Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.Temperature = 32.0f;
    Jungle.Humidity = 0.9f;
    Jungle.VegetationDensity = 0.95f;
    Jungle.DangerLevel = 0.7f;
    Jungle.FogColor = FLinearColor(0.3f, 0.55f, 0.3f, 1.0f);
    Jungle.FogDensity = 0.035f;
    Jungle.DinosaurSpecies = { TEXT("Velociraptor"), TEXT("Dilophosaurus"), TEXT("Compsognathus") };
    BiomeDefinitions.Add(Jungle);

    // --- SAVANNA ---
    FEng_BiomeData Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.Temperature = 38.0f;
    Savanna.Humidity = 0.2f;
    Savanna.VegetationDensity = 0.3f;
    Savanna.DangerLevel = 0.8f;
    Savanna.FogColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
    Savanna.FogDensity = 0.01f;
    Savanna.DinosaurSpecies = { TEXT("TRex"), TEXT("Triceratops"), TEXT("Ankylosaurus"), TEXT("Brachiosaurus") };
    BiomeDefinitions.Add(Savanna);

    // --- SWAMP ---
    FEng_BiomeData Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.Temperature = 28.0f;
    Swamp.Humidity = 0.95f;
    Swamp.VegetationDensity = 0.7f;
    Swamp.DangerLevel = 0.6f;
    Swamp.FogColor = FLinearColor(0.2f, 0.4f, 0.25f, 1.0f);
    Swamp.FogDensity = 0.06f;
    Swamp.DinosaurSpecies = { TEXT("Spinosaurus"), TEXT("Sarcosuchus"), TEXT("Parasaurolophus") };
    BiomeDefinitions.Add(Swamp);

    // --- VOLCANIC ---
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.DangerLevel = 0.95f;
    Volcanic.FogColor = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
    Volcanic.FogDensity = 0.08f;
    Volcanic.DinosaurSpecies = { TEXT("Carnotaurus"), TEXT("Abelisaurus") };
    BiomeDefinitions.Add(Volcanic);

    // --- COASTAL ---
    FEng_BiomeData Coastal;
    Coastal.BiomeType = EEng_BiomeType::Coastal;
    Coastal.Temperature = 26.0f;
    Coastal.Humidity = 0.7f;
    Coastal.VegetationDensity = 0.4f;
    Coastal.DangerLevel = 0.5f;
    Coastal.FogColor = FLinearColor(0.5f, 0.65f, 0.75f, 1.0f);
    Coastal.FogDensity = 0.025f;
    Coastal.DinosaurSpecies = { TEXT("Mosasaurus"), TEXT("Pteranodon"), TEXT("Pachycephalosaurus") };
    BiomeDefinitions.Add(Coastal);

    // --- FOREST ---
    FEng_BiomeData Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.65f;
    Forest.VegetationDensity = 0.8f;
    Forest.DangerLevel = 0.45f;
    Forest.FogColor = FLinearColor(0.35f, 0.5f, 0.35f, 1.0f);
    Forest.FogDensity = 0.02f;
    Forest.DinosaurSpecies = { TEXT("Stegosaurus"), TEXT("Iguanodon"), TEXT("Protoceratops") };
    BiomeDefinitions.Add(Forest);
}

float ABiomeManager::SampleNoise(float X, float Y, float Frequency) const
{
    // Simple deterministic pseudo-noise using sine waves
    // In production this would use FMath::PerlinNoise2D or a proper noise library
    float Val = FMath::Sin(X * Frequency * 17.3f + Y * Frequency * 31.7f) * 0.5f
              + FMath::Sin(X * Frequency * 53.1f - Y * Frequency * 11.9f) * 0.3f
              + FMath::Sin((X + Y) * Frequency * 23.7f) * 0.2f;
    return FMath::Clamp(Val * 0.5f + 0.5f, 0.0f, 1.0f);
}

EEng_BiomeType ABiomeManager::ClassifyBiomeFromNoise(float NoiseValue, float AltitudeValue) const
{
    // High altitude → Volcanic
    if (AltitudeValue > 0.7f)
    {
        return EEng_BiomeType::Volcanic;
    }
    // Low altitude near sea level → Coastal or Swamp
    if (AltitudeValue < 0.1f)
    {
        return NoiseValue > 0.5f ? EEng_BiomeType::Coastal : EEng_BiomeType::Swamp;
    }

    // Mid altitude — classify by noise value
    if (NoiseValue < 0.17f) return EEng_BiomeType::Swamp;
    if (NoiseValue < 0.33f) return EEng_BiomeType::Jungle;
    if (NoiseValue < 0.50f) return EEng_BiomeType::Forest;
    if (NoiseValue < 0.67f) return EEng_BiomeType::Savanna;
    if (NoiseValue < 0.83f) return EEng_BiomeType::Coastal;
    return EEng_BiomeType::Volcanic;
}

FEng_BiomeData* ABiomeManager::FindBiomeData(EEng_BiomeType Type)
{
    for (FEng_BiomeData& Data : BiomeDefinitions)
    {
        if (Data.BiomeType == Type)
        {
            return &Data;
        }
    }
    return nullptr;
}

const FEng_BiomeData* ABiomeManager::FindBiomeDataConst(EEng_BiomeType Type) const
{
    for (const FEng_BiomeData& Data : BiomeDefinitions)
    {
        if (Data.BiomeType == Type)
        {
            return &Data;
        }
    }
    return nullptr;
}
