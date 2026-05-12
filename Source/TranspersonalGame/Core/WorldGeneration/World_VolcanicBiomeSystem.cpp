#include "World_VolcanicBiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"

AWorld_VolcanicBiomeSystem::AWorld_VolcanicBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create volcanic terrain mesh
    VolcanicTerrainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolcanicTerrainMesh"));
    VolcanicTerrainMesh->SetupAttachment(RootComponent);

    // Create volcanic glow light
    VolcanicGlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("VolcanicGlowLight"));
    VolcanicGlowLight->SetupAttachment(RootComponent);
    VolcanicGlowLight->SetLightColor(FLinearColor(1.0f, 0.3f, 0.1f, 1.0f));
    VolcanicGlowLight->SetIntensity(2000.0f);
    VolcanicGlowLight->SetAttenuationRadius(800.0f);
    VolcanicGlowLight->SetCastShadows(true);

    // Create volcanic audio component
    VolcanicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VolcanicAudioComponent"));
    VolcanicAudioComponent->SetupAttachment(RootComponent);
    VolcanicAudioComponent->bAutoActivate = true;

    // Create steam particle system
    SteamParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SteamParticles"));
    SteamParticles->SetupAttachment(RootComponent);
    SteamParticles->bAutoActivate = false;

    // Create lava particle system
    LavaParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LavaParticles"));
    LavaParticles->SetupAttachment(RootComponent);
    LavaParticles->bAutoActivate = false;

    // Initialize volcanic properties
    VolcanicIntensity = 0.7f;
    bIsErupting = false;
    EruptionTimer = 0.0f;
    EruptionDuration = 30.0f;

    // Initialize internal state
    CurrentVolcanicActivity = 0.5f;
    GeothermalHeatLevel = 0.6f;
    LavaTemperature = 1200.0f;
    bVolcanicSystemActive = true;

    // Initialize timers
    VolcanicUpdateInterval = 1.0f;
    LastVolcanicUpdate = 0.0f;
    GeothermalUpdateInterval = 2.0f;
    LastGeothermalUpdate = 0.0f;

    // Initialize performance settings
    CurrentLODLevel = 0;
    LastPerformanceCheck = 0.0f;
    PerformanceCheckInterval = 5.0f;

    // Initialize integration references
    BiomeAudioController = nullptr;
    WeatherController = nullptr;
    TerrainSystem = nullptr;
}

void AWorld_VolcanicBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVolcanicBiome();
    
    // Find integration systems
    IntegrateWithBiomeAudio();
    IntegrateWithWeatherSystem();
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic Biome System initialized at %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicBiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bVolcanicSystemActive)
        return;
    
    UpdateVolcanicActivity(DeltaTime);
    UpdateInternalVolcanicState(DeltaTime);
    
    // Performance optimization check
    if (GetWorld()->GetTimeSeconds() - LastPerformanceCheck > PerformanceCheckInterval)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            OptimizeVolcanicEffects(Distance);
        }
        LastPerformanceCheck = GetWorld()->GetTimeSeconds();
    }
}

void AWorld_VolcanicBiomeSystem::InitializeVolcanicBiome()
{
    ValidateVolcanicConfiguration();
    
    // Set up volcanic lighting
    UpdateVolcanicLighting();
    
    // Initialize audio
    UpdateVolcanicAudio();
    
    // Create initial volcanic features
    for (const FWorld_VolcanicFeatureData& Feature : BiomeConfig.VolcanicFeatures)
    {
        CreateVolcanicFeature(Feature);
    }
    
    // Start particle systems if active
    if (CurrentVolcanicActivity > 0.3f)
    {
        SteamParticles->Activate();
        if (CurrentVolcanicActivity > 0.7f)
        {
            LavaParticles->Activate();
        }
    }
}

void AWorld_VolcanicBiomeSystem::UpdateVolcanicActivity(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update volcanic activity
    if (CurrentTime - LastVolcanicUpdate > VolcanicUpdateInterval)
    {
        // Natural volcanic fluctuation
        float ActivityVariation = FMath::Sin(CurrentTime * 0.1f) * 0.1f;
        CurrentVolcanicActivity = FMath::Clamp(BiomeConfig.VolcanicActivity + ActivityVariation, 0.0f, 1.0f);
        
        // Update visual effects based on activity
        UpdateVolcanicLighting();
        UpdateVolcanicParticles();
        
        LastVolcanicUpdate = CurrentTime;
    }
    
    // Update geothermal activity
    if (CurrentTime - LastGeothermalUpdate > GeothermalUpdateInterval)
    {
        UpdateGeothermalActivity();
        LastGeothermalUpdate = CurrentTime;
    }
    
    // Handle eruption state
    if (bIsErupting)
    {
        EruptionTimer += DeltaTime;
        if (EruptionTimer >= EruptionDuration)
        {
            StopVolcanicEruption();
        }
    }
}

