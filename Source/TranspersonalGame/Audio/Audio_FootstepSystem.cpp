#include "Audio_FootstepSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

UAudio_FootstepSystem::UAudio_FootstepSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check 10 times per second
    
    FootstepVolume = 1.0f;
    FootstepRange = 1000.0f;
    bAutoDetectFootsteps = true;
    MovementThreshold = 50.0f;
    LastFootstepTime = 0.0f;
    AccumulatedDistance = 0.0f;
    LastLocation = FVector::ZeroVector;
}

void UAudio_FootstepSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFootstepAudio();
    
    if (AActor* Owner = GetOwner())
    {
        LastLocation = Owner->GetActorLocation();
    }
}

void UAudio_FootstepSystem::InitializeFootstepAudio()
{
    if (AActor* Owner = GetOwner())
    {
        FootstepAudioComponent = NewObject<UAudioComponent>(Owner);
        if (FootstepAudioComponent)
        {
            FootstepAudioComponent->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
            FootstepAudioComponent->SetVolumeMultiplier(FootstepVolume);
            FootstepAudioComponent->bAutoActivate = false;
        }
    }
}

void UAudio_FootstepSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bAutoDetectFootsteps)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector CurrentLocation = Owner->GetActorLocation();
    float DistanceMoved = FVector::Dist(CurrentLocation, LastLocation);
    
    AccumulatedDistance += DistanceMoved;
    
    if (ShouldPlayFootstep(DeltaTime, DistanceMoved))
    {
        EAudio_SurfaceType SurfaceType = DetectSurfaceType(CurrentLocation);
        PlayFootstepSound(SurfaceType, CurrentLocation);
        
        AccumulatedDistance = 0.0f;
        LastFootstepTime = GetWorld()->GetTimeSeconds();
    }
    
    LastLocation = CurrentLocation;
}

bool UAudio_FootstepSystem::ShouldPlayFootstep(float DeltaTime, float DistanceMoved)
{
    // Check if enough distance has been covered
    if (AccumulatedDistance < MovementThreshold)
    {
        return false;
    }
    
    // Check minimum time between steps
    float CurrentTime = GetWorld()->GetTimeSeconds();
    FAudio_FootstepData* DefaultData = GetFootstepData(EAudio_SurfaceType::Grass);
    float MinTime = DefaultData ? DefaultData->MinTimeBetweenSteps : 0.3f;
    
    return (CurrentTime - LastFootstepTime) >= MinTime;
}

void UAudio_FootstepSystem::PlayFootstepSound(EAudio_SurfaceType SurfaceType, FVector Location)
{
    FAudio_FootstepData* FootstepData = GetFootstepData(SurfaceType);
    if (!FootstepData || FootstepData->FootstepSounds.Num() == 0)
    {
        return;
    }

    // Select random footstep sound
    int32 RandomIndex = FMath::RandRange(0, FootstepData->FootstepSounds.Num() - 1);
    TSoftObjectPtr<USoundBase> SelectedSound = FootstepData->FootstepSounds[RandomIndex];
    
    USoundBase* Sound = SelectedSound.LoadSynchronous();
    if (!Sound)
    {
        return;
    }

    // Calculate volume and pitch
    float FinalVolume = FootstepVolume * FootstepData->VolumeMultiplier;
    float PitchVariation = FMath::RandRange(-FootstepData->PitchVariation, FootstepData->PitchVariation);
    float FinalPitch = 1.0f + PitchVariation;

    // Play the sound
    if (FootstepAudioComponent)
    {
        FootstepAudioComponent->SetSound(Sound);
        FootstepAudioComponent->SetVolumeMultiplier(FinalVolume);
        FootstepAudioComponent->SetPitchMultiplier(FinalPitch);
        FootstepAudioComponent->Play();
    }
    else
    {
        // Fallback to world sound
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, FinalVolume, FinalPitch);
    }
}

EAudio_SurfaceType UAudio_FootstepSystem::DetectSurfaceType(FVector Location)
{
    // Simple surface detection based on location
    // In a full implementation, this would use physics materials or surface tags
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return EAudio_SurfaceType::Grass;
    }

    // Perform a line trace downward to detect surface
    FVector Start = Location;
    FVector End = Location - FVector(0, 0, 200.0f); // Trace 2 meters down
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        // Basic material detection based on actor names or tags
        if (HitResult.GetActor())
        {
            FString ActorName = HitResult.GetActor()->GetName().ToLower();
            
            if (ActorName.Contains("stone") || ActorName.Contains("rock"))
            {
                return EAudio_SurfaceType::Stone;
            }
            else if (ActorName.Contains("water"))
            {
                return EAudio_SurfaceType::Water;
            }
            else if (ActorName.Contains("sand"))
            {
                return EAudio_SurfaceType::Sand;
            }
            else if (ActorName.Contains("wood"))
            {
                return EAudio_SurfaceType::Wood;
            }
            else if (ActorName.Contains("metal"))
            {
                return EAudio_SurfaceType::Metal;
            }
        }
    }
    
    // Default to grass for natural terrain
    return EAudio_SurfaceType::Grass;
}

void UAudio_FootstepSystem::SetFootstepVolume(float Volume)
{
    FootstepVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    
    if (FootstepAudioComponent)
    {
        FootstepAudioComponent->SetVolumeMultiplier(FootstepVolume);
    }
}

void UAudio_FootstepSystem::EnableAutoFootsteps(bool bEnable)
{
    bAutoDetectFootsteps = bEnable;
}

FAudio_FootstepData* UAudio_FootstepSystem::GetFootstepData(EAudio_SurfaceType SurfaceType)
{
    if (!FootstepDataTable)
    {
        return nullptr;
    }

    // Convert enum to string for data table lookup
    FString SurfaceString;
    switch (SurfaceType)
    {
        case EAudio_SurfaceType::Grass: SurfaceString = TEXT("Grass"); break;
        case EAudio_SurfaceType::Dirt: SurfaceString = TEXT("Dirt"); break;
        case EAudio_SurfaceType::Stone: SurfaceString = TEXT("Stone"); break;
        case EAudio_SurfaceType::Sand: SurfaceString = TEXT("Sand"); break;
        case EAudio_SurfaceType::Water: SurfaceString = TEXT("Water"); break;
        case EAudio_SurfaceType::Wood: SurfaceString = TEXT("Wood"); break;
        case EAudio_SurfaceType::Metal: SurfaceString = TEXT("Metal"); break;
        case EAudio_SurfaceType::Snow: SurfaceString = TEXT("Snow"); break;
        default: SurfaceString = TEXT("Grass"); break;
    }

    return FootstepDataTable->FindRow<FAudio_FootstepData>(FName(*SurfaceString), TEXT("FootstepSystem"));
}