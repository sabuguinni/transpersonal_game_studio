#include "Audio_FootstepSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"

UAudio_FootstepSystem::UAudio_FootstepSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudioComponent"));
    if (AudioComponent)
    {
        AudioComponent->bAutoActivate = false;
        AudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UAudio_FootstepSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFootstepSounds();
    
    if (AudioComponent)
    {
        AudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
    }
}

void UAudio_FootstepSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bFootstepsEnabled || !GetOwner())
        return;

    // Auto-trigger footsteps based on movement
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector Velocity = GetOwner()->GetVelocity();
    float Speed = Velocity.Size();

    // Only play footsteps if moving fast enough
    if (Speed > 50.0f)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float TimeSinceLastFootstep = CurrentTime - LastFootstepTime;
        
        // Adjust interval based on speed
        float AdjustedInterval = FootstepInterval * (300.0f / FMath::Max(Speed, 100.0f));
        
        if (TimeSinceLastFootstep >= AdjustedInterval)
        {
            ESurfaceType SurfaceType = GetSurfaceTypeAtLocation(CurrentLocation);
            PlayFootstep(SurfaceType, CurrentLocation, Speed / 300.0f);
            LastFootstepTime = CurrentTime;
        }
    }
}

void UAudio_FootstepSystem::PlayFootstep(ESurfaceType SurfaceType, FVector Location, float VelocityMultiplier)
{
    if (!bFootstepsEnabled || !AudioComponent)
        return;

    FAudio_FootstepData FootstepData = GetFootstepDataForSurface(SurfaceType);
    
    if (FootstepData.FootstepSound.IsValid())
    {
        USoundCue* SoundCue = FootstepData.FootstepSound.LoadSynchronous();
        if (SoundCue)
        {
            float FinalVolume = FootstepData.VolumeMultiplier * CurrentVolume * VelocityMultiplier;
            float FinalPitch = FootstepData.PitchMultiplier * FMath::RandRange(0.9f, 1.1f);
            
            AudioComponent->SetWorldLocation(Location);
            AudioComponent->SetVolumeMultiplier(FinalVolume);
            AudioComponent->SetPitchMultiplier(FinalPitch);
            AudioComponent->SetSound(SoundCue);
            AudioComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("FootstepSystem: Playing footstep at %s, Surface: %d, Volume: %f"), 
                *Location.ToString(), (int32)SurfaceType, FinalVolume);
        }
    }
}

void UAudio_FootstepSystem::PlayPlayerFootstep(FVector Location, float MovementSpeed)
{
    if (!bFootstepsEnabled)
        return;

    ESurfaceType SurfaceType = GetSurfaceTypeAtLocation(Location);
    float SpeedMultiplier = FMath::Clamp(MovementSpeed / 300.0f, 0.5f, 2.0f);
    
    PlayFootstep(SurfaceType, Location, SpeedMultiplier);
}

void UAudio_FootstepSystem::PlayDinosaurFootstep(FVector Location, ECreatureSize CreatureSize)
{
    if (!bFootstepsEnabled)
        return;

    ESurfaceType SurfaceType = GetSurfaceTypeAtLocation(Location);
    
    // Use different footstep data based on creature size
    FAudio_FootstepData FootstepData;
    switch (CreatureSize)
    {
        case ECreatureSize::Small:
            FootstepData = PlayerFootsteps;
            FootstepData.VolumeMultiplier *= 0.7f;
            break;
        case ECreatureSize::Medium:
            FootstepData = DinosaurFootsteps;
            break;
        case ECreatureSize::Large:
            FootstepData = HeavyFootsteps;
            FootstepData.VolumeMultiplier *= 1.5f;
            break;
        case ECreatureSize::Massive:
            FootstepData = HeavyFootsteps;
            FootstepData.VolumeMultiplier *= 2.0f;
            FootstepData.PitchMultiplier *= 0.8f;
            break;
        default:
            FootstepData = DinosaurFootsteps;
            break;
    }
    
    if (AudioComponent && FootstepData.FootstepSound.IsValid())
    {
        USoundCue* SoundCue = FootstepData.FootstepSound.LoadSynchronous();
        if (SoundCue)
        {
            AudioComponent->SetWorldLocation(Location);
            AudioComponent->SetVolumeMultiplier(FootstepData.VolumeMultiplier * CurrentVolume);
            AudioComponent->SetPitchMultiplier(FootstepData.PitchMultiplier);
            AudioComponent->SetSound(SoundCue);
            AudioComponent->Play();
        }
    }
}

