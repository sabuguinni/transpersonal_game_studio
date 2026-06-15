#include "Narr_StorytellingSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UNarr_StorytellingSystem::UNarr_StorytellingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsPlayingStory = false;
    StoryPlaybackTimer = 0.0f;
    ContextualTriggerDistance = 2000.0f;
    PlayerReference = nullptr;
}

void UNarr_StorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerReference = PC->GetPawn();
        }
    }
    
    // Initialize default stories
    InitializeDefaultStories();
}

void UNarr_StorytellingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update story playback timer
    if (bIsPlayingStory)
    {
        StoryPlaybackTimer += DeltaTime;
        
        if (StoryPlaybackTimer >= CurrentStory.StoryDuration)
        {
            OnStoryComplete();
        }
    }
    
    // Check for contextual story triggers
    if (PlayerReference)
    {
        CheckContextualTriggers(PlayerReference->GetActorLocation());
    }
}

void UNarr_StorytellingSystem::StartStory(const FNarr_StoryData& Story)
{
    if (bIsPlayingStory)
    {
        StopCurrentStory();
    }
    
    CurrentStory = Story;
    bIsPlayingStory = true;
    StoryPlaybackTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting Story: %s"), *Story.StoryTitle);
    UE_LOG(LogTemp, Warning, TEXT("Story Text: %s"), *Story.StoryText);
    
    // TODO: Play audio asset when audio system is integrated
    // For now, just log the story content
}

void UNarr_StorytellingSystem::StopCurrentStory()
{
    if (bIsPlayingStory)
    {
        bIsPlayingStory = false;
        StoryPlaybackTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Story stopped: %s"), *CurrentStory.StoryTitle);
    }
}

bool UNarr_StorytellingSystem::IsStoryPlaying() const
{
    return bIsPlayingStory;
}

FNarr_StoryData UNarr_StorytellingSystem::GetRandomStoryByType(ENarr_StoryType StoryType)
{
    TArray<FNarr_StoryData> FilteredStories;
    
    for (const FNarr_StoryData& Story : AvailableStories)
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
    
    return FNarr_StoryData(); // Return empty story if none found
}

void UNarr_StorytellingSystem::AddStoryToLibrary(const FNarr_StoryData& NewStory)
{
    AvailableStories.Add(NewStory);
    UE_LOG(LogTemp, Log, TEXT("Added story to library: %s"), *NewStory.StoryTitle);
}

void UNarr_StorytellingSystem::TriggerContextualStory(FVector PlayerLocation)
{
    if (bIsPlayingStory) return; // Don't interrupt current story
    
    for (const FNarr_StoryData& Story : AvailableStories)
    {
        if (Story.bIsContextual)
        {
            float Distance = FVector::Dist(PlayerLocation, Story.TriggerLocation);
            if (Distance <= Story.TriggerRadius)
            {
                StartStory(Story);
                break; // Only trigger one story at a time
            }
        }
    }
}

void UNarr_StorytellingSystem::InitializeDefaultStories()
{
    // Tribal Elder Stories
    FNarr_StoryData ElderStory1;
    ElderStory1.StoryTitle = TEXT("The Ancient Fire");
    ElderStory1.StoryText = TEXT("The ancient fire burns bright tonight, young hunter. Listen well to the words of your ancestors - the great beasts of this land hold secrets that only the brave may discover.");
    ElderStory1.StoryType = ENarr_StoryType::Tribal_Legend;
    ElderStory1.NarratorType = ENarr_NarratorType::Tribal_Elder;
    ElderStory1.StoryDuration = 12.0f;
    ElderStory1.bIsContextual = true;
    ElderStory1.TriggerLocation = FVector(0, 0, 100); // Near campfire
    ElderStory1.TriggerRadius = 1500.0f;
    AddStoryToLibrary(ElderStory1);
    
    // Scout Warning Stories
    FNarr_StoryData ScoutWarning;
    ScoutWarning.StoryTitle = TEXT("Shadow of Death");
    ScoutWarning.StoryText = TEXT("The shadow of death approaches from the north. Three suns have passed since the great predator claimed its last victim. Prepare your spear, for the hunt begins at dawn.");
    ScoutWarning.StoryType = ENarr_StoryType::Warning_Story;
    ScoutWarning.NarratorType = ENarr_NarratorType::Scout_Warrior;
    ScoutWarning.StoryDuration = 12.0f;
    ScoutWarning.bIsContextual = true;
    ScoutWarning.TriggerLocation = FVector(5000, 0, 100); // North territory
    ScoutWarning.TriggerRadius = 2000.0f;
    AddStoryToLibrary(ScoutWarning);
    
    // Tracker Report Stories
    FNarr_StoryData TrackerReport;
    TrackerReport.StoryTitle = TEXT("River Speaks");
    TrackerReport.StoryText = TEXT("The river speaks of strange movements upstream. The gentle giants flee toward the eastern cliffs, their heavy footsteps shaking the earth beneath our feet.");
    TrackerReport.StoryType = ENarr_StoryType::Beast_Knowledge;
    TrackerReport.NarratorType = ENarr_NarratorType::Tracker_Guide;
    TrackerReport.StoryDuration = 11.0f;
    TrackerReport.bIsContextual = true;
    TrackerReport.TriggerLocation = FVector(0, 5000, 50); // Near river
    TrackerReport.TriggerRadius = 1800.0f;
    AddStoryToLibrary(TrackerReport);
    
    // Hunter Territory Stories
    FNarr_StoryData HunterTerritory;
    HunterTerritory.StoryTitle = TEXT("Blood and Bone");
    HunterTerritory.StoryText = TEXT("Blood and bone mark this territory now. The pack hunters have claimed these caves as their own. Only the strongest will emerge from their domain alive.");
    HunterTerritory.StoryType = ENarr_StoryType::Territory_Lore;
    HunterTerritory.NarratorType = ENarr_NarratorType::Hunter_Veteran;
    HunterTerritory.StoryDuration = 11.0f;
    HunterTerritory.bIsContextual = true;
    HunterTerritory.TriggerLocation = FVector(-3000, -3000, 200); // Cave area
    HunterTerritory.TriggerRadius = 2500.0f;
    AddStoryToLibrary(HunterTerritory);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default stories"), AvailableStories.Num());
}

void UNarr_StorytellingSystem::OnStoryComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Story completed: %s"), *CurrentStory.StoryTitle);
    bIsPlayingStory = false;
    StoryPlaybackTimer = 0.0f;
}

void UNarr_StorytellingSystem::CheckContextualTriggers(FVector PlayerLocation)
{
    if (bIsPlayingStory) return; // Don't check triggers while story is playing
    
    // Check every 2 seconds to avoid performance issues
    static float TriggerCheckTimer = 0.0f;
    TriggerCheckTimer += GetWorld()->GetDeltaSeconds();
    
    if (TriggerCheckTimer >= 2.0f)
    {
        TriggerCheckTimer = 0.0f;
        TriggerContextualStory(PlayerLocation);
    }
}