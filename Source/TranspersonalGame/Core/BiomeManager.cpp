#include "BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Atmosphere/AtmosphericFog.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check biome every second, not every frame

    InitializeDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeTable.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Biome transition logic runs at reduced frequency (TickInterval = 1.0s)
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple noise-based biome assignment based on world coordinates
    // In production this would use a biome map texture or PCG graph
    float nx = WorldLocation.X / BiomeCellSize;
    float ny = WorldLocation.Y / BiomeCellSize;

    // Deterministic pseudo-noise using sine waves
    float noise = FMath::Sin(nx * 1.3f) * FMath::Cos(ny * 0.9f)
                + FMath::Sin(nx * 0.7f + ny * 1.1f) * 0.5f;

    // Map noise [-1.5, 1.5] to biome enum
    if (noise > 1.0f)  return EEng_BiomeType::Jungle;
    if (noise > 0.5f)  return EEng_BiomeType::Forest;
    if (noise > 0.0f)  return EEng_BiomeType::Savanna;
    if (noise > -0.5f) return EEng_BiomeType::Swamp;
    if (noise > -1.0f) return EEng_BiomeType::Coastal;
    return EEng_BiomeType::Volcanic;
}

FEng_BiomeData ABiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeData& Data : BiomeTable)
    {
        if (Data.BiomeType == BiomeType)
        {
            return Data;
        }
    }
    // Return default jungle data if not found
    FEng_BiomeData Default;
    Default.BiomeType = BiomeType;
    return Default;
}

void ABiomeManager::ApplyBiomeFogSettings(EEng_BiomeType BiomeType)
{
    if (!GetWorld()) return;

    FEng_BiomeData Data = GetBiomeData(BiomeType);

    // Find ExponentialHeightFog in the world and update its settings
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);

    for (AActor* FogActor : FogActors)
    {
        AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActor);
        if (Fog && Fog->GetComponent())
        {
            UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
            FogComp->SetFogInscatteringColor(Data.FogColor);
            FogComp->SetFogDensity(Data.FogDensity);
            UE_LOG(LogTemp, Log, TEXT("BiomeManager: Applied fog for biome %d — density=%.3f"),
                (int32)BiomeType, Data.FogDensity);
        }
        break; // Only apply to first fog actor
    }
}

void ABiomeManager::DebugDrawBiomeBounds()
{
    if (!GetWorld()) return;

    // Draw a grid of biome colors for debugging
    const int32 GridSize = 10;
    const float CellSize = BiomeCellSize;

    for (int32 x = -GridSize/2; x < GridSize/2; x++)
    {
        for (int32 y = -GridSize/2; y < GridSize/2; y++)
        {
            FVector Center(x * CellSize + CellSize * 0.5f, y * CellSize + CellSize * 0.5f, 100.0f);
            EEng_BiomeType Biome = GetBiomeAtLocation(Center);

            FColor DebugColor;
            switch (Biome)
            {
                case EEng_BiomeType::Jungle:   DebugColor = FColor::Green;  break;
                case EEng_BiomeType::Savanna:  DebugColor = FColor::Yellow; break;
                case EEng_BiomeType::Swamp:    DebugColor = FColor::Cyan;   break;
                case EEng_BiomeType::Volcanic: DebugColor = FColor::Red;    break;
                case EEng_BiomeType::Coastal:  DebugColor = FColor::Blue;   break;
                case EEng_BiomeType::Forest:   DebugColor = FColor(0, 128, 0); break;
                default:                       DebugColor = FColor::White;  break;
            }

            DrawDebugBox(GetWorld(), Center, FVector(CellSize * 0.45f, CellSize * 0.45f, 50.0f),
                DebugColor, false, 10.0f, 0, 5.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Debug draw complete — %d cells"), GridSize * GridSize);
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeTable.Empty();

    // Cretaceous Jungle — hot, humid, dense
    FEng_BiomeData Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.Temperature = 32.0f;
    Jungle.Humidity = 0.90f;
    Jungle.VegetationDensity = 1.0f;
    Jungle.FogColor = FLinearColor(0.5f, 0.75f, 0.4f, 1.0f);
    Jungle.FogDensity = 0.025f;
    BiomeTable.Add(Jungle);

    // Open Savanna — warm, dry, sparse
    FEng_BiomeData Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.Temperature = 28.0f;
    Savanna.Humidity = 0.35f;
    Savanna.VegetationDensity = 0.3f;
    Savanna.FogColor = FLinearColor(0.85f, 0.75f, 0.5f, 1.0f);
    Savanna.FogDensity = 0.008f;
    BiomeTable.Add(Savanna);

    // Prehistoric Swamp — hot, very humid, murky
    FEng_BiomeData Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.Temperature = 30.0f;
    Swamp.Humidity = 0.95f;
    Swamp.VegetationDensity = 0.7f;
    Swamp.FogColor = FLinearColor(0.4f, 0.55f, 0.3f, 1.0f);
    Swamp.FogDensity = 0.045f;
    BiomeTable.Add(Swamp);

    // Volcanic Badlands — extreme heat, toxic, barren
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.05f;
    Volcanic.VegetationDensity = 0.0f;
    Volcanic.FogColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    Volcanic.FogDensity = 0.035f;
    BiomeTable.Add(Volcanic);

    // Coastal Flats — moderate, breezy
    FEng_BiomeData Coastal;
    Coastal.BiomeType = EEng_BiomeType::Coastal;
    Coastal.Temperature = 24.0f;
    Coastal.Humidity = 0.60f;
    Coastal.VegetationDensity = 0.4f;
    Coastal.FogColor = FLinearColor(0.6f, 0.75f, 0.85f, 1.0f);
    Coastal.FogDensity = 0.015f;
    BiomeTable.Add(Coastal);

    // Conifer Forest — cool, moderate humidity
    FEng_BiomeData Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.Temperature = 18.0f;
    Forest.Humidity = 0.55f;
    Forest.VegetationDensity = 0.8f;
    Forest.FogColor = FLinearColor(0.55f, 0.65f, 0.5f, 1.0f);
    Forest.FogDensity = 0.018f;
    BiomeTable.Add(Forest);
}

float ABiomeManager::SampleNoiseAtLocation(const FVector& Location, float Scale) const
{
    float nx = Location.X * Scale;
    float ny = Location.Y * Scale;
    // Simple deterministic noise — replace with FMath::PerlinNoise2D in production
    return FMath::Sin(nx) * FMath::Cos(ny) + FMath::Sin(nx * 2.3f + ny * 1.7f) * 0.5f;
}
