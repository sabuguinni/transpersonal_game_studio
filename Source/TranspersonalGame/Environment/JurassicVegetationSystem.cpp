#include "JurassicVegetationSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

UJurassicVegetationSystem::UJurassicVegetationSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize noise settings for natural distribution
    NoiseScale = 0.01f;
    NoiseThreshold = 0.3f;
}

void UJurassicVegetationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Jurassic Vegetation System initialized"));
}

void UJurassicVegetationSystem::GenerateVegetationInArea(FVector Center, float Radius, EJurassicBiomeType BiomeType)
{
    UJurassicBiomeData* BiomeData = GetBiomeData(BiomeType);
    if (!BiomeData)
    {
        UE_LOG(LogTemp, Error, TEXT("No biome data found for biome type"));
        return;
    }

    // Generate vegetation in layers from bottom to top
    SpawnVegetationLayer(BiomeData->VegetationProfile.GroundLayer, Center, Radius, 0.0f);
    SpawnVegetationLayer(BiomeData->VegetationProfile.AquaticLayer, Center, Radius, -50.0f);
    SpawnVegetationLayer(BiomeData->VegetationProfile.UnderstoryLayer, Center, Radius, 200.0f);
    SpawnVegetationLayer(BiomeData->VegetationProfile.CanopyLayer, Center, Radius, 800.0f);
    SpawnVegetationLayer(BiomeData->VegetationProfile.DeadVegetationLayer, Center, Radius, 0.0f);

    UE_LOG(LogTemp, Warning, TEXT("Generated vegetation for biome %d at location %s"), 
           (int32)BiomeType, *Center.ToString());
}

void UJurassicVegetationSystem::SpawnVegetationLayer(const TArray<FVegetationLayer>& Layer, FVector Center, float Radius, float LayerHeight)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FVegetationLayer& VegData : Layer)
    {
        if (!VegData.VegetationMesh) continue;

        // Calculate number of instances based on density and area
        float Area = PI * Radius * Radius;
        int32 InstanceCount = FMath::RoundToInt(Area * VegData.Density * 0.0001f); // Convert to per-square-meter

        for (int32 i = 0; i < InstanceCount; i++)
        {
            // Generate random position within radius using Poisson disk sampling approach
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            float Distance = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * Radius;
            
            FVector SpawnLocation = Center + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                LayerHeight
            );

            // Add Perlin noise for natural distribution
            float NoiseValue = FMath::PerlinNoise2D(FVector2D(SpawnLocation.X * NoiseScale, SpawnLocation.Y * NoiseScale));
            if (NoiseValue < NoiseThreshold) continue;

            // Check if location is suitable
            if (!IsLocationSuitableForVegetation(SpawnLocation, VegData)) continue;

            // Trace to ground
            FHitResult HitResult;
            FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
            FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
            
            if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                SpawnLocation = HitResult.Location;
                
                // Create instance
                AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>();
                if (VegetationActor)
                {
                    VegetationActor->GetStaticMeshComponent()->SetStaticMesh(VegData.VegetationMesh);
                    
                    // Random scale within range
                    float Scale = FMath::RandRange(VegData.ScaleRange.X, VegData.ScaleRange.Y);
                    VegetationActor->SetActorScale3D(FVector(Scale));
                    
                    // Random rotation
                    FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                    VegetationActor->SetActorRotation(RandomRotation);
                    
                    VegetationActor->SetActorLocation(SpawnLocation);
                    
                    // Tag for identification
                    VegetationActor->Tags.Add(TEXT("JurassicVegetation"));
                }
            }
        }
    }
}

bool UJurassicVegetationSystem::IsLocationSuitableForVegetation(FVector Location, const FVegetationLayer& VegData)
{
    // Check slope
    float Slope = GetTerrainSlope(Location);
    if (Slope > VegData.MaxSlope) return false;

    // Check water distance
    float WaterDist = GetDistanceToWater(Location);
    if (VegData.WaterDistance > 0 && WaterDist < VegData.WaterDistance) return false;
    if (VegData.WaterDistance < 0 && WaterDist > FMath::Abs(VegData.WaterDistance)) return false;

    // Check altitude
    if (Location.Z < VegData.AltitudeRange.X || Location.Z > VegData.AltitudeRange.Y) return false;

    return true;
}

