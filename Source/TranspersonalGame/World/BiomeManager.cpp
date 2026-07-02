// BiomeManager.cpp — Engine Architect #02 — PROD_CYCLE_AUTO_20260702_005
// Biome system implementation: P1 World Generation priority
// Manages 5 biome zones: Forest, Plains, Swamp, Volcanic, River

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check biome every second

    // Default biome definitions
    InitializeDefaultBiomes();
}

void UBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biomes"), BiomeDefinitions.Num());
}

void UBiomeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update current biome for owner actor
    if (AActor* Owner = GetOwner())
    {
        FVector OwnerLocation = Owner->GetActorLocation();
        EEng_BiomeType NewBiome = GetBiomeAtLocation(OwnerLocation);
        if (NewBiome != CurrentBiome)
        {
            EEng_BiomeType OldBiome = CurrentBiome;
            CurrentBiome = NewBiome;
            OnBiomeChanged.Broadcast(OldBiome, NewBiome);
            UE_LOG(LogTemp, Verbose, TEXT("BiomeManager: Biome changed from %d to %d at location %s"),
                (int32)OldBiome, (int32)NewBiome, *OwnerLocation.ToString());
        }
    }
}

void UBiomeManager::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();

    // Forest Biome
    FEng_BiomeDefinition Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.DisplayName = FText::FromString(TEXT("Cretaceous Forest"));
    Forest.TemperatureMin = 18.0f;
    Forest.TemperatureMax = 28.0f;
    Forest.HumidityMin = 0.6f;
    Forest.HumidityMax = 0.9f;
    Forest.FogDensity = 0.03f;
    Forest.DangerLevel = 0.4f;
    Forest.ResourceMultiplier = 1.5f;
    Forest.BiomeColor = FLinearColor(0.13f, 0.55f, 0.13f, 1.0f);
    BiomeDefinitions.Add(Forest);

    // Plains Biome
    FEng_BiomeDefinition Plains;
    Plains.BiomeType = EEng_BiomeType::Plains;
    Plains.DisplayName = FText::FromString(TEXT("Open Plains"));
    Plains.TemperatureMin = 22.0f;
    Plains.TemperatureMax = 38.0f;
    Plains.HumidityMin = 0.2f;
    Plains.HumidityMax = 0.5f;
    Plains.FogDensity = 0.01f;
    Plains.DangerLevel = 0.6f;
    Plains.ResourceMultiplier = 0.8f;
    Plains.BiomeColor = FLinearColor(0.82f, 0.71f, 0.55f, 1.0f);
    BiomeDefinitions.Add(Plains);

    // Swamp Biome
    FEng_BiomeDefinition Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.DisplayName = FText::FromString(TEXT("Primordial Swamp"));
    Swamp.TemperatureMin = 25.0f;
    Swamp.TemperatureMax = 35.0f;
    Swamp.HumidityMin = 0.8f;
    Swamp.HumidityMax = 1.0f;
    Swamp.FogDensity = 0.06f;
    Swamp.DangerLevel = 0.7f;
    Swamp.ResourceMultiplier = 1.2f;
    Swamp.BiomeColor = FLinearColor(0.33f, 0.42f, 0.18f, 1.0f);
    BiomeDefinitions.Add(Swamp);

    // Volcanic Biome
    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Badlands"));
    Volcanic.TemperatureMin = 40.0f;
    Volcanic.TemperatureMax = 80.0f;
    Volcanic.HumidityMin = 0.05f;
    Volcanic.HumidityMax = 0.2f;
    Volcanic.FogDensity = 0.08f;
    Volcanic.DangerLevel = 0.9f;
    Volcanic.ResourceMultiplier = 0.5f;
    Volcanic.BiomeColor = FLinearColor(0.70f, 0.13f, 0.13f, 1.0f);
    BiomeDefinitions.Add(Volcanic);

    // River Biome
    FEng_BiomeDefinition River;
    River.BiomeType = EEng_BiomeType::River;
    River.DisplayName = FText::FromString(TEXT("River Delta"));
    River.TemperatureMin = 20.0f;
    River.TemperatureMax = 30.0f;
    River.HumidityMin = 0.7f;
    River.HumidityMax = 0.95f;
    River.FogDensity = 0.04f;
    River.DangerLevel = 0.5f;
    River.ResourceMultiplier = 2.0f;
    River.BiomeColor = FLinearColor(0.12f, 0.56f, 1.0f, 1.0f);
    BiomeDefinitions.Add(River);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d default biomes"), BiomeDefinitions.Num());
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple distance-based biome assignment for prototype
    // In production: use noise maps, heightmap sampling, and PCG data

    float X = WorldLocation.X;
    float Y = WorldLocation.Y;

    // River corridor: Y near 0, X negative
    if (FMath::Abs(Y) < 200.0f && X < -200.0f)
    {
        return EEng_BiomeType::River;
    }

    // Volcanic zone: far positive Y
    if (Y > 400.0f)
    {
        return EEng_BiomeType::Volcanic;
    }

    // Swamp zone: far negative X
    if (X < -400.0f)
    {
        return EEng_BiomeType::Swamp;
    }

    // Plains zone: positive X
    if (X > 400.0f)
    {
        return EEng_BiomeType::Plains;
    }

    // Default: Forest (central zone)
    return EEng_BiomeType::Forest;
}

const FEng_BiomeDefinition* UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        if (Def.BiomeType == BiomeType)
        {
            return &Def;
        }
    }
    return nullptr;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeDefinition* Def = GetBiomeDefinition(Biome);
    if (Def)
    {
        // Interpolate based on altitude (Z)
        float AltitudeFactor = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);
        float BaseTemp = FMath::Lerp(Def->TemperatureMax, Def->TemperatureMin, AltitudeFactor);
        return BaseTemp;
    }
    return 25.0f; // Default temperature
}

float UBiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeDefinition* Def = GetBiomeDefinition(Biome);
    if (Def)
    {
        return Def->DangerLevel;
    }
    return 0.5f;
}

float UBiomeManager::GetResourceMultiplierAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeDefinition* Def = GetBiomeDefinition(Biome);
    if (Def)
    {
        return Def->ResourceMultiplier;
    }
    return 1.0f;
}

FText UBiomeManager::GetBiomeDisplayName(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Def = GetBiomeDefinition(BiomeType);
    if (Def)
    {
        return Def->DisplayName;
    }
    return FText::FromString(TEXT("Unknown Biome"));
}

EEng_BiomeType UBiomeManager::GetCurrentBiome() const
{
    return CurrentBiome;
}

void UBiomeManager::ForceSetBiome(EEng_BiomeType NewBiome)
{
    EEng_BiomeType OldBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    if (OldBiome != NewBiome)
    {
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
    }
}
