#include "Narr_SurvivalNarrativeCore.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_SurvivalNarrativeCore::UNarr_SurvivalNarrativeCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentPhase = ENarr_SurvivalPhase::Awakening;
    SurvivalExperience = 0;
    NarrativeIntensity = 0.0f;
    CurrentNarrativeContext = TEXT("You awaken in an ancient world where survival is the only law.");
}

void UNarr_SurvivalNarrativeCore::BeginPlay()
{
    Super::BeginPlay();
    InitializeSurvivalNarrative();
}

void UNarr_SurvivalNarrativeCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Gradually reduce narrative intensity over time
    if (NarrativeIntensity > 0.0f)
    {
        NarrativeIntensity = FMath::Max(0.0f, NarrativeIntensity - (DeltaTime * 0.1f));
    }
}

void UNarr_SurvivalNarrativeCore::AdvanceNarrativePhase()
{
    ENarr_SurvivalPhase NextPhase = static_cast<ENarr_SurvivalPhase>(static_cast<int32>(CurrentPhase) + 1);
    
    if (NextPhase <= ENarr_SurvivalPhase::Legacy && CanProgressToPhase(NextPhase))
    {
        CurrentPhase = NextPhase;
        CurrentNarrativeContext = GeneratePhaseNarrative(CurrentPhase);
        UpdateNarrativeIntensity(1.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("Narrative Phase Advanced to: %d"), static_cast<int32>(CurrentPhase));
        TriggerNarrativeEvent(TEXT("PhaseAdvancement"));
    }
}

void UNarr_SurvivalNarrativeCore::CompleteMilestone(const FString& MilestoneName)
{
    for (FNarr_SurvivalMilestone& Milestone : CompletedMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName && !Milestone.bIsCompleted)
        {
            Milestone.bIsCompleted = true;
            SurvivalExperience += Milestone.ExperienceReward;
            CurrentNarrativeContext = Milestone.UnlockNarrative;
            UpdateNarrativeIntensity(0.8f);
            
            UE_LOG(LogTemp, Warning, TEXT("Milestone Completed: %s"), *MilestoneName);
            TriggerNarrativeEvent(TEXT("MilestoneComplete"));
            break;
        }
    }
}

void UNarr_SurvivalNarrativeCore::DiscoverEnvironmentalStory(const FString& LocationName)
{
    for (FNarr_EnvironmentalStory& Story : DiscoveredStories)
    {
        if (Story.LocationName == LocationName && !Story.bIsDiscovered)
        {
            Story.bIsDiscovered = true;
            CurrentNarrativeContext = Story.StoryFragment;
            UpdateNarrativeIntensity(0.6f);
            
            UE_LOG(LogTemp, Warning, TEXT("Environmental Story Discovered: %s"), *LocationName);
            TriggerNarrativeEvent(TEXT("StoryDiscovery"));
            break;
        }
    }
}

FString UNarr_SurvivalNarrativeCore::GetContextualNarrative(ENarr_ThreatLevel ThreatLevel)
{
    FString NarrativeText = CurrentNarrativeContext;
    
    switch (ThreatLevel)
    {
        case ENarr_ThreatLevel::Peaceful:
            NarrativeText += TEXT(" The land rests in tranquil silence.");
            break;
        case ENarr_ThreatLevel::Cautious:
            NarrativeText += TEXT(" Something stirs in the distance. Stay alert.");
            break;
        case ENarr_ThreatLevel::Dangerous:
            NarrativeText += TEXT(" The air grows thick with tension. Danger lurks nearby.");
            break;
        case ENarr_ThreatLevel::Hostile:
            NarrativeText += TEXT(" Predators circle. Your survival instincts scream warnings.");
            break;
        case ENarr_ThreatLevel::Lethal:
            NarrativeText += TEXT(" Death stalks these grounds. Every moment could be your last.");
            break;
    }
    
    return NarrativeText;
}

void UNarr_SurvivalNarrativeCore::UpdateNarrativeIntensity(float DeltaIntensity)
{
    NarrativeIntensity = FMath::Clamp(NarrativeIntensity + DeltaIntensity, 0.0f, 2.0f);
}

bool UNarr_SurvivalNarrativeCore::CanProgressToPhase(ENarr_SurvivalPhase TargetPhase)
{
    int32 RequiredExperience = static_cast<int32>(TargetPhase) * 100;
    return SurvivalExperience >= RequiredExperience;
}

TArray<FString> UNarr_SurvivalNarrativeCore::GetAvailableMilestones()
{
    TArray<FString> AvailableMilestones;
    
    for (const FNarr_SurvivalMilestone& Milestone : CompletedMilestones)
    {
        if (!Milestone.bIsCompleted && Milestone.RequiredPhase <= CurrentPhase)
        {
            AvailableMilestones.Add(Milestone.MilestoneName);
        }
    }
    
    return AvailableMilestones;
}

void UNarr_SurvivalNarrativeCore::InitializeSurvivalNarrative()
{
    LoadMilestoneData();
    LoadEnvironmentalStories();
    CurrentNarrativeContext = GeneratePhaseNarrative(CurrentPhase);
}

