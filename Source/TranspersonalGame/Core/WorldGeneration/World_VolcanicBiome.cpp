#include "World_VolcanicBiome.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"

UWorld_VolcanicBiome::UWorld_VolcanicBiome()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    GlobalVolcanicActivity = 0.3f;
    EruptionProbability = 0.001f; // Very low chance per tick
    AmbientTemperature = 45.0f; // Celsius
    LastEruptionTime = 0.0f;

    // Initialize arrays
    VolcanicFeatures.Empty();
    LavaFlows.Empty();
    ActiveEffects.Empty();
    ActiveAudioSources.Empty();
}

void UWorld_VolcanicBiome::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate initial volcanic features if none exist
    if (VolcanicFeatures.Num() == 0)
    {
        GenerateVolcanicFeatures(15);
    }

    // Start ambient volcanic sounds
    if (VolcanicAmbientSound && GetWorld())
    {
        UAudioComponent* AmbientAudio = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), 
            VolcanicAmbientSound, 
            GetOwner()->GetActorLocation(),
            FRotator::ZeroRotator,
            0.5f, // Volume
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation
            nullptr, // Concurrency
            true // Auto destroy
        );
        
        if (AmbientAudio)
        {
            ActiveAudioSources.Add(AmbientAudio);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Volcanic Biome initialized with %d features"), VolcanicFeatures.Num());
}

void UWorld_VolcanicBiome::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update lava flows
    UpdateLavaFlows(DeltaTime);

    // Update volcanic effects
    UpdateVolcanicEffects();

    // Check for random eruptions
    if (FMath::RandRange(0.0f, 1.0f) < EruptionProbability * GlobalVolcanicActivity)
    {
        if (VolcanicFeatures.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, VolcanicFeatures.Num() - 1);
            TriggerEruption(VolcanicFeatures[RandomIndex].Location, GlobalVolcanicActivity);
        }
    }
}

void UWorld_VolcanicBiome::GenerateVolcanicFeatures(int32 NumFeatures)
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    VolcanicFeatures.Empty();
    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (int32 i = 0; i < NumFeatures; i++)
    {
        FWorld_VolcanicFeature NewFeature;
        
        // Random location around owner
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(500.0f, 5000.0f);
        
        NewFeature.Location = OwnerLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(-200.0f, 500.0f)
        );

        // Random properties
        NewFeature.Intensity = FMath::RandRange(0.2f, 1.0f);
        NewFeature.Type = static_cast<EWorld_VolcanicType>(FMath::RandRange(0, 3));
        NewFeature.Radius = FMath::RandRange(200.0f, 800.0f);
        NewFeature.bIsActive = FMath::RandBool();

        VolcanicFeatures.Add(NewFeature);

        // Spawn visual representation
        if (NewFeature.bIsActive)
        {
            SpawnVolcanicRocks(NewFeature.Location, NewFeature.Radius, 10);
            
            if (NewFeature.Type == EWorld_VolcanicType::LavaVent)
            {
                CreateLavaFlow(NewFeature.Location, 
                    NewFeature.Location + FVector(
                        FMath::RandRange(-1000.0f, 1000.0f),
                        FMath::RandRange(-1000.0f, 1000.0f),
                        FMath::RandRange(-500.0f, 0.0f)
                    )
                );
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated %d volcanic features"), NumFeatures);
}

void UWorld_VolcanicBiome::CreateLavaFlow(FVector StartLocation, FVector EndLocation)
{
    FWorld_LavaFlow NewFlow;
    
    // Create path from start to end with some variation
    int32 PathPoints = FMath::RandRange(5, 15);
    FVector CurrentPos = StartLocation;
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    float TotalDistance = FVector::Dist(StartLocation, EndLocation);
    float StepDistance = TotalDistance / PathPoints;

    NewFlow.FlowPath.Add(CurrentPos);

    for (int32 i = 1; i < PathPoints; i++)
    {
        // Add some randomness to the path
        FVector Offset = FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-50.0f, 0.0f)
        );
        
        CurrentPos += Direction * StepDistance + Offset;
        NewFlow.FlowPath.Add(CurrentPos);
    }

    NewFlow.FlowPath.Add(EndLocation);
    NewFlow.FlowSpeed = FMath::RandRange(1.0f, 5.0f);
    NewFlow.Temperature = FMath::RandRange(1000.0f, 1300.0f);
    NewFlow.Width = FMath::RandRange(50.0f, 200.0f);
    NewFlow.bIsCooling = false;

    LavaFlows.Add(NewFlow);

    UE_LOG(LogTemp, Warning, TEXT("Created lava flow with %d path points"), NewFlow.FlowPath.Num());
}