void AWorld_VolcanicBiomeSystem::TriggerVolcanicEruption()
{
    if (bIsErupting)
        return;
    
    bIsErupting = true;
    EruptionTimer = 0.0f;
    
    // Increase volcanic activity dramatically
    CurrentVolcanicActivity = 1.0f;
    VolcanicIntensity = 1.0f;
    
    // Activate all particle systems
    SteamParticles->Activate();
    LavaParticles->Activate();
    
    // Increase lighting intensity
    VolcanicGlowLight->SetIntensity(5000.0f);
    VolcanicGlowLight->SetLightColor(FLinearColor(1.0f, 0.2f, 0.0f, 1.0f));
    
    // Increase audio volume
    if (VolcanicAudioComponent && VolcanicAudioComponent->Sound)
    {
        VolcanicAudioComponent->SetVolumeMultiplier(2.0f);
    }
    
    // Notify nearby actors
    NotifyNearbyActors();
    
    UE_LOG(LogTemp, Warning, TEXT("Volcanic eruption triggered at %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicBiomeSystem::StopVolcanicEruption()
{
    if (!bIsErupting)
        return;
    
    bIsErupting = false;
    EruptionTimer = 0.0f;
    
    // Return to normal volcanic activity
    CurrentVolcanicActivity = BiomeConfig.VolcanicActivity;
    VolcanicIntensity = BiomeConfig.VolcanicActivity;
    
    // Deactivate lava particles
    LavaParticles->Deactivate();
    
    // Return lighting to normal
    UpdateVolcanicLighting();
    
    // Return audio to normal
    UpdateVolcanicAudio();
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic eruption ended at %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicBiomeSystem::CreateVolcanicFeature(const FWorld_VolcanicFeatureData& FeatureData)
{
    if (!FeatureData.bIsActive)
        return;
    
    switch (FeatureData.FeatureType)
    {
        case EWorld_VolcanicFeatureType::HotSpring:
            CreateHotSpring(FeatureData.Location, FeatureData.Intensity);
            break;
            
        case EWorld_VolcanicFeatureType::GeothermalVent:
            CreateGeothermalVent(FeatureData.Location, FeatureData.Intensity);
            break;
            
        case EWorld_VolcanicFeatureType::LavaPool:
            // Create lava pool at location
            break;
            
        case EWorld_VolcanicFeatureType::VolcanicCrater:
            // Create volcanic crater
            break;
            
        default:
            break;
    }
}

void AWorld_VolcanicBiomeSystem::RemoveVolcanicFeature(int32 FeatureIndex)
{
    if (BiomeConfig.VolcanicFeatures.IsValidIndex(FeatureIndex))
    {
        BiomeConfig.VolcanicFeatures.RemoveAt(FeatureIndex);
        UE_LOG(LogTemp, Log, TEXT("Removed volcanic feature at index %d"), FeatureIndex);
    }
}

void AWorld_VolcanicBiomeSystem::UpdateGeothermalActivity()
{
    // Update geothermal heat level based on volcanic activity
    GeothermalHeatLevel = FMath::Lerp(GeothermalHeatLevel, CurrentVolcanicActivity * BiomeConfig.GeothermalDensity, 0.1f);
    
    // Update ambient temperature
    float TargetTemperature = BiomeConfig.AmbientTemperature + (GeothermalHeatLevel * 20.0f);
    
    // Apply geothermal effects to nearby actors
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && IsValid(Actor))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < 1000.0f) // Within geothermal influence range
            {
                // Apply heat effect (could be implemented via gameplay effects)
                // For now, just log the effect
                UE_LOG(LogTemp, VeryVerbose, TEXT("Applying geothermal heat to actor %s"), *Actor->GetName());
            }
        }
    }
}

