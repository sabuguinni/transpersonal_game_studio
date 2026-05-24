#include "World_VolcanicSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"

AWorld_VolcanicSystem::AWorld_VolcanicSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    SystemIntensity = 1.0f;
    AmbientTemperature = 35.0f; // Cretaceous period was warm
    bSystemActive = true;
    LastUpdateTime = 0.0f;
    VolcanicCycleTime = 0.0f;
    bEruptionInProgress = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VolcanicSystemRoot"));

    // Create volcanic mesh component
    VolcanoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolcanoMesh"));
    VolcanoMesh->SetupAttachment(RootComponent);

    // Create particle system components
    SmokeParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SmokeParticles"));
    SmokeParticles->SetupAttachment(VolcanoMesh);

    LavaParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LavaParticles"));
    LavaParticles->SetupAttachment(VolcanoMesh);

    // Create audio component
    VolcanicAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VolcanicAudio"));
    VolcanicAudio->SetupAttachment(RootComponent);

    InitializeVolcanicComponents();
}

void AWorld_VolcanicSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bSystemActive)
    {
        ActivateVolcanicSystem();
        CreateVolcanicLandscape();
        SpawnVolcanicEffects();
    }
}

void AWorld_VolcanicSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSystemActive)
    {
        UpdateVolcanicActivity(DeltaTime);
        UpdateVolcanicVisuals(DeltaTime);
        UpdateLavaFlows(DeltaTime);
        UpdateVolcanicAudio();
    }
}

void AWorld_VolcanicSystem::InitializeVolcanicComponents()
{
    // Set default volcanic mesh scale
    if (VolcanoMesh)
    {
        VolcanoMesh->SetWorldScale3D(FVector(3.0f, 3.0f, 2.0f));
    }

    // Configure smoke particles
    if (SmokeParticles)
    {
        SmokeParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f));
        SmokeParticles->bAutoActivate = false;
    }

    // Configure lava particles
    if (LavaParticles)
    {
        LavaParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 400.0f));
        LavaParticles->bAutoActivate = false;
    }

    // Configure volcanic audio
    if (VolcanicAudio)
    {
        VolcanicAudio->bAutoActivate = false;
        VolcanicAudio->SetVolumeMultiplier(0.7f);
    }
}

void AWorld_VolcanicSystem::CreateVolcanicVent(FVector Location, float Intensity, float Radius)
{
    FWorld_VolcanicVent NewVent;
    NewVent.Location = Location;
    NewVent.Intensity = Intensity;
    NewVent.Radius = Radius;
    NewVent.bIsActive = true;
    NewVent.Temperature = 1200.0f + (Intensity * 200.0f);

    VolcanicVents.Add(NewVent);

    UE_LOG(LogTemp, Log, TEXT("Volcanic vent created at location: %s"), *Location.ToString());
}

void AWorld_VolcanicSystem::CreateLavaFlow(const TArray<FVector>& FlowPath, float Speed, float Width)
{
    if (FlowPath.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Lava flow requires at least 2 path points"));
        return;
    }

    FWorld_LavaFlow NewFlow;
    NewFlow.FlowPath = FlowPath;
    NewFlow.FlowSpeed = Speed;
    NewFlow.Width = Width;
    NewFlow.Temperature = 1000.0f;
    NewFlow.bIsActive = true;

    LavaFlows.Add(NewFlow);

    UE_LOG(LogTemp, Log, TEXT("Lava flow created with %d path points"), FlowPath.Num());
}

void AWorld_VolcanicSystem::ActivateVolcanicSystem()
{
    bSystemActive = true;

    if (SmokeParticles)
    {
        SmokeParticles->Activate();
    }

    if (LavaParticles)
    {
        LavaParticles->Activate();
    }

    if (VolcanicAudio)
    {
        VolcanicAudio->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("Volcanic system activated"));
}

void AWorld_VolcanicSystem::DeactivateVolcanicSystem()
{
    bSystemActive = false;

    if (SmokeParticles)
    {
        SmokeParticles->Deactivate();
    }

    if (LavaParticles)
    {
        LavaParticles->Deactivate();
    }

    if (VolcanicAudio)
    {
        VolcanicAudio->Stop();
    }

    UE_LOG(LogTemp, Log, TEXT("Volcanic system deactivated"));
}

