#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

UAudio_AudioSystemManager::UAudio_AudioSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for audio updates
}

void UAudio_AudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] BeginPlay — Audio system initialised. Biome: Jungle, Danger: Safe"));
}

void UAudio_AudioSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAmbientLayers(DeltaTime);
    ApplyGroundShakeToCamera(DeltaTime);
}

// --- Ambient Audio ---

void UAudio_AudioSystemManager::SetBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentBiome == NewZone) return;
    CurrentBiome = NewZone;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Biome changed to: %d"), (int32)NewZone);
    // In full implementation: crossfade ambient layers based on new biome
}

void UAudio_AudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    CurrentDangerLevel = NewLevel;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger level changed to: %d"), (int32)NewLevel);
    // In full implementation: trigger music intensity shift via MetaSounds parameter
}

void UAudio_AudioSystemManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    // Adjust ambient layer volumes: night = more insects, less birds; day = more birds, wind
    float NightBlend = (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f) ? 1.0f : 0.0f;
    UE_LOG(LogTemp, Verbose, TEXT("[AudioSystemManager] TimeOfDay: %.2f NightBlend: %.2f"), NormalizedTime, NightBlend);
}

// --- Dinosaur Audio ---

void UAudio_AudioSystemManager::PlayDinosaurRoar(FName Species, FVector WorldLocation)
{
    // Find matching profile
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.DinosaurSpecies == Species)
        {
            USoundBase* RoarSnd = Profile.RoarSound.LoadSynchronous();
            if (RoarSnd)
            {
                UGameplayStatics::PlaySoundAtLocation(this, RoarSnd, WorldLocation, SFXVolume * MasterVolume);
                UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Roar played for %s at %s"), *Species.ToString(), *WorldLocation.ToString());
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] No roar profile found for species: %s"), *Species.ToString());
}

void UAudio_AudioSystemManager::PlayDinosaurFootstep(FName Species, FVector WorldLocation, float MassKg)
{
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.DinosaurSpecies == Species)
        {
            USoundBase* FootSnd = Profile.FootstepSound.LoadSynchronous();
            if (FootSnd)
            {
                // Scale volume by mass — heavier dinos = louder footsteps
                float VolumeScale = FMath::Clamp(MassKg / 6000.0f, 0.2f, 2.0f);
                UGameplayStatics::PlaySoundAtLocation(this, FootSnd, WorldLocation, SFXVolume * MasterVolume * VolumeScale);
            }
            // Trigger ground shake if heavy enough
            if (MassKg > 2000.0f)
            {
                TriggerGroundShake(WorldLocation, Profile.FootstepGroundShakeRadius, MassKg / 8000.0f);
            }
            return;
        }
    }
}

void UAudio_AudioSystemManager::TriggerGroundShake(FVector EpicenterLocation, float Radius, float Intensity)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), EpicenterLocation);
    if (DistToPlayer > Radius) return;

    // Scale shake by proximity
    float ProximityFactor = 1.0f - (DistToPlayer / Radius);
    GroundShakeIntensity = FMath::Max(GroundShakeIntensity, Intensity * ProximityFactor);
    GroundShakeTimer = 0.5f; // shake for 0.5 seconds

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Ground shake triggered — Intensity: %.2f Proximity: %.2f"), GroundShakeIntensity, ProximityFactor);
}

// --- Campfire Audio ---

void UAudio_AudioSystemManager::StartCampfireAudio(FVector Location)
{
    if (CampfireAudioComponent && CampfireAudioComponent->IsPlaying())
    {
        CampfireAudioComponent->SetWorldLocation(Location);
        return;
    }
    // Campfire audio component would be created here with a loaded sound asset
    // Freesound ID 681366: Campfire (Position 1) — crackling fire at night
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Campfire audio started at %s (Freesound #681366)"), *Location.ToString());
}

void UAudio_AudioSystemManager::StopCampfireAudio()
{
    if (CampfireAudioComponent && CampfireAudioComponent->IsPlaying())
    {
        CampfireAudioComponent->FadeOut(2.0f, 0.0f);
    }
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Campfire audio stopped"));
}

// --- Survival Feedback ---

void UAudio_AudioSystemManager::PlayCraftingSound(FName ItemCrafted)
{
    // Stone-on-stone, wood snapping, bone scraping — based on item type
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Crafting sound: %s"), *ItemCrafted.ToString());
}

void UAudio_AudioSystemManager::PlayDamageImpact(float DamageAmount)
{
    // Scale audio intensity by damage — small scrape vs massive bite
    float NormalizedDamage = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Damage impact audio — Amount: %.1f Normalised: %.2f"), DamageAmount, NormalizedDamage);
}

void UAudio_AudioSystemManager::PlayHungerWarning()
{
    // Stomach growl SFX + subtle music shift to minor key
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Hunger warning audio triggered"));
}

void UAudio_AudioSystemManager::PlayThirstWarning()
{
    // Dry throat sound + ambient water sounds increase in mix
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Thirst warning audio triggered"));
}

// --- Dialogue / Narration ---

void UAudio_AudioSystemManager::PlayNarrationLine(const FString& AudioURL, float Volume)
{
    // In full implementation: stream audio from URL via HTTP and play via AudioComponent
    // URLs from ElevenLabs TTS (e.g., Elder_Survivor voice lines)
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Narration line queued — URL: %s Volume: %.2f"), *AudioURL, Volume);
}

void UAudio_AudioSystemManager::StopNarration()
{
    if (NarrationAudioComponent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->FadeOut(1.0f, 0.0f);
    }
}

// --- Private ---

void UAudio_AudioSystemManager::UpdateAmbientLayers(float DeltaTime)
{
    // Crossfade ambient layers based on current biome, danger level, and time of day
    // Each layer has a target volume based on current state
    // Smooth transitions prevent jarring audio cuts
}

void UAudio_AudioSystemManager::ApplyGroundShakeToCamera(float DeltaTime)
{
    if (GroundShakeTimer <= 0.0f) return;

    GroundShakeTimer -= DeltaTime;
    if (GroundShakeTimer <= 0.0f)
    {
        GroundShakeIntensity = 0.0f;
        return;
    }

    // Apply procedural camera shake via PlayerController
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Shake intensity decays over time
    float CurrentIntensity = GroundShakeIntensity * (GroundShakeTimer / 0.5f);
    // In full implementation: apply camera shake class with intensity parameter
}

float UAudio_AudioSystemManager::GetDangerMusicIntensity() const
{
    switch (CurrentDangerLevel)
    {
        case EAudio_DangerLevel::Safe:     return 0.0f;
        case EAudio_DangerLevel::Cautious: return 0.33f;
        case EAudio_DangerLevel::Danger:   return 0.66f;
        case EAudio_DangerLevel::Critical: return 1.0f;
        default:                           return 0.0f;
    }
}
