#include "World_VolcanicEcosystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"

UWorld_VolcanicEcosystemManager::UWorld_VolcanicEcosystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize volcanic parameters
    BaseVolcanicActivity = 0.5f;
    VolcanicActivityVariation = 0.3f;
    MaxVolcanicTemperature = 1500.0f;
    VolcanicInfluenceRadius = 2000.0f;
    bVolcanicSystemActive = true;

    // Initialize timers
    VolcanicActivityTimer = 0.0f;
    AudioUpdateTimer = 0.0f;
    ParticleUpdateTimer = 0.0f;

    // Initialize sound cues to nullptr
    EruptionSoundCue = nullptr;
    RumblingSoundCue = nullptr;
    SteamHissingSoundCue = nullptr;

    // Initialize materials to nullptr
    LavaMaterial = nullptr;
    VolcanicRockMaterial = nullptr;
    GeothermalSteamMaterial = nullptr;
}

void UWorld_VolcanicEcosystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Initializing volcanic ecosystem"));
    InitializeVolcanicEcosystem();
}

void UWorld_VolcanicEcosystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bVolcanicSystemActive)
    {
        return;
    }

    UpdateVolcanicActivity(DeltaTime);
    UpdateLavaFlows(DeltaTime);
    UpdateVolcanicAudio(DeltaTime);
    UpdateVolcanicParticles(DeltaTime);
    UpdateEcosystemTemperature(DeltaTime);
}

void UWorld_VolcanicEcosystemManager::InitializeVolcanicEcosystem()
{
    UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Setting up volcanic ecosystem"));

    // Create default volcanic vents
    CreateVolcanicVent(FVector(2000, 2000, 400), 1200.0f, 0.8f);
    CreateVolcanicVent(FVector(2500, 1500, 350), 900.0f, 0.6f);
    CreateVolcanicVent(FVector(1800, 2200, 380), 1000.0f, 0.7f);

    // Create default lava flows
    TArray<FVector> MainLavaFlow = {
        FVector(2000, 2000, 400),
        FVector(1950, 1950, 380),
        FVector(1900, 1900, 360),
        FVector(1850, 1850, 340)
    };
    CreateLavaFlow(MainLavaFlow, 2.5f);

    TArray<FVector> SecondaryLavaFlow = {
        FVector(2500, 1500, 350),
        FVector(2450, 1450, 330),
        FVector(2400, 1400, 310)
    };
    CreateLavaFlow(SecondaryLavaFlow, 1.8f);

    // Create volcanic audio zones
    CreateVolcanicAudioZone(FVector(2000, 2000, 300), 1200.0f);
    CreateVolcanicAudioZone(FVector(2200, 1800, 300), 800.0f);

    // Spawn volcanic vegetation
    SpawnVolcanicVegetation();

    // Create volcanic lighting
    CreateVolcanicLighting();

    UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Volcanic ecosystem initialized with %d vents, %d lava flows, %d audio zones"), 
           VolcanicVents.Num(), LavaFlows.Num(), AudioZones.Num());
}

void UWorld_VolcanicEcosystemManager::UpdateVolcanicActivity(float DeltaTime)
{
    VolcanicActivityTimer += DeltaTime;

    // Update each volcanic vent
    for (FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        UpdateVolcanicVentActivity(Vent, DeltaTime);
    }

    // Periodic activity fluctuations
    if (VolcanicActivityTimer >= 5.0f)
    {
        for (FWorld_VolcanicVent& Vent : VolcanicVents)
        {
            float ActivityChange = FMath::RandRange(-VolcanicActivityVariation, VolcanicActivityVariation);
            Vent.Activity = FMath::Clamp(Vent.Activity + ActivityChange, 0.1f, 1.0f);
            
            // Temperature follows activity
            Vent.Temperature = FMath::Lerp(800.0f, MaxVolcanicTemperature, Vent.Activity);
        }
        VolcanicActivityTimer = 0.0f;
    }
}

