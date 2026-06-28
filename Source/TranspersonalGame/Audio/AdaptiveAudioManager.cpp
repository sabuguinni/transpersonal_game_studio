#include "Audio/AdaptiveAudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Constructor
// ============================================================

UAudio_AdaptiveAudioManager::UAudio_AdaptiveAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for audio fades
}

// ============================================================
// BeginPlay
// ============================================================

void UAudio_AdaptiveAudioManager::BeginPlay()
{
    Super::BeginPlay();
    PopulateDefaultSoundEvents();
    SelectMusicForState();
}

// ============================================================
// TickComponent
// ============================================================

void UAudio_AdaptiveAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAmbientLayers(DeltaTime);
    UpdateMusicFade(DeltaTime);

    // Damage flash timer
    if (bDamageFlashActive)
    {
        DamageFlashTimer -= DeltaTime;
        if (DamageFlashTimer <= 0.0f)
        {
            bDamageFlashActive = false;
            DamageFlashTimer = 0.0f;
        }
    }
}

// ============================================================
// State setters
// ============================================================

void UAudio_AdaptiveAudioManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel) return;
    CurrentThreatLevel = NewLevel;
    OnThreatLevelChanged.Broadcast(NewLevel);
    SelectMusicForState();
}

void UAudio_AdaptiveAudioManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime == CurrentTimeOfDay) return;
    CurrentTimeOfDay = NewTime;
    OnTimeOfDayChanged.Broadcast(NewTime);
}

void UAudio_AdaptiveAudioManager::SetWeather(EAudio_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
}

// ============================================================
// Sound events
// ============================================================

void UAudio_AdaptiveAudioManager::FireSoundEvent(FName EventID, FVector WorldLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FAudio_SoundEvent& Event : SoundEventLibrary)
    {
        if (Event.EventID == EventID)
        {
            USoundBase* Sound = Event.SoundAsset.LoadSynchronous();
            if (Sound)
            {
                float FinalVolume = Event.Volume * SFXVolume * MasterVolume;
                if (Event.bIs3D)
                {
                    UGameplayStatics::PlaySoundAtLocation(World, Sound, WorldLocation, FinalVolume, Event.Pitch);
                }
                else
                {
                    UGameplayStatics::PlaySound2D(World, Sound, FinalVolume, Event.Pitch);
                }
            }
            OnSoundEventFired.Broadcast(EventID, WorldLocation);
            return;
        }
    }
}

void UAudio_AdaptiveAudioManager::FireSoundEventAtActor(FName EventID, AActor* SourceActor)
{
    if (!SourceActor) return;
    FireSoundEvent(EventID, SourceActor->GetActorLocation());
}

// ============================================================
// Music control
// ============================================================

void UAudio_AdaptiveAudioManager::ForcePlayMusicCue(FName CueID)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FAudio_MusicCue& Cue : MusicCues)
    {
        if (Cue.CueID == CueID)
        {
            // Fade out current music first
            if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
            {
                ActiveMusicComponent->FadeOut(Cue.TransitionTime, 0.0f);
            }

            USoundBase* Music = Cue.MusicAsset.LoadSynchronous();
            if (Music)
            {
                float FinalVolume = Cue.Volume * MusicVolume * MasterVolume;
                ActiveMusicComponent = UGameplayStatics::SpawnSound2D(World, Music, FinalVolume, 1.0f, 0.0f, nullptr, true, false);
                if (ActiveMusicComponent)
                {
                    ActiveMusicComponent->FadeIn(Cue.TransitionTime, FinalVolume);
                }
            }
            return;
        }
    }
}

void UAudio_AdaptiveAudioManager::StopMusic(float FadeOutTime)
{
    if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
    {
        ActiveMusicComponent->FadeOut(FadeOutTime, 0.0f);
    }
}

// ============================================================
// Screen shake — T-Rex proximity
// ============================================================

