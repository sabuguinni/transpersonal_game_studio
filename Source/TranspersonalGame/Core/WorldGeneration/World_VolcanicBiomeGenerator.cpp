#include "World_VolcanicBiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneComponent.h"

UWorld_VolcanicBiomeGenerator::UWorld_VolcanicBiomeGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    BaseTemperature = 25.0f;
    VolcanicInfluenceRadius = 1000.0f;
    MaxVolcanicFeatures = 50;
    MaxLavaFlows = 20;
    bEnableVolcanicSimulation = true;
    VolcanicUpdateInterval = 1.0f;
    LastVolcanicUpdate = 0.0f;
    CurrentLODLevel = 0;
}

void UWorld_VolcanicBiomeGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: BeginPlay - Initializing volcanic terrain system"));
    
    // Initialize with a default volcanic feature for testing
    FWorld_VolcanicFeature DefaultVolcano;
    DefaultVolcano.Location = FVector(0.0f, 0.0f, 500.0f);
    DefaultVolcano.Intensity = 1.0f;
    DefaultVolcano.Radius = 800.0f;
    DefaultVolcano.BiomeType = EWorld_BiomeType::Volcanic;
    DefaultVolcano.bIsActive = true;
    DefaultVolcano.Temperature = 80.0f;
    DefaultVolcano.LavaFlowSpeed = 10.0f;
    
    VolcanicFeatures.Add(DefaultVolcano);
    
    // Create initial lava flow
    FWorld_LavaFlow InitialFlow;
    InitialFlow.FlowPath.Add(FVector(0.0f, 0.0f, 500.0f));
    InitialFlow.FlowPath.Add(FVector(200.0f, 100.0f, 300.0f));
    InitialFlow.FlowPath.Add(FVector(400.0f, -50.0f, 200.0f));
    InitialFlow.FlowWidth = 100.0f;
    InitialFlow.FlowSpeed = 5.0f;
    InitialFlow.Temperature = 1200.0f;
    InitialFlow.bIsFlowing = true;
    InitialFlow.DamagePerSecond = 100.0f;
    
    LavaFlows.Add(InitialFlow);
}

void UWorld_VolcanicBiomeGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableVolcanicSimulation)
    {
        return;
    }
    
    LastVolcanicUpdate += DeltaTime;
    
    if (LastVolcanicUpdate >= VolcanicUpdateInterval)
    {
        UpdateLavaFlows(DeltaTime);
        LastVolcanicUpdate = 0.0f;
    }
}

void UWorld_VolcanicBiomeGenerator::GenerateVolcanicTerrain(const FVector& CenterLocation, float Radius, int32 NumVolcanoes)
{
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Generating volcanic terrain at %s with radius %f"), 
           *CenterLocation.ToString(), Radius);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("VolcanicBiomeGenerator: No valid world found"));
        return;
    }
    
    // Clear existing volcanic features in the area
    VolcanicFeatures.RemoveAll([&](const FWorld_VolcanicFeature& Feature)
    {
        return FVector::Dist(Feature.Location, CenterLocation) <= Radius;
    });
    
    // Generate new volcanic features
    for (int32 i = 0; i < NumVolcanoes; ++i)
    {
        FWorld_VolcanicFeature NewVolcano;
        
        // Random location within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius * 0.8f);
        
        NewVolcano.Location = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(100.0f, 500.0f)
        );
        
        NewVolcano.Intensity = FMath::RandRange(0.5f, 1.5f);
        NewVolcano.Radius = FMath::RandRange(300.0f, 800.0f);
        NewVolcano.BiomeType = EWorld_BiomeType::Volcanic;
        NewVolcano.bIsActive = FMath::RandBool();
        NewVolcano.Temperature = FMath::RandRange(60.0f, 100.0f);
        NewVolcano.LavaFlowSpeed = FMath::RandRange(5.0f, 15.0f);
        
        VolcanicFeatures.Add(NewVolcano);
        
        // Create volcanic rocks around this volcano
        CreateVolcanicRocks(NewVolcano.Location, NewVolcano.Intensity);
        
        // Generate lava flows if volcano is active
        if (NewVolcano.bIsActive)
        {
            GenerateLavaFlows(NewVolcano.Location, FMath::RandRange(2, 6));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Generated %d volcanic features"), NumVolcanoes);
}

