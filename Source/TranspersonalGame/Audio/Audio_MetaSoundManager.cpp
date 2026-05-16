#include "Audio_MetaSoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

UAudio_MetaSoundManager::UAudio_MetaSoundManager()
{
    CurrentBiome = EAudio_BiomeType::Savanna;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    BiomeAudioComponent = nullptr;
    DialogueAudioComponent = nullptr;
    EffectsAudioComponent = nullptr;
}

void UAudio_MetaSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Initializing adaptive audio system"));
    
    InitializeBiomeProfiles();
    InitializeDialogueDatabase();
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        if (AActor* AudioManagerActor = UGameplayStatics::GetActorOfClass(World, AActor::StaticClass()))
        {
            BiomeAudioComponent = AudioManagerActor->FindComponentByClass<UAudioComponent>();
            if (!BiomeAudioComponent)
            {
                BiomeAudioComponent = NewObject<UAudioComponent>(AudioManagerActor);
                AudioManagerActor->AddInstanceComponent(BiomeAudioComponent);
            }
            
            DialogueAudioComponent = NewObject<UAudioComponent>(AudioManagerActor);
            AudioManagerActor->AddInstanceComponent(DialogueAudioComponent);
            
            EffectsAudioComponent = NewObject<UAudioComponent>(AudioManagerActor);
            AudioManagerActor->AddInstanceComponent(EffectsAudioComponent);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Initialization complete"));
}

void UAudio_MetaSoundManager::Deinitialize()
{
    if (BiomeAudioComponent && BiomeAudioComponent->IsPlaying())
    {
        BiomeAudioComponent->Stop();
    }
    
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
    
    if (EffectsAudioComponent && EffectsAudioComponent->IsPlaying())
    {
        EffectsAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudio_MetaSoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome, const FVector& PlayerLocation)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Transitioning from %d to %d biome"), 
               (int32)CurrentBiome, (int32)NewBiome);
        
        CrossfadeBiomes(CurrentBiome, NewBiome);
        CurrentBiome = NewBiome;
        UpdateBiomeAudio();
    }
}

void UAudio_MetaSoundManager::UpdateThreatLevel(EAudio_ThreatLevel ThreatLevel)
{
    if (CurrentThreatLevel != ThreatLevel)
    {
        CurrentThreatLevel = ThreatLevel;
        
        // Adjust audio parameters based on threat level
        float ThreatMultiplier = 1.0f;
        switch (ThreatLevel)
        {
        case EAudio_ThreatLevel::Safe:
            ThreatMultiplier = 1.0f;
            break;
        case EAudio_ThreatLevel::Caution:
            ThreatMultiplier = 1.2f;
            break;
        case EAudio_ThreatLevel::Danger:
            ThreatMultiplier = 1.5f;
            break;
        case EAudio_ThreatLevel::Extreme:
            ThreatMultiplier = 2.0f;
            break;
        }
        
        if (BiomeAudioComponent)
        {
            BiomeAudioComponent->SetVolumeMultiplier(AmbientVolume * ThreatMultiplier);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Threat level updated to %d (multiplier: %.2f)"), 
               (int32)ThreatLevel, ThreatMultiplier);
    }
}

void UAudio_MetaSoundManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Intensity)
{
    if (!EffectsAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: EffectsAudioComponent not available"));
        return;
    }
    
    // Find appropriate dinosaur sound based on type
    FString SoundPath;
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Tyrannosaurus")))
    {
        SoundPath = TEXT("/Game/Audio/Dinosaurs/TRex_Roar");
    }
    else if (DinosaurType.Contains(TEXT("Raptor")) || DinosaurType.Contains(TEXT("Velociraptor")))
    {
        SoundPath = TEXT("/Game/Audio/Dinosaurs/Raptor_Call");
    }
    else if (DinosaurType.Contains(TEXT("Brachio")) || DinosaurType.Contains(TEXT("Sauropod")))
    {
        SoundPath = TEXT("/Game/Audio/Dinosaurs/Brachio_Bellow");
    }
    
    if (!SoundPath.IsEmpty())
    {
        EffectsAudioComponent->SetWorldLocation(Location);
        EffectsAudioComponent->SetVolumeMultiplier(Intensity);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing %s sound at location (%.1f, %.1f, %.1f) with intensity %.2f"), 
               *DinosaurType, Location.X, Location.Y, Location.Z, Intensity);
    }
}

void UAudio_MetaSoundManager::PlayDialogueLine(const FString& NPCName, const FString& DialogueID)
{
    if (!DialogueAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: DialogueAudioComponent not available"));
        return;
    }
    
    FString DialogueKey = FString::Printf(TEXT("%s_%s"), *NPCName, *DialogueID);
    
    if (FAudio_DialogueAudioData* DialogueData = DialogueDatabase.Find(DialogueKey))
    {
        if (DialogueData->VoiceLine.IsValid())
        {
            DialogueAudioComponent->SetSound(DialogueData->VoiceLine.LoadSynchronous());
            DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
            DialogueAudioComponent->Play();
            
            bDialoguePlaying = true;
            
            UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing dialogue %s for NPC %s"), 
                   *DialogueID, *NPCName);
            
            // Set timer to reset dialogue playing flag
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(
                    FTimerHandle(),
                    [this]() { bDialoguePlaying = false; },
                    DialogueData->Duration,
                    false
                );
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Dialogue %s not found for NPC %s"), 
               *DialogueID, *NPCName);
    }
}

