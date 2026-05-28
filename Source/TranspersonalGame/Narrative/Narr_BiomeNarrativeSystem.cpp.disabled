#include "Narr_BiomeNarrativeSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"

UNarr_BiomeNarrativeSystem::UNarr_BiomeNarrativeSystem()
{
    NarrativeAudioComponent = nullptr;
    EmergencyAlertSound = nullptr;
}

void UNarr_BiomeNarrativeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Initializing biome narrative system"));
    
    InitializeBiomeNarratives();
    
    // Create audio component for narrative playback
    if (GetWorld())
    {
        AActor* WorldActor = GetWorld()->GetFirstPlayerController();
        if (WorldActor)
        {
            NarrativeAudioComponent = NewObject<UAudioComponent>(WorldActor);
            if (NarrativeAudioComponent)
            {
                NarrativeAudioComponent->SetVolumeMultiplier(0.8f);
                NarrativeAudioComponent->AttachToComponent(WorldActor->GetRootComponent(), 
                    FAttachmentTransformRules::KeepWorldTransform);
            }
        }
    }
}

void UNarr_BiomeNarrativeSystem::Deinitialize()
{
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->Stop();
        NarrativeAudioComponent = nullptr;
    }
    
    BiomeNarratives.Empty();
    LastTriggerTimes.Empty();
    
    Super::Deinitialize();
}

void UNarr_BiomeNarrativeSystem::InitializeBiomeNarratives()
{
    // Initialize swamp narrative
    FNarr_BiomeNarrativeData SwampData;
    SwampData.BiomeType = ENarr_BiomeType::Swamp;
    SwampData.NarrativeText = TEXT("Day 143 in the wilderness. The volcanic ash has cleared, revealing new hunting grounds to the east. The Carnotaurus pack has established territory near the sulfur springs, making water collection extremely dangerous.");
    SwampData.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778401656201_Kael_Tracker.mp3");
    SwampData.TriggerCooldown = 45.0f;
    BiomeNarratives.Add(ENarr_BiomeType::Swamp, SwampData);

    // Initialize forest narrative  
    FNarr_BiomeNarrativeData ForestData;
    ForestData.BiomeType = ENarr_BiomeType::Forest;
    ForestData.NarrativeText = TEXT("The ancient forest holds secrets written in stone and bone. I am Vera, the Bone Reader. These fossilized remains tell stories of great migrations, of herds that numbered in the thousands.");
    ForestData.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778401670925_Vera_BoneReader.mp3");
    ForestData.TriggerCooldown = 60.0f;
    BiomeNarratives.Add(ENarr_BiomeType::Forest, ForestData);

    // Initialize savana narrative
    FNarr_BiomeNarrativeData SavanaData;
    SavanaData.BiomeType = ENarr_BiomeType::Savana;
    SavanaData.NarrativeText = TEXT("Resource cache discovered! Hidden beneath the fallen Sequoia, I've found obsidian shards perfect for spear tips, dried meat that's still edible, and most importantly - a water purification tablet.");
    SavanaData.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778401678767_Cache_Discovery.mp3");
    SavanaData.TriggerCooldown = 30.0f;
    BiomeNarratives.Add(ENarr_BiomeType::Savana, SavanaData);

    // Initialize desert emergency narrative
    FNarr_BiomeNarrativeData DesertData;
    DesertData.BiomeType = ENarr_BiomeType::Desert;
    DesertData.NarrativeText = TEXT("Emergency alert! Massive Giganotosaurus detected approaching the northern perimeter. This apex predator is larger than any Tyrannosaurus Rex and shows aggressive territorial behavior.");
    DesertData.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778401662590_Emergency_Narrator.mp3");
    DesertData.TriggerCooldown = 120.0f;
    DesertData.bIsEmergencyAlert = true;
    BiomeNarratives.Add(ENarr_BiomeType::Desert, DesertData);

    // Initialize mountain narrative
    FNarr_BiomeNarrativeData MountainData;
    MountainData.BiomeType = ENarr_BiomeType::Mountain;
    MountainData.NarrativeText = TEXT("The high peaks offer both refuge and danger. Altitude sickness threatens the unprepared, but the vantage point reveals migration routes of the great herds below.");
    MountainData.AudioURL = TEXT("");
    MountainData.TriggerCooldown = 90.0f;
    BiomeNarratives.Add(ENarr_BiomeType::Mountain, MountainData);

    UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Initialized %d biome narratives"), BiomeNarratives.Num());
}

