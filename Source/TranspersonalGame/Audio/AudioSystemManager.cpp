#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

UAudio_SystemManager::UAudio_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio state
}

void UAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialise ambient layer for current biome + time
    UpdateAmbientLayers();

    // Start with safe music state
    CrossfadeToMusicState(EAudio_ThreatLevel::Safe);
}

void UAudio_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastThreatChange += DeltaTime;
}

// ─── Threat System ────────────────────────────────────────────────────────────

void UAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel) return;
    if (TimeSinceLastThreatChange < ThreatTransitionCooldown) return;

    CurrentThreatLevel = NewLevel;
    TimeSinceLastThreatChange = 0.0f;

    CrossfadeToMusicState(NewLevel);
}

// ─── Ambient System ───────────────────────────────────────────────────────────

void UAudio_SystemManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome) return;
    CurrentBiome = NewBiome;
    UpdateAmbientLayers();
}

void UAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime == CurrentTimeOfDay) return;
    CurrentTimeOfDay = NewTime;
    UpdateAmbientLayers();
}

void UAudio_SystemManager::UpdateAmbientLayers()
{
    FAudio_AmbientLayer* Layer = FindAmbientLayer(CurrentBiome, CurrentTimeOfDay);
    if (!Layer || !Layer->AmbientLoop) return;

    CrossfadeAudioComponent(ActiveAmbientComponent, Layer->AmbientLoop,
        Layer->FadeOutTime, Layer->FadeInTime, Layer->BaseVolume * AmbientVolume * MasterVolume);
}

// ─── Dinosaur Audio ───────────────────────────────────────────────────────────

void UAudio_SystemManager::PlayDinosaurFootstep(FName Species, FVector Location, float Mass)
{
    FAudio_DinosaurSoundProfile* Profile = FindDinosaurProfile(Species);
    if (!Profile || !Profile->FootstepHeavy) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Volume scales with mass (heavier = louder)
    float VolumeScale = FMath::Clamp(Mass / 8000.0f, 0.3f, 1.5f);
    UGameplayStatics::PlaySoundAtLocation(World, Profile->FootstepHeavy, Location,
        VolumeScale * SFXVolume * MasterVolume);

    // Trigger proximity shake based on distance to player
    TriggerProximityShake(Location, Mass);
}

void UAudio_SystemManager::PlayDinosaurRoar(FName Species, FVector Location, bool bIsAlert)
{
    FAudio_DinosaurSoundProfile* Profile = FindDinosaurProfile(Species);
    if (!Profile) return;

    UWorld* World = GetWorld();
    if (!World) return;

    USoundBase* RoarSound = bIsAlert ? Profile->AlertRoar : Profile->AttackRoar;
    if (!RoarSound) return;

    UGameplayStatics::PlaySoundAtLocation(World, RoarSound, Location,
        SFXVolume * MasterVolume);

    // Roar escalates threat level
    if (bIsAlert && CurrentThreatLevel == EAudio_ThreatLevel::Safe)
    {
        SetThreatLevel(EAudio_ThreatLevel::Aware);
    }
    else if (!bIsAlert)
    {
        SetThreatLevel(EAudio_ThreatLevel::Combat);
    }
}

void UAudio_SystemManager::TriggerProximityShake(FVector DinosaurLocation, float DinosaurMass)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
    if (!PlayerChar) return;

    float Distance = FVector::Dist(DinosaurLocation, PlayerChar->GetActorLocation());

    // Scale shake intensity by distance and mass
    float MaxShakeRadius = TRexShakeDistance * FMath::Clamp(DinosaurMass / 8000.0f, 0.5f, 2.0f);
    if (Distance > MaxShakeRadius) return;

    float IntensityFactor = FMath::Clamp(1.0f - (Distance / MaxShakeRadius), 0.0f, 1.0f);
    IntensityFactor = FMath::Pow(IntensityFactor, 1.5f); // Non-linear falloff

    TSubclassOf<UCameraShakeBase> ShakeClass = (DinosaurMass > 5000.0f) ? HeavyShakeClass : LightShakeClass;
    if (!ShakeClass) return;

    PC->ClientStartCameraShake(ShakeClass, IntensityFactor);
}

// ─── Player Feedback ──────────────────────────────────────────────────────────

void UAudio_SystemManager::PlayFootstep(bool bIsRunning, FName SurfaceType)
{
    // Footstep audio is handled by surface-specific sound cues
    // This function is called by the character's animation notify
    UWorld* World = GetWorld();
    if (!World) return;

    // Volume: running footsteps are louder and could attract predators
    float Volume = bIsRunning ? 1.0f : 0.5f;

    // Surface type determines which sound asset to use
    // Assets are loaded at runtime from the sound cue library
    // For now, log the footstep event for debugging
    UE_LOG(LogTemp, Verbose, TEXT("Footstep: Surface=%s Running=%s Vol=%.2f"),
        *SurfaceType.ToString(), bIsRunning ? TEXT("true") : TEXT("false"), Volume);
}

