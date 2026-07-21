#include "Core/World/BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ============================================================
// UWorld_BiomeComponent
// ============================================================

UWorld_BiomeComponent::UWorld_BiomeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update once per second
    CurrentBiome = EWorld_BiomeType::None;
    TransitionBlend = 0.0f;
}

void UWorld_BiomeComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultBiomes();
}

void UWorld_BiomeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (Owner)
    {
        UpdateBiomeForActor(Owner);
    }
}

void UWorld_BiomeComponent::InitializeDefaultBiomes()
{
    RegisteredBiomes.Empty();

    // Forest biome — dense jungle, high humidity
    FWorld_BiomeData Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.WorldCenter = FVector(-1500.f, 0.f, 0.f);
    Forest.Radius = 1800.f;
    Forest.VegetationDensity = 2.5f;
    Forest.AmbientTemperature = 26.f;
    Forest.Humidity = 0.85f;
    Forest.FogColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
    RegisteredBiomes.Add(Forest);

    // Plains biome — open grassland, moderate conditions
    FWorld_BiomeData Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.WorldCenter = FVector(1500.f, 0.f, 0.f);
    Plains.Radius = 1800.f;
    Plains.VegetationDensity = 0.8f;
    Plains.AmbientTemperature = 28.f;
    Plains.Humidity = 0.4f;
    Plains.FogColor = FLinearColor(0.7f, 0.75f, 0.6f, 1.0f);
    RegisteredBiomes.Add(Plains);

    // Rocky badlands — sparse vegetation, hot and dry
    FWorld_BiomeData Rocky;
    Rocky.BiomeType = EWorld_BiomeType::Rocky;
    Rocky.WorldCenter = FVector(0.f, -1500.f, 0.f);
    Rocky.Radius = 1600.f;
    Rocky.VegetationDensity = 0.2f;
    Rocky.AmbientTemperature = 38.f;
    Rocky.Humidity = 0.15f;
    Rocky.FogColor = FLinearColor(0.8f, 0.65f, 0.4f, 1.0f);
    RegisteredBiomes.Add(Rocky);

    // Swamp biome — waterlogged, high humidity, dense low vegetation
    FWorld_BiomeData Swamp;
    Swamp.BiomeType = EWorld_BiomeType::Swamp;
    Swamp.WorldCenter = FVector(0.f, 1500.f, 0.f);
    Swamp.Radius = 1600.f;
    Swamp.VegetationDensity = 1.8f;
    Swamp.AmbientTemperature = 30.f;
    Swamp.Humidity = 0.95f;
    Swamp.FogColor = FLinearColor(0.3f, 0.5f, 0.35f, 1.0f);
    RegisteredBiomes.Add(Swamp);

    // River valley — central corridor
    FWorld_BiomeData River;
    River.BiomeType = EWorld_BiomeType::River;
    River.WorldCenter = FVector(0.f, 800.f, -80.f);
    River.Radius = 600.f;
    River.VegetationDensity = 1.5f;
    River.AmbientTemperature = 24.f;
    River.Humidity = 0.9f;
    River.FogColor = FLinearColor(0.5f, 0.65f, 0.8f, 1.0f);
    RegisteredBiomes.Add(River);
}

EWorld_BiomeType UWorld_BiomeComponent::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::None;

    for (const FWorld_BiomeData& Biome : RegisteredBiomes)
    {
        float Dist = FVector::Dist2D(WorldLocation, Biome.WorldCenter);
        if (Dist < Biome.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData UWorld_BiomeComponent::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : RegisteredBiomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    return FWorld_BiomeData(); // Return default if not found
}

void UWorld_BiomeComponent::UpdateBiomeForActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    FVector ActorLocation = TargetActor->GetActorLocation();
    EWorld_BiomeType NewBiome = GetBiomeAtLocation(ActorLocation);

    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        TransitionBlend = 0.0f;
    }
    else
    {
        TransitionBlend = FMath::Clamp(TransitionBlend + 0.1f, 0.0f, 1.0f);
    }
}

// ============================================================
// AWorld_BiomeManager
// ============================================================

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f;

    BiomeComponent = CreateDefaultSubobject<UWorld_BiomeComponent>(TEXT("BiomeComponent"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize biome definitions from component defaults
    if (BiomeComponent)
    {
        for (EWorld_BiomeType BiomeType : {
            EWorld_BiomeType::Forest,
            EWorld_BiomeType::Plains,
            EWorld_BiomeType::Rocky,
            EWorld_BiomeType::Swamp,
            EWorld_BiomeType::River
        })
        {
            FWorld_BiomeData Data = BiomeComponent->GetBiomeData(BiomeType);
            if (Data.BiomeType != EWorld_BiomeType::None)
            {
                BiomeDefinitions.Add(Data);
            }
        }
    }
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Periodic biome state update — lightweight, 2s interval
}

void AWorld_BiomeManager::RegenerateBiomes()
{
    BiomeDefinitions.Empty();
    if (BiomeComponent)
    {
        for (EWorld_BiomeType BiomeType : {
            EWorld_BiomeType::Forest,
            EWorld_BiomeType::Plains,
            EWorld_BiomeType::Rocky,
            EWorld_BiomeType::Swamp,
            EWorld_BiomeType::River
        })
        {
            FWorld_BiomeData Data = BiomeComponent->GetBiomeData(BiomeType);
            if (Data.BiomeType != EWorld_BiomeType::None)
            {
                BiomeDefinitions.Add(Data);
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AWorld_BiomeManager: Biomes regenerated — %d biomes active"), BiomeDefinitions.Num());
}

EWorld_BiomeType AWorld_BiomeManager::QueryBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!BiomeComponent)
    {
        return EWorld_BiomeType::None;
    }
    return BiomeComponent->GetBiomeAtLocation(WorldLocation);
}

float AWorld_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    if (!BiomeComponent)
    {
        return 25.f; // Default temperature
    }

    EWorld_BiomeType Biome = BiomeComponent->GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData Data = BiomeComponent->GetBiomeData(Biome);
    return Data.AmbientTemperature;
}

float AWorld_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    if (!BiomeComponent)
    {
        return 0.5f; // Default humidity
    }

    EWorld_BiomeType Biome = BiomeComponent->GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData Data = BiomeComponent->GetBiomeData(Biome);
    return Data.Humidity;
}
