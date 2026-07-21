#include "NarrativeAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UNarrativeAudioManager::UNarrativeAudioManager()
{
    CurrentAudioComponent = nullptr;
    CurrentSpeaker = TEXT("");
    NarrativeVolume = 1.0f;
    bIsCurrentlyPlaying = false;
}

void UNarrativeAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeAudioManager: Initializing narrative audio system"));
    
    // Initialize default audio clips with survival-focused content
    InitializeDefaultAudioClips();
    
    // Create default trigger zones in savanna biome
    CreateAudioTriggerZone(FVector(500, 500, 100), 1000.0f, TEXT("AncientStones"), TArray<FNarr_NarrativeAudioClip>());
    CreateAudioTriggerZone(FVector(1500, -800, 150), 1000.0f, TEXT("RaptorWarning"), TArray<FNarr_NarrativeAudioClip>());
    CreateAudioTriggerZone(FVector(-1200, 1000, 120), 1000.0f, TEXT("BloodTrail"), TArray<FNarr_NarrativeAudioClip>());
    CreateAudioTriggerZone(FVector(800, -1500, 180), 1000.0f, TEXT("FireWisdom"), TArray<FNarr_NarrativeAudioClip>());
}

void UNarrativeAudioManager::Deinitialize()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent = nullptr;
    }
    
    RegisteredAudioClips.Empty();
    AudioTriggerZones.Empty();
    
    Super::Deinitialize();
}

void UNarrativeAudioManager::PlayNarrativeAudio(const FString& CharacterName, const FString& AudioURL, const FString& DialogueText)
{
    if (bIsCurrentlyPlaying && CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
    }
    
    CurrentSpeaker = CharacterName;
    bIsCurrentlyPlaying = true;
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeAudioManager: Playing audio for %s: %s"), *CharacterName, *DialogueText);
    
    // For now, we'll use a placeholder sound and display the text
    // In a full implementation, this would load the audio from the URL
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *CharacterName, *DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, DisplayText);
    }
    
    // Simulate audio duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        bIsCurrentlyPlaying = false;
        CurrentSpeaker = TEXT("");
    }, 8.0f, false);
}

void UNarrativeAudioManager::StopNarrativeAudio()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
    }
    
    bIsCurrentlyPlaying = false;
    CurrentSpeaker = TEXT("");
}

void UNarrativeAudioManager::RegisterAudioClip(const FNarr_NarrativeAudioClip& AudioClip)
{
    RegisteredAudioClips.Add(AudioClip);
    UE_LOG(LogTemp, Log, TEXT("NarrativeAudioManager: Registered audio clip for %s"), *AudioClip.CharacterName);
}

void UNarrativeAudioManager::CreateAudioTriggerZone(const FVector& Location, float Radius, const FString& TriggerID, const TArray<FNarr_NarrativeAudioClip>& Clips)
{
    FNarr_AudioTriggerZone NewZone;
    NewZone.Location = Location;
    NewZone.TriggerRadius = Radius;
    NewZone.TriggerID = TriggerID;
    NewZone.AudioClips = Clips;
    NewZone.bHasBeenTriggered = false;
    NewZone.bCanRetrigger = false;
    
    AudioTriggerZones.Add(NewZone);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeAudioManager: Created audio trigger zone %s at %s"), *TriggerID, *Location.ToString());
}

void UNarrativeAudioManager::CheckPlayerProximity(const FVector& PlayerLocation)
{
    for (FNarr_AudioTriggerZone& Zone : AudioTriggerZones)
    {
        if (Zone.bHasBeenTriggered && !Zone.bCanRetrigger)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Zone.Location);
        if (Distance <= Zone.TriggerRadius)
        {
            TriggerAudioZone(Zone);
            Zone.bHasBeenTriggered = true;
        }
    }
}

bool UNarrativeAudioManager::IsAudioPlaying() const
{
    return bIsCurrentlyPlaying;
}

FString UNarrativeAudioManager::GetCurrentCharacterSpeaking() const
{
    return CurrentSpeaker;
}

void UNarrativeAudioManager::PlayContextualAudio(ENarr_NarrativeContext Context, const FVector& PlayerLocation)
{
    FNarr_NarrativeAudioClip* FoundClip = FindAudioClipByContext(Context);
    if (FoundClip)
    {
        PlayNarrativeAudio(FoundClip->CharacterName, FoundClip->AudioURL, FoundClip->DialogueText);
    }
}

