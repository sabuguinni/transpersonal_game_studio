#include "StorytellingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UStorytellingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsPlaying = false;
    CurrentAudioComponent = nullptr;
    
    InitializeDefaultStories();
    InitializeDefaultStorytellers();
    
    UE_LOG(LogTemp, Warning, TEXT("StorytellingSystem initialized with %d stories and %d storytellers"), 
           Stories.Num(), Storytellers.Num());
}

void UStorytellingSystem::Deinitialize()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent = nullptr;
    }
    
    Stories.Empty();
    Storytellers.Empty();
    bIsPlaying = false;
    
    Super::Deinitialize();
}

void UStorytellingSystem::RegisterStory(const FNarr_StoryData& StoryData)
{
    if (!StoryData.StoryID.IsEmpty())
    {
        Stories.Add(StoryData.StoryID, StoryData);
        UE_LOG(LogTemp, Log, TEXT("Registered story: %s"), *StoryData.Title);
    }
}

FNarr_StoryData UStorytellingSystem::GetStoryByID(const FString& StoryID)
{
    if (Stories.Contains(StoryID))
    {
        return Stories[StoryID];
    }
    return FNarr_StoryData();
}

TArray<FNarr_StoryData> UStorytellingSystem::GetStoriesByType(ENarr_StoryType StoryType)
{
    TArray<FNarr_StoryData> FilteredStories;
    
    for (const auto& StoryPair : Stories)
    {
        if (StoryPair.Value.StoryType == StoryType && StoryPair.Value.bIsUnlocked)
        {
            FilteredStories.Add(StoryPair.Value);
        }
    }
    
    return FilteredStories;
}

TArray<FNarr_StoryData> UStorytellingSystem::GetUnlockedStories()
{
    TArray<FNarr_StoryData> UnlockedStories;
    
    for (const auto& StoryPair : Stories)
    {
        if (StoryPair.Value.bIsUnlocked)
        {
            UnlockedStories.Add(StoryPair.Value);
        }
    }
    
    return UnlockedStories;
}

bool UStorytellingSystem::UnlockStory(const FString& StoryID)
{
    if (Stories.Contains(StoryID))
    {
        FNarr_StoryData& Story = Stories[StoryID];
        
        if (!Story.bIsUnlocked && CheckUnlockConditions(Story))
        {
            Story.bIsUnlocked = true;
            OnStoryUnlocked.Broadcast(Story);
            UE_LOG(LogTemp, Warning, TEXT("Story unlocked: %s"), *Story.Title);
            return true;
        }
    }
    
    return false;
}

void UStorytellingSystem::RegisterStoryteller(const FNarr_StorytellerProfile& Profile)
{
    if (!Profile.StorytellerID.IsEmpty())
    {
        Storytellers.Add(Profile.StorytellerID, Profile);
        UE_LOG(LogTemp, Log, TEXT("Registered storyteller: %s"), *Profile.Name);
    }
}

FNarr_StorytellerProfile UStorytellingSystem::GetStorytellerByID(const FString& StorytellerID)
{
    if (Storytellers.Contains(StorytellerID))
    {
        return Storytellers[StorytellerID];
    }
    return FNarr_StorytellerProfile();
}

TArray<FNarr_StorytellerProfile> UStorytellingSystem::GetAvailableStorytellers()
{
    TArray<FNarr_StorytellerProfile> AvailableStorytellers;
    
    for (const auto& StorytellerPair : Storytellers)
    {
        if (StorytellerPair.Value.bIsAvailable)
        {
            AvailableStorytellers.Add(StorytellerPair.Value);
        }
    }
    
    return AvailableStorytellers;
}

void UStorytellingSystem::PlayStory(const FString& StoryID, AActor* ListenerActor)
{
    if (bIsPlaying)
    {
        StopCurrentStory();
    }
    
    if (!Stories.Contains(StoryID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Story not found: %s"), *StoryID);
        return;
    }
    
    CurrentStory = Stories[StoryID];
    
    if (!CurrentStory.bIsUnlocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("Story is locked: %s"), *CurrentStory.Title);
        return;
    }
    
    // For now, we'll simulate audio playback since we have the audio URLs
    bIsPlaying = true;
    OnStoryStarted.Broadcast(CurrentStory);
    
    UE_LOG(LogTemp, Warning, TEXT("Playing story: %s by %s"), 
           *CurrentStory.Title, *CurrentStory.CharacterName);
    
    // Simulate story duration and finish
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UStorytellingSystem::OnAudioFinished, 
                                         CurrentStory.Duration, false);
    }
}