void UWorld_VolcanicEcosystemManager::CreateVolcanicVent(const FVector& Location, float Temperature, float Activity)
{
    FWorld_VolcanicVent NewVent;
    NewVent.Location = Location;
    NewVent.Temperature = Temperature;
    NewVent.Activity = Activity;
    NewVent.GasEmission = Activity * 0.5f;
    NewVent.bIsActive = true;

    VolcanicVents.Add(NewVent);

    // Create geothermal effects at this location
    CreateGeothermalEffects(Location);

    UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Created volcanic vent at %s with temperature %.1f°C"), 
           *Location.ToString(), Temperature);
}

void UWorld_VolcanicEcosystemManager::CreateLavaFlow(const TArray<FVector>& FlowPath, float FlowSpeed)
{
    if (FlowPath.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("VolcanicEcosystemManager: Lava flow needs at least 2 points"));
        return;
    }

    FWorld_LavaFlow NewFlow;
    NewFlow.FlowPath = FlowPath;
    NewFlow.FlowSpeed = FlowSpeed;
    NewFlow.Temperature = 1200.0f;
    NewFlow.Width = 5.0f;
    NewFlow.bIsFlowing = true;

    LavaFlows.Add(NewFlow);

    UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Created lava flow with %d points, speed %.1f m/s"), 
           FlowPath.Num(), FlowSpeed);
}

void UWorld_VolcanicEcosystemManager::UpdateLavaFlows(float DeltaTime)
{
    for (FWorld_LavaFlow& Flow : LavaFlows)
    {
        if (Flow.bIsFlowing)
        {
            UpdateLavaFlowMovement(Flow, DeltaTime);
        }
    }
}

void UWorld_VolcanicEcosystemManager::CreateVolcanicAudioZone(const FVector& Center, float Radius)
{
    FWorld_VolcanicAudioZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.RumblingIntensity = 0.7f;
    NewZone.HissingSteamVolume = 0.5f;
    NewZone.bPlayEruptionSounds = false;

    AudioZones.Add(NewZone);

    // Create audio component for this zone
    if (AActor* Owner = GetOwner())
    {
        UAudioComponent* RumblingAudio = Owner->CreateDefaultSubobject<UAudioComponent>(
            *FString::Printf(TEXT("VolcanicRumbling_%d"), AudioZones.Num()));
        
        if (RumblingAudio)
        {
            RumblingAudio->SetWorldLocation(Center);
            RumblingAudio->SetVolumeMultiplier(NewZone.RumblingIntensity);
            RumblingAudioComponents.Add(RumblingAudio);
        }

        UAudioComponent* SteamAudio = Owner->CreateDefaultSubobject<UAudioComponent>(
            *FString::Printf(TEXT("VolcanicSteam_%d"), AudioZones.Num()));
        
        if (SteamAudio)
        {
            SteamAudio->SetWorldLocation(Center);
            SteamAudio->SetVolumeMultiplier(NewZone.HissingSteamVolume);
            SteamAudioComponents.Add(SteamAudio);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Created volcanic audio zone at %s with radius %.1f"), 
           *Center.ToString(), Radius);
}

void UWorld_VolcanicEcosystemManager::UpdateVolcanicAudio(float DeltaTime)
{
    AudioUpdateTimer += DeltaTime;

    if (AudioUpdateTimer >= 0.5f) // Update audio every 0.5 seconds
    {
        for (int32 i = 0; i < AudioZones.Num(); ++i)
        {
            UpdateAudioZoneIntensity(AudioZones[i], DeltaTime);

            // Update audio component volumes
            if (RumblingAudioComponents.IsValidIndex(i) && RumblingAudioComponents[i])
            {
                RumblingAudioComponents[i]->SetVolumeMultiplier(AudioZones[i].RumblingIntensity);
            }

            if (SteamAudioComponents.IsValidIndex(i) && SteamAudioComponents[i])
            {
                SteamAudioComponents[i]->SetVolumeMultiplier(AudioZones[i].HissingSteamVolume);
            }
        }
        AudioUpdateTimer = 0.0f;
    }
}

void UWorld_VolcanicEcosystemManager::PlayEruptionSound(const FVector& Location)
{
    if (EruptionSoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), EruptionSoundCue, Location, 1.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Playing eruption sound at %s"), *Location.ToString());
    }
}