void AWorld_VolcanicSystem::UpdateVolcanicActivity(float DeltaTime)
{
    VolcanicCycleTime += DeltaTime;
    
    // Update volcanic vents
    for (FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        UpdateVolcanicVent(Vent, DeltaTime);
    }

    // Check for eruption cycles (every 5 minutes)
    if (VolcanicCycleTime > 300.0f)
    {
        if (!bEruptionInProgress && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            bEruptionInProgress = true;
            SystemIntensity = FMath::Min(SystemIntensity * 1.5f, 3.0f);
            UE_LOG(LogTemp, Log, TEXT("Volcanic eruption started"));
        }
        else if (bEruptionInProgress)
        {
            bEruptionInProgress = false;
            SystemIntensity = FMath::Max(SystemIntensity * 0.7f, 0.5f);
            UE_LOG(LogTemp, Log, TEXT("Volcanic eruption subsided"));
        }
        
        VolcanicCycleTime = 0.0f;
    }
}

void AWorld_VolcanicSystem::UpdateVolcanicVisuals(float DeltaTime)
{
    float IntensityMultiplier = CalculateVolcanicIntensity();

    // Update smoke particles
    if (SmokeParticles)
    {
        SmokeParticles->SetFloatParameter(FName("Intensity"), IntensityMultiplier);
    }

    // Update lava particles
    if (LavaParticles)
    {
        LavaParticles->SetFloatParameter(FName("Intensity"), IntensityMultiplier);
        LavaParticles->SetFloatParameter(FName("Temperature"), AmbientTemperature + (IntensityMultiplier * 50.0f));
    }
}

void AWorld_VolcanicSystem::UpdateLavaFlows(float DeltaTime)
{
    for (FWorld_LavaFlow& LavaFlow : LavaFlows)
    {
        if (LavaFlow.bIsActive)
        {
            ProcessLavaFlow(LavaFlow, DeltaTime);
        }
    }
}

void AWorld_VolcanicSystem::ProcessLavaFlow(FWorld_LavaFlow& LavaFlow, float DeltaTime)
{
    // Simulate lava flow movement along path
    float FlowProgress = LavaFlow.FlowSpeed * DeltaTime;
    
    // Update temperature based on distance from source
    if (LavaFlow.FlowPath.Num() > 1)
    {
        float TotalDistance = 0.0f;
        for (int32 i = 1; i < LavaFlow.FlowPath.Num(); i++)
        {
            TotalDistance += FVector::Dist(LavaFlow.FlowPath[i-1], LavaFlow.FlowPath[i]);
        }
        
        // Lava cools as it flows
        float CoolingFactor = FMath::Clamp(TotalDistance / 5000.0f, 0.0f, 0.8f);
        LavaFlow.Temperature = FMath::Max(LavaFlow.Temperature * (1.0f - CoolingFactor * DeltaTime * 0.1f), 200.0f);
    }
}

void AWorld_VolcanicSystem::UpdateVolcanicVent(FWorld_VolcanicVent& Vent, float DeltaTime)
{
    if (Vent.bIsActive)
    {
        // Fluctuate vent intensity
        float IntensityVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 0.2f;
        Vent.Intensity = FMath::Clamp(Vent.Intensity + IntensityVariation * DeltaTime, 0.1f, 2.0f);
        
        // Update temperature based on intensity
        Vent.Temperature = 1200.0f + (Vent.Intensity * 200.0f);
    }
}

void AWorld_VolcanicSystem::SpawnVolcanicEffects()
{
    // Create additional volcanic effects around the main volcano
    for (int32 i = 0; i < 5; i++)
    {
        FVector VentLocation = GetActorLocation() + FVector(
            FMath::RandRange(-2000.0f, 2000.0f),
            FMath::RandRange(-2000.0f, 2000.0f),
            FMath::RandRange(-200.0f, 200.0f)
        );
        
        CreateVolcanicVent(VentLocation, FMath::RandRange(0.5f, 1.5f), FMath::RandRange(300.0f, 800.0f));
    }

    // Create lava flows
    for (int32 i = 0; i < 3; i++)
    {
        TArray<FVector> FlowPath;
        FVector StartLocation = GetActorLocation() + FVector(0.0f, 0.0f, 400.0f);
        FlowPath.Add(StartLocation);
        
        // Create a winding lava flow path
        FVector CurrentLocation = StartLocation;
        for (int32 j = 0; j < 8; j++)
        {
            FVector NextLocation = CurrentLocation + FVector(
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-100.0f, -50.0f) // Flow downhill
            );
            FlowPath.Add(NextLocation);
            CurrentLocation = NextLocation;
        }
        
        CreateLavaFlow(FlowPath, FMath::RandRange(30.0f, 70.0f), FMath::RandRange(150.0f, 250.0f));
    }
}

