#include "World_VolcanicBiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_VolcanicBiomeGenerator::AWorld_VolcanicBiomeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Default volcanic parameters
    MaxVolcanicRocks = 50;
    LavaFlowIntensity = 1.0f;
    AshDensity = 0.7f;
    TemperatureMultiplier = 2.5f;
    bGenerateOnBeginPlay = true;

    // Initialize with default volcanic feature
    FWorld_VolcanicFeature DefaultFeature;
    DefaultFeature.Location = FVector(0.0f, 0.0f, 0.0f);
    DefaultFeature.Intensity = 1.0f;
    DefaultFeature.Radius = 1000.0f;
    DefaultFeature.BiomeType = EWorld_BiomeType::Volcanic;
    VolcanicFeatures.Add(DefaultFeature);
}

void AWorld_VolcanicBiomeGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bGenerateOnBeginPlay)
    {
        GenerateVolcanicTerrain();
    }
}

void AWorld_VolcanicBiomeGenerator::GenerateVolcanicTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Starting volcanic terrain generation"));

    // Clear existing terrain first
    ClearVolcanicTerrain();

    // Generate volcanic rocks
    SpawnVolcanicRocks(MaxVolcanicRocks);

    // Create lava flows
    CreateLavaFlows();

    // Apply volcanic effects
    ApplyVolcanicEffects();

    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Volcanic terrain generation complete"));
}

void AWorld_VolcanicBiomeGenerator::SpawnVolcanicRocks(int32 Count)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("VolcanicBiomeGenerator: No valid world found"));
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Generate random location within volcanic zones
        FVector SpawnLocation = FVector::ZeroVector;
        bool bValidLocation = false;

        for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
        {
            if (FMath::RandFloat() < 0.7f) // 70% chance to spawn near volcanic features
            {
                float RandomAngle = FMath::RandFloat() * 2.0f * PI;
                float RandomDistance = FMath::RandFloat() * Feature.Radius;
                
                SpawnLocation = Feature.Location + FVector(
                    FMath::Cos(RandomAngle) * RandomDistance,
                    FMath::Sin(RandomAngle) * RandomDistance,
                    FMath::RandRange(-100.0f, 200.0f)
                );
                bValidLocation = true;
                break;
            }
        }

        if (!bValidLocation)
        {
            // Fallback to random location
            SpawnLocation = FVector(
                FMath::RandRange(-2000.0f, 2000.0f),
                FMath::RandRange(-2000.0f, 2000.0f),
                FMath::RandRange(0.0f, 500.0f)
            );
        }

        SpawnVolcanicRockAtLocation(SpawnLocation);
    }

    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Spawned %d volcanic rocks"), Count);
}

void AWorld_VolcanicBiomeGenerator::SpawnVolcanicRockAtLocation(FVector Location)
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn static mesh actor for volcanic rock
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* VolcanicRock = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (VolcanicRock)
    {
        VolcanicRock->SetActorLabel(FString::Printf(TEXT("VolcanicRock_%d"), SpawnedVolcanicActors.Num()));
        
        // Apply volcanic material and effects
        ApplyVolcanicMaterial(VolcanicRock);
        
        SpawnedVolcanicActors.Add(VolcanicRock);
    }
}

void AWorld_VolcanicBiomeGenerator::CreateLavaFlows()
{
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        if (Feature.Intensity > 0.5f) // Only create lava flows for high-intensity features
        {
            int32 FlowCount = FMath::RandRange(2, 5);
            
            for (int32 i = 0; i < FlowCount; i++)
            {
                // Create lava flow path
                FVector FlowStart = Feature.Location;
                FVector FlowDirection = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    -0.5f // Flows downward
                ).GetSafeNormal();

                float FlowLength = Feature.Radius * 0.8f;
                FVector FlowEnd = FlowStart + (FlowDirection * FlowLength);

                // Spawn lava flow actors along the path
                int32 FlowSegments = FMath::RandRange(5, 10);
                for (int32 j = 0; j < FlowSegments; j++)
                {
                    float Alpha = static_cast<float>(j) / static_cast<float>(FlowSegments - 1);
                    FVector SegmentLocation = FMath::Lerp(FlowStart, FlowEnd, Alpha);
                    
                    SpawnVolcanicRockAtLocation(SegmentLocation);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Created lava flows"));
}

void AWorld_VolcanicBiomeGenerator::ApplyVolcanicEffects()
{
    // Apply temperature and environmental effects
    for (AActor* Actor : SpawnedVolcanicActors)
    {
        if (Actor)
        {
            // Add temperature effect component or modify material properties
            ApplyVolcanicMaterial(Actor);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Applied volcanic effects"));
}

FWorld_VolcanicFeature AWorld_VolcanicBiomeGenerator::CreateVolcanicFeature(FVector Location, float Intensity, float Radius)
{
    FWorld_VolcanicFeature NewFeature;
    NewFeature.Location = Location;
    NewFeature.Intensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    NewFeature.Radius = FMath::Max(Radius, 100.0f);
    NewFeature.BiomeType = EWorld_BiomeType::Volcanic;

    VolcanicFeatures.Add(NewFeature);
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Created volcanic feature at %s"), *Location.ToString());
    
    return NewFeature;
}

void AWorld_VolcanicBiomeGenerator::ClearVolcanicTerrain()
{
    for (AActor* Actor : SpawnedVolcanicActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedVolcanicActors.Empty();
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Cleared existing volcanic terrain"));
}

float AWorld_VolcanicBiomeGenerator::GetTemperatureAtLocation(FVector Location) const
{
    float BaseTemperature = 25.0f; // Base temperature in Celsius
    float VolcanicHeat = 0.0f;

    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance < Feature.Radius)
        {
            float HeatContribution = Feature.Intensity * TemperatureMultiplier * (1.0f - (Distance / Feature.Radius));
            VolcanicHeat += HeatContribution;
        }
    }

    return BaseTemperature + VolcanicHeat;
}

bool AWorld_VolcanicBiomeGenerator::IsLocationInVolcanicZone(FVector Location) const
{
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance <= Feature.Radius)
        {
            return true;
        }
    }
    
    return false;
}

void AWorld_VolcanicBiomeGenerator::ApplyVolcanicMaterial(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
    if (MeshActor && MeshActor->GetStaticMeshComponent())
    {
        // Set volcanic material properties
        // Note: In a real implementation, you would load and apply volcanic materials here
        UE_LOG(LogTemp, Log, TEXT("VolcanicBiomeGenerator: Applied volcanic material to %s"), *Actor->GetName());
    }
}

float AWorld_VolcanicBiomeGenerator::CalculateVolcanicIntensity(FVector Location) const
{
    float TotalIntensity = 0.0f;
    
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance < Feature.Radius)
        {
            float IntensityContribution = Feature.Intensity * (1.0f - (Distance / Feature.Radius));
            TotalIntensity += IntensityContribution;
        }
    }
    
    return FMath::Clamp(TotalIntensity, 0.0f, 2.0f);
}