void UWorld_VolcanicEcosystemManager::UpdateRumblingAudio(float Intensity)
{
    for (UAudioComponent* AudioComp : RumblingAudioComponents)
    {
        if (AudioComp)
        {
            AudioComp->SetVolumeMultiplier(Intensity);
        }
    }
}

void UWorld_VolcanicEcosystemManager::CreateGeothermalEffects(const FVector& Location)
{
    if (AActor* Owner = GetOwner())
    {
        // Create steam particle effect
        UParticleSystemComponent* SteamParticle = Owner->CreateDefaultSubobject<UParticleSystemComponent>(
            *FString::Printf(TEXT("GeothermalSteam_%d"), SteamParticles.Num()));
        
        if (SteamParticle)
        {
            SteamParticle->SetWorldLocation(Location + FVector(0, 0, 50));
            SteamParticles.Add(SteamParticle);
        }

        UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Created geothermal effects at %s"), *Location.ToString());
    }
}

void UWorld_VolcanicEcosystemManager::UpdateVolcanicParticles(float DeltaTime)
{
    ParticleUpdateTimer += DeltaTime;

    if (ParticleUpdateTimer >= 1.0f) // Update particles every second
    {
        // Update steam particle intensity based on volcanic activity
        for (int32 i = 0; i < SteamParticles.Num() && i < VolcanicVents.Num(); ++i)
        {
            if (SteamParticles[i] && VolcanicVents.IsValidIndex(i))
            {
                float Intensity = VolcanicVents[i].Activity * VolcanicVents[i].GasEmission;
                // Particle intensity would be set here if we had access to the particle parameters
            }
        }

        ParticleUpdateTimer = 0.0f;
    }
}

void UWorld_VolcanicEcosystemManager::CreateVolcanicLighting()
{
    // Create warm, orange lighting for volcanic areas
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        // Lighting would be created here using UE5's lighting system
        UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Creating volcanic lighting at %s"), *Vent.Location.ToString());
    }
}

void UWorld_VolcanicEcosystemManager::SpawnVolcanicVegetation()
{
    // Spawn heat-resistant vegetation around volcanic areas
    TArray<FVector> VegetationLocations = {
        FVector(1700, 1700, 280),
        FVector(1750, 1650, 285),
        FVector(1650, 1750, 275),
        FVector(2300, 1300, 300),
        FVector(2350, 1250, 305)
    };

    for (const FVector& Location : VegetationLocations)
    {
        // Vegetation spawning would be handled here
        UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Spawning volcanic vegetation at %s"), *Location.ToString());
    }
}

void UWorld_VolcanicEcosystemManager::UpdateEcosystemTemperature(float DeltaTime)
{
    // Update temperature effects on the ecosystem
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        if (Vent.bIsActive)
        {
            // Temperature effects would be calculated and applied here
            float TemperatureRadius = VolcanicInfluenceRadius * (Vent.Activity * 0.5f + 0.5f);
            // Apply temperature effects to nearby vegetation and wildlife
        }
    }
}

bool UWorld_VolcanicEcosystemManager::IsLocationInVolcanicZone(const FVector& Location) const
{
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        float Distance = FVector::Dist(Location, Vent.Location);
        if (Distance <= VolcanicInfluenceRadius)
        {
            return true;
        }
    }
    return false;
}

