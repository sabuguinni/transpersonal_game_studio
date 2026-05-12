#include "Narr_MainStoryArc.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"
#include "TranspersonalGame/SharedTypes.h"

UNarr_MainStoryArc::UNarr_MainStoryArc()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize story progression
    CurrentChapter = ENarr_StoryChapter::Arrival;
    CurrentObjective = ENarr_StoryObjective::FindShelter;
    StoryProgress = 0.0f;
    
    // Initialize chapter data
    InitializeChapterData();
    
    // Audio components for narration
    NarrationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudio"));
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->bAutoActivate = false;
        NarrationAudioComponent->SetVolumeMultiplier(0.8f);
    }
    
    // Initialize survival thresholds
    CriticalHealthThreshold = 25.0f;
    CriticalHungerThreshold = 20.0f;
    CriticalThirstThreshold = 15.0f;
    
    bIsNarrationPlaying = false;
    LastNarrationTime = 0.0f;
    NarrationCooldown = 30.0f;
}

void UNarr_MainStoryArc::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with arrival chapter
    StartChapter(ENarr_StoryChapter::Arrival);
    
    // Set initial objective
    SetCurrentObjective(ENarr_StoryObjective::FindShelter);
    
    UE_LOG(LogTemp, Warning, TEXT("Main Story Arc initialized - Chapter: Arrival"));
}

void UNarr_MainStoryArc::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update story progression based on player state
    UpdateStoryProgression(DeltaTime);
    
    // Check for chapter transitions
    CheckChapterTransitions();
    
    // Handle contextual narration
    HandleContextualNarration(DeltaTime);
}

void UNarr_MainStoryArc::InitializeChapterData()
{
    // Chapter 1: Arrival
    FNarr_ChapterData ArrivalChapter;
    ArrivalChapter.ChapterName = TEXT("Arrival");
    ArrivalChapter.Description = TEXT("Stranded in the Cretaceous period, find immediate shelter and resources");
    ArrivalChapter.RequiredObjectives.Add(ENarr_StoryObjective::FindShelter);
    ArrivalChapter.RequiredObjectives.Add(ENarr_StoryObjective::FindWater);
    ArrivalChapter.RequiredObjectives.Add(ENarr_StoryObjective::FindFood);
    ArrivalChapter.NarrationTriggers.Add(TEXT("Welcome to the Cretaceous period, survivor"));
    ArrivalChapter.NarrationTriggers.Add(TEXT("Your immediate priority is shelter"));
    ChapterDatabase.Add(ENarr_StoryChapter::Arrival, ArrivalChapter);
    
    // Chapter 2: First Contact
    FNarr_ChapterData FirstContactChapter;
    FirstContactChapter.ChapterName = TEXT("First Contact");
    FirstContactChapter.Description = TEXT("Encounter your first dinosaurs and learn survival tactics");
    FirstContactChapter.RequiredObjectives.Add(ENarr_StoryObjective::EncounterDinosaur);
    FirstContactChapter.RequiredObjectives.Add(ENarr_StoryObjective::CraftWeapon);
    FirstContactChapter.RequiredObjectives.Add(ENarr_StoryObjective::LearnHunting);
    FirstContactChapter.NarrationTriggers.Add(TEXT("The giants of this world are not to be underestimated"));
    FirstContactChapter.NarrationTriggers.Add(TEXT("Craft your first weapon for protection"));
    ChapterDatabase.Add(ENarr_StoryChapter::FirstContact, FirstContactChapter);
    
    // Chapter 3: Territorial Expansion
    FNarr_ChapterData ExpansionChapter;
    ExpansionChapter.ChapterName = TEXT("Territorial Expansion");
    ExpansionChapter.Description = TEXT("Establish a base and explore the surrounding territory");
    ExpansionChapter.RequiredObjectives.Add(ENarr_StoryObjective::BuildBase);
    ExpansionChapter.RequiredObjectives.Add(ENarr_StoryObjective::ExploreTerritory);
    ExpansionChapter.RequiredObjectives.Add(ENarr_StoryObjective::MapResources);
    ExpansionChapter.NarrationTriggers.Add(TEXT("Time to claim your territory in this ancient world"));
    ExpansionChapter.NarrationTriggers.Add(TEXT("Map the resources around your base"));
    ChapterDatabase.Add(ENarr_StoryChapter::TerritorialExpansion, ExpansionChapter);
    
    // Chapter 4: Apex Predator
    FNarr_ChapterData ApexChapter;
    ApexChapter.ChapterName = TEXT("Apex Predator");
    ApexChapter.Description = TEXT("Face the ultimate challenge - surviving apex predator encounters");
    ApexChapter.RequiredObjectives.Add(ENarr_StoryObjective::SurviveApexPredator);
    ApexChapter.RequiredObjectives.Add(ENarr_StoryObjective::MasterCombat);
    ApexChapter.RequiredObjectives.Add(ENarr_StoryObjective::BecomeApexSurvivor);
    ApexChapter.NarrationTriggers.Add(TEXT("The apex predators have noticed your presence"));
    ApexChapter.NarrationTriggers.Add(TEXT("Only the strongest survive in this world"));
    ChapterDatabase.Add(ENarr_StoryChapter::ApexPredator, ApexChapter);
}