void UNarr_BiomeNarrativeSystem::TriggerBiomeNarrative(ENarr_BiomeType BiomeType, const FVector& PlayerLocation)
{
    if (IsBiomeNarrativeOnCooldown(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Biome narrative %d on cooldown"), (int32)BiomeType);
        return;
    }

    if (FNarr_BiomeNarrativeData* NarrativeData = BiomeNarratives.Find(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Triggering narrative for biome %d"), (int32)BiomeType);
        
        // Update last trigger time
        LastTriggerTimes.Add(BiomeType, GetCurrentTime());
        
        // Play narrative audio if available
        if (!NarrativeData->AudioURL.IsEmpty())
        {
            PlayNarrativeAudio(NarrativeData->AudioURL);
        }
        
        // Display narrative text (could be connected to UI system)
        if (GEngine)
        {
            FString DisplayText = FString::Printf(TEXT("NARRATIVE: %s"), *NarrativeData->NarrativeText);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, 
                NarrativeData->bIsEmergencyAlert ? FColor::Red : FColor::Yellow, 
                DisplayText);
        }
        
        // Log for debugging
        UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: %s"), *NarrativeData->NarrativeText);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Narr_BiomeNarrativeSystem: No narrative data found for biome %d"), (int32)BiomeType);
    }
}

void UNarr_BiomeNarrativeSystem::RegisterBiomeTrigger(ATriggerBox* TriggerBox, ENarr_BiomeType BiomeType)
{
    if (!TriggerBox)
    {
        UE_LOG(LogTemp, Error, TEXT("Narr_BiomeNarrativeSystem: Cannot register null trigger box"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Registered trigger box %s for biome %d"), 
        *TriggerBox->GetName(), (int32)BiomeType);
    
    // Store biome type in trigger box for later reference
    TriggerBox->Tags.Add(FName(*FString::Printf(TEXT("BiomeType_%d"), (int32)BiomeType)));
}

void UNarr_BiomeNarrativeSystem::PlayEmergencyAlert(const FString& AlertMessage, const FVector& DangerLocation)
{
    UE_LOG(LogTemp, Error, TEXT("Narr_BiomeNarrativeSystem: EMERGENCY ALERT - %s"), *AlertMessage);
    
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("EMERGENCY: %s"), *AlertMessage);
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, DisplayText);
    }
    
    // Play emergency sound if available
    if (EmergencyAlertSound && NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetSound(EmergencyAlertSound);
        NarrativeAudioComponent->Play();
    }
    
    // Could trigger additional emergency systems here
    // e.g., UI alerts, screen effects, etc.
}

bool UNarr_BiomeNarrativeSystem::IsBiomeNarrativeOnCooldown(ENarr_BiomeType BiomeType) const
{
    const float* LastTriggerTime = LastTriggerTimes.Find(BiomeType);
    if (!LastTriggerTime)
    {
        return false; // Never triggered, not on cooldown
    }
    
    const FNarr_BiomeNarrativeData* NarrativeData = BiomeNarratives.Find(BiomeType);
    if (!NarrativeData)
    {
        return true; // No data, consider on cooldown
    }
    
    float TimeSinceLastTrigger = GetCurrentTime() - *LastTriggerTime;
    return TimeSinceLastTrigger < NarrativeData->TriggerCooldown;
}

void UNarr_BiomeNarrativeSystem::PlayNarrativeAudio(const FString& AudioURL)
{
    if (AudioURL.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Empty audio URL provided"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_BiomeNarrativeSystem: Playing narrative audio from URL: %s"), *AudioURL);
    
    // In a full implementation, this would load and play audio from the URL
    // For now, we log the URL and could trigger a Blueprint event
    if (NarrativeAudioComponent)
    {
        // This would require additional audio loading system
        // NarrativeAudioComponent->Play();
    }
}

float UNarr_BiomeNarrativeSystem::GetCurrentTime() const
{
    if (GetWorld())
    {
        return GetWorld()->GetTimeSeconds();
    }
    return 0.0f;
}