float UJurassicVegetationSystem::GetTerrainSlope(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World) return 0.0f;

    // Sample height at multiple points to calculate slope
    float SampleDistance = 100.0f;
    TArray<FVector> SamplePoints = {
        Location + FVector(SampleDistance, 0, 0),
        Location + FVector(-SampleDistance, 0, 0),
        Location + FVector(0, SampleDistance, 0),
        Location + FVector(0, -SampleDistance, 0)
    };

    float CenterHeight = Location.Z;
    float MaxHeightDiff = 0.0f;

    for (const FVector& SamplePoint : SamplePoints)
    {
        FHitResult HitResult;
        FVector TraceStart = SamplePoint + FVector(0, 0, 1000);
        FVector TraceEnd = SamplePoint - FVector(0, 0, 1000);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            float HeightDiff = FMath::Abs(HitResult.Location.Z - CenterHeight);
            MaxHeightDiff = FMath::Max(MaxHeightDiff, HeightDiff);
        }
    }

    // Convert to degrees
    return FMath::RadiansToDegrees(FMath::Atan(MaxHeightDiff / SampleDistance));
}

float UJurassicVegetationSystem::GetDistanceToWater(FVector Location)
{
    // Simplified water detection - in a full implementation, this would check for water bodies
    // For now, assume water is at Z = 0 and calculate distance to that plane
    return FMath::Abs(Location.Z);
}

void UJurassicVegetationSystem::ClearVegetationInArea(FVector Center, float Radius, float ClearancePercentage)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find all vegetation actors in the area
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(World, TEXT("JurassicVegetation"), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor) continue;

        float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
        if (Distance <= Radius)
        {
            // Random chance to remove based on clearance percentage
            if (FMath::RandRange(0.0f, 1.0f) < ClearancePercentage)
            {
                Actor->Destroy();
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Cleared vegetation in area: Center=%s, Radius=%f"), 
           *Center.ToString(), Radius);
}

void UJurassicVegetationSystem::PlaceStorytellingProps(FVector Location, EJurassicBiomeType BiomeType, int32 PropCount)
{
    UJurassicBiomeData* BiomeData = GetBiomeData(BiomeType);
    if (!BiomeData || BiomeData->StorytellingProps.Num() == 0) return;

    UWorld* World = GetWorld();
    if (!World) return;

    for (int32 i = 0; i < PropCount; i++)
    {
        // Random prop from the biome's storytelling props
        UStaticMesh* PropMesh = BiomeData->StorytellingProps[FMath::RandRange(0, BiomeData->StorytellingProps.Num() - 1)];
        if (!PropMesh) continue;

        // Random position near the location
        FVector PropLocation = Location + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0
        );

        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = PropLocation + FVector(0, 0, 1000);
        FVector TraceEnd = PropLocation - FVector(0, 0, 1000);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            PropLocation = HitResult.Location;
            
            AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>();
            if (PropActor)
            {
                PropActor->GetStaticMeshComponent()->SetStaticMesh(PropMesh);
                PropActor->SetActorLocation(PropLocation);
                PropActor->SetActorRotation(FRotator(0, FMath::RandRange(0.0f, 360.0f), 0));
                PropActor->Tags.Add(TEXT("StorytellingProp"));
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Placed %d storytelling props at location %s"), PropCount, *Location.ToString());
}

UJurassicBiomeData* UJurassicVegetationSystem::GetBiomeData(EJurassicBiomeType BiomeType)
{
    if (BiomeDataMap.Contains(BiomeType))
    {
        return BiomeDataMap[BiomeType];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No biome data found for type %d"), (int32)BiomeType);
    return nullptr;
}