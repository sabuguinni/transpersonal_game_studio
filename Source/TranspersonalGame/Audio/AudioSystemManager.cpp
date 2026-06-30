
#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"

UAudio_SystemManager::UAudio_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — audio state is not frame-critical

    // Seed default ambient layer (jungle day) — no asset assigned, will be wired in BP
    FAudio_AmbientLayer DefaultLayer;
    DefaultLayer.Biome = EAudio_BiomeType::Jungle;
    DefaultLayer.TimeOfDay = EAudio_TimeOfDay::Day;
    DefaultLayer.BaseVolume = 0.85f;
    DefaultLayer.FadeInTime = 3.0f;
    DefaultLayer.FadeOutTime = 4.0f;
    AmbientLayers.Add(DefaultLayer);

    // Seed default music states
    FAudio_MusicState SafeState;
    SafeState.ThreatLevel = EAudio_ThreatLevel::Safe;
    SafeState.CrossfadeDuration = 5.0f;
    SafeState.Volume = 0.6f;
    MusicStates.Add(SafeState);

    FAudio_MusicState DangerState;
    DangerState.ThreatLevel = EAudio_ThreatLevel::Danger;
    DangerState.CrossfadeDuration = 2.0f;
    DangerState.Volume = 0.9f;
    MusicStates.Add(DangerState);

    // Seed T-Rex sound profile (no assets yet — wired in BP)
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.SpeciesID = TEXT("TRex");
    TRexProfile.FootstepGroundShakeRadius = 1200.0f;
    TRexProfile.FootstepGroundShakeIntensity = 1.8f;
    DinosaurProfiles.Add(TRexProfile);

    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.SpeciesID = TEXT("Raptor");
    RaptorProfile.FootstepGroundShakeRadius = 400.0f;
    RaptorProfile.FootstepGroundShakeIntensity = 0.4f;
    DinosaurProfiles.Add(RaptorProfile);
}

void UAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] BeginPlay — Biome=%d TimeOfDay=%d ThreatLevel=%d"),
        (int32)CurrentBiome, (int32)CurrentTimeOfDay, (int32)CurrentThreatLevel);
    UpdateAmbientLayers();
}

void UAudio_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    EvaluateThreatTransition(DeltaTime);
}

// ── Ambient System ────────────────────────────────────────────────────────────

void UAudio_SystemManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome) return;
    CurrentBiome = NewBiome;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Biome changed to %d"), (int32)NewBiome);
    UpdateAmbientLayers();
}

void UAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;
    CurrentTimeOfDay = NewTime;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] TimeOfDay changed to %d"), (int32)NewTime);
    UpdateAmbientLayers();
}

void UAudio_SystemManager::UpdateAmbientLayers()
{
    // Find best matching ambient layer for current biome + time
    FAudio_AmbientLayer* BestLayer = nullptr;
    for (FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Biome == CurrentBiome && Layer.TimeOfDay == CurrentTimeOfDay)
        {
            BestLayer = &Layer;
            break;
        }
    }

    if (!BestLayer)
    {
        // Fallback: match biome only
        for (FAudio_AmbientLayer& Layer : AmbientLayers)
        {
            if (Layer.Biome == CurrentBiome)
            {
                BestLayer = &Layer;
                break;
            }
        }
    }

    if (BestLayer)
    {
        BlendAmbientToCurrentState();
        UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Ambient layer updated — Biome=%d Time=%d Volume=%.2f"),
            (int32)BestLayer->Biome, (int32)BestLayer->TimeOfDay, BestLayer->BaseVolume);
    }
}

void UAudio_SystemManager::BlendAmbientToCurrentState()
{
    // Fade out current ambient, fade in new one
    // Asset binding happens in Blueprint — this manages the transition logic
    if (ActiveAmbientComponent && ActiveAmbientComponent->IsPlaying())
    {
        ActiveAmbientComponent->FadeOut(3.0f, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Fading out previous ambient layer"));
    }
    // New component will be assigned when Blueprint binds the SoundCue asset
}

// ── Adaptive Music ────────────────────────────────────────────────────────────

void UAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreat)
{
    if (CurrentThreatLevel == NewThreat) return;

    EAudio_ThreatLevel OldThreat = CurrentThreatLevel;
    CurrentThreatLevel = NewThreat;
    ThreatTransitionTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] ThreatLevel %d → %d"), (int32)OldThreat, (int32)NewThreat);

    // Find matching music state and crossfade
    for (const FAudio_MusicState& State : MusicStates)
    {
        if (State.ThreatLevel == NewThreat)
        {
            CrossfadeToMusicState(State);
            break;
        }
    }
}