void UWorld_VolcanicBiome::SpawnVolcanicRocks(FVector Center, float Radius, int32 Count)
{
    if (!GetWorld())
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Random position within radius
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(0.0f, Radius);
        
        FVector SpawnLocation = Center + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        // Spawn a basic rock actor (using cube for now)
        AStaticMeshActor* RockActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnLocation,
            FRotator(FMath::RandRange(0.0f, 360.0f), FMath::RandRange(0.0f, 360.0f), FMath::RandRange(0.0f, 360.0f))
        );

        if (RockActor)
        {
            RockActor->SetActorLabel(FString::Printf(TEXT("VolcanicRock_%d"), i));
            
            // Scale randomly
            float Scale = FMath::RandRange(0.5f, 3.0f);
            RockActor->SetActorScale3D(FVector(Scale));
        }
    }
}

void UWorld_VolcanicBiome::CreateObsidianFormations(FVector Location, float Size)
{
    if (!GetWorld())
    {
        return;
    }

    // Create several obsidian spikes
    int32 SpikeCount = FMath::RandRange(3, 8);
    
    for (int32 i = 0; i < SpikeCount; i++)
    {
        float Angle = (360.0f / SpikeCount) * i + FMath::RandRange(-30.0f, 30.0f);
        float Distance = FMath::RandRange(Size * 0.2f, Size * 0.8f);
        
        FVector SpikeLocation = Location + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        AStaticMeshActor* ObsidianSpike = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpikeLocation,
            FRotator(0.0f, Angle, 0.0f)
        );

        if (ObsidianSpike)
        {
            ObsidianSpike->SetActorLabel(FString::Printf(TEXT("ObsidianSpike_%d"), i));
            
            // Make it tall and thin
            float Height = FMath::RandRange(2.0f, 8.0f);
            float Width = FMath::RandRange(0.3f, 1.0f);
            ObsidianSpike->SetActorScale3D(FVector(Width, Width, Height));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Created obsidian formation with %d spikes at %s"), SpikeCount, *Location.ToString());
}

void UWorld_VolcanicBiome::UpdateVolcanicActivity(float NewActivity)
{
    GlobalVolcanicActivity = FMath::Clamp(NewActivity, 0.0f, 1.0f);
    
    // Update eruption probability based on activity
    EruptionProbability = GlobalVolcanicActivity * 0.002f;
    
    // Update ambient temperature
    AmbientTemperature = 25.0f + (GlobalVolcanicActivity * 50.0f);

    UE_LOG(LogTemp, Warning, TEXT("Volcanic activity updated to %f"), GlobalVolcanicActivity);
}

void UWorld_VolcanicBiome::TriggerEruption(FVector Location, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }

    Intensity = FMath::Clamp(Intensity, 0.1f, 1.0f);
    LastEruptionTime = GetWorld()->GetTimeSeconds();

    // Play eruption sound
    if (EruptionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), EruptionSound, Location, Intensity);
    }

    // Create new lava flows from eruption
    int32 FlowCount = FMath::RoundToInt(Intensity * 5.0f);
    for (int32 i = 0; i < FlowCount; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(500.0f, 2000.0f) * Intensity;
        
        FVector EndLocation = Location + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(-300.0f, -100.0f)
        );

        CreateLavaFlow(Location, EndLocation);
    }

    // Spawn volcanic rocks around eruption
    SpawnVolcanicRocks(Location, 1000.0f * Intensity, FMath::RoundToInt(20 * Intensity));

    UE_LOG(LogTemp, Warning, TEXT("Eruption triggered at %s with intensity %f"), *Location.ToString(), Intensity);
}

float UWorld_VolcanicBiome::GetTemperatureAtLocation(FVector Location) const
{
    float Temperature = AmbientTemperature;

    // Check distance to volcanic features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        if (Feature.bIsActive)
        {
            float Distance = FVector::Dist(Location, Feature.Location);
            if (Distance < Feature.Radius)
            {
                float HeatContribution = (1.0f - (Distance / Feature.Radius)) * Feature.Intensity * 100.0f;
                Temperature += HeatContribution;
            }
        }
    }

    // Check distance to lava flows
    for (const FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (!Flow.bIsCooling)
        {
            for (const FVector& FlowPoint : Flow.FlowPath)
            {
                float Distance = FVector::Dist(Location, FlowPoint);
                if (Distance < Flow.Width)
                {
                    float HeatContribution = (1.0f - (Distance / Flow.Width)) * (Flow.Temperature / 10.0f);
                    Temperature += HeatContribution;
                }
            }
        }
    }

    return Temperature;
}