float AWorld_VolcanicSystem::GetTemperatureAtLocation(FVector Location) const
{
    float MaxTemperature = AmbientTemperature;
    
    // Check temperature influence from volcanic vents
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        if (Vent.bIsActive)
        {
            float Distance = FVector::Dist(Location, Vent.Location);
            if (Distance < Vent.Radius)
            {
                float TemperatureInfluence = Vent.Temperature * (1.0f - (Distance / Vent.Radius));
                MaxTemperature = FMath::Max(MaxTemperature, AmbientTemperature + TemperatureInfluence);
            }
        }
    }
    
    // Check temperature influence from lava flows
    for (const FWorld_LavaFlow& LavaFlow : LavaFlows)
    {
        if (LavaFlow.bIsActive)
        {
            for (const FVector& FlowPoint : LavaFlow.FlowPath)
            {
                float Distance = FVector::Dist(Location, FlowPoint);
                if (Distance < LavaFlow.Width)
                {
                    float TemperatureInfluence = LavaFlow.Temperature * (1.0f - (Distance / LavaFlow.Width));
                    MaxTemperature = FMath::Max(MaxTemperature, AmbientTemperature + TemperatureInfluence * 0.5f);
                }
            }
        }
    }
    
    return MaxTemperature;
}

bool AWorld_VolcanicSystem::IsLocationInVolcanicZone(FVector Location) const
{
    // Check if location is within any volcanic vent radius
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        if (Vent.bIsActive && FVector::Dist(Location, Vent.Location) < Vent.Radius)
        {
            return true;
        }
    }
    
    // Check if location is near any lava flow
    for (const FWorld_LavaFlow& LavaFlow : LavaFlows)
    {
        if (LavaFlow.bIsActive)
        {
            for (const FVector& FlowPoint : LavaFlow.FlowPath)
            {
                if (FVector::Dist(Location, FlowPoint) < LavaFlow.Width * 1.5f)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void AWorld_VolcanicSystem::ModifyTerrainForVolcanic()
{
    // This would modify the landscape heightmap to create volcanic terrain
    // Implementation would depend on the specific landscape system being used
    UE_LOG(LogTemp, Log, TEXT("Modifying terrain for volcanic activity"));
}

void AWorld_VolcanicSystem::CreateVolcanicLandscape()
{
    // Create volcanic landscape features
    SpawnVolcanicRocks();
    CreateGeothermalVents();
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic landscape created"));
}

void AWorld_VolcanicSystem::SpawnVolcanicRocks()
{
    // Spawn volcanic rock formations around the volcano
    for (int32 i = 0; i < 15; i++)
    {
        FVector RockLocation = GetActorLocation() + FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            0.0f
        );
        
        // This would spawn actual rock actors in a real implementation
        UE_LOG(LogTemp, Log, TEXT("Volcanic rock spawned at: %s"), *RockLocation.ToString());
    }
}

void AWorld_VolcanicSystem::CreateGeothermalVents()
{
    // Create smaller geothermal vents around the main volcanic area
    for (int32 i = 0; i < 8; i++)
    {
        FVector VentLocation = GetActorLocation() + FVector(
            FMath::RandRange(-1500.0f, 1500.0f),
            FMath::RandRange(-1500.0f, 1500.0f),
            FMath::RandRange(-100.0f, 100.0f)
        );
        
        CreateVolcanicVent(VentLocation, FMath::RandRange(0.2f, 0.8f), FMath::RandRange(200.0f, 500.0f));
    }
}

void AWorld_VolcanicSystem::UpdateVolcanicAudio()
{
    if (VolcanicAudio && bSystemActive)
    {
        float IntensityMultiplier = CalculateVolcanicIntensity();
        VolcanicAudio->SetVolumeMultiplier(0.7f * IntensityMultiplier);
        VolcanicAudio->SetPitchMultiplier(0.8f + (IntensityMultiplier * 0.4f));
    }
}

float AWorld_VolcanicSystem::CalculateVolcanicIntensity() const
{
    float TotalIntensity = SystemIntensity;
    
    // Add intensity from active vents
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        if (Vent.bIsActive)
        {
            TotalIntensity += Vent.Intensity * 0.1f;
        }
    }
    
    // Add intensity from active lava flows
    for (const FWorld_LavaFlow& LavaFlow : LavaFlows)
    {
        if (LavaFlow.bIsActive)
        {
            TotalIntensity += (LavaFlow.Temperature / 1000.0f) * 0.05f;
        }
    }
    
    return FMath::Clamp(TotalIntensity, 0.1f, 3.0f);
}

FVector AWorld_VolcanicSystem::CalculateLavaFlowDirection(const FWorld_LavaFlow& LavaFlow, int32 PathIndex) const
{
    if (PathIndex >= LavaFlow.FlowPath.Num() - 1)
    {
        return FVector::ZeroVector;
    }
    
    FVector Direction = LavaFlow.FlowPath[PathIndex + 1] - LavaFlow.FlowPath[PathIndex];
    return Direction.GetSafeNormal();
}