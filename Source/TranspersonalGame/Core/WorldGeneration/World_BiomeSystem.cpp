#include "World_BiomeSystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AWorld_BiomeSystem::AWorld_BiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create biome volume component
    BiomeVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BiomeVolume"));
    RootComponent = BiomeVolume;
    BiomeVolume->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f));
    BiomeVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BiomeVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    BiomeVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visual marker
    BiomeMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BiomeMarker"));
    BiomeMarker->SetupAttachment(RootComponent);
    BiomeMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default biome data
    BiomeData.BiomeType = EWorld_BiomeType::Forest;
    BiomeData.VegetationDensity = 1.0f;
    BiomeData.Temperature = 25.0f;
    BiomeData.Humidity = 0.6f;

    // Initialize weather parameters
    RainIntensity = 0.0f;
    WindStrength = 1.0f;
    FogDensity = 0.1f;
}

void AWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome-specific settings
    switch (BiomeData.BiomeType)
    {
        case EWorld_BiomeType::Forest:
            BiomeData.Temperature = 22.0f;
            BiomeData.Humidity = 0.8f;
            FogDensity = 0.15f;
            break;
        case EWorld_BiomeType::Plains:
            BiomeData.Temperature = 28.0f;
            BiomeData.Humidity = 0.4f;
            WindStrength = 1.5f;
            break;
        case EWorld_BiomeType::Rocky:
            BiomeData.Temperature = 20.0f;
            BiomeData.Humidity = 0.3f;
            BiomeData.VegetationDensity = 0.3f;
            break;
        case EWorld_BiomeType::Wetlands:
            BiomeData.Temperature = 26.0f;
            BiomeData.Humidity = 0.9f;
            RainIntensity = 0.3f;
            break;
        case EWorld_BiomeType::Desert:
            BiomeData.Temperature = 35.0f;
            BiomeData.Humidity = 0.1f;
            BiomeData.VegetationDensity = 0.1f;
            break;
    }
}

void AWorld_BiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateWeatherConditions(DeltaTime);
}

EWorld_BiomeType AWorld_BiomeSystem::GetBiomeType() const
{
    return BiomeData.BiomeType;
}

void AWorld_BiomeSystem::SetBiomeType(EWorld_BiomeType NewBiomeType)
{
    BiomeData.BiomeType = NewBiomeType;
    BeginPlay(); // Reinitialize biome settings
}

void AWorld_BiomeSystem::UpdateWeatherConditions(float DeltaTime)
{
    // Simple weather simulation
    static float WeatherTimer = 0.0f;
    WeatherTimer += DeltaTime;

    // Vary rain intensity over time
    RainIntensity = FMath::Sin(WeatherTimer * 0.1f) * 0.5f + 0.5f;
    
    // Adjust based on biome type
    switch (BiomeData.BiomeType)
    {
        case EWorld_BiomeType::Desert:
            RainIntensity *= 0.1f;
            break;
        case EWorld_BiomeType::Wetlands:
            RainIntensity = FMath::Max(RainIntensity, 0.3f);
            break;
        case EWorld_BiomeType::Forest:
            FogDensity = 0.1f + (RainIntensity * 0.2f);
            break;
    }
}

void AWorld_BiomeSystem::SpawnVegetation()
{
    if (!GetWorld())
        return;

    FVector BiomeCenter = GetActorLocation();
    FVector BiomeExtent = BiomeVolume->GetScaledBoxExtent();
    
    int32 VegetationCount = FMath::RoundToInt(BiomeData.VegetationDensity * 50.0f);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = BiomeCenter + FVector(
            FMath::RandRange(-BiomeExtent.X, BiomeExtent.X),
            FMath::RandRange(-BiomeExtent.Y, BiomeExtent.Y),
            0.0f
        );
        
        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            SpawnLocation = HitResult.Location;
            
            // Spawn vegetation actor (placeholder - would use actual vegetation meshes)
            AActor* VegetationActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            if (VegetationActor)
            {
                VegetationActor->SetActorLabel(FString::Printf(TEXT("%s_Vegetation_%d"), 
                    *UEnum::GetValueAsString(BiomeData.BiomeType), i));
            }
        }
    }
}

bool AWorld_BiomeSystem::IsPointInBiome(const FVector& WorldPosition) const
{
    if (!BiomeVolume)
        return false;
        
    FVector LocalPosition = GetActorTransform().InverseTransformPosition(WorldPosition);
    FVector BoxExtent = BiomeVolume->GetScaledBoxExtent();
    
    return FMath::Abs(LocalPosition.X) <= BoxExtent.X &&
           FMath::Abs(LocalPosition.Y) <= BoxExtent.Y &&
           FMath::Abs(LocalPosition.Z) <= BoxExtent.Z;
}

float AWorld_BiomeSystem::GetEnvironmentalFactor(const FString& FactorName) const
{
    if (FactorName == "Temperature")
        return BiomeData.Temperature;
    else if (FactorName == "Humidity")
        return BiomeData.Humidity;
    else if (FactorName == "VegetationDensity")
        return BiomeData.VegetationDensity;
    else if (FactorName == "RainIntensity")
        return RainIntensity;
    else if (FactorName == "WindStrength")
        return WindStrength;
    else if (FactorName == "FogDensity")
        return FogDensity;
    
    return 0.0f;
}