void UStorytellingSystem::StopCurrentStory()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent = nullptr;
    }
    
    bIsPlaying = false;
    UE_LOG(LogTemp, Log, TEXT("Story playback stopped"));
}

bool UStorytellingSystem::IsStoryPlaying() const
{
    return bIsPlaying;
}

FNarr_StoryData UStorytellingSystem::SelectAppropriateStory(ENarr_StoryMood DesiredMood, const FString& StorytellerID)
{
    TArray<FNarr_StoryData> CandidateStories;
    
    for (const auto& StoryPair : Stories)
    {
        const FNarr_StoryData& Story = StoryPair.Value;
        
        if (!Story.bIsUnlocked)
            continue;
            
        bool bMoodMatch = (Story.Mood == DesiredMood);
        bool bStorytellerMatch = (StorytellerID.IsEmpty() || Story.CharacterName == StorytellerID);
        
        if (bMoodMatch && bStorytellerMatch)
        {
            CandidateStories.Add(Story);
        }
    }
    
    if (CandidateStories.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CandidateStories.Num() - 1);
        return CandidateStories[RandomIndex];
    }
    
    return FNarr_StoryData();
}

FNarr_StoryData UStorytellingSystem::SelectRandomStory(ENarr_StoryType PreferredType)
{
    TArray<FNarr_StoryData> CandidateStories = GetStoriesByType(PreferredType);
    
    if (CandidateStories.Num() == 0)
    {
        CandidateStories = GetUnlockedStories();
    }
    
    if (CandidateStories.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CandidateStories.Num() - 1);
        return CandidateStories[RandomIndex];
    }
    
    return FNarr_StoryData();
}

void UStorytellingSystem::InitializeDefaultStories()
{
    // Story 1: Ancient Fire
    FNarr_StoryData Story1;
    Story1.StoryID = TEXT("ancient_fire");
    Story1.Title = TEXT("The Ancient Fire");
    Story1.Content = TEXT("The ancient fire burns bright tonight, young hunter. Listen well to the stories of our ancestors who walked among the great beasts.");
    Story1.StoryType = ENarr_StoryType::Teaching;
    Story1.Mood = ENarr_StoryMood::Hopeful;
    Story1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781486869123_TribalStoryteller.mp3");
    Story1.Duration = 18.0f;
    Story1.CharacterName = TEXT("TribalStoryteller");
    Story1.bIsUnlocked = true;
    RegisterStory(Story1);
    
    // Story 2: Shadow Warning
    FNarr_StoryData Story2;
    Story2.StoryID = TEXT("shadow_warning");
    Story2.Title = TEXT("Beware the Shadow");
    Story2.Content = TEXT("Beware the shadow that moves through the tall grass! The pack hunters are cunning, they strike from darkness.");
    Story2.StoryType = ENarr_StoryType::Warning;
    Story2.Mood = ENarr_StoryMood::Fearful;
    Story2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781486874560_TribalGuard.mp3");
    Story2.Duration = 15.0f;
    Story2.CharacterName = TEXT("TribalGuard");
    Story2.bIsUnlocked = true;
    RegisterStory(Story2);
    
    // Story 3: River Prophecy
    FNarr_StoryData Story3;
    Story3.StoryID = TEXT("river_prophecy");
    Story3.Title = TEXT("The River's Warning");
    Story3.Content = TEXT("The great river speaks of change coming to our lands. The herds move restlessly, the earth trembles with new footsteps.");
    Story3.StoryType = ENarr_StoryType::Prophecy;
    Story3.Mood = ENarr_StoryMood::Mysterious;
    Story3.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781486880363_TribalOracle.mp3");
    Story3.Duration = 14.0f;
    Story3.CharacterName = TEXT("TribalOracle");
    Story3.bIsUnlocked = true;
    RegisterStory(Story3);
    
    // Story 4: Path of Leadership
    FNarr_StoryData Story4;
    Story4.StoryID = TEXT("path_leadership");
    Story4.Title = TEXT("The Path of Leadership");
    Story4.Content = TEXT("You have walked the path of survival and proven your strength against the ancient beasts. Now you must choose your destiny.");
    Story4.StoryType = ENarr_StoryType::Victory;
    Story4.Mood = ENarr_StoryMood::Triumphant;
    Story4.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781486887118_TribalChief.mp3");
    Story4.Duration = 17.0f;
    Story4.CharacterName = TEXT("TribalChief");
    Story4.UnlockConditions.Add(TEXT("complete_first_hunt"));
    Story4.bIsUnlocked = false;
    RegisterStory(Story4);
}