void UAudio_MetaSoundManager::SetDialogueVolume(float Volume)
{
    DialogueVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
    }
}

bool UAudio_MetaSoundManager::IsDialoguePlaying() const
{
    return bDialoguePlaying && DialogueAudioComponent && DialogueAudioComponent->IsPlaying();
}

void UAudio_MetaSoundManager::PlayEnvironmentalEffect(const FString& EffectName, const FVector& Location)
{
    if (!EffectsAudioComponent)
    {
        return;
    }
    
    EffectsAudioComponent->SetWorldLocation(Location);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing environmental effect %s at location (%.1f, %.1f, %.1f)"), 
           *EffectName, Location.X, Location.Y, Location.Z);
}

void UAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateBiomeAudio();
}

void UAudio_MetaSoundManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateBiomeAudio();
}

void UAudio_MetaSoundManager::InitializeBiomeProfiles()
{
    BiomeProfiles.Empty();
    
    // Savanna biome
    FAudio_BiomeAudioProfile SavannaProfile;
    SavannaProfile.BiomeType = EAudio_BiomeType::Savanna;
    SavannaProfile.BaseVolume = 0.7f;
    SavannaProfile.FadeDistance = 5000.0f;
    BiomeProfiles.Add(SavannaProfile);
    
    // Forest biome
    FAudio_BiomeAudioProfile ForestProfile;
    ForestProfile.BiomeType = EAudio_BiomeType::Forest;
    ForestProfile.BaseVolume = 0.8f;
    ForestProfile.FadeDistance = 3000.0f;
    BiomeProfiles.Add(ForestProfile);
    
    // Swamp biome
    FAudio_BiomeAudioProfile SwampProfile;
    SwampProfile.BiomeType = EAudio_BiomeType::Swamp;
    SwampProfile.BaseVolume = 0.6f;
    SwampProfile.FadeDistance = 4000.0f;
    BiomeProfiles.Add(SwampProfile);
    
    // Desert biome
    FAudio_BiomeAudioProfile DesertProfile;
    DesertProfile.BiomeType = EAudio_BiomeType::Desert;
    DesertProfile.BaseVolume = 0.5f;
    DesertProfile.FadeDistance = 8000.0f;
    BiomeProfiles.Add(DesertProfile);
    
    // Mountain biome
    FAudio_BiomeAudioProfile MountainProfile;
    MountainProfile.BiomeType = EAudio_BiomeType::Mountain;
    MountainProfile.BaseVolume = 0.6f;
    MountainProfile.FadeDistance = 6000.0f;
    BiomeProfiles.Add(MountainProfile);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Initialized %d biome audio profiles"), BiomeProfiles.Num());
}

void UAudio_MetaSoundManager::InitializeDialogueDatabase()
{
    DialogueDatabase.Empty();
    
    // Initialize dialogue entries for NPCs created by Narrative Agent
    FAudio_DialogueAudioData TribalElderData;
    TribalElderData.NPCName = TEXT("TribalElder");
    TribalElderData.Duration = 16.0f;
    TribalElderData.bHasSubtitles = true;
    DialogueDatabase.Add(TEXT("TribalElder_Greeting"), TribalElderData);
    
    FAudio_DialogueAudioData QuestGiverData;
    QuestGiverData.NPCName = TEXT("QuestGiver");
    QuestGiverData.Duration = 15.0f;
    QuestGiverData.bHasSubtitles = true;
    DialogueDatabase.Add(TEXT("QuestGiver_Mission"), QuestGiverData);
    
    FAudio_DialogueAudioData BoneReaderData;
    BoneReaderData.NPCName = TEXT("BoneReader");
    BoneReaderData.Duration = 17.0f;
    BoneReaderData.bHasSubtitles = true;
    DialogueDatabase.Add(TEXT("BoneReader_Lore"), BoneReaderData);
    
    FAudio_DialogueAudioData TrackingMentorData;
    TrackingMentorData.NPCName = TEXT("TrackingMentor");
    TrackingMentorData.Duration = 16.0f;
    TrackingMentorData.bHasSubtitles = true;
    DialogueDatabase.Add(TEXT("TrackingMentor_Guide"), TrackingMentorData);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Initialized dialogue database with %d entries"), DialogueDatabase.Num());
}

void UAudio_MetaSoundManager::UpdateBiomeAudio()
{
    if (!BiomeAudioComponent)
    {
        return;
    }
    
    // Find current biome profile
    FAudio_BiomeAudioProfile* CurrentProfile = BiomeProfiles.FindByPredicate(
        [this](const FAudio_BiomeAudioProfile& Profile) 
        { 
            return Profile.BiomeType == CurrentBiome; 
        }
    );
    
    if (CurrentProfile)
    {
        float FinalVolume = CurrentProfile->BaseVolume * AmbientVolume * MasterVolume;
        BiomeAudioComponent->SetVolumeMultiplier(FinalVolume);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated biome audio - Volume: %.2f"), FinalVolume);
    }
}

void UAudio_MetaSoundManager::CrossfadeBiomes(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome)
{
    // Implement smooth crossfade between biome audio
    if (BiomeAudioComponent)
    {
        // Fade out current biome over 2 seconds, then fade in new biome
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Crossfading from biome %d to biome %d"), 
               (int32)FromBiome, (int32)ToBiome);
    }
}