void UNarr_SurvivalNarrativeCore::LoadMilestoneData()
{
    CompletedMilestones.Empty();
    
    // Awakening Phase Milestones
    FNarr_SurvivalMilestone FirstSteps;
    FirstSteps.MilestoneName = TEXT("First Steps");
    FirstSteps.Description = TEXT("Take your first steps in the prehistoric world");
    FirstSteps.RequiredPhase = ENarr_SurvivalPhase::Awakening;
    FirstSteps.ExperienceReward = 25;
    FirstSteps.UnlockNarrative = TEXT("Your legs remember the ancient rhythm of survival.");
    CompletedMilestones.Add(FirstSteps);
    
    FNarr_SurvivalMilestone FirstTool;
    FirstTool.MilestoneName = TEXT("First Tool");
    FirstTool.Description = TEXT("Craft your first primitive tool");
    FirstTool.RequiredPhase = ENarr_SurvivalPhase::Discovery;
    FirstTool.ExperienceReward = 50;
    FirstTool.UnlockNarrative = TEXT("Stone and bone become extensions of your will.");
    CompletedMilestones.Add(FirstTool);
    
    FNarr_SurvivalMilestone FirstHunt;
    FirstHunt.MilestoneName = TEXT("First Hunt");
    FirstHunt.Description = TEXT("Successfully hunt your first prey");
    FirstHunt.RequiredPhase = ENarr_SurvivalPhase::Adaptation;
    FirstHunt.ExperienceReward = 75;
    FirstHunt.UnlockNarrative = TEXT("The hunter awakens within you, ancient and primal.");
    CompletedMilestones.Add(FirstHunt);
    
    FNarr_SurvivalMilestone TribalLeader;
    TribalLeader.MilestoneName = TEXT("Tribal Leader");
    TribalLeader.Description = TEXT("Become the leader of your tribe");
    TribalLeader.RequiredPhase = ENarr_SurvivalPhase::Leadership;
    TribalLeader.ExperienceReward = 200;
    TribalLeader.UnlockNarrative = TEXT("Others look to you for guidance in this harsh world.");
    CompletedMilestones.Add(TribalLeader);
}

void UNarr_SurvivalNarrativeCore::LoadEnvironmentalStories()
{
    DiscoveredStories.Empty();
    
    FNarr_EnvironmentalStory RiverStones;
    RiverStones.LocationName = TEXT("Ancient River Stones");
    RiverStones.StoryFragment = TEXT("Carved symbols tell of great hunts and fallen warriors.");
    RiverStones.ThreatLevel = ENarr_ThreatLevel::Peaceful;
    RiverStones.DiscoveryClues.Add(TEXT("Follow the river downstream"));
    RiverStones.DiscoveryClues.Add(TEXT("Look for weathered stone formations"));
    DiscoveredStories.Add(RiverStones);
    
    FNarr_EnvironmentalStory BoneYard;
    BoneYard.LocationName = TEXT("Dinosaur Bone Yard");
    BoneYard.StoryFragment = TEXT("Massive bones scattered like fallen giants. This was a battleground.");
    BoneYard.ThreatLevel = ENarr_ThreatLevel::Dangerous;
    BoneYard.DiscoveryClues.Add(TEXT("Follow the scent of old death"));
    BoneYard.DiscoveryClues.Add(TEXT("Listen for the silence where nothing lives"));
    DiscoveredStories.Add(BoneYard);
    
    FNarr_EnvironmentalStory CaveDrawings;
    CaveDrawings.LocationName = TEXT("Sacred Cave Drawings");
    CaveDrawings.StoryFragment = TEXT("Ancient hands painted warnings and wisdom on these walls.");
    CaveDrawings.ThreatLevel = ENarr_ThreatLevel::Cautious;
    CaveDrawings.DiscoveryClues.Add(TEXT("Seek shelter in the mountain caves"));
    CaveDrawings.DiscoveryClues.Add(TEXT("Look for smoke stains on cave walls"));
    DiscoveredStories.Add(CaveDrawings);
}

FString UNarr_SurvivalNarrativeCore::GeneratePhaseNarrative(ENarr_SurvivalPhase Phase)
{
    switch (Phase)
    {
        case ENarr_SurvivalPhase::Awakening:
            return TEXT("You awaken in a world where only the strong survive. Every breath is earned.");
        case ENarr_SurvivalPhase::Discovery:
            return TEXT("The land reveals its secrets to those brave enough to explore.");
        case ENarr_SurvivalPhase::Adaptation:
            return TEXT("You begin to understand the rhythm of this ancient world.");
        case ENarr_SurvivalPhase::Mastery:
            return TEXT("The environment bends to your will. You are becoming apex predator.");
        case ENarr_SurvivalPhase::Leadership:
            return TEXT("Others follow your lead. You shape the destiny of your tribe.");
        case ENarr_SurvivalPhase::Legacy:
            return TEXT("Your name will be carved in stone, remembered by future generations.");
        default:
            return TEXT("The story continues...");
    }
}

void UNarr_SurvivalNarrativeCore::TriggerNarrativeEvent(const FString& EventName)
{
    // Broadcast narrative events to other systems
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Narrative Event: %s"), *EventName));
    }
}