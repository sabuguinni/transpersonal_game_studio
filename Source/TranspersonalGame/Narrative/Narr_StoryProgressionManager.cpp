#include "Narr_StoryProgressionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalGameState.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "TranspersonalGame/SharedTypes.h"

UNarr_StoryProgressionManager::UNarr_StoryProgressionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize story progression data
    CurrentChapter = ENarr_StoryChapter::Arrival;
    CurrentObjective = ENarr_ObjectiveType::Explore;
    StoryProgress = 0.0f;
    
    // Initialize survival thresholds for story progression
    HealthThresholdForDanger = 30.0f;
    HungerThresholdForUrgency = 70.0f;
    FearThresholdForPanic = 80.0f;
    
    // Initialize discovery counters
    DinosaurSpeciesDiscovered = 0;
    BiomesExplored = 0;
    Dayssurvived = 0;
    
    bIsStoryActive = true;
    bCanProgressStory = true;
    LastStoryEventTime = 0.0f;
    StoryEventCooldown = 30.0f; // 30 seconds between story events
}

void UNarr_StoryProgressionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to game state for survival stats
    if (UWorld* World = GetWorld())
    {
        GameStateRef = Cast<ATranspersonalGameState>(World->GetGameState());
        if (GameStateRef)
        {
            UE_LOG(LogTemp, Warning, TEXT("StoryProgressionManager: Connected to GameState"));
        }
    }
    
    // Initialize story with arrival chapter
    StartStoryChapter(ENarr_StoryChapter::Arrival);
}

void UNarr_StoryProgressionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsStoryActive || !bCanProgressStory)
    {
        return;
    }
    
    // Check for story progression conditions
    CheckStoryProgressionConditions();
    
    // Update story event timing
    LastStoryEventTime += DeltaTime;
    
    // Check for automatic story events based on survival state
    CheckSurvivalBasedStoryEvents();
}

void UNarr_StoryProgressionManager::CheckStoryProgressionConditions()
{
    if (!GameStateRef)
    {
        return;
    }
    
    // Check chapter progression based on current chapter
    switch (CurrentChapter)
    {
        case ENarr_StoryChapter::Arrival:
            CheckArrivalProgression();
            break;
            
        case ENarr_StoryChapter::FirstContact:
            CheckFirstContactProgression();
            break;
            
        case ENarr_StoryChapter::Survival:
            CheckSurvivalProgression();
            break;
            
        case ENarr_StoryChapter::Discovery:
            CheckDiscoveryProgression();
            break;
            
        case ENarr_StoryChapter::Adaptation:
            CheckAdaptationProgression();
            break;
            
        case ENarr_StoryChapter::Mastery:
            CheckMasteryProgression();
            break;
            
        default:
            break;
    }
}

void UNarr_StoryProgressionManager::CheckArrivalProgression()
{
    // Progress from Arrival to FirstContact when player moves significantly
    if (GameStateRef)
    {
        // Check if player has moved from spawn point
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            FVector CurrentLocation = Player->GetActorLocation();
            float DistanceFromSpawn = FVector::Dist(CurrentLocation, FVector::ZeroVector);
            
            if (DistanceFromSpawn > 1000.0f) // Player moved 10 meters from spawn
            {
                StartStoryChapter(ENarr_StoryChapter::FirstContact);
            }
        }
    }
}

void UNarr_StoryProgressionManager::CheckFirstContactProgression()
{
    // Progress when player first encounters a dinosaur
    if (DinosaurSpeciesDiscovered >= 1)
    {
        StartStoryChapter(ENarr_StoryChapter::Survival);
    }
}

void UNarr_StoryProgressionManager::CheckSurvivalProgression()
{
    // Progress when player has survived for a while and explored
    if (DaysEnabled >= 2 && BiomesExplored >= 2)
    {
        StartStoryChapter(ENarr_StoryChapter::Discovery);
    }
}

void UNarr_StoryProgressionManager::CheckDiscoveryProgression()
{
    // Progress when player has discovered multiple species and biomes
    if (DinosaurSpeciesDiscovered >= 5 && BiomesExplored >= 4)
    {
        StartStoryChapter(ENarr_StoryChapter::Adaptation);
    }
}

void UNarr_StoryProgressionManager::CheckAdaptationProgression()
{
    // Progress when player has mastered survival mechanics
    if (DaysEnabled >= 7 && GameStateRef && GameStateRef->Health > 80.0f && GameStateRef->Hunger < 30.0f)
    {
        StartStoryChapter(ENarr_StoryChapter::Mastery);
    }
}

void UNarr_StoryProgressionManager::CheckMasteryProgression()
{
    // Final chapter - story complete
    StoryProgress = 100.0f;
}

void UNarr_StoryProgressionManager::StartStoryChapter(ENarr_StoryChapter NewChapter)
{
    if (NewChapter == CurrentChapter)
    {
        return;
    }
    
    ENarr_StoryChapter PreviousChapter = CurrentChapter;
    CurrentChapter = NewChapter;
    
    // Calculate story progress percentage
    int32 ChapterIndex = static_cast<int32>(CurrentChapter);
    int32 TotalChapters = static_cast<int32>(ENarr_StoryChapter::Mastery) + 1;
    StoryProgress = (static_cast<float>(ChapterIndex) / static_cast<float>(TotalChapters)) * 100.0f;
    
    // Trigger chapter start event
    OnChapterStarted.Broadcast(CurrentChapter, PreviousChapter);
    
    // Set appropriate objective for new chapter
    SetObjectiveForChapter(NewChapter);
    
    UE_LOG(LogTemp, Warning, TEXT("Story Chapter Started: %d, Progress: %.1f%%"), 
           static_cast<int32>(NewChapter), StoryProgress);
}