void UAudio_SystemManager::TriggerDamageFlashAudio(float DamageAmount)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Escalate threat level on damage
    if (DamageAmount > 50.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Combat);
    }
    else if (DamageAmount > 20.0f && CurrentThreatLevel == EAudio_ThreatLevel::Safe)
    {
        SetThreatLevel(EAudio_ThreatLevel::Stalked);
    }

    // Trigger heavy screen shake on significant damage
    if (DamageAmount > 30.0f)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && HeavyShakeClass)
        {
            float ShakeScale = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.0f);
            PC->ClientStartCameraShake(HeavyShakeClass, ShakeScale);
        }
    }
}

void UAudio_SystemManager::PlayCraftingSound(FName ItemCrafted)
{
    // Crafting sounds: stone-on-stone, wood splitting, bone scraping
    // Specific sound assets assigned per item type in the Blueprint subclass
    UE_LOG(LogTemp, Verbose, TEXT("Crafting sound: %s"), *ItemCrafted.ToString());
}

// ─── Music System ─────────────────────────────────────────────────────────────

void UAudio_SystemManager::CrossfadeToMusicState(EAudio_ThreatLevel TargetState)
{
    FAudio_MusicState* State = FindMusicState(TargetState);
    if (!State || !State->MusicTrack) return;

    CrossfadeAudioComponent(ActiveMusicComponent, State->MusicTrack,
        State->CrossfadeDuration, State->CrossfadeDuration,
        State->Volume * MusicVolume * MasterVolume);
}

void UAudio_SystemManager::StopAllMusic(float FadeTime)
{
    if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
    {
        ActiveMusicComponent->FadeOut(FadeTime, 0.0f);
    }
}

// ─── Day/Night Audio Cycle ────────────────────────────────────────────────────

void UAudio_SystemManager::UpdateDayNightAudio(float TimeOfDayNormalized)
{
    // Prevent rapid transitions
    if (FMath::Abs(TimeOfDayNormalized - LastDayNightNormalized) < 0.05f) return;
    LastDayNightNormalized = TimeOfDayNormalized;

    EAudio_TimeOfDay NewTime = NormalizedTimeToEnum(TimeOfDayNormalized);
    SetTimeOfDay(NewTime);
}

// ─── Screen Shake ─────────────────────────────────────────────────────────────

void UAudio_SystemManager::TriggerScreenShake(float Intensity, float Duration, FVector SourceLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    TSubclassOf<UCameraShakeBase> ShakeClass = (Intensity > 0.5f) ? HeavyShakeClass : LightShakeClass;
    if (!ShakeClass) return;

    PC->ClientStartCameraShake(ShakeClass, Intensity);
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void UAudio_SystemManager::CrossfadeAudioComponent(UAudioComponent*& Component,
    USoundBase* NewSound, float FadeOut, float FadeIn, float Volume)
{
    if (!NewSound) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Fade out existing component
    if (Component && Component->IsPlaying())
    {
        Component->FadeOut(FadeOut, 0.0f);
    }

    // Spawn new audio component and fade in
    Component = UGameplayStatics::SpawnSound2D(World, NewSound, Volume, 1.0f, 0.0f, nullptr, false, false);
    if (Component)
    {
        Component->FadeIn(FadeIn, Volume);
    }
}

FAudio_DinosaurSoundProfile* UAudio_SystemManager::FindDinosaurProfile(FName Species)
{
    for (FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.DinosaurSpecies == Species)
        {
            return &Profile;
        }
    }
    return nullptr;
}

FAudio_MusicState* UAudio_SystemManager::FindMusicState(EAudio_ThreatLevel Level)
{
    for (FAudio_MusicState& State : MusicStates)
    {
        if (State.ThreatLevel == Level)
        {
            return &State;
        }
    }
    return nullptr;
}

FAudio_AmbientLayer* UAudio_SystemManager::FindAmbientLayer(EAudio_BiomeType Biome, EAudio_TimeOfDay Time)
{
    for (FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Biome == Biome && Layer.TimeOfDay == Time)
        {
            return &Layer;
        }
    }
    return nullptr;
}

EAudio_TimeOfDay UAudio_SystemManager::NormalizedTimeToEnum(float Normalized) const
{
    // 0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk, 1.0 = midnight
    if (Normalized < 0.15f || Normalized > 0.90f) return EAudio_TimeOfDay::Night;
    if (Normalized < 0.30f) return EAudio_TimeOfDay::Dawn;
    if (Normalized < 0.70f) return EAudio_TimeOfDay::Day;
    return EAudio_TimeOfDay::Dusk;
}
