// DinosaurAudioComponent.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260629_002
// Full implementation of adaptive dinosaur audio system

#include "DinosaurAudioComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ────────────────────────────────────────────────────────────

UDinosaurAudioComponent::UDinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio updates

    // Default sound profile (medium dinosaur)
    SoundProfile.Species = EAudio_DinoSpecies::Unknown;
    SoundProfile.FootstepWeight = EAudio_FootstepWeight::Medium;
    SoundProfile.RoarRadius = 5000.0f;
    SoundProfile.FootstepInterval = 0.8f;
    SoundProfile.GroundShakeRadius = 2000.0f;
    SoundProfile.BreathingRate = 12.0f;
    SoundProfile.VolumeMultiplier = 1.0f;
}

// ─── BeginPlay ──────────────────────────────────────────────────────────────

void UDinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache player pawn reference
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        CachedPlayerPawn = PC->GetPawn();
    }

    // Apply species defaults if not manually configured
    if (SoundProfile.Species != EAudio_DinoSpecies::Unknown)
    {
        FAudio_DinoSoundProfile Defaults = GetDefaultProfileForSpecies(SoundProfile.Species);
        // Only apply defaults if values are still at generic defaults
        if (SoundProfile.FootstepWeight == EAudio_FootstepWeight::Medium)
        {
            SoundProfile.FootstepWeight = Defaults.FootstepWeight;
        }
        if (SoundProfile.RoarRadius == 5000.0f)
        {
            SoundProfile.RoarRadius = Defaults.RoarRadius;
        }
        SoundProfile.GroundShakeRadius = Defaults.GroundShakeRadius;
        SoundProfile.BreathingRate = Defaults.BreathingRate;
        SoundProfile.VolumeMultiplier = Defaults.VolumeMultiplier;
    }

    // Start breathing audio if asset assigned
    if (BreathingSound)
    {
        BreathingAudioComp = UGameplayStatics::SpawnSoundAttached(
            BreathingSound,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            false,
            SoundProfile.VolumeMultiplier * 0.4f, // Breathing is quiet
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true
        );
    }
}

// ─── TickComponent ──────────────────────────────────────────────────────────

void UDinosaurAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Advance timers
    TimeSinceLastRoar += DeltaTime;
    TimeSinceLastFootstep += DeltaTime;
    FootstepCooldown = FMath::Max(0.0f, FootstepCooldown - DeltaTime);

    // Update player proximity
    UpdatePlayerDistance();

    // Update breathing audio volume based on player distance
    if (BreathingAudioComp)
    {
        bool bInRange = DistanceToPlayer < BreathingAudibleRadius;
        if (bInRange != bPlayerInBreathingRange)
        {
            bPlayerInBreathingRange = bInRange;
            float TargetVolume = bInRange ? (SoundProfile.VolumeMultiplier * 0.6f) : 0.0f;
            BreathingAudioComp->SetVolumeMultiplier(TargetVolume);
        }
    }

    // Auto-footstep when moving (state-based)
    if (CurrentState == EAudio_DinoState::Patrolling ||
        CurrentState == EAudio_DinoState::Hunting ||
        CurrentState == EAudio_DinoState::Attacking)
    {
        if (TimeSinceLastFootstep >= SoundProfile.FootstepInterval && FootstepCooldown <= 0.0f)
        {
            TriggerFootstep();
        }
    }

    // Auto-roar when hunting and player is in range
    if (CurrentState == EAudio_DinoState::Hunting && DistanceToPlayer < SoundProfile.RoarRadius)
    {
        if (TimeSinceLastRoar >= RoarCooldown)
        {
            TriggerRoar();
        }
    }
}

// ─── SetDinoState ───────────────────────────────────────────────────────────

void UDinosaurAudioComponent::SetDinoState(EAudio_DinoState NewState)
{
    if (NewState == CurrentState) return;

    EAudio_DinoState OldState = CurrentState;
    CurrentState = NewState;
    ApplyStateAudio(OldState, NewState);
}

// ─── TriggerRoar ────────────────────────────────────────────────────────────

void UDinosaurAudioComponent::TriggerRoar()
{
    if (!RoarSound) return;
    if (!GetOwner()) return;

    TimeSinceLastRoar = 0.0f;

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        RoarSound,
        GetOwner()->GetActorLocation(),
        SoundProfile.VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr
    );

    // Heavy species trigger ground shake on roar
    if (SoundProfile.FootstepWeight == EAudio_FootstepWeight::Heavy ||
        SoundProfile.FootstepWeight == EAudio_FootstepWeight::Massive)
    {
        TriggerGroundShake(0.5f);
    }
}

