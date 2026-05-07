#include "Audio_FootstepParticleSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFootstepParticles, Log, All);

AAudio_FootstepParticleSystem::AAudio_FootstepParticleSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create particle system component
    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);
    ParticleSystemComponent->bAutoActivate = false;
    
    // Create audio component for footstep sounds
    FootstepAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudioComponent"));
    FootstepAudioComponent->SetupAttachment(RootComponent);
    FootstepAudioComponent->bAutoActivate = false;
    
    // Default settings
    DustIntensityMultiplier = 1.0f;
    FootstepVolumeMultiplier = 1.0f;
    MinFootstepInterval = 0.3f;
    MaxFootstepInterval = 0.8f;
    ParticleLifetime = 2.0f;
    bIsActive = false;
    LastFootstepTime = 0.0f;
    
    // Initialize surface type mappings
    InitializeSurfaceTypeMappings();
}

void AAudio_FootstepParticleSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogFootstepParticles, Log, TEXT("Footstep Particle System initialized"));
    
    // Start monitoring for movement if auto-activate is enabled
    if (bAutoActivate)
    {
        SetActive(true);
    }
}

void AAudio_FootstepParticleSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsActive)
    {
        MonitorMovement(DeltaTime);
    }
}

void AAudio_FootstepParticleSystem::SetActive(bool bNewActive)
{
    bIsActive = bNewActive;
    
    if (bIsActive)
    {
        UE_LOG(LogFootstepParticles, Log, TEXT("Footstep system activated"));
    }
    else
    {
        UE_LOG(LogFootstepParticles, Log, TEXT("Footstep system deactivated"));
        StopAllEffects();
    }
}

void AAudio_FootstepParticleSystem::TriggerFootstepEffect(const FVector& Location, EAudio_SurfaceType SurfaceType, float MovementSpeed)
{
    if (!bIsActive)
    {
        return;
    }
    
    // Check minimum interval between footsteps
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFootstepTime < MinFootstepInterval)
    {
        return;
    }
    
    LastFootstepTime = CurrentTime;
    
    // Calculate effect intensity based on movement speed
    float SpeedRatio = FMath::Clamp(MovementSpeed / 600.0f, 0.1f, 2.0f); // 600 = typical walk speed
    
    // Spawn dust particles
    SpawnDustParticles(Location, SurfaceType, SpeedRatio);
    
    // Play footstep sound
    PlayFootstepSound(Location, SurfaceType, SpeedRatio);
    
    UE_LOG(LogFootstepParticles, VeryVerbose, TEXT("Footstep effect triggered at %s, Surface: %d, Speed: %.2f"), 
           *Location.ToString(), (int32)SurfaceType, MovementSpeed);
}

void AAudio_FootstepParticleSystem::SpawnDustParticles(const FVector& Location, EAudio_SurfaceType SurfaceType, float Intensity)
{
    if (!ParticleSystemComponent)
    {
        return;
    }
    
    // Get surface-specific particle settings
    FAudio_SurfaceEffects* SurfaceEffects = SurfaceTypeMappings.Find(SurfaceType);
    if (!SurfaceEffects || !SurfaceEffects->DustParticleSystem)
    {
        // Use default dust effect
        SurfaceEffects = SurfaceTypeMappings.Find(EAudio_SurfaceType::Dirt);
    }
    
    if (SurfaceEffects && SurfaceEffects->DustParticleSystem)
    {
        // Set particle system
        ParticleSystemComponent->SetTemplate(SurfaceEffects->DustParticleSystem);
        
        // Set location and spawn
        ParticleSystemComponent->SetWorldLocation(Location);
        
        // Adjust particle parameters based on intensity
        float FinalIntensity = Intensity * DustIntensityMultiplier * SurfaceEffects->ParticleIntensityMultiplier;
        ParticleSystemComponent->SetFloatParameter(FName("Intensity"), FinalIntensity);
        ParticleSystemComponent->SetFloatParameter(FName("Lifetime"), ParticleLifetime);
        
        // Activate particles
        ParticleSystemComponent->ActivateSystem(true);
        
        // Auto-deactivate after lifetime
        GetWorld()->GetTimerManager().SetTimer(
            ParticleCleanupTimer,
            [this]() { ParticleSystemComponent->DeactivateSystem(); },
            ParticleLifetime,
            false
        );
    }
}

void AAudio_FootstepParticleSystem::PlayFootstepSound(const FVector& Location, EAudio_SurfaceType SurfaceType, float Intensity)
{
    if (!FootstepAudioComponent)
    {
        return;
    }
    
    // Get surface-specific audio settings
    FAudio_SurfaceEffects* SurfaceEffects = SurfaceTypeMappings.Find(SurfaceType);
    if (!SurfaceEffects || SurfaceEffects->FootstepSounds.Num() == 0)
    {
        // Use default footstep sound
        SurfaceEffects = SurfaceTypeMappings.Find(EAudio_SurfaceType::Dirt);
    }
    
    if (SurfaceEffects && SurfaceEffects->FootstepSounds.Num() > 0)
    {
        // Select random footstep sound
        int32 SoundIndex = FMath::RandRange(0, SurfaceEffects->FootstepSounds.Num() - 1);
        USoundBase* FootstepSound = SurfaceEffects->FootstepSounds[SoundIndex];
        
        if (FootstepSound)
        {
            // Calculate final volume
            float FinalVolume = Intensity * FootstepVolumeMultiplier * SurfaceEffects->VolumeMultiplier;
            
            // Set audio component properties
            FootstepAudioComponent->SetSound(FootstepSound);
            FootstepAudioComponent->SetVolumeMultiplier(FinalVolume);
            FootstepAudioComponent->SetWorldLocation(Location);
            
            // Play sound
            FootstepAudioComponent->Play();
        }
    }
}