ESurfaceType UAudio_FootstepSystem::GetSurfaceTypeAtLocation(FVector Location)
{
    if (!GetWorld())
        return ESurfaceType::Grass;

    // Perform line trace downward to detect surface
    FVector StartLocation = Location + FVector(0, 0, 50);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
        ECC_WorldStatic, QueryParams))
    {
        if (HitResult.PhysMaterial.IsValid())
        {
            // Map physical materials to surface types
            FString MaterialName = HitResult.PhysMaterial->GetName();
            
            if (MaterialName.Contains("Stone") || MaterialName.Contains("Rock"))
                return ESurfaceType::Stone;
            else if (MaterialName.Contains("Water") || MaterialName.Contains("Mud"))
                return ESurfaceType::Water;
            else if (MaterialName.Contains("Sand") || MaterialName.Contains("Desert"))
                return ESurfaceType::Sand;
            else if (MaterialName.Contains("Snow") || MaterialName.Contains("Ice"))
                return ESurfaceType::Snow;
            else if (MaterialName.Contains("Metal") || MaterialName.Contains("Wood"))
                return ESurfaceType::Wood;
        }
        
        // Default based on hit result
        if (HitResult.GetComponent())
        {
            FString ComponentName = HitResult.GetComponent()->GetName();
            if (ComponentName.Contains("Water"))
                return ESurfaceType::Water;
            else if (ComponentName.Contains("Rock") || ComponentName.Contains("Stone"))
                return ESurfaceType::Stone;
        }
    }

    // Default to grass for natural terrain
    return ESurfaceType::Grass;
}

void UAudio_FootstepSystem::SetFootstepVolume(float NewVolume)
{
    CurrentVolume = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
}

void UAudio_FootstepSystem::EnableFootsteps(bool bEnable)
{
    bFootstepsEnabled = bEnable;
    
    if (!bEnable && AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}

void UAudio_FootstepSystem::InitializeFootstepSounds()
{
    // Initialize default footstep sounds for different surfaces
    // These would typically be loaded from data assets or configured in Blueprint
    
    // Player footsteps (lighter, higher pitch)
    PlayerFootsteps.VolumeMultiplier = 0.8f;
    PlayerFootsteps.PitchMultiplier = 1.2f;
    PlayerFootsteps.MinDistance = 50.0f;
    PlayerFootsteps.MaxDistance = 500.0f;
    
    // Dinosaur footsteps (medium weight)
    DinosaurFootsteps.VolumeMultiplier = 1.0f;
    DinosaurFootsteps.PitchMultiplier = 1.0f;
    DinosaurFootsteps.MinDistance = 100.0f;
    DinosaurFootsteps.MaxDistance = 1000.0f;
    
    // Heavy footsteps (T-Rex, large dinosaurs)
    HeavyFootsteps.VolumeMultiplier = 1.5f;
    HeavyFootsteps.PitchMultiplier = 0.8f;
    HeavyFootsteps.MinDistance = 200.0f;
    HeavyFootsteps.MaxDistance = 2000.0f;
    
    UE_LOG(LogTemp, Log, TEXT("FootstepSystem: Initialized footstep sound configuration"));
}

FAudio_FootstepData UAudio_FootstepSystem::GetFootstepDataForSurface(ESurfaceType SurfaceType)
{
    if (FootstepSounds.Contains(SurfaceType))
    {
        return FootstepSounds[SurfaceType];
    }
    
    // Return default footstep data based on owner type
    if (GetOwner()->GetName().Contains("Player") || GetOwner()->GetName().Contains("Character"))
    {
        return PlayerFootsteps;
    }
    else if (GetOwner()->GetName().Contains("TRex") || GetOwner()->GetName().Contains("Heavy"))
    {
        return HeavyFootsteps;
    }
    else
    {
        return DinosaurFootsteps;
    }
}