#include "DinosaurAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"

UAudio_DinosaurAudioComponent::UAudio_DinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio updates
}

void UAudio_DinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start idle vocalisation loop immediately
    if (AudioProfile.IdleVocalisation)
    {
        StartLoopingAudio(AudioProfile.IdleVocalisation);
    }
}

void UAudio_DinosaurAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopLoopingAudio();
    Super::EndPlay(EndPlayReason);
}

void UAudio_DinosaurAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Throttled player distance update
    DistanceUpdateAccumulator += DeltaTime;
    if (DistanceUpdateAccumulator >= DistanceUpdateInterval)
    {
        DistanceUpdateAccumulator = 0.0f;

        APawn* NearestPlayer = FindNearestPlayer();
        if (NearestPlayer && GetOwner())
        {
            DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), NearestPlayer->GetActorLocation());
        }
        else
        {
            DistanceToPlayer = 99999.0f;
        }
    }
}

void UAudio_DinosaurAudioComponent::SetDinoState(EAudio_DinoState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }

    EAudio_DinoState PreviousState = CurrentState;
    CurrentState = NewState;

    // Stop current looping audio
    StopLoopingAudio();

    // Select audio for new state
    switch (NewState)
    {
        case EAudio_DinoState::Idle:
            if (AudioProfile.IdleVocalisation)
            {
                StartLoopingAudio(AudioProfile.IdleVocalisation);
            }
            break;

        case EAudio_DinoState::Alert:
            // Alert call is a one-shot, then loop idle
            if (AudioProfile.AlertCall && GetOwner())
            {
                UGameplayStatics::PlaySoundAtLocation(
                    GetWorld(),
                    AudioProfile.AlertCall,
                    GetOwner()->GetActorLocation(),
                    AudioProfile.VolumeMultiplier,
                    1.0f,
                    0.0f
                );
            }
            break;

        case EAudio_DinoState::Hunting:
            // No specific hunting loop — silence builds tension
            break;

        case EAudio_DinoState::Attacking:
            if (AudioProfile.AttackRoar && GetOwner())
            {
                UGameplayStatics::PlaySoundAtLocation(
                    GetWorld(),
                    AudioProfile.AttackRoar,
                    GetOwner()->GetActorLocation(),
                    AudioProfile.VolumeMultiplier,
                    1.0f,
                    0.0f
                );
            }
            break;

        case EAudio_DinoState::Fleeing:
            // Fleeing — no specific vocalisation, footsteps carry it
            break;

        case EAudio_DinoState::Feeding:
            // Feeding sounds — could add wet/crunching sounds here
            break;

        case EAudio_DinoState::Dead:
            PlayDeathAudio();
            break;

        default:
            break;
    }
}

void UAudio_DinosaurAudioComponent::PlayFootstep()
{
    if (!GetOwner())
    {
        return;
    }

    // Play footstep sound at actor location
    if (AudioProfile.Footstep)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            AudioProfile.Footstep,
            GetOwner()->GetActorLocation(),
            AudioProfile.VolumeMultiplier,
            1.0f,
            0.0f
        );
    }

    // Propagate ground shake for heavy species
    const bool bIsHeavySpecies = (Species == EAudio_DinoSpecies::TyrannosaurusRex ||
                                   Species == EAudio_DinoSpecies::Brachiosaurus ||
                                   Species == EAudio_DinoSpecies::Spinosaurus);

    if (bIsHeavySpecies && AudioProfile.GroundShakeRadius > 0.0f)
    {
        PropagateGroundShake();
    }
}

void UAudio_DinosaurAudioComponent::PlayDeathAudio()
{
    StopLoopingAudio();

    if (AudioProfile.DeathSound && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            AudioProfile.DeathSound,
            GetOwner()->GetActorLocation(),
            AudioProfile.VolumeMultiplier,
            1.0f,
            0.0f
        );
    }

    // Disable tick after death — no more audio updates needed
    PrimaryComponentTick.bCanEverTick = false;
}