void UStorytellingSystem::InitializeDefaultStorytellers()
{
    // Tribal Storyteller
    FNarr_StorytellerProfile Storyteller1;
    Storyteller1.StorytellerID = TEXT("tribal_storyteller");
    Storyteller1.Name = TEXT("Kael the Storyteller");
    Storyteller1.Role = TEXT("Elder Storyteller");
    Storyteller1.KnownStories.Add(TEXT("ancient_fire"));
    Storyteller1.PreferredTypes.Add(ENarr_StoryType::Teaching);
    Storyteller1.PreferredTypes.Add(ENarr_StoryType::Legend);
    Storyteller1.TrustLevel = 0.9f;
    Storyteller1.bIsAvailable = true;
    RegisterStoryteller(Storyteller1);
    
    // Tribal Guard
    FNarr_StorytellerProfile Storyteller2;
    Storyteller2.StorytellerID = TEXT("tribal_guard");
    Storyteller2.Name = TEXT("Thane the Watchful");
    Storyteller2.Role = TEXT("Night Guard");
    Storyteller2.KnownStories.Add(TEXT("shadow_warning"));
    Storyteller2.PreferredTypes.Add(ENarr_StoryType::Warning);
    Storyteller2.TrustLevel = 0.8f;
    Storyteller2.bIsAvailable = true;
    RegisterStoryteller(Storyteller2);
    
    // Tribal Oracle
    FNarr_StorytellerProfile Storyteller3;
    Storyteller3.StorytellerID = TEXT("tribal_oracle");
    Storyteller3.Name = TEXT("Vera the Seer");
    Storyteller3.Role = TEXT("Oracle");
    Storyteller3.KnownStories.Add(TEXT("river_prophecy"));
    Storyteller3.PreferredTypes.Add(ENarr_StoryType::Prophecy);
    Storyteller3.PreferredTypes.Add(ENarr_StoryType::Mysterious);
    Storyteller3.TrustLevel = 0.7f;
    Storyteller3.bIsAvailable = true;
    RegisterStoryteller(Storyteller3);
    
    // Tribal Chief
    FNarr_StorytellerProfile Storyteller4;
    Storyteller4.StorytellerID = TEXT("tribal_chief");
    Storyteller4.Name = TEXT("Ragnar the Bold");
    Storyteller4.Role = TEXT("Tribe Chief");
    Storyteller4.KnownStories.Add(TEXT("path_leadership"));
    Storyteller4.PreferredTypes.Add(ENarr_StoryType::Victory);
    Storyteller4.PreferredTypes.Add(ENarr_StoryType::Teaching);
    Storyteller4.TrustLevel = 1.0f;
    Storyteller4.bIsAvailable = false; // Only available after certain achievements
    RegisterStoryteller(Storyteller4);
}

void UStorytellingSystem::OnAudioFinished()
{
    bIsPlaying = false;
    OnStoryFinished.Broadcast(CurrentStory);
    UE_LOG(LogTemp, Warning, TEXT("Story finished: %s"), *CurrentStory.Title);
}

bool UStorytellingSystem::CheckUnlockConditions(const FNarr_StoryData& Story)
{
    // For now, simple implementation - in real game would check game state
    if (Story.UnlockConditions.Num() == 0)
    {
        return true;
    }
    
    // Placeholder logic - would integrate with quest/achievement system
    for (const FString& Condition : Story.UnlockConditions)
    {
        if (Condition == TEXT("complete_first_hunt"))
        {
            // Check if player has completed first hunt quest
            return false; // Placeholder
        }
    }
    
    return false;
}