float UWorld_VolcanicEcosystemManager::GetVolcanicTemperatureAtLocation(const FVector& Location) const
{
    float MaxTemperature = 20.0f; // Base ambient temperature
    
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        float Distance = FVector::Dist(Location, Vent.Location);
        if (Distance <= VolcanicInfluenceRadius)
        {
            float TemperatureContribution = Vent.Temperature * (1.0f - (Distance / VolcanicInfluenceRadius));
            MaxTemperature = FMath::Max(MaxTemperature, TemperatureContribution);
        }
    }
    
    return MaxTemperature;
}

// Private helper functions
void UWorld_VolcanicEcosystemManager::UpdateVolcanicVentActivity(FWorld_VolcanicVent& Vent, float DeltaTime)
{
    if (!Vent.bIsActive)
    {
        return;
    }

    // Simulate natural activity fluctuations
    float ActivityNoise = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 0.1f;
    Vent.Activity = FMath::Clamp(BaseVolcanicActivity + ActivityNoise, 0.1f, 1.0f);
    
    // Gas emission follows activity
    Vent.GasEmission = Vent.Activity * 0.6f;
    
    // Temperature fluctuates with activity
    float TargetTemperature = FMath::Lerp(800.0f, MaxVolcanicTemperature, Vent.Activity);
    Vent.Temperature = FMath::FInterpTo(Vent.Temperature, TargetTemperature, DeltaTime, 0.5f);
}

void UWorld_VolcanicEcosystemManager::UpdateLavaFlowMovement(FWorld_LavaFlow& Flow, float DeltaTime)
{
    // Simulate lava flow movement and cooling
    float CoolingRate = 50.0f; // Degrees per second
    Flow.Temperature = FMath::Max(Flow.Temperature - (CoolingRate * DeltaTime), 600.0f);
    
    // If temperature drops too low, stop flowing
    if (Flow.Temperature < 800.0f)
    {
        Flow.bIsFlowing = false;
        Flow.FlowSpeed *= 0.9f; // Gradual slowdown
    }
}

void UWorld_VolcanicEcosystemManager::UpdateAudioZoneIntensity(FWorld_VolcanicAudioZone& Zone, float DeltaTime)
{
    // Find nearest volcanic vent to determine audio intensity
    float NearestVentDistance = CalculateDistanceToNearestVent(Zone.Center);
    
    if (NearestVentDistance < Zone.Radius)
    {
        float DistanceFactor = 1.0f - (NearestVentDistance / Zone.Radius);
        Zone.RumblingIntensity = FMath::Lerp(0.2f, 1.0f, DistanceFactor);
        Zone.HissingSteamVolume = FMath::Lerp(0.1f, 0.8f, DistanceFactor);
    }
    else
    {
        Zone.RumblingIntensity = FMath::FInterpTo(Zone.RumblingIntensity, 0.1f, DeltaTime, 1.0f);
        Zone.HissingSteamVolume = FMath::FInterpTo(Zone.HissingSteamVolume, 0.05f, DeltaTime, 1.0f);
    }
}

float UWorld_VolcanicEcosystemManager::CalculateDistanceToNearestVent(const FVector& Location) const
{
    float MinDistance = MAX_FLT;
    
    for (const FWorld_VolcanicVent& Vent : VolcanicVents)
    {
        if (Vent.bIsActive)
        {
            float Distance = FVector::Dist(Location, Vent.Location);
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }
    
    return MinDistance;
}

void UWorld_VolcanicEcosystemManager::CreateVolcanicParticleEffect(const FVector& Location, float Intensity)
{
    if (AActor* Owner = GetOwner())
    {
        UParticleSystemComponent* NewParticle = Owner->CreateDefaultSubobject<UParticleSystemComponent>(
            *FString::Printf(TEXT("VolcanicParticle_%d"), LavaParticles.Num()));
        
        if (NewParticle)
        {
            NewParticle->SetWorldLocation(Location);
            LavaParticles.Add(NewParticle);
            
            UE_LOG(LogTemp, Log, TEXT("VolcanicEcosystemManager: Created volcanic particle effect at %s with intensity %.2f"), 
                   *Location.ToString(), Intensity);
        }
    }
}