float UAudio_DinosaurAudioComponent::GetFearIntensity() const
{
    // Base fear by species
    float SpeciesFear = 0.3f;
    switch (Species)
    {
        case EAudio_DinoSpecies::TyrannosaurusRex:  SpeciesFear = 1.0f; break;
        case EAudio_DinoSpecies::Spinosaurus:        SpeciesFear = 0.9f; break;
        case EAudio_DinoSpecies::Velociraptor:       SpeciesFear = 0.7f; break;
        case EAudio_DinoSpecies::Triceratops:        SpeciesFear = 0.5f; break;
        case EAudio_DinoSpecies::Brachiosaurus:      SpeciesFear = 0.4f; break;
        case EAudio_DinoSpecies::Pterodactyl:        SpeciesFear = 0.6f; break;
        default:                                      SpeciesFear = 0.3f; break;
    }

    // State multiplier — hunting/attacking maximises fear
    float StateMult = 1.0f;
    switch (CurrentState)
    {
        case EAudio_DinoState::Attacking: StateMult = 1.0f;  break;
        case EAudio_DinoState::Hunting:   StateMult = 0.85f; break;
        case EAudio_DinoState::Alert:     StateMult = 0.6f;  break;
        case EAudio_DinoState::Idle:      StateMult = 0.2f;  break;
        case EAudio_DinoState::Fleeing:   StateMult = 0.3f;  break;
        case EAudio_DinoState::Dead:      StateMult = 0.0f;  break;
        default:                          StateMult = 0.2f;  break;
    }

    // Distance falloff — full fear within 500cm, zero beyond AttenuationMaxDistance
    const float MaxFearDist = 500.0f;
    const float DistanceFactor = FMath::Clamp(
        1.0f - ((DistanceToPlayer - MaxFearDist) / (AudioProfile.AttenuationMaxDistance - MaxFearDist)),
        0.0f,
        1.0f
    );

    return FMath::Clamp(SpeciesFear * StateMult * DistanceFactor, 0.0f, 1.0f);
}

// ─── Private ──────────────────────────────────────────────────────────────────

void UAudio_DinosaurAudioComponent::StopLoopingAudio()
{
    if (LoopingAudioComp && LoopingAudioComp->IsPlaying())
    {
        LoopingAudioComp->FadeOut(0.5f, 0.0f);
        LoopingAudioComp = nullptr;
    }
}

void UAudio_DinosaurAudioComponent::StartLoopingAudio(USoundCue* SoundCue)
{
    if (!SoundCue || !GetOwner())
    {
        return;
    }

    LoopingAudioComp = UGameplayStatics::SpawnSoundAttached(
        SoundCue,
        GetOwner()->GetRootComponent(),
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::SnapToTarget,
        false,
        AudioProfile.VolumeMultiplier,
        1.0f,
        0.0f
    );
}

APawn* UAudio_DinosaurAudioComponent::FindNearestPlayer() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APawn* NearestPawn = nullptr;
    float NearestDist = FLT_MAX;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn() && GetOwner())
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PC->GetPawn()->GetActorLocation());
            if (Dist < NearestDist)
            {
                NearestDist = Dist;
                NearestPawn = PC->GetPawn();
            }
        }
    }

    return NearestPawn;
}

void UAudio_DinosaurAudioComponent::PropagateGroundShake()
{
    UWorld* World = GetWorld();
    if (!World || !GetOwner())
    {
        return;
    }

    const FVector DinoLocation = GetOwner()->GetActorLocation();

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !PC->GetPawn())
        {
            continue;
        }

        const float Dist = FVector::Dist(DinoLocation, PC->GetPawn()->GetActorLocation());
        if (Dist <= AudioProfile.GroundShakeRadius)
        {
            // Intensity falls off with distance squared
            const float Intensity = FMath::Clamp(
                1.0f - (Dist / AudioProfile.GroundShakeRadius),
                0.0f,
                1.0f
            );

            // Apply camera shake proportional to intensity
            // Scale: T-Rex at 0m = full shake, at GroundShakeRadius = no shake
            PC->ClientStartCameraShake(
                nullptr, // Camera shake class — assign in Blueprint
                Intensity
            );
        }
    }
}