void AWorld_VolcanicBiomeSystem::CreateHotSpring(const FVector& Location, float Intensity)
{
    // Create hot spring visual effect
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Spawn steam particles at location
    FVector SteamLocation = Location + FVector(0, 0, 50);
    
    // Create hot spring audio source
    // This would typically spawn a separate audio actor
    UE_LOG(LogTemp, Log, TEXT("Created hot spring at %s with intensity %f"), *Location.ToString(), Intensity);
}

void AWorld_VolcanicBiomeSystem::CreateGeothermalVent(const FVector& Location, float SteamIntensity)
{
    // Create geothermal vent with steam emission
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Create vent visual and audio effects
    UE_LOG(LogTemp, Log, TEXT("Created geothermal vent at %s with steam intensity %f"), *Location.ToString(), SteamIntensity);
}

void AWorld_VolcanicBiomeSystem::InitializeLavaFlow(const FVector& StartLocation, const FVector& Direction)
{
    // Initialize lava flow system
    UE_LOG(LogTemp, Log, TEXT("Initializing lava flow from %s in direction %s"), *StartLocation.ToString(), *Direction.ToString());
}

void AWorld_VolcanicBiomeSystem::UpdateLavaFlow(float DeltaTime)
{
    // Update lava flow progression
    if (bIsErupting && BiomeConfig.LavaFlowRate > 0.0f)
    {
        // Progress lava flow based on flow rate and terrain
        float FlowProgress = BiomeConfig.LavaFlowRate * DeltaTime;
        // Implementation would update lava flow mesh and effects
    }
}