void UWorld_VolcanicBiomeGenerator::CreateVolcanicRocks(const FVector& Location, float Density)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 NumRocks = FMath::RoundToInt(Density * 20.0f);
    
    for (int32 i = 0; i < NumRocks; ++i)
    {
        FVector RockLocation = Location + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-50.0f, 100.0f)
        );
        
        AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            RockLocation,
            FRotator::ZeroRotator
        );
        
        if (RockActor)
        {
            RockActor->SetActorLabel(FString::Printf(TEXT("VolcanicRock_%d"), i));
            
            // Random scale for variety
            FVector Scale = FVector(
                FMath::RandRange(1.0f, 3.0f),
                FMath::RandRange(1.0f, 3.0f),
                FMath::RandRange(1.0f, 4.0f)
            );
            RockActor->SetActorScale3D(Scale);
        }
    }
}

void UWorld_VolcanicBiomeGenerator::GenerateLavaFlows(const FVector& VolcanoLocation, int32 NumFlows)
{
    for (int32 i = 0; i < NumFlows; ++i)
    {
        FWorld_LavaFlow NewFlow;
        
        // Start at volcano location
        NewFlow.FlowPath.Add(VolcanoLocation);
        
        // Generate flow path downhill
        FVector CurrentLocation = VolcanoLocation;
        int32 PathSegments = FMath::RandRange(3, 8);
        
        for (int32 j = 0; j < PathSegments; ++j)
        {
            // Flow generally downward with some randomness
            FVector NextLocation = CurrentLocation + FVector(
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(-100.0f, -20.0f) // Always flow downward
            );
            
            NewFlow.FlowPath.Add(NextLocation);
            CurrentLocation = NextLocation;
        }
        
        NewFlow.FlowWidth = FMath::RandRange(50.0f, 150.0f);
        NewFlow.FlowSpeed = FMath::RandRange(3.0f, 12.0f);
        NewFlow.Temperature = FMath::RandRange(1000.0f, 1300.0f);
        NewFlow.bIsFlowing = true;
        NewFlow.DamagePerSecond = FMath::RandRange(80.0f, 120.0f);
        
        LavaFlows.Add(NewFlow);
    }
}

void UWorld_VolcanicBiomeGenerator::CreateAshClouds(const FVector& Location, float Intensity)
{
    // This would typically spawn particle systems for ash clouds
    // For now, we'll log the creation
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Creating ash clouds at %s with intensity %f"), 
           *Location.ToString(), Intensity);
}

void UWorld_VolcanicBiomeGenerator::SpawnVolcanicVegetation(const FVector& Location, float SafeDistance)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Only spawn vegetation outside the safe distance from volcanic features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        if (FVector::Dist(Location, Feature.Location) < SafeDistance)
        {
            return; // Too close to volcanic activity
        }
    }
    
    // Spawn heat-resistant vegetation
    int32 NumPlants = FMath::RandRange(5, 15);
    
    for (int32 i = 0; i < NumPlants; ++i)
    {
        FVector PlantLocation = Location + FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        );
        
        AStaticMeshActor* PlantActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            PlantLocation,
            FRotator::ZeroRotator
        );
        
        if (PlantActor)
        {
            PlantActor->SetActorLabel(FString::Printf(TEXT("VolcanicPlant_%d"), i));
        }
    }
}

void UWorld_VolcanicBiomeGenerator::AddVolcanicFeature(const FWorld_VolcanicFeature& Feature)
{
    if (VolcanicFeatures.Num() < MaxVolcanicFeatures)
    {
        VolcanicFeatures.Add(Feature);
        UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Added volcanic feature at %s"), 
               *Feature.Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Maximum volcanic features reached (%d)"), 
               MaxVolcanicFeatures);
    }
}

void UWorld_VolcanicBiomeGenerator::RemoveVolcanicFeature(int32 FeatureIndex)
{
    if (VolcanicFeatures.IsValidIndex(FeatureIndex))
    {
        VolcanicFeatures.RemoveAt(FeatureIndex);
        UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Removed volcanic feature at index %d"), 
               FeatureIndex);
    }
}