void UNarrativeAudioManager::SetNarrativeVolume(float Volume)
{
    NarrativeVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->SetVolumeMultiplier(NarrativeVolume);
    }
}

void UNarrativeAudioManager::InitializeDefaultAudioClips()
{
    // Ancient Narrator - Exploration warnings
    FNarr_NarrativeAudioClip AncientClip;
    AncientClip.CharacterName = TEXT("Ancient Narrator");
    AncientClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779700520568_AncientNarrator.mp3");
    AncientClip.DialogueText = TEXT("The ancient stones speak of danger ahead, survivor. The great predators hunt in packs when the moon is dark. Trust your instincts and keep your spear ready.");
    AncientClip.Context = ENarr_NarrativeContext::Exploration;
    AncientClip.Duration = 11.0f;
    RegisterAudioClip(AncientClip);
    
    // Tribal Scout - Combat warnings
    FNarr_NarrativeAudioClip ScoutClip;
    ScoutClip.CharacterName = TEXT("Tribal Scout");
    ScoutClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779700525338_TribalScout.mp3");
    ScoutClip.DialogueText = TEXT("Warning! Raptor pack detected in the eastern cliffs. They hunt in coordinated strikes. Find high ground immediately and prepare your weapons.");
    ScoutClip.Context = ENarr_NarrativeContext::Combat;
    ScoutClip.Duration = 10.0f;
    RegisterAudioClip(ScoutClip);
    
    // Experienced Hunter - Tracking
    FNarr_NarrativeAudioClip HunterClip;
    HunterClip.CharacterName = TEXT("Experienced Hunter");
    HunterClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779700530313_ExperiencedHunter.mp3");
    HunterClip.DialogueText = TEXT("The water runs red with blood. Something massive has passed through here. Follow the tracks carefully, but stay downwind. The beast may still be near.");
    HunterClip.Context = ENarr_NarrativeContext::Discovery;
    HunterClip.Duration = 10.0f;
    RegisterAudioClip(HunterClip);
    
    // Tribal Elder - Survival wisdom
    FNarr_NarrativeAudioClip ElderClip;
    ElderClip.CharacterName = TEXT("Tribal Elder");
    ElderClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779700535175_TribalElder.mp3");
    ElderClip.DialogueText = TEXT("Fire keeps the night hunters away, but smoke reveals your position to others. Choose wisely between warmth and stealth, young one.");
    ElderClip.Context = ENarr_NarrativeContext::Survival;
    ElderClip.Duration = 9.0f;
    RegisterAudioClip(ElderClip);
}

void UNarrativeAudioManager::TriggerAudioZone(const FNarr_AudioTriggerZone& Zone)
{
    if (Zone.AudioClips.Num() > 0)
    {
        // Play the first audio clip in the zone
        const FNarr_NarrativeAudioClip& Clip = Zone.AudioClips[0];
        PlayNarrativeAudio(Clip.CharacterName, Clip.AudioURL, Clip.DialogueText);
    }
    else
    {
        // Use default clips based on zone ID
        if (Zone.TriggerID == TEXT("AncientStones"))
        {
            PlayContextualAudio(ENarr_NarrativeContext::Exploration, Zone.Location);
        }
        else if (Zone.TriggerID == TEXT("RaptorWarning"))
        {
            PlayContextualAudio(ENarr_NarrativeContext::Combat, Zone.Location);
        }
        else if (Zone.TriggerID == TEXT("BloodTrail"))
        {
            PlayContextualAudio(ENarr_NarrativeContext::Discovery, Zone.Location);
        }
        else if (Zone.TriggerID == TEXT("FireWisdom"))
        {
            PlayContextualAudio(ENarr_NarrativeContext::Survival, Zone.Location);
        }
    }
}

FNarr_NarrativeAudioClip* UNarrativeAudioManager::FindAudioClipByContext(ENarr_NarrativeContext Context)
{
    for (FNarr_NarrativeAudioClip& Clip : RegisteredAudioClips)
    {
        if (Clip.Context == Context)
        {
            return &Clip;
        }
    }
    return nullptr;
}