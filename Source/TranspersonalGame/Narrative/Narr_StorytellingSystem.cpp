#include "Narr_StorytellingSystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UNarr_StorytellingSystem::UNarr_StorytellingSystem()
{
    bIsPlayingStory = false;
    StoryStartTime = 0.0f;
    StoryDataTable = nullptr;
}

void UNarr_StorytellingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadStoryData();
    UE_LOG(LogTemp, Log, TEXT("Narrative Storytelling System initialized"));
}

void UNarr_StorytellingSystem::LoadStoryData()
{
    CachedStories.Empty();
    
    // Add hardcoded story data for immediate functionality
    FNarr_StoryData DinosaurStory;
    DinosaurStory.StoryTitle = TEXT("The Great Predator");
    DinosaurStory.StoryText = FText::FromString(TEXT("The Tyrannosaurus stalks through the dense ferns, its yellow eyes searching for prey."));
    DinosaurStory.StoryType = ENarr_StoryType::DinosaurEncounter;
    DinosaurStory.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482258631_TribalWarrior.mp3");
    DinosaurStory.StoryDuration = 13.0f;
    DinosaurStory.RequiredTags.Add(TEXT("TRex"));
    CachedStories.Add(DinosaurStory);

    FNarr_StoryData HerdStory;
    HerdStory.StoryTitle = TEXT("The Thundering Herd");
    HerdStory.StoryText = FText::FromString(TEXT("The Triceratops herd moves through the eastern valley at dawn."));
    HerdStory.StoryType = ENarr_StoryType::DinosaurEncounter;
    HerdStory.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482253207_TribalScout.mp3");
    HerdStory.StoryDuration = 16.0f;
    HerdStory.RequiredTags.Add(TEXT("Triceratops"));
    CachedStories.Add(HerdStory);

    FNarr_StoryData PackStory;
    PackStory.StoryTitle = TEXT("The Pack Hunters");
    PackStory.StoryText = FText::FromString(TEXT("Three Velociraptors move as one mind, one deadly purpose."));
    PackStory.StoryType = ENarr_StoryType::DinosaurEncounter;
    PackStory.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482265324_TribalHunter.mp3");
    PackStory.StoryDuration = 14.0f;
    PackStory.RequiredTags.Add(TEXT("Velociraptor"));
    CachedStories.Add(PackStory);

    FNarr_StoryData GiantStory;
    GiantStory.StoryTitle = TEXT("The Gentle Giants");
    GiantStory.StoryText = FText::FromString(TEXT("The Brachiosaurus towers above the canopy, reaching for the sweetest leaves."));
    GiantStory.StoryType = ENarr_StoryType::TribalWisdom;
    GiantStory.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482275958_TribalElder.mp3");
    GiantStory.StoryDuration = 16.0f;
    GiantStory.RequiredTags.Add(TEXT("Brachiosaurus"));
    CachedStories.Add(GiantStory);

    UE_LOG(LogTemp, Log, TEXT("Loaded %d hardcoded stories"), CachedStories.Num());
}

void UNarr_StorytellingSystem::TriggerContextualStory(const FNarr_StorytellingContext& Context)
{
    if (bIsPlayingStory)
    {
        UE_LOG(LogTemp, Warning, TEXT("Story already playing, ignoring trigger"));
        return;
    }

    TArray<FNarr_StoryData> MatchingStories = GetStoriesForContext(Context);
    if (MatchingStories.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, MatchingStories.Num() - 1);
        FNarr_StoryData SelectedStory = MatchingStories[RandomIndex];
        
        CurrentStory = SelectedStory;
        PlayStoryAudio(SelectedStory);
        
        UE_LOG(LogTemp, Log, TEXT("Triggered contextual story: %s"), *SelectedStory.StoryTitle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No matching stories found for context"));
    }
}

FNarr_StoryData UNarr_StorytellingSystem::GetRandomStoryByType(ENarr_StoryType StoryType)
{
    TArray<FNarr_StoryData> FilteredStories;
    
    for (const FNarr_StoryData& Story : CachedStories)
    {
        if (Story.StoryType == StoryType)
        {
            FilteredStories.Add(Story);
        }
    }
    
    if (FilteredStories.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, FilteredStories.Num() - 1);
        return FilteredStories[RandomIndex];
    }
    
    return FNarr_StoryData();
}

TArray<FNarr_StoryData> UNarr_StorytellingSystem::GetStoriesForContext(const FNarr_StorytellingContext& Context)
{
    TArray<FNarr_StoryData> MatchingStories;
    
    for (const FNarr_StoryData& Story : CachedStories)
    {
        if (DoesStoryMatchContext(Story, Context))
        {
            MatchingStories.Add(Story);
        }
    }
    
    return MatchingStories;
}

bool UNarr_StorytellingSystem::DoesStoryMatchContext(const FNarr_StoryData& Story, const FNarr_StorytellingContext& Context)
{
    // Check if any nearby dinosaurs match story requirements
    for (const FString& RequiredTag : Story.RequiredTags)
    {
        for (const FString& NearbyDino : Context.NearbyDinosaurs)
        {
            if (NearbyDino.Contains(RequiredTag))
            {
                return true;
            }
        }
    }
    
    // Context-based matching for story types
    if (Story.StoryType == ENarr_StoryType::TribalWisdom && Context.bIsNearFire)
    {
        return true;
    }
    
    if (Story.StoryType == ENarr_StoryType::TerrainWarning && Context.CurrentBiome.Contains(TEXT("Mountain")))
    {
        return true;
    }
    
    return false;
}

void UNarr_StorytellingSystem::PlayStoryAudio(const FNarr_StoryData& Story)
{
    if (Story.AudioAssetPath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No audio path for story: %s"), *Story.StoryTitle);
        return;
    }
    
    bIsPlayingStory = true;
    StoryStartTime = GetWorld()->GetTimeSeconds();
    
    // Log the story being played (audio URLs are external, so we log for reference)
    UE_LOG(LogTemp, Log, TEXT("Playing story audio: %s (Duration: %.1fs)"), *Story.StoryTitle, Story.StoryDuration);
    UE_LOG(LogTemp, Log, TEXT("Audio URL: %s"), *Story.AudioAssetPath);
    
    // Set timer to mark story as finished
    FTimerHandle StoryTimer;
    GetWorld()->GetTimerManager().SetTimer(StoryTimer, this, &UNarr_StorytellingSystem::OnStoryFinished, Story.StoryDuration, false);
}

bool UNarr_StorytellingSystem::IsStoryPlaying() const
{
    return bIsPlayingStory;
}

void UNarr_StorytellingSystem::StopCurrentStory()
{
    if (bIsPlayingStory)
    {
        bIsPlayingStory = false;
        UE_LOG(LogTemp, Log, TEXT("Stopped current story"));
    }
}

void UNarr_StorytellingSystem::OnStoryFinished()
{
    bIsPlayingStory = false;
    UE_LOG(LogTemp, Log, TEXT("Story finished: %s"), *CurrentStory.StoryTitle);
}