void UWorld_VolcanicBiomeGenerator::UpdateLavaFlows(float DeltaTime)
{
    for (FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (!Flow.bIsFlowing)
        {
            continue;
        }
        
        // Update flow properties over time
        Flow.Temperature = FMath::Max(Flow.Temperature - (DeltaTime * 10.0f), 500.0f);
        
        // Stop flowing if temperature gets too low
        if (Flow.Temperature < 600.0f)
        {
            Flow.bIsFlowing = false;
            Flow.FlowSpeed = 0.0f;
        }
    }
    
    // Remove cooled lava flows
    LavaFlows.RemoveAll([](const FWorld_LavaFlow& Flow)
    {
        return !Flow.bIsFlowing && Flow.Temperature < 500.0f;
    });
}

void UWorld_VolcanicBiomeGenerator::CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation)
{
    if (LavaFlows.Num() >= MaxLavaFlows)
    {
        UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Maximum lava flows reached (%d)"), MaxLavaFlows);
        return;
    }
    
    FWorld_LavaFlow NewFlow;
    NewFlow.FlowPath.Add(StartLocation);
    NewFlow.FlowPath.Add(EndLocation);
    NewFlow.FlowWidth = 80.0f;
    NewFlow.FlowSpeed = 8.0f;
    NewFlow.Temperature = 1200.0f;
    NewFlow.bIsFlowing = true;
    NewFlow.DamagePerSecond = 100.0f;
    
    LavaFlows.Add(NewFlow);
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Created lava flow from %s to %s"), 
           *StartLocation.ToString(), *EndLocation.ToString());
}

bool UWorld_VolcanicBiomeGenerator::IsLocationInLava(const FVector& Location) const
{
    for (const FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (!Flow.bIsFlowing)
        {
            continue;
        }
        
        // Check if location is within any segment of the lava flow
        for (int32 i = 0; i < Flow.FlowPath.Num() - 1; ++i)
        {
            FVector SegmentStart = Flow.FlowPath[i];
            FVector SegmentEnd = Flow.FlowPath[i + 1];
            
            // Simple distance check to flow path
            FVector ClosestPoint = FMath::ClosestPointOnSegment(Location, SegmentStart, SegmentEnd);
            float DistanceToFlow = FVector::Dist(Location, ClosestPoint);
            
            if (DistanceToFlow <= Flow.FlowWidth * 0.5f)
            {
                return true;
            }
        }
    }
    
    return false;
}

float UWorld_VolcanicBiomeGenerator::GetTemperatureAtLocation(const FVector& Location) const
{
    float Temperature = BaseTemperature;
    
    // Add temperature from volcanic features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance <= Feature.Radius)
        {
            float Influence = 1.0f - (Distance / Feature.Radius);
            Temperature += Feature.Temperature * Feature.Intensity * Influence;
        }
    }
    
    // Add temperature from lava flows
    for (const FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (!Flow.bIsFlowing)
        {
            continue;
        }
        
        for (int32 i = 0; i < Flow.FlowPath.Num() - 1; ++i)
        {
            FVector ClosestPoint = FMath::ClosestPointOnSegment(Location, Flow.FlowPath[i], Flow.FlowPath[i + 1]);
            float Distance = FVector::Dist(Location, ClosestPoint);
            
            if (Distance <= Flow.FlowWidth * 2.0f)
            {
                float Influence = 1.0f - (Distance / (Flow.FlowWidth * 2.0f));
                Temperature += Flow.Temperature * 0.1f * Influence;
            }
        }
    }
    
    return Temperature;
}

bool UWorld_VolcanicBiomeGenerator::IsLocationSafeForVegetation(const FVector& Location) const
{
    float Temperature = GetTemperatureAtLocation(Location);
    
    // Vegetation can't survive above 50°C
    if (Temperature > 50.0f)
    {
        return false;
    }
    
    // Check if location is too close to active volcanic features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        if (Feature.bIsActive)
        {
            float Distance = FVector::Dist(Location, Feature.Location);
            if (Distance < Feature.Radius * 0.5f)
            {
                return false;
            }
        }
    }
    
    return true;
}