void UNarr_MainStoryArc::StartChapter(ENarr_StoryChapter NewChapter)
{
    if (ChapterDatabase.Contains(NewChapter))
    {
        CurrentChapter = NewChapter;
        const FNarr_ChapterData& ChapterData = ChapterDatabase[NewChapter];
        
        // Trigger chapter start narration
        TriggerNarration(FString::Printf(TEXT("Chapter: %s - %s"), 
            *ChapterData.ChapterName, *ChapterData.Description));
        
        // Set first objective of the chapter
        if (ChapterData.RequiredObjectives.Num() > 0)
        {
            SetCurrentObjective(ChapterData.RequiredObjectives[0]);
        }
        
        // Broadcast chapter change
        OnChapterChanged.Broadcast(NewChapter);
        
        UE_LOG(LogTemp, Warning, TEXT("Started Chapter: %s"), *ChapterData.ChapterName);
    }
}

void UNarr_MainStoryArc::SetCurrentObjective(ENarr_StoryObjective NewObjective)
{
    CurrentObjective = NewObjective;
    
    // Trigger objective narration
    FString ObjectiveText = GetObjectiveDescription(NewObjective);
    TriggerNarration(FString::Printf(TEXT("New Objective: %s"), *ObjectiveText));
    
    // Broadcast objective change
    OnObjectiveChanged.Broadcast(NewObjective);
    
    UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *ObjectiveText);
}

void UNarr_MainStoryArc::CompleteObjective(ENarr_StoryObjective CompletedObjective)
{
    if (CurrentObjective == CompletedObjective)
    {
        // Mark objective as completed
        CompletedObjectives.AddUnique(CompletedObjective);
        
        // Update story progress
        StoryProgress += 10.0f; // Each objective worth 10%
        StoryProgress = FMath::Clamp(StoryProgress, 0.0f, 100.0f);
        
        // Trigger completion narration
        TriggerNarration(TEXT("Objective completed. Well done, survivor."));
        
        // Broadcast objective completion
        OnObjectiveCompleted.Broadcast(CompletedObjective);
        
        // Check for next objective or chapter transition
        AdvanceStoryProgression();
        
        UE_LOG(LogTemp, Warning, TEXT("Objective Completed: %d"), (int32)CompletedObjective);
    }
}

void UNarr_MainStoryArc::UpdateStoryProgression(float DeltaTime)
{
    // Check player survival state for contextual story triggers
    if (ATranspersonalGameState* GameState = GetWorld()->GetGameState<ATranspersonalGameState>())
    {
        // Check critical survival situations for emergency narration
        if (GameState->PlayerHealth < CriticalHealthThreshold)
        {
            TriggerEmergencyNarration(TEXT("Critical health detected. Seek medical supplies immediately."));
        }
        
        if (GameState->PlayerHunger < CriticalHungerThreshold)
        {
            TriggerEmergencyNarration(TEXT("Severe hunger detected. Find food sources urgently."));
        }
        
        if (GameState->PlayerThirst < CriticalThirstThreshold)
        {
            TriggerEmergencyNarration(TEXT("Dehydration critical. Locate water source immediately."));
        }
    }
}

void UNarr_MainStoryArc::CheckChapterTransitions()
{
    if (!ChapterDatabase.Contains(CurrentChapter))
        return;
        
    const FNarr_ChapterData& CurrentChapterData = ChapterDatabase[CurrentChapter];
    
    // Check if all required objectives for current chapter are completed
    bool bAllObjectivesComplete = true;
    for (ENarr_StoryObjective RequiredObjective : CurrentChapterData.RequiredObjectives)
    {
        if (!CompletedObjectives.Contains(RequiredObjective))
        {
            bAllObjectivesComplete = false;
            break;
        }
    }
    
    // Advance to next chapter if all objectives complete
    if (bAllObjectivesComplete)
    {
        ENarr_StoryChapter NextChapter = GetNextChapter(CurrentChapter);
        if (NextChapter != CurrentChapter)
        {
            StartChapter(NextChapter);
        }
    }
}

