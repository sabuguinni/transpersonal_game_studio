#include "Audio_FootstepSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_FootstepSystem::UAudio_FootstepSystem()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize default settings
    FootstepVolumeScale = 1.0f;
    MinTimeBetweenSteps = 0.3f;
    bEnableFootstepParticles = true;
    LastFootstepTime = 0.0f;
    LastSurfaceType = ESurfaceType::Grass;

    // Setup default surface sounds
    FAudio_SurfaceSound GrassSound;
    GrassSound.SurfaceType = ESurfaceType::Grass;
    GrassSound.VolumeMultiplier = 0.6f;
    GrassSound.PitchVariation = 0.3f;
    SurfaceSounds.Add(GrassSound);

    FAudio_SurfaceSound DirtSound;
    DirtSound.SurfaceType = ESurfaceType::Dirt;
    DirtSound.VolumeMultiplier = 0.7f;
    DirtSound.PitchVariation = 0.2f;
    SurfaceSounds.Add(DirtSound);

    FAudio_SurfaceSound RockSound;
    RockSound.SurfaceType = ESurfaceType::Rock;
    RockSound.VolumeMultiplier = 0.9f;
    RockSound.PitchVariation = 0.15f;
    SurfaceSounds.Add(RockSound);

    FAudio_SurfaceSound WaterSound;
    WaterSound.SurfaceType = ESurfaceType::Water;
    WaterSound.VolumeMultiplier = 0.8f;
    WaterSound.PitchVariation = 0.25f;
    SurfaceSounds.Add(WaterSound);
}

void UAudio_FootstepSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_FootstepSystem initialized with %d surface types"), SurfaceSounds.Num());
}

void UAudio_FootstepSystem::PlayFootstep(const FVector& Location, ESurfaceType Surface, float MovementSpeed)
{
    // Check minimum time between steps
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFootstepTime < MinTimeBetweenSteps)
    {
        return;
    }

    FAudio_SurfaceSound* SurfaceSound = GetSurfaceSound(Surface);
    if (!SurfaceSound || !SurfaceSound->FootstepSound)
    {
        return;
    }

    // Calculate volume based on movement speed
    float SpeedVolume = CalculateVolumeFromSpeed(MovementSpeed);
    float FinalVolume = SpeedVolume * SurfaceSound->VolumeMultiplier * FootstepVolumeScale;

    // Calculate pitch variation
    float PitchVariation = CalculatePitchVariation() * SurfaceSound->PitchVariation;
    float FinalPitch = 1.0f + PitchVariation;

    // Play the footstep sound
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SurfaceSound->FootstepSound,
        Location,
        FinalVolume,
        FinalPitch
    );

    LastFootstepTime = CurrentTime;
    LastSurfaceType = Surface;

    UE_LOG(LogTemp, VeryVerbose, TEXT("Played footstep on %s surface at volume %.2f"), 
           *UEnum::GetValueAsString(Surface), FinalVolume);
}

void UAudio_FootstepSystem::PlayLanding(const FVector& Location, ESurfaceType Surface, float ImpactForce)
{
    FAudio_SurfaceSound* SurfaceSound = GetSurfaceSound(Surface);
    if (!SurfaceSound || !SurfaceSound->LandingSound)
    {
        // Fall back to footstep sound if no landing sound
        if (SurfaceSound && SurfaceSound->FootstepSound)
        {
            float LandingVolume = FMath::Clamp(ImpactForce * 0.8f, 0.3f, 1.5f);
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                SurfaceSound->FootstepSound,
                Location,
                LandingVolume * SurfaceSound->VolumeMultiplier * FootstepVolumeScale,
                0.8f // Lower pitch for landing
            );
        }
        return;
    }

    // Calculate volume based on impact force
    float ImpactVolume = FMath::Clamp(ImpactForce * 0.7f, 0.4f, 1.2f);
    float FinalVolume = ImpactVolume * SurfaceSound->VolumeMultiplier * FootstepVolumeScale;

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SurfaceSound->LandingSound,
        Location,
        FinalVolume,
        1.0f
    );

    UE_LOG(LogTemp, Log, TEXT("Played landing sound on %s surface with impact %.2f"), 
           *UEnum::GetValueAsString(Surface), ImpactForce);
}

ESurfaceType UAudio_FootstepSystem::DetectSurfaceType(const FVector& Location)
{
    // Perform a line trace downward to detect surface material
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 50);
    FVector EndLocation = Location - FVector(0, 0, 100);

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // In a full implementation, this would check the physical material
        // For now, we use simple heuristics based on actor names or tags
        if (HitResult.GetActor())
        {
            FString ActorName = HitResult.GetActor()->GetName().ToLower();
            
            if (ActorName.Contains(TEXT("water")) || ActorName.Contains(TEXT("river")))
            {
                return ESurfaceType::Water;
            }
            else if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")))
            {
                return ESurfaceType::Rock;
            }
            else if (ActorName.Contains(TEXT("dirt")) || ActorName.Contains(TEXT("mud")))
            {
                return ESurfaceType::Dirt;
            }
        }
    }

    // Default to grass
    return ESurfaceType::Grass;
}

FAudio_SurfaceSound* UAudio_FootstepSystem::GetSurfaceSound(ESurfaceType SurfaceType)
{
    for (FAudio_SurfaceSound& SurfaceSound : SurfaceSounds)
    {
        if (SurfaceSound.SurfaceType == SurfaceType)
        {
            return &SurfaceSound;
        }
    }
    return nullptr;
}

float UAudio_FootstepSystem::CalculateVolumeFromSpeed(float MovementSpeed)
{
    // Map movement speed to volume (0.3 to 1.2 range)
    float NormalizedSpeed = FMath::Clamp(MovementSpeed / 600.0f, 0.0f, 2.0f);
    return 0.3f + (NormalizedSpeed * 0.9f);
}

float UAudio_FootstepSystem::CalculatePitchVariation()
{
    // Random pitch variation between -1 and 1
    return FMath::RandRange(-1.0f, 1.0f);
}