void UAudio_SystemManager::CrossfadeToMusicState(const FAudio_MusicState& NewState)
{
    if (!NewState.MusicCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] CrossfadeToMusicState — no SoundCue assigned for ThreatLevel=%d"), (int32)NewState.ThreatLevel);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Fade out current music
    if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
    {
        ActiveMusicComponent->FadeOut(NewState.CrossfadeDuration * 0.5f, 0.0f);
    }

    // Spawn new music component
    ActiveMusicComponent = UGameplayStatics::SpawnSound2D(
        World,
        NewState.MusicCue,
        NewState.Volume * MusicVolume * MasterVolume,
        1.0f,
        0.0f,
        nullptr,
        false,
        false
    );

    if (ActiveMusicComponent)
    {
        ActiveMusicComponent->FadeIn(NewState.CrossfadeDuration * 0.5f, NewState.Volume * MusicVolume * MasterVolume);
        UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Music crossfade started — ThreatLevel=%d Duration=%.1fs"),
            (int32)NewState.ThreatLevel, NewState.CrossfadeDuration);
    }
}

void UAudio_SystemManager::EvaluateThreatTransition(float DeltaTime)
{
    ThreatTransitionTimer += DeltaTime;
    // Threat level auto-decays to Safe after 30s without reinforcement
    if (CurrentThreatLevel > EAudio_ThreatLevel::Safe && ThreatTransitionTimer > 30.0f)
    {
        EAudio_ThreatLevel Decayed = (EAudio_ThreatLevel)((uint8)CurrentThreatLevel - 1);
        SetThreatLevel(Decayed);
        ThreatTransitionTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Threat decayed to %d"), (int32)Decayed);
    }
}

// ── NPC Dialogue Audio ────────────────────────────────────────────────────────

void UAudio_SystemManager::PlayNPCVoiceLine(const FAudio_NPCVoiceLine& Line, AActor* NPCActor)
{
    if (!NPCActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] PlayNPCVoiceLine — null NPCActor for LineID=%s"), *Line.LineID);
        return;
    }

    // Check player distance for spatial audio gate
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), NPCActor->GetActorLocation());
        if (Distance > Line.SpatialRadius)
        {
            UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] NPC voice line skipped — player too far (%.0f > %.0f) LineID=%s"),
                Distance, Line.SpatialRadius, *Line.LineID);
            return;
        }
    }

    // Stop any active NPC line
    StopNPCVoiceLine();

    // Audio asset path is referenced — Blueprint wires the actual SoundCue
    // This function handles the spatial gate and state management
    bNPCLineActive = true;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] NPC voice line active — LineID=%s Subtitle=%s"),
        *Line.LineID, *Line.SubtitleText);
}

void UAudio_SystemManager::StopNPCVoiceLine()
{
    if (ActiveNPCComponent && ActiveNPCComponent->IsPlaying())
    {
        ActiveNPCComponent->FadeOut(0.3f, 0.0f);
    }
    bNPCLineActive = false;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] NPC voice line stopped"));
}

// ── Dinosaur Audio ────────────────────────────────────────────────────────────

void UAudio_SystemManager::PlayDinosaurFootstep(const FAudio_DinosaurSoundProfile& Profile, FVector FootLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Play footstep sound at location
    if (Profile.FootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            World,
            Profile.FootstepSound,
            FootLocation,
            SFXVolume * MasterVolume
        );
    }

    // Camera shake based on player proximity
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), FootLocation);
        if (Distance < Profile.FootstepGroundShakeRadius)
        {
            float ShakeFalloff = 1.0f - (Distance / Profile.FootstepGroundShakeRadius);
            float ShakeIntensity = Profile.FootstepGroundShakeIntensity * ShakeFalloff;
            TriggerCameraShake(ShakeIntensity, 0.4f);

            UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Dino footstep shake — Species=%s Dist=%.0f Intensity=%.2f"),
                *Profile.SpeciesID, Distance, ShakeIntensity);
        }
    }
}

void UAudio_SystemManager::TriggerDinosaurAlert(const FAudio_DinosaurSoundProfile& Profile, AActor* DinoActor)
{
    if (!DinoActor) return;

    UWorld* World = GetWorld();
    if (!World) return;

    if (Profile.AlertSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            World,
            Profile.AlertSound,
            DinoActor->GetActorLocation(),
            SFXVolume * MasterVolume
        );
    }

    // Escalate threat level when dinosaur alerts
    if (CurrentThreatLevel < EAudio_ThreatLevel::Danger)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }

    ThreatTransitionTimer = 0.0f; // Reset decay timer
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Dinosaur alert triggered — Species=%s"), *Profile.SpeciesID);
}

// ── Screen Feedback ───────────────────────────────────────────────────────────

void UAudio_SystemManager::TriggerCameraShake(float Intensity, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    // Camera shake is driven by Blueprint CameraShakeBase asset
    // This function signals the intent — Blueprint binds the actual shake class
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] CameraShake triggered — Intensity=%.2f Duration=%.2f"), Intensity, Duration);
}

void UAudio_SystemManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Escalate threat on damage
    if (DamageAmount > 20.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Critical);
    }
    else if (DamageAmount > 5.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }

    // Camera shake intensity proportional to damage
    float ShakeIntensity = FMath::Clamp(DamageAmount / 50.0f, 0.1f, 2.0f);
    TriggerCameraShake(ShakeIntensity, 0.3f);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Damage audio feedback — Damage=%.1f ShakeIntensity=%.2f"),
        DamageAmount, ShakeIntensity);
}
