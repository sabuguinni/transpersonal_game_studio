#include "Audio_FootstepSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"

UAudio_FootstepSystem::UAudio_FootstepSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bFootstepsEnabled = true;
    GlobalVolumeMultiplier = 1.0f;
    FootstepCooldown = 0.3f;
    LastFootstepTime = 0.0f;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudioComponent"));
}

void UAudio_FootstepSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeFootstepData();
}

void UAudio_FootstepSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update last footstep time tracking
    LastFootstepTime += DeltaTime;
}

void UAudio_FootstepSystem::PlayFootstep(EAudio_FootstepType FootstepType, EAudio_SurfaceType SurfaceType, FVector Location)
{
    if (!bFootstepsEnabled || LastFootstepTime < FootstepCooldown)
    {
        return;
    }

    // Find footstep data for this combination
    if (FootstepDataMap.Contains(FootstepType))
    {
        const TMap<EAudio_SurfaceType, FAudio_FootstepData>& SurfaceMap = FootstepDataMap[FootstepType];
        if (SurfaceMap.Contains(SurfaceType))
        {
            const FAudio_FootstepData& FootstepData = SurfaceMap[SurfaceType];
            
            // Play footstep sound
            if (FootstepData.FootstepSound && AudioComponent)
            {
                AudioComponent->SetWorldLocation(Location);
                AudioComponent->SetSound(FootstepData.FootstepSound);
                AudioComponent->SetVolumeMultiplier(FootstepData.VolumeMultiplier * GlobalVolumeMultiplier);
                AudioComponent->SetPitchMultiplier(FootstepData.PitchMultiplier);
                AudioComponent->Play();
            }

            // Spawn dust particle effect
            if (FootstepData.DustParticle)
            {
                SpawnDustParticle(FootstepData.DustParticle, Location);
            }

            LastFootstepTime = 0.0f;
        }
    }
}

void UAudio_FootstepSystem::SetFootstepEnabled(bool bEnabled)
{
    bFootstepsEnabled = bEnabled;
}

void UAudio_FootstepSystem::SetVolumeMultiplier(float Multiplier)
{
    GlobalVolumeMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
}

EAudio_SurfaceType UAudio_FootstepSystem::DetectSurfaceType(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return EAudio_SurfaceType::Dirt;
    }

    // Perform line trace downward to detect surface
    FVector TraceStart = Location + FVector(0, 0, 50);
    FVector TraceEnd = Location - FVector(0, 0, 100);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Check physical material if available
        if (HitResult.PhysMaterial.IsValid())
        {
            FString MaterialName = HitResult.PhysMaterial->GetName().ToLower();
            
            if (MaterialName.Contains("grass") || MaterialName.Contains("vegetation"))
            {
                return EAudio_SurfaceType::Grass;
            }
            else if (MaterialName.Contains("rock") || MaterialName.Contains("stone"))
            {
                return EAudio_SurfaceType::Rock;
            }
            else if (MaterialName.Contains("mud") || MaterialName.Contains("swamp"))
            {
                return EAudio_SurfaceType::Mud;
            }
            else if (MaterialName.Contains("sand"))
            {
                return EAudio_SurfaceType::Sand;
            }
            else if (MaterialName.Contains("water"))
            {
                return EAudio_SurfaceType::Water;
            }
        }
        
        // Check actor name/tag as fallback
        if (HitResult.GetActor())
        {
            FString ActorName = HitResult.GetActor()->GetName().ToLower();
            if (ActorName.Contains("grass") || ActorName.Contains("vegetation"))
            {
                return EAudio_SurfaceType::Grass;
            }
            else if (ActorName.Contains("rock") || ActorName.Contains("stone"))
            {
                return EAudio_SurfaceType::Rock;
            }
        }
    }

    // Default to dirt for prehistoric environment
    return EAudio_SurfaceType::Dirt;
}

void UAudio_FootstepSystem::InitializeFootstepData()
{
    // Initialize default footstep data for all combinations
    // This would typically be loaded from data assets or configured in Blueprint
    
    for (int32 FootstepTypeInt = 0; FootstepTypeInt < (int32)EAudio_FootstepType::Dinosaur + 1; FootstepTypeInt++)
    {
        EAudio_FootstepType FootstepType = (EAudio_FootstepType)FootstepTypeInt;
        
        for (int32 SurfaceTypeInt = 0; SurfaceTypeInt < (int32)EAudio_SurfaceType::Water + 1; SurfaceTypeInt++)
        {
            EAudio_SurfaceType SurfaceType = (EAudio_SurfaceType)SurfaceTypeInt;
            
            FAudio_FootstepData FootstepData;
            
            // Set volume based on footstep type
            switch (FootstepType)
            {
                case EAudio_FootstepType::Light:
                    FootstepData.VolumeMultiplier = 0.6f;
                    FootstepData.PitchMultiplier = 1.1f;
                    break;
                case EAudio_FootstepType::Medium:
                    FootstepData.VolumeMultiplier = 0.8f;
                    FootstepData.PitchMultiplier = 1.0f;
                    break;
                case EAudio_FootstepType::Heavy:
                    FootstepData.VolumeMultiplier = 1.0f;
                    FootstepData.PitchMultiplier = 0.9f;
                    break;
                case EAudio_FootstepType::Dinosaur:
                    FootstepData.VolumeMultiplier = 1.5f;
                    FootstepData.PitchMultiplier = 0.7f;
                    break;
            }
            
            // Adjust volume based on surface type
            switch (SurfaceType)
            {
                case EAudio_SurfaceType::Grass:
                    FootstepData.VolumeMultiplier *= 0.8f;
                    break;
                case EAudio_SurfaceType::Rock:
                    FootstepData.VolumeMultiplier *= 1.2f;
                    FootstepData.PitchMultiplier *= 1.1f;
                    break;
                case EAudio_SurfaceType::Mud:
                    FootstepData.VolumeMultiplier *= 0.9f;
                    FootstepData.PitchMultiplier *= 0.8f;
                    break;
                case EAudio_SurfaceType::Sand:
                    FootstepData.VolumeMultiplier *= 0.7f;
                    break;
                case EAudio_SurfaceType::Water:
                    FootstepData.VolumeMultiplier *= 1.1f;
                    FootstepData.PitchMultiplier *= 0.9f;
                    break;
                default:
                    break;
            }
            
            FootstepDataMap.FindOrAdd(FootstepType).Add(SurfaceType, FootstepData);
        }
    }
}

void UAudio_FootstepSystem::SpawnDustParticle(UParticleSystem* ParticleSystem, FVector Location)
{
    if (ParticleSystem && GetWorld())
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ParticleSystem,
            Location,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true
        );
    }
}