void UNarr_StoryProgressionManager::SetObjectiveForChapter(ENarr_StoryChapter Chapter)
{
    switch (Chapter)
    {
        case ENarr_StoryChapter::Arrival:
            CurrentObjective = ENarr_ObjectiveType::Explore;
            break;
            
        case ENarr_StoryChapter::FirstContact:
            CurrentObjective = ENarr_ObjectiveType::Observe;
            break;
            
        case ENarr_StoryChapter::Survival:
            CurrentObjective = ENarr_ObjectiveType::Survive;
            break;
            
        case ENarr_StoryChapter::Discovery:
            CurrentObjective = ENarr_ObjectiveType::Document;
            break;
            
        case ENarr_StoryChapter::Adaptation:
            CurrentObjective = ENarr_ObjectiveType::Adapt;
            break;
            
        case ENarr_StoryChapter::Mastery:
            CurrentObjective = ENarr_ObjectiveType::Master;
            break;
    }
    
    OnObjectiveChanged.Broadcast(CurrentObjective);
}

void UNarr_StoryProgressionManager::CheckSurvivalBasedStoryEvents()
{
    if (!GameStateRef || LastStoryEventTime < StoryEventCooldown)
    {
        return;
    }
    
    // Check for critical health story event
    if (GameStateRef->Health <= HealthThresholdForDanger)
    {
        TriggerStoryEvent(ENarr_StoryEventType::HealthCritical);
    }
    // Check for high hunger story event
    else if (GameStateRef->Hunger >= HungerThresholdForUrgency)
    {
        TriggerStoryEvent(ENarr_StoryEventType::HungerUrgent);
    }
    // Check for high fear story event
    else if (GameStateRef->Fear >= FearThresholdForPanic)
    {
        TriggerStoryEvent(ENarr_StoryEventType::FearPanic);
    }
}

void UNarr_StoryProgressionManager::TriggerStoryEvent(ENarr_StoryEventType EventType)
{
    LastStoryEventTime = 0.0f; // Reset cooldown
    
    OnStoryEventTriggered.Broadcast(EventType, CurrentChapter);
    
    UE_LOG(LogTemp, Warning, TEXT("Story Event Triggered: %d in Chapter: %d"), 
           static_cast<int32>(EventType), static_cast<int32>(CurrentChapter));
}

void UNarr_StoryProgressionManager::OnDinosaurDiscovered(const FString& SpeciesName)
{
    if (!DiscoveredSpecies.Contains(SpeciesName))
    {
        DiscoveredSpecies.Add(SpeciesName);
        DinosaurSpeciesDiscovered = DiscoveredSpecies.Num();
        
        TriggerStoryEvent(ENarr_StoryEventType::DinosaurDiscovered);
        
        UE_LOG(LogTemp, Warning, TEXT("New dinosaur species discovered: %s (Total: %d)"), 
               *SpeciesName, DinosaurSpeciesDiscovered);
    }
}

void UNarr_StoryProgressionManager::OnBiomeEntered(ENarr_BiomeType BiomeType)
{
    if (!ExploredBiomes.Contains(BiomeType))
    {
        ExploredBiomes.Add(BiomeType);
        BiomesExplored = ExploredBiomes.Num();
        
        TriggerStoryEvent(ENarr_StoryEventType::BiomeDiscovered);
        
        UE_LOG(LogTemp, Warning, TEXT("New biome explored: %d (Total: %d)"), 
               static_cast<int32>(BiomeType), BiomesExplored);
    }
}

void UNarr_StoryProgressionManager::OnDayPassed()
{
    DaysEnabled++;
    
    if (DaysEnabled % 3 == 0) // Every 3 days
    {
        TriggerStoryEvent(ENarr_StoryEventType::TimeProgression);
    }
}

bool UNarr_StoryProgressionManager::CanProgressToChapter(ENarr_StoryChapter TargetChapter) const
{
    // Check if we can progress to the target chapter based on current progress
    int32 CurrentChapterIndex = static_cast<int32>(CurrentChapter);
    int32 TargetChapterIndex = static_cast<int32>(TargetChapter);
    
    // Can only progress to next chapter or stay in current
    return TargetChapterIndex <= CurrentChapterIndex + 1;
}

FString UNarr_StoryProgressionManager::GetCurrentChapterName() const
{
    switch (CurrentChapter)
    {
        case ENarr_StoryChapter::Arrival: return TEXT("Arrival");
        case ENarr_StoryChapter::FirstContact: return TEXT("First Contact");
        case ENarr_StoryChapter::Survival: return TEXT("Survival");
        case ENarr_StoryChapter::Discovery: return TEXT("Discovery");
        case ENarr_StoryChapter::Adaptation: return TEXT("Adaptation");
        case ENarr_StoryChapter::Mastery: return TEXT("Mastery");
        default: return TEXT("Unknown");
    }
}

FString UNarr_StoryProgressionManager::GetCurrentObjectiveName() const
{
    switch (CurrentObjective)
    {
        case ENarr_ObjectiveType::Explore: return TEXT("Explore the Environment");
        case ENarr_ObjectiveType::Survive: return TEXT("Maintain Survival");
        case ENarr_ObjectiveType::Observe: return TEXT("Observe Wildlife");
        case ENarr_ObjectiveType::Document: return TEXT("Document Discoveries");
        case ENarr_ObjectiveType::Adapt: return TEXT("Adapt to Environment");
        case ENarr_ObjectiveType::Master: return TEXT("Master the Prehistoric World");
        default: return TEXT("Unknown Objective");
    }
}