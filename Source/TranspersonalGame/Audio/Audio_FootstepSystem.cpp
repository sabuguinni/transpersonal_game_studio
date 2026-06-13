#include "Audio_FootstepSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

UAudio_FootstepSystem::UAudio_FootstepSystem()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize state
    CurrentSurface = EAudio_SurfaceType::Dirt;
    CurrentMovement = EAudio_MovementType::Walk;
    bIsMoving = false;
    TimeSinceLastFootstep = 0.0f;

    // Initialize footstep intervals
    FootstepInterval = 0.5f;
    RunFootstepInterval = 0.3f;
    SneakFootstepInterval = 0.8f;
    SurfaceDetectionDistance = 100.0f;
}

void UAudio_FootstepSystem::BeginPlay()
{
    Super::BeginPlay();

    // Create audio component
    FootstepAudioComponent = NewObject<UAudioComponent>(this);
    if (FootstepAudioComponent)
    {
        FootstepAudioComponent->bAutoActivate = false;
        FootstepAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);
    }

    // Initialize footstep data
    InitializeFootstepData();
}

void UAudio_FootstepSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Detect surface type
    DetectSurfaceType();

    // Update footstep timing
    UpdateFootstepTiming(DeltaTime);
}

void UAudio_FootstepSystem::TriggerFootstep(EAudio_MovementType MovementType)
{
    PlayFootstepSound(MovementType);
    TimeSinceLastFootstep = 0.0f;
}

void UAudio_FootstepSystem::SetMovementState(bool bMoving, bool bRunning, bool bSneaking)
{
    bIsMoving = bMoving;

    if (bSneaking)
    {
        CurrentMovement = EAudio_MovementType::Sneak;
    }
    else if (bRunning)
    {
        CurrentMovement = EAudio_MovementType::Run;
    }
    else
    {
        CurrentMovement = EAudio_MovementType::Walk;
    }
}

void UAudio_FootstepSystem::TriggerJumpSound()
{
    PlayFootstepSound(EAudio_MovementType::Jump);
}

void UAudio_FootstepSystem::TriggerLandSound()
{
    PlayFootstepSound(EAudio_MovementType::Land);
}

void UAudio_FootstepSystem::DetectSurfaceType()
{
    if (!GetOwner())
        return;

    // Perform line trace downward to detect surface
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, SurfaceDetectionDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Determine surface type based on hit result
        // This is a simplified version - in a full implementation,
        // you'd check material types or use physical materials
        FString SurfaceName = HitResult.GetActor() ? HitResult.GetActor()->GetName() : TEXT("");
        
        if (SurfaceName.Contains(TEXT("Grass")) || SurfaceName.Contains(TEXT("Foliage")))
        {
            CurrentSurface = EAudio_SurfaceType::Grass;
        }
        else if (SurfaceName.Contains(TEXT("Rock")) || SurfaceName.Contains(TEXT("Stone")))
        {
            CurrentSurface = EAudio_SurfaceType::Rock;
        }
        else if (SurfaceName.Contains(TEXT("Water")))
        {
            CurrentSurface = EAudio_SurfaceType::Water;
        }
        else if (SurfaceName.Contains(TEXT("Sand")))
        {
            CurrentSurface = EAudio_SurfaceType::Sand;
        }
        else if (SurfaceName.Contains(TEXT("Wood")))
        {
            CurrentSurface = EAudio_SurfaceType::Wood;
        }
        else if (SurfaceName.Contains(TEXT("Mud")))
        {
            CurrentSurface = EAudio_SurfaceType::Mud;
        }
        else
        {
            CurrentSurface = EAudio_SurfaceType::Dirt; // Default
        }
    }
}

void UAudio_FootstepSystem::UpdateFootstepTiming(float DeltaTime)
{
    if (!bIsMoving)
    {
        TimeSinceLastFootstep = 0.0f;
        return;
    }

    TimeSinceLastFootstep += DeltaTime;

    float CurrentInterval = GetFootstepInterval();
    if (TimeSinceLastFootstep >= CurrentInterval)
    {
        TriggerFootstep(CurrentMovement);
    }
}

