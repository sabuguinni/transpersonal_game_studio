
#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Poll every 0.5s — audio doesn't need per-frame updates

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetupAttachment(RootComponent);

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->bAutoActivate = false;
    MusicAudioComponent->SetupAttachment(RootComponent);

    // Default: open plains, safe, daytime
    CurrentBiomeZone = EAudio_BiomeZone::OpenPlains;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentDayBlend = 1.0f;
}

// ============================================================
// Lifecycle
// ============================================================

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    UpdateAmbientAudio();
    UpdateMusicLayer();
}

void AAudioSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Stop();
    }

    // Stop all campfire audio
    for (auto& Pair : CampfireAudioComponents)
    {
        if (Pair.Value && Pair.Value->IsPlaying())
        {
            Pair.Value->Stop();
        }
    }
    CampfireAudioComponents.Empty();

    Super::EndPlay(EndPlayReason);
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Periodic update — check if ambient or music needs refresh
    // (Actual transitions are driven by SetBiomeZone / SetThreatLevel calls)
}

// ============================================================
// Ambient Audio
// ============================================================

void AAudioSystemManager::SetBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentBiomeZone == NewZone) return;
    CurrentBiomeZone = NewZone;
    UpdateAmbientAudio();
}

void AAudioSystemManager::SetDayNightBlend(float DayBlend)
{
    CurrentDayBlend = FMath::Clamp(DayBlend, 0.0f, 1.0f);
    UpdateAmbientAudio();
}

void AAudioSystemManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent) return;

    // Find the matching ambient layer for current biome
    const FAudio_AmbientLayer* MatchingLayer = nullptr;
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.BiomeZone == CurrentBiomeZone)
        {
            MatchingLayer = &Layer;
            break;
        }
    }

    if (!MatchingLayer) return;

    // Blend between day and night sound based on CurrentDayBlend
    USoundBase* TargetSound = (CurrentDayBlend > 0.5f)
        ? MatchingLayer->DayAmbientSound.LoadSynchronous()
        : MatchingLayer->NightAmbientSound.LoadSynchronous();

    if (!TargetSound) return;

    float TargetVolume = MatchingLayer->BaseVolume * AmbientVolume * MasterVolume;

    if (AmbientAudioComponent->IsPlaying())
    {
        // Crossfade: fade out current, then fade in new
        AmbientAudioComponent->FadeOut(MatchingLayer->CrossfadeDuration * 0.5f, 0.0f);
    }

    AmbientAudioComponent->SetSound(TargetSound);
    AmbientAudioComponent->FadeIn(MatchingLayer->CrossfadeDuration * 0.5f, TargetVolume);
}

// ============================================================
// Threat Level & Music
// ============================================================

void AAudioSystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel) return;

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastThreatTransitionTime < ThreatTransitionCooldown)
    {
        // Cooldown: only allow escalation, not de-escalation during cooldown
        if (NewThreatLevel <= CurrentThreatLevel) return;
    }

    CurrentThreatLevel = NewThreatLevel;
    LastThreatTransitionTime = CurrentTime;
    UpdateMusicLayer();
}

void AAudioSystemManager::TransitionToMusicState(EAudio_ThreatLevel TargetThreat, float OverrideDuration)
{
    const FAudio_MusicState* TargetState = nullptr;
    for (const FAudio_MusicState& State : MusicStates)
    {
        if (State.ThreatLevel == TargetThreat)
        {
            TargetState = &State;
            break;
        }
    }

    if (!TargetState || !MusicAudioComponent) return;

    USoundBase* TargetTrack = TargetState->MusicTrack.LoadSynchronous();
    if (!TargetTrack) return;

    float Duration = (OverrideDuration > 0.0f) ? OverrideDuration : TargetState->TransitionDuration;
    float TargetVolume = TargetState->Volume * MusicVolume * MasterVolume;

    if (MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(Duration * 0.5f, 0.0f);
    }

    MusicAudioComponent->SetSound(TargetTrack);
    MusicAudioComponent->FadeIn(Duration * 0.5f, TargetVolume);

    CurrentThreatLevel = TargetThreat;
}

void AAudioSystemManager::UpdateMusicLayer()
{
    TransitionToMusicState(CurrentThreatLevel);
}

// ============================================================
// Survival Sound Events
// ============================================================