void AWorld_VolcanicBiomeSystem::CoolLavaFlow()
{
    // Cool and solidify lava flow
    UE_LOG(LogTemp, Log, TEXT("Cooling lava flow at %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicBiomeSystem::UpdateVolcanicLighting()
{
    if (!VolcanicGlowLight)
        return;
    
    FLinearColor GlowColor = CalculateVolcanicGlowColor();
    float LightIntensity = 2000.0f * CurrentVolcanicActivity;
    
    if (bIsErupting)
    {
        LightIntensity *= 2.5f;
        GlowColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f);
    }
    
    VolcanicGlowLight->SetLightColor(GlowColor);
    VolcanicGlowLight->SetIntensity(LightIntensity);
}

void AWorld_VolcanicBiomeSystem::UpdateVolcanicAudio()
{
    if (!VolcanicAudioComponent)
        return;
    
    float SoundVolume = CalculateVolcanicSoundVolume();
    VolcanicAudioComponent->SetVolumeMultiplier(SoundVolume);
    
    // Adjust pitch based on volcanic activity
    float PitchMultiplier = 0.8f + (CurrentVolcanicActivity * 0.4f);
    VolcanicAudioComponent->SetPitchMultiplier(PitchMultiplier);
}

void AWorld_VolcanicBiomeSystem::UpdateVolcanicParticles()
{
    if (SteamParticles)
    {
        if (CurrentVolcanicActivity > 0.3f)
        {
            SteamParticles->Activate();
        }
        else
        {
            SteamParticles->Deactivate();
        }
    }
    
    if (LavaParticles)
    {
        if (CurrentVolcanicActivity > 0.7f || bIsErupting)
        {
            LavaParticles->Activate();
        }
        else
        {
            LavaParticles->Deactivate();
        }
    }
}

void AWorld_VolcanicBiomeSystem::OptimizeVolcanicEffects(float PlayerDistance)
{
    int32 NewLODLevel = 0;
    
    if (PlayerDistance > 2000.0f)
        NewLODLevel = 2; // Distant LOD
    else if (PlayerDistance > 1000.0f)
        NewLODLevel = 1; // Medium LOD
    else
        NewLODLevel = 0; // High quality LOD
    
    if (NewLODLevel != CurrentLODLevel)
    {
        SetVolcanicLOD(NewLODLevel);
    }
}

void AWorld_VolcanicBiomeSystem::SetVolcanicLOD(int32 LODLevel)
{
    CurrentLODLevel = LODLevel;
    
    switch (LODLevel)
    {
        case 0: // High quality
            PrimaryActorTick.TickInterval = 0.1f;
            VolcanicUpdateInterval = 1.0f;
            break;
            
        case 1: // Medium quality
            PrimaryActorTick.TickInterval = 0.2f;
            VolcanicUpdateInterval = 2.0f;
            break;
            
        case 2: // Low quality
            PrimaryActorTick.TickInterval = 0.5f;
            VolcanicUpdateInterval = 5.0f;
            // Disable some particle systems
            if (SteamParticles && !bIsErupting)
                SteamParticles->Deactivate();
            break;
    }
}

void AWorld_VolcanicBiomeSystem::IntegrateWithWeatherSystem()
{
    // Find weather controller in the world
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // This would typically find the weather system and register for updates
    UE_LOG(LogTemp, Log, TEXT("Integrating volcanic biome with weather system"));
}

void AWorld_VolcanicBiomeSystem::IntegrateWithBiomeAudio()
{
    // Find biome audio controller
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // This would register volcanic sounds with the biome audio system
    UE_LOG(LogTemp, Log, TEXT("Integrating volcanic biome with audio system"));
}

void AWorld_VolcanicBiomeSystem::NotifyNearbyActors()
{
    // Update nearby actors list
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    NearbyActors.Empty();
    
    // Find actors within volcanic influence range
    FVector VolcanicLocation = GetActorLocation();
    float InfluenceRadius = 1500.0f;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor != this)
        {
            float Distance = FVector::Dist(VolcanicLocation, Actor->GetActorLocation());
            if (Distance <= InfluenceRadius)
            {
                NearbyActors.Add(Actor);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic biome affecting %d nearby actors"), NearbyActors.Num());
}

void AWorld_VolcanicBiomeSystem::UpdateInternalVolcanicState(float DeltaTime)
{
    // Update lava temperature based on activity
    float TargetTemperature = 1200.0f + (CurrentVolcanicActivity * 300.0f);
    LavaTemperature = FMath::FInterpTo(LavaTemperature, TargetTemperature, DeltaTime, 2.0f);
    
    // Check for automatic eruption triggers
    CheckVolcanicTriggers();
    
    // Process ongoing volcanic effects
    ProcessVolcanicEffects();
    
    // Clean up inactive features
    CleanupInactiveFeatures();
}

void AWorld_VolcanicBiomeSystem::CheckVolcanicTriggers()
{
    // Check for conditions that might trigger eruptions
    if (!bIsErupting && CurrentVolcanicActivity > 0.9f)
    {
        // Random chance for eruption when activity is very high
        if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // 0.1% chance per check
        {
            TriggerVolcanicEruption();
        }
    }
}

void AWorld_VolcanicBiomeSystem::ProcessVolcanicEffects()
{
    // Process ongoing volcanic effects like lava flow
    if (bIsErupting)
    {
        UpdateLavaFlow(GetWorld()->GetDeltaSeconds());
    }
}

void AWorld_VolcanicBiomeSystem::ValidateVolcanicConfiguration()
{
    // Ensure configuration values are within valid ranges
    BiomeConfig.VolcanicActivity = FMath::Clamp(BiomeConfig.VolcanicActivity, 0.0f, 1.0f);
    BiomeConfig.GeothermalDensity = FMath::Clamp(BiomeConfig.GeothermalDensity, 0.0f, 1.0f);
    BiomeConfig.LavaFlowRate = FMath::Clamp(BiomeConfig.LavaFlowRate, 0.0f, 1.0f);
    BiomeConfig.AmbientTemperature = FMath::Clamp(BiomeConfig.AmbientTemperature, 20.0f, 80.0f);
}

FLinearColor AWorld_VolcanicBiomeSystem::CalculateVolcanicGlowColor() const
{
    // Calculate glow color based on volcanic activity
    float RedIntensity = 0.8f + (CurrentVolcanicActivity * 0.2f);
    float GreenIntensity = 0.1f + (CurrentVolcanicActivity * 0.2f);
    float BlueIntensity = 0.05f;
    
    return FLinearColor(RedIntensity, GreenIntensity, BlueIntensity, 1.0f);
}

float AWorld_VolcanicBiomeSystem::CalculateVolcanicSoundVolume() const
{
    // Calculate sound volume based on activity and eruption state
    float BaseVolume = 0.3f + (CurrentVolcanicActivity * 0.4f);
    
    if (bIsErupting)
    {
        BaseVolume *= 2.0f;
    }
    
    return FMath::Clamp(BaseVolume, 0.0f, 1.0f);
}

void AWorld_VolcanicBiomeSystem::CleanupInactiveFeatures()
{
    // Remove inactive volcanic features from the configuration
    BiomeConfig.VolcanicFeatures.RemoveAll([](const FWorld_VolcanicFeatureData& Feature)
    {
        return !Feature.bIsActive;
    });
}