void UWorld_VolcanicBiomeGenerator::ApplyVolcanicWeathering(AActor* Actor, float WeatheringRate)
{
    if (!Actor)
    {
        return;
    }
    
    float Temperature = GetTemperatureAtLocation(Actor->GetActorLocation());
    
    if (Temperature > 40.0f)
    {
        // Apply weathering effects based on temperature
        float WeatheringIntensity = (Temperature - 40.0f) / 60.0f; // Normalize to 0-1
        WeatheringIntensity *= WeatheringRate;
        
        // This would typically modify the actor's materials or mesh
        UE_LOG(LogTemp, Log, TEXT("VolcanicBiomeGenerator: Applying weathering to %s (intensity: %f)"), 
               *Actor->GetName(), WeatheringIntensity);
    }
}

void UWorld_VolcanicBiomeGenerator::OptimizeVolcanicFeatures(const FVector& PlayerLocation, float CullingDistance)
{
    // Remove distant volcanic features to improve performance
    int32 InitialCount = VolcanicFeatures.Num();
    
    VolcanicFeatures.RemoveAll([&](const FWorld_VolcanicFeature& Feature)
    {
        return FVector::Dist(PlayerLocation, Feature.Location) > CullingDistance;
    });
    
    int32 RemovedCount = InitialCount - VolcanicFeatures.Num();
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Culled %d distant volcanic features"), RemovedCount);
    }
}

void UWorld_VolcanicBiomeGenerator::SetVolcanicLOD(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 4);
    
    // Adjust simulation complexity based on LOD
    switch (CurrentLODLevel)
    {
        case 0: // Highest detail
            VolcanicUpdateInterval = 0.1f;
            MaxVolcanicFeatures = 100;
            MaxLavaFlows = 50;
            break;
        case 1:
            VolcanicUpdateInterval = 0.5f;
            MaxVolcanicFeatures = 75;
            MaxLavaFlows = 30;
            break;
        case 2:
            VolcanicUpdateInterval = 1.0f;
            MaxVolcanicFeatures = 50;
            MaxLavaFlows = 20;
            break;
        case 3:
            VolcanicUpdateInterval = 2.0f;
            MaxVolcanicFeatures = 25;
            MaxLavaFlows = 10;
            break;
        case 4: // Lowest detail
            VolcanicUpdateInterval = 5.0f;
            MaxVolcanicFeatures = 10;
            MaxLavaFlows = 5;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeGenerator: Set LOD level to %d"), CurrentLODLevel);
}

FVector UWorld_VolcanicBiomeGenerator::CalculateLavaFlowDirection(const FVector& StartLocation, const FVector& EndLocation) const
{
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    
    // Add some randomness to make flows more natural
    Direction += FVector(
        FMath::RandRange(-0.2f, 0.2f),
        FMath::RandRange(-0.2f, 0.2f),
        FMath::RandRange(-0.1f, 0.0f) // Slight downward bias
    );
    
    return Direction.GetSafeNormal();
}

float UWorld_VolcanicBiomeGenerator::CalculateVolcanicInfluence(const FVector& Location, const FWorld_VolcanicFeature& Feature) const
{
    float Distance = FVector::Dist(Location, Feature.Location);
    
    if (Distance > Feature.Radius)
    {
        return 0.0f;
    }
    
    float NormalizedDistance = Distance / Feature.Radius;
    float Influence = 1.0f - (NormalizedDistance * NormalizedDistance); // Quadratic falloff
    
    return Influence * Feature.Intensity;
}

void UWorld_VolcanicBiomeGenerator::CleanupDistantVolcanicFeatures(const FVector& PlayerLocation, float MaxDistance)
{
    VolcanicFeatures.RemoveAll([&](const FWorld_VolcanicFeature& Feature)
    {
        return FVector::Dist(PlayerLocation, Feature.Location) > MaxDistance;
    });
    
    LavaFlows.RemoveAll([&](const FWorld_LavaFlow& Flow)
    {
        if (Flow.FlowPath.Num() == 0)
        {
            return true;
        }
        
        return FVector::Dist(PlayerLocation, Flow.FlowPath[0]) > MaxDistance;
    });
}