void UNarr_MainStoryArc::AdvanceStoryProgression()
{
    if (!ChapterDatabase.Contains(CurrentChapter))
        return;
        
    const FNarr_ChapterData& CurrentChapterData = ChapterDatabase[CurrentChapter];
    
    // Find next incomplete objective in current chapter
    ENarr_StoryObjective NextObjective = ENarr_StoryObjective::FindShelter;
    bool bFoundNextObjective = false;
    
    for (ENarr_StoryObjective Objective : CurrentChapterData.RequiredObjectives)
    {
        if (!CompletedObjectives.Contains(Objective))
        {
            NextObjective = Objective;
            bFoundNextObjective = true;
            break;
        }
    }
    
    if (bFoundNextObjective)
    {
        SetCurrentObjective(NextObjective);
    }
}

ENarr_StoryChapter UNarr_MainStoryArc::GetNextChapter(ENarr_StoryChapter CurrentChapterEnum)
{
    switch (CurrentChapterEnum)
    {
        case ENarr_StoryChapter::Arrival:
            return ENarr_StoryChapter::FirstContact;
        case ENarr_StoryChapter::FirstContact:
            return ENarr_StoryChapter::TerritorialExpansion;
        case ENarr_StoryChapter::TerritorialExpansion:
            return ENarr_StoryChapter::ApexPredator;
        case ENarr_StoryChapter::ApexPredator:
            return ENarr_StoryChapter::ApexPredator; // Final chapter
        default:
            return CurrentChapterEnum;
    }
}

FString UNarr_MainStoryArc::GetObjectiveDescription(ENarr_StoryObjective Objective)
{
    switch (Objective)
    {
        case ENarr_StoryObjective::FindShelter:
            return TEXT("Find or build shelter for protection");
        case ENarr_StoryObjective::FindWater:
            return TEXT("Locate a reliable water source");
        case ENarr_StoryObjective::FindFood:
            return TEXT("Gather or hunt for food");
        case ENarr_StoryObjective::EncounterDinosaur:
            return TEXT("Encounter your first dinosaur");
        case ENarr_StoryObjective::CraftWeapon:
            return TEXT("Craft your first weapon");
        case ENarr_StoryObjective::LearnHunting:
            return TEXT("Learn hunting techniques");
        case ENarr_StoryObjective::BuildBase:
            return TEXT("Establish a permanent base");
        case ENarr_StoryObjective::ExploreTerritory:
            return TEXT("Explore the surrounding territory");
        case ENarr_StoryObjective::MapResources:
            return TEXT("Map available resources");
        case ENarr_StoryObjective::SurviveApexPredator:
            return TEXT("Survive an apex predator encounter");
        case ENarr_StoryObjective::MasterCombat:
            return TEXT("Master combat techniques");
        case ENarr_StoryObjective::BecomeApexSurvivor:
            return TEXT("Become the apex survivor");
        default:
            return TEXT("Unknown Objective");
    }
}

void UNarr_MainStoryArc::TriggerNarration(const FString& NarrationText)
{
    if (bIsNarrationPlaying)
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastNarrationTime < NarrationCooldown)
        return;
    
    // Log narration
    UE_LOG(LogTemp, Warning, TEXT("NARRATION: %s"), *NarrationText);
    
    // Broadcast narration event
    OnNarrationTriggered.Broadcast(NarrationText);
    
    // Update timing
    bIsNarrationPlaying = true;
    LastNarrationTime = CurrentTime;
    
    // Set timer to reset narration state
    GetWorld()->GetTimerManager().SetTimer(NarrationTimerHandle, 
        [this]() { bIsNarrationPlaying = false; }, 5.0f, false);
}

void UNarr_MainStoryArc::TriggerEmergencyNarration(const FString& EmergencyText)
{
    // Emergency narration bypasses cooldown
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY NARRATION: %s"), *EmergencyText);
    
    // Broadcast emergency narration
    OnEmergencyNarrationTriggered.Broadcast(EmergencyText);
    
    // Reset timing to allow immediate emergency responses
    LastNarrationTime = GetWorld()->GetTimeSeconds() - NarrationCooldown;
}

void UNarr_MainStoryArc::HandleContextualNarration(float DeltaTime)
{
    // Handle time-based or location-based contextual narration
    if (ChapterDatabase.Contains(CurrentChapter))
    {
        const FNarr_ChapterData& ChapterData = ChapterDatabase[CurrentChapter];
        
        // Randomly trigger chapter-specific narration
        if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // Very low chance per tick
        {
            if (ChapterData.NarrationTriggers.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, ChapterData.NarrationTriggers.Num() - 1);
                TriggerNarration(ChapterData.NarrationTriggers[RandomIndex]);
            }
        }
    }
}

float UNarr_MainStoryArc::GetStoryProgress() const
{
    return StoryProgress;
}

ENarr_StoryChapter UNarr_MainStoryArc::GetCurrentChapter() const
{
    return CurrentChapter;
}

ENarr_StoryObjective UNarr_MainStoryArc::GetCurrentObjective() const
{
    return CurrentObjective;
}

TArray<ENarr_StoryObjective> UNarr_MainStoryArc::GetCompletedObjectives() const
{
    return CompletedObjectives;
}