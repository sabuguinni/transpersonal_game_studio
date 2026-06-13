#include "World_BiomeSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Engine.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize default biome definitions
    BiomeDefinitions.Empty();
    
    // Tropical Forest
    FWorld_BiomeData TropicalForest;
    TropicalForest.BiomeType = EWorld_BiomeType::TropicalForest;
    TropicalForest.BiomeName = "Tropical Forest";
    TropicalForest.FogColor = FLinearColor(0.2f, 0.6f, 0.3f, 1.0f);
    TropicalForest.FogDensity = 0.05f;
    TropicalForest.Temperature = 28.0f;
    TropicalForest.Humidity = 0.85f;
    BiomeDefinitions.Add(TropicalForest);

    // Desert Canyon
    FWorld_BiomeData DesertCanyon;
    DesertCanyon.BiomeType = EWorld_BiomeType::DesertCanyon;
    DesertCanyon.BiomeName = "Desert Canyon";
    DesertCanyon.FogColor = FLinearColor(0.9f, 0.7f, 0.4f, 1.0f);
    DesertCanyon.FogDensity = 0.01f;
    DesertCanyon.Temperature = 35.0f;
    DesertCanyon.Humidity = 0.15f;
    BiomeDefinitions.Add(DesertCanyon);

    // Swampland
    FWorld_BiomeData Swampland;
    Swampland.BiomeType = EWorld_BiomeType::Swampland;
    Swampland.BiomeName = "Swampland";
    Swampland.FogColor = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);
    Swampland.FogDensity = 0.08f;
    Swampland.Temperature = 22.0f;
    Swampland.Humidity = 0.95f;
    BiomeDefinitions.Add(Swampland);

    BiomeTransitionRadius = 5000.0f;
    WeatherUpdateInterval = 30.0f;
    CurrentBiome = EWorld_BiomeType::TropicalForest;
    CurrentTemperature = 25.0f;
    CurrentHumidity = 0.5f;
    WeatherTimer = 0.0f;
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeLocations();
    UpdateCurrentBiome();
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem initialized with %d biomes"), BiomeDefinitions.Num());
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    WeatherTimer += DeltaTime;
    if (WeatherTimer >= WeatherUpdateInterval)
    {
        UpdateCurrentBiome();
        UpdateWeatherForBiome(CurrentBiome);
        WeatherTimer = 0.0f;
    }
}

void UWorld_BiomeSystem::InitializeBiomeLocations()
{
    BiomeCenterLocations.Empty();
    
    // Define biome center locations in world space
    BiomeCenterLocations.Add(EWorld_BiomeType::TropicalForest, FVector(-5000.0f, -5000.0f, 0.0f));
    BiomeCenterLocations.Add(EWorld_BiomeType::DesertCanyon, FVector(5000.0f, 0.0f, 0.0f));
    BiomeCenterLocations.Add(EWorld_BiomeType::Swampland, FVector(0.0f, 5000.0f, 0.0f));
    BiomeCenterLocations.Add(EWorld_BiomeType::RockyPlains, FVector(-3000.0f, 3000.0f, 0.0f));
    BiomeCenterLocations.Add(EWorld_BiomeType::VolcanicRegion, FVector(3000.0f, -3000.0f, 0.0f));
    BiomeCenterLocations.Add(EWorld_BiomeType::CoastalBeach, FVector(0.0f, -8000.0f, 0.0f));
}

EWorld_BiomeType UWorld_BiomeSystem::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::TropicalForest;

    for (const auto& BiomePair : BiomeCenterLocations)
    {
        float Distance = FVector::Dist(Location, BiomePair.Value);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& BiomeData : BiomeDefinitions)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            return BiomeData;
        }
    }

    // Return default if not found
    return BiomeDefinitions.Num() > 0 ? BiomeDefinitions[0] : FWorld_BiomeData();
}

void UWorld_BiomeSystem::UpdateWeatherForBiome(EWorld_BiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    CurrentTemperature = BiomeData.Temperature + FMath::RandRange(-5.0f, 5.0f);
    CurrentHumidity = FMath::Clamp(BiomeData.Humidity + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Weather updated for %s: Temp=%.1f, Humidity=%.2f"), 
           *BiomeData.BiomeName, CurrentTemperature, CurrentHumidity);
}

void UWorld_BiomeSystem::TransitionToBiome(EWorld_BiomeType NewBiome, float TransitionTime)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        FWorld_BiomeData BiomeData = GetBiomeData(NewBiome);
        
        UE_LOG(LogTemp, Log, TEXT("Transitioning to biome: %s"), *BiomeData.BiomeName);
        
        // Immediate weather update for new biome
        UpdateWeatherForBiome(NewBiome);
    }
}

TArray<FVector> UWorld_BiomeSystem::GetBiomeCenters() const
{
    TArray<FVector> Centers;
    BiomeCenterLocations.GenerateValueArray(Centers);
    return Centers;
}

void UWorld_BiomeSystem::SpawnBiomeVegetation(const FVector& Location, float Radius, EWorld_BiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Spawn vegetation based on biome type
    int32 VegetationCount = FMath::RandRange(5, 15);
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = Location + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );

        // Create a basic vegetation actor (placeholder)
        AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
        if (VegetationActor)
        {
            FString VegLabel = FString::Printf(TEXT("Vegetation_%s_%d"), *BiomeData.BiomeName, i);
            VegetationActor->SetActorLabel(VegLabel);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d vegetation actors for %s biome at %s"), 
           VegetationCount, *BiomeData.BiomeName, *Location.ToString());
}

void UWorld_BiomeSystem::UpdateCurrentBiome()
{
    FVector PlayerLocation = GetPlayerLocation();
    EWorld_BiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        TransitionToBiome(NewBiome);
    }
}

FVector UWorld_BiomeSystem::GetPlayerLocation() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        return PC->GetPawn()->GetActorLocation();
    }

    return FVector::ZeroVector;
}