void UAudio_FootstepSystem::PlayFootstepSound(EAudio_MovementType MovementType)
{
    if (!FootstepAudioComponent)
        return;

    // Find the appropriate sound for current surface and movement type
    TMap<EAudio_MovementType, FAudio_FootstepData>* SurfaceSounds = FootstepSoundMap.Find(CurrentSurface);
    if (!SurfaceSounds)
        return;

    FAudio_FootstepData* FootstepData = SurfaceSounds->Find(MovementType);
    if (!FootstepData || !FootstepData->FootstepMetaSound)
        return;

    // Set the MetaSound
    FootstepAudioComponent->SetSound(FootstepData->FootstepMetaSound);

    // Apply volume variation
    float VolumeVariation = FMath::RandRange(-FootstepData->VolumeVariation, FootstepData->VolumeVariation);
    float FinalVolume = FootstepData->BaseVolume + VolumeVariation;
    FootstepAudioComponent->SetVolumeMultiplier(FMath::Clamp(FinalVolume, 0.1f, 1.0f));

    // Apply pitch variation
    float PitchVariation = FMath::RandRange(-FootstepData->PitchVariation, FootstepData->PitchVariation);
    float FinalPitch = 1.0f + PitchVariation;
    FootstepAudioComponent->SetPitchMultiplier(FMath::Clamp(FinalPitch, 0.5f, 2.0f));

    // Play the sound
    FootstepAudioComponent->Play();
}

float UAudio_FootstepSystem::GetFootstepInterval() const
{
    switch (CurrentMovement)
    {
        case EAudio_MovementType::Run:
            return RunFootstepInterval;
        case EAudio_MovementType::Sneak:
            return SneakFootstepInterval;
        case EAudio_MovementType::Walk:
        default:
            return FootstepInterval;
    }
}

void UAudio_FootstepSystem::InitializeFootstepData()
{
    // Initialize footstep data for different surface/movement combinations
    // This would typically be loaded from data assets or configured in Blueprint

    // Dirt surface
    TMap<EAudio_MovementType, FAudio_FootstepData> DirtSounds;
    
    FAudio_FootstepData DirtWalk;
    DirtWalk.BaseVolume = 0.6f;
    DirtWalk.PitchVariation = 0.15f;
    DirtSounds.Add(EAudio_MovementType::Walk, DirtWalk);

    FAudio_FootstepData DirtRun;
    DirtRun.BaseVolume = 0.8f;
    DirtRun.PitchVariation = 0.2f;
    DirtSounds.Add(EAudio_MovementType::Run, DirtRun);

    FAudio_FootstepData DirtSneak;
    DirtSneak.BaseVolume = 0.3f;
    DirtSneak.PitchVariation = 0.1f;
    DirtSounds.Add(EAudio_MovementType::Sneak, DirtSneak);

    FootstepSoundMap.Add(EAudio_SurfaceType::Dirt, DirtSounds);

    // Grass surface
    TMap<EAudio_MovementType, FAudio_FootstepData> GrassSounds;
    
    FAudio_FootstepData GrassWalk;
    GrassWalk.BaseVolume = 0.5f;
    GrassWalk.PitchVariation = 0.12f;
    GrassSounds.Add(EAudio_MovementType::Walk, GrassWalk);

    FAudio_FootstepData GrassRun;
    GrassRun.BaseVolume = 0.7f;
    GrassRun.PitchVariation = 0.18f;
    GrassSounds.Add(EAudio_MovementType::Run, GrassRun);

    FAudio_FootstepData GrassSneak;
    GrassSneak.BaseVolume = 0.25f;
    GrassSneak.PitchVariation = 0.08f;
    GrassSounds.Add(EAudio_MovementType::Sneak, GrassSneak);

    FootstepSoundMap.Add(EAudio_SurfaceType::Grass, GrassSounds);

    // Rock surface
    TMap<EAudio_MovementType, FAudio_FootstepData> RockSounds;
    
    FAudio_FootstepData RockWalk;
    RockWalk.BaseVolume = 0.8f;
    RockWalk.PitchVariation = 0.25f;
    RockSounds.Add(EAudio_MovementType::Walk, RockWalk);

    FAudio_FootstepData RockRun;
    RockRun.BaseVolume = 1.0f;
    RockRun.PitchVariation = 0.3f;
    RockSounds.Add(EAudio_MovementType::Run, RockRun);

    FAudio_FootstepData RockSneak;
    RockSneak.BaseVolume = 0.4f;
    RockSneak.PitchVariation = 0.15f;
    RockSounds.Add(EAudio_MovementType::Sneak, RockSneak);

    FootstepSoundMap.Add(EAudio_SurfaceType::Rock, RockSounds);
}