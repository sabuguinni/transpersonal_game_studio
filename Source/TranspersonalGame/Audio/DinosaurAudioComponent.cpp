// DinosaurAudioComponent.cpp
// Audio Agent #16 — PROD_CYCLE_AUTO_20260628_005
// Dinosaur audio system: state-driven sound playback with distance attenuation
// Species-specific footstep intervals, idle breathing, roars, and death sounds

#include "DinosaurAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"

UDinosaurAudioComponent::UDinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Species-specific footstep intervals (larger = slower steps)
    // Defaults for generic — overridden in BeginPlay based on Species
    FootstepInterval = 0.6f;
    IdleBreathInterval = 4.0f;
    MaxHearingDistance = 5000.0f;
}

void UDinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Adjust timing based on species
    switch (Species)
    {
        case EAudio_DinoSpecies::TRex:
            FootstepInterval = 1.2f;   // Slow, heavy
            IdleBreathInterval = 5.0f;
            MaxHearingDistance = 8000.0f;
            break;
        case EAudio_DinoSpecies::Raptor:
            FootstepInterval = 0.35f;  // Fast, light
            IdleBreathInterval = 2.5f;
            MaxHearingDistance = 3000.0f;
            break;
        case EAudio_DinoSpecies::Triceratops:
            FootstepInterval = 0.9f;
            IdleBreathInterval = 4.5f;
            MaxHearingDistance = 4000.0f;
            break;
        case EAudio_DinoSpecies::Brachiosaurus:
            FootstepInterval = 1.8f;   // Very slow, massive
            IdleBreathInterval = 6.0f;
            MaxHearingDistance = 10000.0f;
            break;
        case EAudio_DinoSpecies::Pterodactyl:
            FootstepInterval = 0.4f;
            IdleBreathInterval = 3.0f;
            MaxHearingDistance = 6000.0f;
            break;
        default:
            break;
    }

    // Stagger timers so all dinos don't sync up
    FootstepTimer = FMath::RandRange(0.0f, FootstepInterval);
    BreathTimer = FMath::RandRange(0.0f, IdleBreathInterval);
}

void UDinosaurAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only process audio if player is within hearing range
    if (PlayerDistanceMeters > MaxHearingDistance)
    {
        return;
    }

    // Tick idle audio (breathing + footsteps when moving)
    TickIdleAudio(DeltaTime);
}

void UDinosaurAudioComponent::TickIdleAudio(float DeltaTime)
{
    // Breathing — only when idle or alert
    if (CurrentState == EAudio_DinoState::Idle || CurrentState == EAudio_DinoState::Alert)
    {
        BreathTimer -= DeltaTime;
        if (BreathTimer <= 0.0f)
        {
            if (SoundSet.IdleBreath)
            {
                float VolMult = FMath::GetMappedRangeValueClamped(
                    FVector2D(0.0f, MaxHearingDistance),
                    FVector2D(1.0f, 0.1f),
                    PlayerDistanceMeters
                );
                PlaySoundAtOwner(SoundSet.IdleBreath, VolMult);
            }
            BreathTimer = IdleBreathInterval + FMath::RandRange(-0.5f, 0.5f);
        }
    }

    // Footsteps — only when moving (aggressive or fleeing)
    if (CurrentState == EAudio_DinoState::Aggressive || CurrentState == EAudio_DinoState::Fleeing)
    {
        FootstepTimer -= DeltaTime;
        if (FootstepTimer <= 0.0f)
        {
            PlayFootstep();
            FootstepTimer = FootstepInterval;
        }
    }
}

void UDinosaurAudioComponent::SetDinoState(EAudio_DinoState NewState)
{
    if (CurrentState == NewState) return;

    EAudio_DinoState PrevState = CurrentState;
    CurrentState = NewState;

    // Trigger state-change sounds
    switch (NewState)
    {
        case EAudio_DinoState::Alert:
            PlayAlert();
            break;
        case EAudio_DinoState::Aggressive:
            PlayRoar();
            break;
        case EAudio_DinoState::Dead:
            PlayDeath();
            break;
        case EAudio_DinoState::Feeding:
            if (SoundSet.Feeding)
            {
                PlaySoundAtOwner(SoundSet.Feeding, 0.8f);
            }
            break;
        default:
            break;
    }
}

void UDinosaurAudioComponent::SetDinoSpecies(EAudio_DinoSpecies NewSpecies)
{
    Species = NewSpecies;
    // Re-run BeginPlay timing adjustments
    BeginPlay();
}

void UDinosaurAudioComponent::PlayRoar()
{
    if (!SoundSet.Roar) return;

    // Roars are always loud — ignore distance attenuation partially
    float VolMult = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, MaxHearingDistance),
        FVector2D(1.5f, 0.4f),
        PlayerDistanceMeters
    );
    PlaySoundAtOwner(SoundSet.Roar, VolMult);

    // Trigger ground rumble for large species
    if (Species == EAudio_DinoSpecies::TRex || Species == EAudio_DinoSpecies::Brachiosaurus)
    {
        float RumbleIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 3000.0f),
            FVector2D(1.0f, 0.0f),
            PlayerDistanceMeters
        );
        TriggerGroundRumble(RumbleIntensity);
    }
}

void UDinosaurAudioComponent::PlayFootstep()
{
    if (!SoundSet.Footstep) return;

    float VolMult = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, MaxHearingDistance),
        FVector2D(1.2f, 0.05f),
        PlayerDistanceMeters
    );

    // T-Rex footsteps trigger rumble at close range
    if (Species == EAudio_DinoSpecies::TRex && PlayerDistanceMeters < 1500.0f)
    {
        float RumbleIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 1500.0f),
            FVector2D(0.8f, 0.0f),
            PlayerDistanceMeters
        );
        TriggerGroundRumble(RumbleIntensity * 0.5f);
    }

    PlaySoundAtOwner(SoundSet.Footstep, VolMult);
}

void UDinosaurAudioComponent::PlayDeath()
{
    if (!SoundSet.Death) return;
    PlaySoundAtOwner(SoundSet.Death, 1.2f);
}

void UDinosaurAudioComponent::PlayAlert()
{
    if (!SoundSet.AlertCall) return;

    float VolMult = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, MaxHearingDistance),
        FVector2D(1.0f, 0.2f),
        PlayerDistanceMeters
    );
    PlaySoundAtOwner(SoundSet.AlertCall, VolMult);
}

void UDinosaurAudioComponent::UpdatePlayerDistance(float DistanceMeters)
{
    PlayerDistanceMeters = DistanceMeters;
}

void UDinosaurAudioComponent::TriggerGroundRumble(float Intensity)
{
    if (Intensity <= 0.01f) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    // Get the player controller to apply camera shake
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Apply camera shake proportional to intensity
    // The shake class would be set in Blueprint — here we just log the trigger
    // In full implementation: PC->ClientStartCameraShake(ShakeClass, Intensity);
    // For now, use console rumble command as fallback
    FString RumbleCmd = FString::Printf(TEXT("r.MotionBlurAmount %f"), Intensity * 0.3f);
    // Note: actual camera shake requires a UCameraShakeBase subclass assigned in BP
}

void UDinosaurAudioComponent::PlaySoundAtOwner(USoundCue* Cue, float VolumeMultiplier)
{
    if (!Cue) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UGameplayStatics::PlaySoundAtLocation(
        Owner,
        Cue,
        Owner->GetActorLocation(),
        VolumeMultiplier,
        1.0f,   // PitchMultiplier
        0.0f,   // StartTime
        nullptr, // AttenuationSettings — use Cue's built-in
        nullptr  // Concurrency
    );
}
