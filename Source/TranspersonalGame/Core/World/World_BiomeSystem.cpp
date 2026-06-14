#include "World_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    BiomeTransitionRadius = 5000.0f;
    BiomeResolution = 512;
    bEnableBiomeTransitions = true;
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomes();
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableBiomeTransitions)
    {
        UpdateBiomeTransitions();
    }
}

void UWorld_BiomeSystem::InitializeBiomes()
{
    CreateDefaultBiomes();
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Initialized %d biomes"), BiomeDatabase.Num());
}

void UWorld_BiomeSystem::CreateDefaultBiomes()
{
    BiomeDatabase.Empty();

    // Swampland Biome
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeName = TEXT("Swampland");
    SwampBiome.BiomeColor = FLinearColor(0.2f, 0.4f, 0.3f, 1.0f);
    SwampBiome.Temperature = 28.0f;
    SwampBiome.Humidity = 0.9f;
    SwampBiome.Elevation = -50.0f;
    BiomeDatabase.Add(SwampBiome.BiomeName, SwampBiome);

    // Canyon Biome
    FWorld_BiomeData CanyonBiome;
    CanyonBiome.BiomeName = TEXT("Canyon");
    CanyonBiome.BiomeColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    CanyonBiome.Temperature = 35.0f;
    CanyonBiome.Humidity = 0.2f;
    CanyonBiome.Elevation = 200.0f;
    BiomeDatabase.Add(CanyonBiome.BiomeName, CanyonBiome);

    // Forest Biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeName = TEXT("Forest");
    ForestBiome.BiomeColor = FLinearColor(0.3f, 0.6f, 0.2f, 1.0f);
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.7f;
    ForestBiome.Elevation = 100.0f;
    BiomeDatabase.Add(ForestBiome.BiomeName, ForestBiome);

    // Plains Biome
    FWorld_BiomeData PlainsBiome;
    PlainsBiome.BiomeName = TEXT("Plains");
    PlainsBiome.BiomeColor = FLinearColor(0.6f, 0.8f, 0.3f, 1.0f);
    PlainsBiome.Temperature = 25.0f;
    PlainsBiome.Humidity = 0.4f;
    PlainsBiome.Elevation = 50.0f;
    BiomeDatabase.Add(PlainsBiome.BiomeName, PlainsBiome);

    // Mountain Biome
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeName = TEXT("Mountain");
    MountainBiome.BiomeColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 0.3f;
    MountainBiome.Elevation = 500.0f;
    BiomeDatabase.Add(MountainBiome.BiomeName, MountainBiome);
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation)
{
    // Simple noise-based biome selection for now
    float NoiseX = WorldLocation.X * 0.0001f;
    float NoiseY = WorldLocation.Y * 0.0001f;
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(NoiseX, NoiseY));
    
    // Map noise to biome types
    if (NoiseValue < -0.4f)
    {
        return BiomeDatabase.Contains(TEXT("Swampland")) ? BiomeDatabase[TEXT("Swampland")] : FWorld_BiomeData();
    }
    else if (NoiseValue < -0.1f)
    {
        return BiomeDatabase.Contains(TEXT("Forest")) ? BiomeDatabase[TEXT("Forest")] : FWorld_BiomeData();
    }
    else if (NoiseValue < 0.2f)
    {
        return BiomeDatabase.Contains(TEXT("Plains")) ? BiomeDatabase[TEXT("Plains")] : FWorld_BiomeData();
    }
    else if (NoiseValue < 0.5f)
    {
        return BiomeDatabase.Contains(TEXT("Canyon")) ? BiomeDatabase[TEXT("Canyon")] : FWorld_BiomeData();
    }
    else
    {
        return BiomeDatabase.Contains(TEXT("Mountain")) ? BiomeDatabase[TEXT("Mountain")] : FWorld_BiomeData();
    }
}

void UWorld_BiomeSystem::UpdateBiomeTransitions()
{
    // Update biome transitions based on player location or other criteria
    // This would be called periodically to smooth biome boundaries
}

TArray<FString> UWorld_BiomeSystem::GetAvailableBiomes() const
{
    TArray<FString> BiomeNames;
    BiomeDatabase.GetKeys(BiomeNames);
    return BiomeNames;
}

void UWorld_BiomeSystem::SetBiomeAtLocation(const FVector& WorldLocation, const FString& BiomeName)
{
    if (BiomeDatabase.Contains(BiomeName))
    {
        // Implementation for setting biome at specific location
        UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Set biome %s at location %s"), *BiomeName, *WorldLocation.ToString());
    }
}

FWorld_BiomeData UWorld_BiomeSystem::BlendBiomes(const FWorld_BiomeData& BiomeA, const FWorld_BiomeData& BiomeB, float BlendFactor)
{
    FWorld_BiomeData BlendedBiome;
    BlendedBiome.BiomeName = FString::Printf(TEXT("%s_%s_Blend"), *BiomeA.BiomeName, *BiomeB.BiomeName);
    BlendedBiome.BiomeColor = FMath::Lerp(BiomeA.BiomeColor, BiomeB.BiomeColor, BlendFactor);
    BlendedBiome.Temperature = FMath::Lerp(BiomeA.Temperature, BiomeB.Temperature, BlendFactor);
    BlendedBiome.Humidity = FMath::Lerp(BiomeA.Humidity, BiomeB.Humidity, BlendFactor);
    BlendedBiome.Elevation = FMath::Lerp(BiomeA.Elevation, BiomeB.Elevation, BlendFactor);
    return BlendedBiome;
}

FString UWorld_BiomeSystem::GetDominantBiome(const FVector& WorldLocation)
{
    FWorld_BiomeData CurrentBiome = GetBiomeAtLocation(WorldLocation);
    return CurrentBiome.BiomeName;
}