// ─── TriggerFootstep ────────────────────────────────────────────────────────

void UDinosaurAudioComponent::TriggerFootstep()
{
    if (!FootstepSound) return;
    if (!GetOwner()) return;

    TimeSinceLastFootstep = 0.0f;
    FootstepCooldown = SoundProfile.FootstepInterval;

    float VolumeScale = SoundProfile.VolumeMultiplier;

    // Heavy footsteps are louder and carry further
    switch (SoundProfile.FootstepWeight)
    {
        case EAudio_FootstepWeight::Light:   VolumeScale *= 0.5f; break;
        case EAudio_FootstepWeight::Medium:  VolumeScale *= 0.8f; break;
        case EAudio_FootstepWeight::Heavy:   VolumeScale *= 1.2f; break;
        case EAudio_FootstepWeight::Massive: VolumeScale *= 1.8f; break;
    }

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        FootstepSound,
        GetOwner()->GetActorLocation(),
        VolumeScale,
        FMath::RandRange(0.9f, 1.1f), // Slight pitch variation for naturalness
        0.0f,
        nullptr,
        nullptr
    );

    // Ground shake for heavy/massive dinosaurs within range
    if (DistanceToPlayer < FootstepShakeRadius)
    {
        float ShakeIntensity = 0.0f;
        switch (SoundProfile.FootstepWeight)
        {
            case EAudio_FootstepWeight::Heavy:   ShakeIntensity = 0.3f; break;
            case EAudio_FootstepWeight::Massive: ShakeIntensity = 0.7f; break;
            default: break;
        }
        if (ShakeIntensity > 0.0f)
        {
            TriggerGroundShake(ShakeIntensity);
        }
    }
}

// ─── TriggerAlert ───────────────────────────────────────────────────────────

void UDinosaurAudioComponent::TriggerAlert()
{
    if (!AlertSound) return;
    if (!GetOwner()) return;

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        AlertSound,
        GetOwner()->GetActorLocation(),
        SoundProfile.VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr
    );
}

// ─── SetSpecies ─────────────────────────────────────────────────────────────

void UDinosaurAudioComponent::SetSpecies(EAudio_DinoSpecies Species)
{
    SoundProfile.Species = Species;
    SoundProfile = GetDefaultProfileForSpecies(Species);
}

// ─── GetThreatLevelForPlayer ────────────────────────────────────────────────

float UDinosaurAudioComponent::GetThreatLevelForPlayer() const
{
    if (CurrentState == EAudio_DinoState::Idle || CurrentState == EAudio_DinoState::Sleeping)
    {
        return 0.0f;
    }

    float BaseThreat = 0.0f;
    switch (CurrentState)
    {
        case EAudio_DinoState::Patrolling: BaseThreat = 0.1f; break;
        case EAudio_DinoState::Alerted:    BaseThreat = 0.4f; break;
        case EAudio_DinoState::Hunting:    BaseThreat = 0.8f; break;
        case EAudio_DinoState::Attacking:  BaseThreat = 1.0f; break;
        case EAudio_DinoState::Fleeing:    BaseThreat = 0.2f; break;
        case EAudio_DinoState::Feeding:    BaseThreat = 0.3f; break;
        default: break;
    }

    // Scale by distance — closer = more threatening
    float DistanceFactor = FMath::Clamp(1.0f - (DistanceToPlayer / SoundProfile.RoarRadius), 0.0f, 1.0f);
    return BaseThreat * DistanceFactor;
}

// ─── IsPlayerInDanger ───────────────────────────────────────────────────────

bool UDinosaurAudioComponent::IsPlayerInDanger() const
{
    return GetThreatLevelForPlayer() > 0.6f;
}

// ─── GetDefaultProfileForSpecies ────────────────────────────────────────────