void AAudio_FootstepParticleSystem::MonitorMovement(float DeltaTime)
{
    // This would typically monitor a character's movement
    // For now, we'll implement a basic position tracking system
    
    FVector CurrentLocation = GetActorLocation();
    float MovementDistance = FVector::Dist(CurrentLocation, LastTrackedLocation);
    
    if (MovementDistance > 100.0f) // Minimum distance for footstep
    {
        float MovementSpeed = MovementDistance / DeltaTime;
        
        // Determine surface type (simplified - would normally use line trace)
        EAudio_SurfaceType SurfaceType = DetermineSurfaceType(CurrentLocation);
        
        // Trigger footstep effect
        TriggerFootstepEffect(CurrentLocation, SurfaceType, MovementSpeed);
        
        LastTrackedLocation = CurrentLocation;
    }
}

EAudio_SurfaceType AAudio_FootstepParticleSystem::DetermineSurfaceType(const FVector& Location)
{
    // Simplified surface detection based on location
    // In a real implementation, this would use line traces to detect surface materials
    
    // For now, use biome-based detection
    if (Location.X < -25000.0f && Location.Y < -15000.0f)
    {
        return EAudio_SurfaceType::Mud; // Swamp biome
    }
    else if (Location.X > 25000.0f)
    {
        return EAudio_SurfaceType::Sand; // Desert biome
    }
    else if (Location.X < -15000.0f && Location.Y > 15000.0f)
    {
        return EAudio_SurfaceType::Grass; // Forest biome
    }
    else if (Location.Z > 300.0f)
    {
        return EAudio_SurfaceType::Rock; // Mountain biome
    }
    else
    {
        return EAudio_SurfaceType::Dirt; // Default savanna
    }
}

void AAudio_FootstepParticleSystem::InitializeSurfaceTypeMappings()
{
    // Initialize default surface effect mappings
    // These would typically be loaded from data assets
    
    FAudio_SurfaceEffects DirtEffects;
    DirtEffects.ParticleIntensityMultiplier = 1.0f;
    DirtEffects.VolumeMultiplier = 1.0f;
    SurfaceTypeMappings.Add(EAudio_SurfaceType::Dirt, DirtEffects);
    
    FAudio_SurfaceEffects GrassEffects;
    GrassEffects.ParticleIntensityMultiplier = 0.7f;
    GrassEffects.VolumeMultiplier = 0.8f;
    SurfaceTypeMappings.Add(EAudio_SurfaceType::Grass, GrassEffects);
    
    FAudio_SurfaceEffects RockEffects;
    RockEffects.ParticleIntensityMultiplier = 0.3f;
    RockEffects.VolumeMultiplier = 1.2f;
    SurfaceTypeMappings.Add(EAudio_SurfaceType::Rock, RockEffects);
    
    FAudio_SurfaceEffects MudEffects;
    MudEffects.ParticleIntensityMultiplier = 1.5f;
    MudEffects.VolumeMultiplier = 0.9f;
    SurfaceTypeMappings.Add(EAudio_SurfaceType::Mud, MudEffects);
    
    FAudio_SurfaceEffects SandEffects;
    SandEffects.ParticleIntensityMultiplier = 1.2f;
    SandEffects.VolumeMultiplier = 0.6f;
    SurfaceTypeMappings.Add(EAudio_SurfaceType::Sand, SandEffects);
    
    UE_LOG(LogFootstepParticles, Log, TEXT("Surface type mappings initialized"));
}

void AAudio_FootstepParticleSystem::StopAllEffects()
{
    if (ParticleSystemComponent)
    {
        ParticleSystemComponent->DeactivateSystem();
    }
    
    if (FootstepAudioComponent)
    {
        FootstepAudioComponent->Stop();
    }
    
    // Clear any active timers
    GetWorld()->GetTimerManager().ClearTimer(ParticleCleanupTimer);
}

void AAudio_FootstepParticleSystem::SetDustIntensity(float NewIntensity)
{
    DustIntensityMultiplier = FMath::Clamp(NewIntensity, 0.0f, 5.0f);
}

void AAudio_FootstepParticleSystem::SetFootstepVolume(float NewVolume)
{
    FootstepVolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
}

void AAudio_FootstepParticleSystem::SetFootstepInterval(float MinInterval, float MaxInterval)
{
    MinFootstepInterval = FMath::Max(0.1f, MinInterval);
    MaxFootstepInterval = FMath::Max(MinFootstepInterval, MaxInterval);
}