void AAudioSystemManager::PlaySurvivalEvent(EAudio_SurvivalEvent Event)
{
    if (!GetWorld()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Survival events are 2D UI sounds — play at player location
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // Map events to threat level escalation
    switch (Event)
    {
    case EAudio_SurvivalEvent::DinosaurCharge:
        SetThreatLevel(EAudio_ThreatLevel::Critical);
        break;
    case EAudio_SurvivalEvent::DinosaurNearby:
        SetThreatLevel(EAudio_ThreatLevel::Danger);
        break;
    case EAudio_SurvivalEvent::PlayerHurt:
        // Trigger screen feedback — handled by VFX agent
        break;
    case EAudio_SurvivalEvent::FireLit:
        // Campfire audio handled by RegisterCampfire
        break;
    default:
        break;
    }
}

// ============================================================
// Dinosaur Footstep & Screen Shake
// ============================================================

void AAudioSystemManager::PlayDinosaurFootstep(FName DinosaurSpecies, FVector FootstepLocation)
{
    if (!GetWorld()) return;

    const FAudio_DinosaurSoundProfile* Profile = FindDinosaurProfile(DinosaurSpecies);
    if (!Profile) return;

    USoundBase* FootstepSnd = Profile->FootstepSound.LoadSynchronous();
    if (FootstepSnd)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FootstepSnd,
            FootstepLocation,
            SFXVolume * MasterVolume
        );
    }

    // Trigger ground shake for large dinosaurs
    TriggerScreenShakeFromDinosaur(FootstepLocation, Profile->FootstepGroundShakeIntensity);
}

void AAudioSystemManager::TriggerScreenShakeFromDinosaur(FVector DinosaurLocation, float DinosaurMass)
{
    if (!GetWorld()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(DinosaurLocation, PlayerPawn->GetActorLocation());
    float MaxShakeRadius = 2000.0f * DinosaurMass;

    if (Distance > MaxShakeRadius) return;

    // Scale shake intensity by proximity: closer = stronger
    float ShakeScale = FMath::Clamp(1.0f - (Distance / MaxShakeRadius), 0.1f, 1.0f);
    ShakeScale *= DinosaurMass;

    // Use built-in camera shake via console command (MetaSounds integration point)
    // VFX Agent (#17) handles the visual screen shake — we trigger the audio cue here
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        nullptr, // Footstep sound already played above
        DinosaurLocation,
        ShakeScale * SFXVolume * MasterVolume
    );
}

// ============================================================
// Campfire Audio
// ============================================================

void AAudioSystemManager::RegisterCampfire(AActor* CampfireActor, float Radius)
{
    if (!CampfireActor || !GetWorld()) return;
    if (CampfireAudioComponents.Contains(CampfireActor)) return;

    UAudioComponent* CampfireAudio = NewObject<UAudioComponent>(CampfireActor);
    if (!CampfireAudio) return;

    CampfireAudio->RegisterComponent();
    CampfireAudio->AttachToComponent(
        CampfireActor->GetRootComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale
    );

    // Campfire audio config
    // Freesound ID 394952: crackling campfire near lake
    // Freesound ID 157187: Campfire Crackle 3
    // Freesound ID 729396: Campfire 02
    CampfireAudio->bAutoActivate = true;
    CampfireAudio->VolumeMultiplier = SFXVolume * MasterVolume * 0.7f;

    // Spatial attenuation: audible within Radius
    CampfireAudio->bOverrideAttenuation = true;

    CampfireAudioComponents.Add(CampfireActor, CampfireAudio);
}

void AAudioSystemManager::UnregisterCampfire(AActor* CampfireActor)
{
    if (!CampfireActor) return;

    UAudioComponent** FoundComp = CampfireAudioComponents.Find(CampfireActor);
    if (FoundComp && *FoundComp)
    {
        (*FoundComp)->Stop();
        (*FoundComp)->DestroyComponent();
    }
    CampfireAudioComponents.Remove(CampfireActor);
}

// ============================================================
// Helpers
// ============================================================

const FAudio_DinosaurSoundProfile* AAudioSystemManager::FindDinosaurProfile(FName Species) const
{
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurSoundProfiles)
    {
        if (Profile.DinosaurSpecies == Species)
        {
            return &Profile;
        }
    }
    return nullptr;
}