FAudio_DinoSoundProfile UDinosaurAudioComponent::GetDefaultProfileForSpecies(EAudio_DinoSpecies Species)
{
    FAudio_DinoSoundProfile Profile;
    Profile.Species = Species;

    switch (Species)
    {
        case EAudio_DinoSpecies::TRex:
            Profile.FootstepWeight = EAudio_FootstepWeight::Massive;
            Profile.RoarRadius = 8000.0f;
            Profile.FootstepInterval = 1.2f;
            Profile.GroundShakeRadius = 3000.0f;
            Profile.BreathingRate = 8.0f;
            Profile.VolumeMultiplier = 1.8f;
            break;

        case EAudio_DinoSpecies::Raptor:
            Profile.FootstepWeight = EAudio_FootstepWeight::Light;
            Profile.RoarRadius = 3000.0f;
            Profile.FootstepInterval = 0.3f; // Fast, frequent steps
            Profile.GroundShakeRadius = 0.0f; // No shake
            Profile.BreathingRate = 20.0f;
            Profile.VolumeMultiplier = 0.8f;
            break;

        case EAudio_DinoSpecies::Brachiosaurus:
            Profile.FootstepWeight = EAudio_FootstepWeight::Massive;
            Profile.RoarRadius = 6000.0f;
            Profile.FootstepInterval = 1.8f; // Slow, deliberate
            Profile.GroundShakeRadius = 4000.0f;
            Profile.BreathingRate = 6.0f;
            Profile.VolumeMultiplier = 1.5f;
            break;

        case EAudio_DinoSpecies::Triceratops:
            Profile.FootstepWeight = EAudio_FootstepWeight::Heavy;
            Profile.RoarRadius = 4000.0f;
            Profile.FootstepInterval = 0.9f;
            Profile.GroundShakeRadius = 1500.0f;
            Profile.BreathingRate = 10.0f;
            Profile.VolumeMultiplier = 1.2f;
            break;

        case EAudio_DinoSpecies::Pterodactyl:
            Profile.FootstepWeight = EAudio_FootstepWeight::Light;
            Profile.RoarRadius = 5000.0f; // Screech carries far
            Profile.FootstepInterval = 0.5f;
            Profile.GroundShakeRadius = 0.0f;
            Profile.BreathingRate = 24.0f;
            Profile.VolumeMultiplier = 1.0f;
            break;

        default:
            // Generic medium dinosaur defaults
            Profile.FootstepWeight = EAudio_FootstepWeight::Medium;
            Profile.RoarRadius = 5000.0f;
            Profile.FootstepInterval = 0.8f;
            Profile.GroundShakeRadius = 2000.0f;
            Profile.BreathingRate = 12.0f;
            Profile.VolumeMultiplier = 1.0f;
            break;
    }

    return Profile;
}

// ─── UpdatePlayerDistance ───────────────────────────────────────────────────

void UDinosaurAudioComponent::UpdatePlayerDistance()
{
    if (!CachedPlayerPawn)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC) CachedPlayerPawn = PC->GetPawn();
    }

    if (CachedPlayerPawn && GetOwner())
    {
        DistanceToPlayer = FVector::Dist(
            GetOwner()->GetActorLocation(),
            CachedPlayerPawn->GetActorLocation()
        );
    }
}

// ─── ApplyStateAudio ────────────────────────────────────────────────────────

void UDinosaurAudioComponent::ApplyStateAudio(EAudio_DinoState OldState, EAudio_DinoState NewState)
{
    // Trigger alert sound when transitioning to Alerted or Hunting
    if ((NewState == EAudio_DinoState::Alerted || NewState == EAudio_DinoState::Hunting) &&
        OldState == EAudio_DinoState::Idle)
    {
        TriggerAlert();
    }

    // Trigger roar when beginning attack
    if (NewState == EAudio_DinoState::Attacking)
    {
        TriggerRoar();
    }

    // Adjust breathing rate based on state
    if (BreathingAudioComp)
    {
        float PitchMod = 1.0f;
        switch (NewState)
        {
            case EAudio_DinoState::Hunting:   PitchMod = 1.3f; break;
            case EAudio_DinoState::Attacking: PitchMod = 1.6f; break;
            case EAudio_DinoState::Fleeing:   PitchMod = 1.5f; break;
            case EAudio_DinoState::Sleeping:  PitchMod = 0.5f; break;
            default: PitchMod = 1.0f; break;
        }
        BreathingAudioComp->SetPitchMultiplier(PitchMod);
    }
}

// ─── TriggerGroundShake ─────────────────────────────────────────────────────

void UDinosaurAudioComponent::TriggerGroundShake(float Intensity)
{
    // Camera shake is triggered via player controller
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Only shake if player is within range
    if (DistanceToPlayer > SoundProfile.GroundShakeRadius) return;

    // Scale shake by distance — closer = stronger
    float DistanceFactor = 1.0f - (DistanceToPlayer / SoundProfile.GroundShakeRadius);
    float FinalIntensity = Intensity * DistanceFactor;

    // Use built-in camera shake via console command as fallback
    // (Blueprint camera shake class would be assigned in BP subclass)
    if (FinalIntensity > 0.1f)
    {
        FString ShakeCmd = FString::Printf(
            TEXT("ce PlayCameraShake intensity=%f"),
            FinalIntensity
        );
        PC->ConsoleCommand(*ShakeCmd, false);
    }
}
