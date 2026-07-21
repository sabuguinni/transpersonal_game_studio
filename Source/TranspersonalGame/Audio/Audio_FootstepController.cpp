#include "Audio_FootstepController.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"

UAudio_FootstepController::UAudio_FootstepController()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize default values
    FootstepVolumeMultiplier = 1.0f;
    MinFootstepInterval = 0.3f;
    bEnableFootstepParticles = true;
    LastFootstepTime = 0.0f;
    CurrentSurface = EAudio_FootstepSurface::Grass;
}

void UAudio_FootstepController::BeginPlay()
{
    Super::BeginPlay();
    InitializeFootstepSounds();
}

void UAudio_FootstepController::InitializeFootstepSounds()
{
    // Initialize default footstep data for each surface type
    FAudio_FootstepData GrassData;
    GrassData.VolumeScale = 0.8f;
    GrassData.PitchVariation = 0.2f;
    FootstepSounds.Add(EAudio_FootstepSurface::Grass, GrassData);

    FAudio_FootstepData RockData;
    RockData.VolumeScale = 1.2f;
    RockData.PitchVariation = 0.15f;
    FootstepSounds.Add(EAudio_FootstepSurface::Rock, RockData);

    FAudio_FootstepData MudData;
    MudData.VolumeScale = 1.0f;
    MudData.PitchVariation = 0.25f;
    FootstepSounds.Add(EAudio_FootstepSurface::Mud, MudData);

    FAudio_FootstepData SandData;
    SandData.VolumeScale = 0.7f;
    SandData.PitchVariation = 0.3f;
    FootstepSounds.Add(EAudio_FootstepSurface::Sand, SandData);

    FAudio_FootstepData WaterData;
    WaterData.VolumeScale = 1.1f;
    WaterData.PitchVariation = 0.2f;
    FootstepSounds.Add(EAudio_FootstepSurface::Water, WaterData);

    FAudio_FootstepData WoodData;
    WoodData.VolumeScale = 1.3f;
    WoodData.PitchVariation = 0.1f;
    FootstepSounds.Add(EAudio_FootstepSurface::Wood, WoodData);
}

void UAudio_FootstepController::PlayFootstep(FVector FootLocation, EAudio_FootstepSurface Surface)
{
    // Check minimum interval
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFootstepTime < MinFootstepInterval)
    {
        return;
    }

    // Get footstep data for surface
    FAudio_FootstepData* FootstepData = FootstepSounds.Find(Surface);
    if (!FootstepData || !FootstepData->FootstepSound)
    {
        return;
    }

    // Calculate pitch variation
    float PitchVariation = FMath::RandRange(
        1.0f - FootstepData->PitchVariation,
        1.0f + FootstepData->PitchVariation
    );

    // Play footstep sound
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        FootstepData->FootstepSound,
        FootLocation,
        FootstepData->VolumeScale * FootstepVolumeMultiplier,
        PitchVariation,
        0.0f,
        nullptr,
        nullptr,
        GetOwner()
    );

    // Spawn particles if enabled
    if (bEnableFootstepParticles)
    {
        SpawnFootstepParticles(FootLocation, Surface);
    }

    LastFootstepTime = CurrentTime;
}

void UAudio_FootstepController::SetCurrentSurface(EAudio_FootstepSurface NewSurface)
{
    CurrentSurface = NewSurface;
}

EAudio_FootstepSurface UAudio_FootstepController::DetectSurfaceAtLocation(FVector Location)
{
    // Perform line trace downward to detect surface
    FVector StartLocation = Location;
    FVector EndLocation = Location - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams))
    {
        // Basic surface detection based on actor name or material
        FString ActorName = HitResult.GetActor()->GetName().ToLower();
        
        if (ActorName.Contains("rock") || ActorName.Contains("stone"))
        {
            return EAudio_FootstepSurface::Rock;
        }
        else if (ActorName.Contains("water") || ActorName.Contains("river"))
        {
            return EAudio_FootstepSurface::Water;
        }
        else if (ActorName.Contains("mud") || ActorName.Contains("swamp"))
        {
            return EAudio_FootstepSurface::Mud;
        }
        else if (ActorName.Contains("sand") || ActorName.Contains("desert"))
        {
            return EAudio_FootstepSurface::Sand;
        }
        else if (ActorName.Contains("wood") || ActorName.Contains("tree"))
        {
            return EAudio_FootstepSurface::Wood;
        }
    }

    // Default to grass
    return EAudio_FootstepSurface::Grass;
}

void UAudio_FootstepController::SetFootstepVolumeMultiplier(float NewMultiplier)
{
    FootstepVolumeMultiplier = FMath::Clamp(NewMultiplier, 0.0f, 3.0f);
}

void UAudio_FootstepController::SpawnFootstepParticles(FVector Location, EAudio_FootstepSurface Surface)
{
    if (!DustParticleEffect)
    {
        return;
    }

    // Adjust particle spawn location slightly above ground
    FVector ParticleLocation = Location + FVector(0, 0, 5);
    
    // Spawn particle system
    UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        DustParticleEffect,
        ParticleLocation,
        FRotator::ZeroRotator,
        FVector(0.5f), // Scale based on surface
        true // Auto destroy
    );
}