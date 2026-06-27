#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    AmbientDayComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientDayAudio"));
    AmbientDayComponent->SetupAttachment(RootComponent);
    AmbientDayComponent->bAutoActivate = false;
    AmbientDayComponent->VolumeMultiplier = 0.8f;

    AmbientNightComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientNightAudio"));
    AmbientNightComponent->SetupAttachment(RootComponent);
    AmbientNightComponent->bAutoActivate = false;
    AmbientNightComponent->VolumeMultiplier = 0.0f;

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicComponent->SetupAttachment(RootComponent);
    MusicComponent->bAutoActivate = false;
    MusicComponent->VolumeMultiplier = 0.7f;

    CurrentBiomeZone = EAudio_BiomeZone::OpenPlains;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentDayNightBlend = 0.0f;

    // Pre-populate narration URLs from ElevenLabs TTS (generated this cycle)
    NarrationAudioURLs.Add(TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782534909525_Narrator_Survival.mp3"));
    NarrationAudioURLs.Add(TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782534928593_Narrator_Danger.mp3"));
}

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Start ambient audio for default biome
    if (AmbientDayComponent && AmbientDayComponent->Sound)
    {
        AmbientDayComponent->Play();
    }
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAmbientCrossfade(DeltaTime);
    UpdateMusicLayer(DeltaTime);
}

void AAudioSystemManager::SetActiveBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentBiomeZone == NewZone) return;
    CurrentBiomeZone = NewZone;

    // Find matching ambient layer and update audio components
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.BiomeZone == NewZone)
        {
            // Load and assign sounds (async-safe via soft references)
            if (Layer.DayAmbience.IsValid() && AmbientDayComponent)
            {
                AmbientDayComponent->SetSound(Layer.DayAmbience.Get());
                AmbientDayComponent->Play();
            }
            if (Layer.NightAmbience.IsValid() && AmbientNightComponent)
            {
                AmbientNightComponent->SetSound(Layer.NightAmbience.Get());
            }
            break;
        }
    }
}

void AAudioSystemManager::SetDayNightBlend(float NormalizedTimeOfDay)
{
    // 0.0 = midday, 1.0 = midnight
    CurrentDayNightBlend = FMath::Clamp(NormalizedTimeOfDay, 0.0f, 1.0f);

    // Crossfade ambient layers
    if (AmbientDayComponent)
    {
        AmbientDayComponent->SetVolumeMultiplier(AmbientVolume * (1.0f - CurrentDayNightBlend));
    }
    if (AmbientNightComponent)
    {
        AmbientNightComponent->SetVolumeMultiplier(AmbientVolume * CurrentDayNightBlend);
        if (CurrentDayNightBlend > 0.1f && !AmbientNightComponent->IsPlaying())
        {
            AmbientNightComponent->Play();
        }
    }
}

void AAudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    CurrentDangerLevel = NewLevel;

    // Music intensity changes based on danger level
    float TargetMusicVolume = 0.0f;
    switch (NewLevel)
    {
        case EAudio_DangerLevel::Safe:       TargetMusicVolume = 0.3f; break;
        case EAudio_DangerLevel::Cautious:   TargetMusicVolume = 0.5f; break;
        case EAudio_DangerLevel::Threatened: TargetMusicVolume = 0.7f; break;
        case EAudio_DangerLevel::Combat:     TargetMusicVolume = 1.0f; break;
        case EAudio_DangerLevel::Fleeing:    TargetMusicVolume = 0.9f; break;
    }

    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume * TargetMusicVolume);
        if (!MusicComponent->IsPlaying() && NewLevel != EAudio_DangerLevel::Safe)
        {
            MusicComponent->Play();
        }
    }
}

void AAudioSystemManager::OnDinosaurFootstep(FVector FootstepLocation, FName DinosaurSpecies, float DinosaurMass)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Play footstep sound at location
    FAudio_DinosaurSoundProfile* Profile = FindDinosaurProfile(DinosaurSpecies);
    if (Profile && Profile->FootstepSound.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(
            World,
            Profile->FootstepSound.Get(),
            FootstepLocation,
            SFXVolume,
            1.0f,
            0.0f
        );
    }

    // Screen shake based on mass and distance to player
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->GetPawn())
    {
        float DistToPlayer = FVector::Dist(FootstepLocation, PC->GetPawn()->GetActorLocation());
        float ShakeRadius = Profile ? Profile->FootstepScreenShakeRadius : 2000.0f;
        float ShakeIntensity = Profile ? Profile->FootstepShakeIntensity : 1.0f;

        if (DistToPlayer < ShakeRadius)
        {
            float FalloffFactor = 1.0f - (DistToPlayer / ShakeRadius);
            float MassFactor = FMath::Clamp(DinosaurMass / 8000.0f, 0.1f, 1.0f);
            TriggerScreenShake(ShakeIntensity * FalloffFactor * MassFactor, 0.4f);
        }
    }
}

void AAudioSystemManager::OnDinosaurRoar(FVector RoarLocation, FName DinosaurSpecies, float Intensity)
{
    UWorld* World = GetWorld();
    if (!World) return;

    FAudio_DinosaurSoundProfile* Profile = FindDinosaurProfile(DinosaurSpecies);
    if (Profile && Profile->AlertSound.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(
            World,
            Profile->AlertSound.Get(),
            RoarLocation,
            SFXVolume * Intensity,
            1.0f,
            0.0f
        );
    }

    // A roar always triggers danger level escalation
    SetDangerLevel(EAudio_DangerLevel::Threatened);
}

void AAudioSystemManager::PlayCraftingSound(FVector Location)
{
    // Stone-on-stone crafting sound — primitive tool making
    UWorld* World = GetWorld();
    if (!World) return;
    // Sound asset to be assigned in Blueprint; placeholder log
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Crafting sound at %s"), *Location.ToString());
}

void AAudioSystemManager::PlayFireIgniteSound(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World) return;
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Fire ignite sound at %s"), *Location.ToString());
}

void AAudioSystemManager::PlayPlayerDamageSound(float DamageAmount)
{
    // Pain grunt — intensity scales with damage
    float PitchMultiplier = FMath::Lerp(1.2f, 0.8f, FMath::Clamp(DamageAmount / 100.0f, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Player damage sound, amount=%.1f, pitch=%.2f"), DamageAmount, PitchMultiplier);
}

void AAudioSystemManager::PlayPlayerDeathSound()
{
    // Death audio — silence music, play death sting
    if (MusicComponent)
    {
        MusicComponent->Stop();
    }
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Player death sound triggered"));
}

void AAudioSystemManager::TriggerScreenShake(float Intensity, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC)
    {
        // Use legacy camera shake (compatible with UE5.5)
        PC->ClientStartCameraShake(
            UCameraShakeBase::StaticClass(),
            Intensity
        );
    }
}

void AAudioSystemManager::PlayNarrationLine(int32 LineIndex)
{
    if (NarrationAudioURLs.IsValidIndex(LineIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing narration line %d: %s"),
            LineIndex, *NarrationAudioURLs[LineIndex]);
        // In production: use UMediaPlayer or HTTP audio streaming to play URL
        // For now: log URL for Blueprint pickup
    }
}

void AAudioSystemManager::UpdateAmbientCrossfade(float DeltaTime)
{
    // Smooth crossfade handled via SetDayNightBlend calls from GameState
}

void AAudioSystemManager::UpdateMusicLayer(float DeltaTime)
{
    // Music tension system — gradually relaxes danger level if no threat detected
    // This would be driven by proximity checks in full implementation
}

FAudio_DinosaurSoundProfile* AAudioSystemManager::FindDinosaurProfile(FName Species)
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