bool UWorld_VolcanicBiome::IsLocationInLavaZone(FVector Location) const
{
    // Check if location is near any active lava flows
    for (const FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (!Flow.bIsCooling)
        {
            for (const FVector& FlowPoint : Flow.FlowPath)
            {
                float Distance = FVector::Dist(Location, FlowPoint);
                if (Distance < Flow.Width * 0.5f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void UWorld_VolcanicBiome::ClearAllVolcanicFeatures()
{
    VolcanicFeatures.Empty();
    LavaFlows.Empty();
    
    // Clean up active effects
    for (UParticleSystemComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Cleared all volcanic features"));
}

FVector UWorld_VolcanicBiome::GetNearestVolcanicFeature(FVector Location) const
{
    if (VolcanicFeatures.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector NearestLocation = VolcanicFeatures[0].Location;
    float MinDistance = FVector::Dist(Location, NearestLocation);

    for (int32 i = 1; i < VolcanicFeatures.Num(); i++)
    {
        float Distance = FVector::Dist(Location, VolcanicFeatures[i].Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestLocation = VolcanicFeatures[i].Location;
        }
    }

    return NearestLocation;
}

TArray<FVector> UWorld_VolcanicBiome::GetSafeZones(float MinDistance) const
{
    TArray<FVector> SafeZones;
    
    if (!GetOwner())
    {
        return SafeZones;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Generate potential safe zone locations in a grid
    for (int32 X = -5; X <= 5; X++)
    {
        for (int32 Y = -5; Y <= 5; Y++)
        {
            FVector TestLocation = OwnerLocation + FVector(X * 1000.0f, Y * 1000.0f, 0.0f);
            bool bIsSafe = true;

            // Check distance to all volcanic features
            for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
            {
                if (Feature.bIsActive)
                {
                    float Distance = FVector::Dist(TestLocation, Feature.Location);
                    if (Distance < MinDistance)
                    {
                        bIsSafe = false;
                        break;
                    }
                }
            }

            // Check distance to lava flows
            if (bIsSafe)
            {
                for (const FWorld_LavaFlow& Flow : LavaFlows)
                {
                    if (!Flow.bIsCooling)
                    {
                        for (const FVector& FlowPoint : Flow.FlowPath)
                        {
                            float Distance = FVector::Dist(TestLocation, FlowPoint);
                            if (Distance < MinDistance)
                            {
                                bIsSafe = false;
                                break;
                            }
                        }
                        if (!bIsSafe) break;
                    }
                }
            }

            if (bIsSafe)
            {
                SafeZones.Add(TestLocation);
            }
        }
    }

    return SafeZones;
}

void UWorld_VolcanicBiome::RegenerateVolcanicBiome()
{
    ClearAllVolcanicFeatures();
    GenerateVolcanicFeatures(FMath::RandRange(10, 20));
    
    // Create some obsidian formations
    for (int32 i = 0; i < 5; i++)
    {
        if (VolcanicFeatures.Num() > i)
        {
            CreateObsidianFormations(VolcanicFeatures[i].Location, 400.0f);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Volcanic biome regenerated"));
}

void UWorld_VolcanicBiome::PreviewVolcanicEffects()
{
    if (VolcanicFeatures.Num() > 0)
    {
        TriggerEruption(VolcanicFeatures[0].Location, 0.8f);
    }
}

void UWorld_VolcanicBiome::UpdateLavaFlows(float DeltaTime)
{
    for (FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (!Flow.bIsCooling)
        {
            // Cool down lava over time
            Flow.Temperature -= DeltaTime * 10.0f;
            
            // Mark as cooling if temperature drops below threshold
            if (Flow.Temperature < 800.0f)
            {
                Flow.bIsCooling = true;
            }
        }
    }
}

void UWorld_VolcanicBiome::UpdateVolcanicEffects()
{
    // Update particle effects and audio based on volcanic activity
    // This would typically involve updating particle system parameters
    // and audio component volumes based on GlobalVolcanicActivity
}

void UWorld_VolcanicBiome::SpawnVolcanicVegetation()
{
    // Spawn heat-resistant plants like cycads and primitive conifers
    // around volcanic features at safe distances
}

FVector UWorld_VolcanicBiome::CalculateLavaFlowDirection(FVector CurrentPos) const
{
    // Calculate flow direction based on terrain slope
    // For now, just flow generally downward with some randomness
    return FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-2.0f, -0.5f)
    ).GetSafeNormal();
}

void UWorld_VolcanicBiome::CreateVolcanicTerrain(FVector Center, float Radius)
{
    // Create heightmap modifications for volcanic terrain
    // This would typically involve modifying landscape heightmaps
    // or spawning terrain actors to create volcanic features
}