void UAudio_AdaptiveAudioManager::TriggerTRexScreenShake(FVector TRexLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), TRexLocation);
    if (Distance > TRexShakeRadius) return;

    // Scale intensity by proximity — closer = stronger shake
    float ProximityFactor = FMath::Clamp(1.0f - (Distance / TRexShakeRadius), 0.1f, 1.0f);
    float ShakeScale = TRexShakeIntensity * ProximityFactor;

    // Use built-in camera shake via console command (works without custom shake class)
    FString ShakeCmd = FString::Printf(TEXT("shake_cam %.2f %.2f %.2f"), ShakeScale * 2.0f, ShakeScale * 0.5f, 0.3f);
    UGameplayStatics::GetPlayerCameraManager(World, 0);

    // Broadcast as sound event so Blueprint can wire camera shake
    OnSoundEventFired.Broadcast(FName("TRex_Footstep_Shake"), TRexLocation);
}

// ============================================================
// Damage flash
// ============================================================

void UAudio_AdaptiveAudioManager::TriggerDamageFlash()
{
    bDamageFlashActive = true;
    DamageFlashTimer = DamageFlashDuration;
    // Blueprint listens to bDamageFlashActive via GetDamageFlashActive()
    // UMG widget polls this flag to show red overlay
    OnSoundEventFired.Broadcast(FName("Player_DamageFlash"), FVector::ZeroVector);
}

// ============================================================
// Threat blend (0.0 = Calm, 1.0 = Combat)
// ============================================================

float UAudio_AdaptiveAudioManager::GetThreatBlend() const
{
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Calm:   return 0.0f;
        case EAudio_ThreatLevel::Aware:  return 0.25f;
        case EAudio_ThreatLevel::Danger: return 0.6f;
        case EAudio_ThreatLevel::Combat: return 0.85f;
        case EAudio_ThreatLevel::Flee:   return 1.0f;
        default: return 0.0f;
    }
}

// ============================================================
// Private — ambient layer fading
// ============================================================

void UAudio_AdaptiveAudioManager::UpdateAmbientLayers(float DeltaTime)
{
    for (FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        bool bShouldBeActive = (Layer.ActiveOnThreat == CurrentThreatLevel) &&
                               (Layer.ActiveOnTime == CurrentTimeOfDay);

        float TargetVolume = bShouldBeActive ? Layer.BaseVolume : 0.0f;
        Layer.CurrentVolume = FMath::FInterpTo(Layer.CurrentVolume, TargetVolume, DeltaTime, Layer.FadeSpeed);
    }
}

void UAudio_AdaptiveAudioManager::UpdateMusicFade(float DeltaTime)
{
    if (!bFadingOut) return;
    if (!ActiveMusicComponent) return;

    MusicFadeTimer += DeltaTime;
    float Alpha = FMath::Clamp(MusicFadeTimer / MusicFadeDuration, 0.0f, 1.0f);
    float CurrentVol = FMath::Lerp(MusicVolume * MasterVolume, MusicFadeTarget, Alpha);
    ActiveMusicComponent->SetVolumeMultiplier(CurrentVol);

    if (Alpha >= 1.0f)
    {
        bFadingOut = false;
        MusicFadeTimer = 0.0f;
        if (MusicFadeTarget <= 0.0f)
        {
            ActiveMusicComponent->Stop();
        }
    }
}

void UAudio_AdaptiveAudioManager::SelectMusicForState()
{
    // Find the best matching music cue for current threat level
    for (const FAudio_MusicCue& Cue : MusicCues)
    {
        if (Cue.TriggerThreat == CurrentThreatLevel)
        {
            ForcePlayMusicCue(Cue.CueID);
            return;
        }
    }
}

// ============================================================
// Default sound event library
// ============================================================

void UAudio_AdaptiveAudioManager::PopulateDefaultSoundEvents()
{
    // Default entries — no asset paths assigned (assigned via Blueprint or DataTable)
    // These define the event IDs the game uses — assets wired in editor

    FAudio_SoundEvent FootstepDirt;
    FootstepDirt.EventID = FName("Player_Footstep_Dirt");
    FootstepDirt.Volume = 0.6f;
    FootstepDirt.Pitch = 1.0f;
    FootstepDirt.MaxDistance = 500.0f;
    FootstepDirt.bIs3D = true;
    SoundEventLibrary.Add(FootstepDirt);

    FAudio_SoundEvent FootstepRock;
    FootstepRock.EventID = FName("Player_Footstep_Rock");
    FootstepRock.Volume = 0.7f;
    FootstepRock.Pitch = 0.9f;
    FootstepRock.MaxDistance = 600.0f;
    FootstepRock.bIs3D = true;
    SoundEventLibrary.Add(FootstepRock);

    FAudio_SoundEvent TRexFootstep;
    TRexFootstep.EventID = FName("TRex_Footstep");
    TRexFootstep.Volume = 1.0f;
    TRexFootstep.Pitch = 0.7f;
    TRexFootstep.MaxDistance = 3000.0f;
    TRexFootstep.bIs3D = true;
    SoundEventLibrary.Add(TRexFootstep);

    FAudio_SoundEvent TRexRoar;
    TRexRoar.EventID = FName("TRex_Roar");
    TRexRoar.Volume = 1.0f;
    TRexRoar.Pitch = 0.8f;
    TRexRoar.MaxDistance = 5000.0f;
    TRexRoar.bIs3D = true;
    SoundEventLibrary.Add(TRexRoar);

    FAudio_SoundEvent RaptorCall;
    RaptorCall.EventID = FName("Raptor_Call");
    RaptorCall.Volume = 0.85f;
    RaptorCall.Pitch = 1.1f;
    RaptorCall.MaxDistance = 2500.0f;
    RaptorCall.bIs3D = true;
    SoundEventLibrary.Add(RaptorCall);

    FAudio_SoundEvent CraftingStone;
    CraftingStone.EventID = FName("Crafting_Stone_Strike");
    CraftingStone.Volume = 0.8f;
    CraftingStone.Pitch = 1.0f;
    CraftingStone.MaxDistance = 400.0f;
    CraftingStone.bIs3D = true;
    SoundEventLibrary.Add(CraftingStone);

    FAudio_SoundEvent FireCrackle;
    FireCrackle.EventID = FName("Fire_Crackle_Loop");
    FireCrackle.Volume = 0.7f;
    FireCrackle.Pitch = 1.0f;
    FireCrackle.MaxDistance = 800.0f;
    FireCrackle.bIs3D = true;
    SoundEventLibrary.Add(FireCrackle);

    FAudio_SoundEvent PlayerDamage;
    PlayerDamage.EventID = FName("Player_TakeDamage");
    PlayerDamage.Volume = 1.0f;
    PlayerDamage.Pitch = 1.0f;
    PlayerDamage.MaxDistance = 0.0f;
    PlayerDamage.bIs3D = false;
    SoundEventLibrary.Add(PlayerDamage);

    FAudio_SoundEvent PlayerDeath;
    PlayerDeath.EventID = FName("Player_Death");
    PlayerDeath.Volume = 1.0f;
    PlayerDeath.Pitch = 0.9f;
    PlayerDeath.MaxDistance = 0.0f;
    PlayerDeath.bIs3D = false;
    SoundEventLibrary.Add(PlayerDeath);

    FAudio_SoundEvent RiverAmbient;
    RiverAmbient.EventID = FName("Ambient_River");
    RiverAmbient.Volume = 0.5f;
    RiverAmbient.Pitch = 1.0f;
    RiverAmbient.MaxDistance = 1500.0f;
    RiverAmbient.bIs3D = true;
    SoundEventLibrary.Add(RiverAmbient);
}
