#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentPhase = ENarr_StoryPhase::Awakening;
    StoryProgressionScore = 0.0f;
    
    // Get dialogue system reference
    DialogueSystem = GetGameInstance()->GetSubsystem<UNarr_DialogueSystem>();
    
    CreatePrehistoricStoryNodes();
    CreateCharacterArcs();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Story Manager initialized with %d story nodes"), StoryNodes.Num());
}

void UNarr_StoryManager::Deinitialize()
{
    StoryNodes.Empty();
    CharacterArcs.Empty();
    CompletedEvents.Empty();
    
    Super::Deinitialize();
}

void UNarr_StoryManager::TriggerStoryEvent(ENarr_StoryEvent Event, const FString& EventData)
{
    FString EventString;
    switch (Event)
    {
        case ENarr_StoryEvent::PlayerSpawn:
            EventString = TEXT("PlayerSpawn");
            break;
        case ENarr_StoryEvent::FirstDinosaurEncounter:
            EventString = TEXT("FirstDinosaurEncounter");
            break;
        case ENarr_StoryEvent::FirstKill:
            EventString = TEXT("FirstKill");
            break;
        case ENarr_StoryEvent::DiscoverWater:
            EventString = TEXT("DiscoverWater");
            break;
        case ENarr_StoryEvent::MeetNPC:
            EventString = TEXT("MeetNPC");
            break;
        case ENarr_StoryEvent::CompleteQuest:
            EventString = TEXT("CompleteQuest");
            break;
        case ENarr_StoryEvent::DinosaurDeath:
            EventString = TEXT("DinosaurDeath");
            break;
        case ENarr_StoryEvent::PlayerDeath:
            EventString = TEXT("PlayerDeath");
            break;
    }
    
    if (!EventString.IsEmpty())
    {
        CompletedEvents.AddUnique(EventString);
        UE_LOG(LogTemp, Log, TEXT("Story event triggered: %s"), *EventString);
        
        CheckStoryProgression();
    }
}

void UNarr_StoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase != CurrentPhase)
    {
        CurrentPhase = NewPhase;
        UE_LOG(LogTemp, Log, TEXT("Story phase advanced to: %d"), (int32)NewPhase);
        
        CheckStoryProgression();
    }
}

TArray<FNarr_StoryNode> UNarr_StoryManager::GetActiveStoryNodes()
{
    TArray<FNarr_StoryNode> ActiveNodes;
    
    for (const auto& NodePair : StoryNodes)
    {
        const FNarr_StoryNode& Node = NodePair.Value;
        if (!Node.bIsCompleted && Node.Phase == CurrentPhase && AreRequirementsMet(Node))
        {
            ActiveNodes.Add(Node);
        }
    }
    
    // Sort by priority
    ActiveNodes.Sort([](const FNarr_StoryNode& A, const FNarr_StoryNode& B) {
        return A.Priority > B.Priority;
    });
    
    return ActiveNodes;
}

void UNarr_StoryManager::RegisterCharacterArc(const FNarr_CharacterArc& CharacterArc)
{
    CharacterArcs.Add(CharacterArc.CharacterName, CharacterArc);
    UE_LOG(LogTemp, Log, TEXT("Registered character arc: %s"), *CharacterArc.CharacterName);
}

FNarr_CharacterArc UNarr_StoryManager::GetCharacterArc(const FString& CharacterName)
{
    if (CharacterArcs.Contains(CharacterName))
    {
        return CharacterArcs[CharacterName];
    }
    
    return FNarr_CharacterArc();
}

void UNarr_StoryManager::UpdateCharacterRelationship(const FString& CharacterName, float RelationshipDelta)
{
    if (CharacterArcs.Contains(CharacterName))
    {
        CharacterArcs[CharacterName].RelationshipWithPlayer += RelationshipDelta;
        CharacterArcs[CharacterName].RelationshipWithPlayer = FMath::Clamp(
            CharacterArcs[CharacterName].RelationshipWithPlayer, -100.0f, 100.0f
        );
        
        UE_LOG(LogTemp, Log, TEXT("Updated relationship with %s: %f"), 
               *CharacterName, CharacterArcs[CharacterName].RelationshipWithPlayer);
    }
}

void UNarr_StoryManager::InitializePrehistoricStory()
{
    CreatePrehistoricStoryNodes();
    CreateCharacterArcs();
}

FString UNarr_StoryManager::GetCurrentNarrativeContext()
{
    FString Context = FString::Printf(TEXT("Phase: %d, Events: %d, Characters: %d"), 
                                     (int32)CurrentPhase, CompletedEvents.Num(), CharacterArcs.Num());
    return Context;
}

void UNarr_StoryManager::CreatePrehistoricStoryNodes()
{
    // Awakening Phase - Player's first moments
    FNarr_StoryNode AwakeningNode;
    AwakeningNode.NodeID = TEXT("Awakening_Introduction");
    AwakeningNode.Phase = ENarr_StoryPhase::Awakening;
    AwakeningNode.Title = FText::FromString(TEXT("The Awakening"));
    AwakeningNode.Description = FText::FromString(TEXT("You awaken in a world ruled by giants. The ancient storytellers speak of survival."));
    AwakeningNode.RequiredEvents.Add(TEXT("PlayerSpawn"));
    AwakeningNode.DialogueSequences.Add(TEXT("AncientStoryteller_Intro"));
    AwakeningNode.Priority = 10;
    StoryNodes.Add(AwakeningNode.NodeID, AwakeningNode);
    
    // First Hunt Phase
    FNarr_StoryNode FirstHuntNode;
    FirstHuntNode.NodeID = TEXT("FirstHunt_Preparation");
    FirstHuntNode.Phase = ENarr_StoryPhase::FirstHunt;
    FirstHuntNode.Title = FText::FromString(TEXT("The First Hunt"));
    FirstHuntNode.Description = FText::FromString(TEXT("The hunt chief speaks of courage and sacrifice in the face of razor-claws."));
    FirstHuntNode.RequiredEvents.Add(TEXT("FirstDinosaurEncounter"));
    FirstHuntNode.DialogueSequences.Add(TEXT("HuntChief_PostBattle"));
    FirstHuntNode.Priority = 8;
    StoryNodes.Add(FirstHuntNode.NodeID, FirstHuntNode);
    
    // Tribal Contact Phase
    FNarr_StoryNode TribalContactNode;
    TribalContactNode.NodeID = TEXT("TribalContact_Warning");
    TribalContactNode.Phase = ENarr_StoryPhase::TribalContact;
    TribalContactNode.Title = FText::FromString(TEXT("Tribal Warnings"));
    TribalContactNode.Description = FText::FromString(TEXT("Scouts bring word of approaching danger from the eastern ridge."));
    TribalContactNode.RequiredEvents.Add(TEXT("MeetNPC"));
    TribalContactNode.DialogueSequences.Add(TEXT("TribalScout_DangerWarning"));
    TribalContactNode.Priority = 7;
    StoryNodes.Add(TribalContactNode.NodeID, TribalContactNode);
    
    // Territory Establishment Phase
    FNarr_StoryNode TerritoryNode;
    TerritoryNode.NodeID = TEXT("Territory_ResourceGuide");
    TerritoryNode.Phase = ENarr_StoryPhase::TerritoryEstablishment;
    TerritoryNode.Title = FText::FromString(TEXT("Claiming Territory"));
    TerritoryNode.Description = FText::FromString(TEXT("Water guides share knowledge of resources and territorial dangers."));
    TerritoryNode.RequiredEvents.Add(TEXT("DiscoverWater"));
    TerritoryNode.DialogueSequences.Add(TEXT("WaterGuide_ResourceInfo"));
    TerritoryNode.Priority = 6;
    StoryNodes.Add(TerritoryNode.NodeID, TerritoryNode);
    
    UE_LOG(LogTemp, Log, TEXT("Created %d prehistoric story nodes"), StoryNodes.Num());
}

void UNarr_StoryManager::CreateCharacterArcs()
{
    // Ancient Storyteller
    FNarr_CharacterArc StorytellerArc;
    StorytellerArc.CharacterName = TEXT("Ancient Storyteller");
    StorytellerArc.CharacterType = ENarr_CharacterType::TribalElder;
    StorytellerArc.BackgroundStory = FText::FromString(TEXT("The keeper of ancient wisdom, who remembers the old ways of survival."));
    StorytellerArc.PersonalityTraits.Add(TEXT("Wise"));
    StorytellerArc.PersonalityTraits.Add(TEXT("Patient"));
    StorytellerArc.PersonalityTraits.Add(TEXT("Protective"));
    StorytellerArc.RelatedStoryNodes.Add(TEXT("Awakening_Introduction"));
    StorytellerArc.RelationshipWithPlayer = 25.0f;
    RegisterCharacterArc(StorytellerArc);
    
    // Tribal Scout
    FNarr_CharacterArc ScoutArc;
    ScoutArc.CharacterName = TEXT("Tribal Scout");
    ScoutArc.CharacterType = ENarr_CharacterType::Scout;
    ScoutArc.BackgroundStory = FText::FromString(TEXT("A vigilant watcher who monitors the movements of great beasts."));
    ScoutArc.PersonalityTraits.Add(TEXT("Alert"));
    ScoutArc.PersonalityTraits.Add(TEXT("Brave"));
    ScoutArc.PersonalityTraits.Add(TEXT("Quick"));
    ScoutArc.RelatedStoryNodes.Add(TEXT("TribalContact_Warning"));
    ScoutArc.RelationshipWithPlayer = 10.0f;
    RegisterCharacterArc(ScoutArc);
    
    // Hunt Chief
    FNarr_CharacterArc HuntChiefArc;
    HuntChiefArc.CharacterName = TEXT("Hunt Chief");
    HuntChiefArc.CharacterType = ENarr_CharacterType::HuntLeader;
    HuntChiefArc.BackgroundStory = FText::FromString(TEXT("A battle-hardened leader who has faced the razor-claws and lived."));
    HuntChiefArc.PersonalityTraits.Add(TEXT("Courageous"));
    HuntChiefArc.PersonalityTraits.Add(TEXT("Tactical"));
    HuntChiefArc.PersonalityTraits.Add(TEXT("Honorable"));
    HuntChiefArc.RelatedStoryNodes.Add(TEXT("FirstHunt_Preparation"));
    HuntChiefArc.RelationshipWithPlayer = 15.0f;
    RegisterCharacterArc(HuntChiefArc);
    
    // Water Guide
    FNarr_CharacterArc WaterGuideArc;
    WaterGuideArc.CharacterName = TEXT("Water Guide");
    WaterGuideArc.CharacterType = ENarr_CharacterType::Scout;
    WaterGuideArc.BackgroundStory = FText::FromString(TEXT("An expert navigator who knows the safe paths to vital resources."));
    WaterGuideArc.PersonalityTraits.Add(TEXT("Knowledgeable"));
    WaterGuideArc.PersonalityTraits.Add(TEXT("Cautious"));
    WaterGuideArc.PersonalityTraits.Add(TEXT("Resourceful"));
    WaterGuideArc.RelatedStoryNodes.Add(TEXT("Territory_ResourceGuide"));
    WaterGuideArc.RelationshipWithPlayer = 20.0f;
    RegisterCharacterArc(WaterGuideArc);
}

void UNarr_StoryManager::CheckStoryProgression()
{
    TArray<FNarr_StoryNode> ActiveNodes = GetActiveStoryNodes();
    
    for (const FNarr_StoryNode& Node : ActiveNodes)
    {
        if (!Node.bIsCompleted && AreRequirementsMet(Node))
        {
            TriggerNarrativeSequence(Node.NodeID);
        }
    }
}

bool UNarr_StoryManager::AreRequirementsMet(const FNarr_StoryNode& Node)
{
    for (const FString& RequiredEvent : Node.RequiredEvents)
    {
        if (!CompletedEvents.Contains(RequiredEvent))
        {
            return false;
        }
    }
    return true;
}

void UNarr_StoryManager::TriggerNarrativeSequence(const FString& NodeID)
{
    if (!StoryNodes.Contains(NodeID))
    {
        return;
    }
    
    FNarr_StoryNode& Node = StoryNodes[NodeID];
    
    // Trigger associated dialogue sequences
    if (DialogueSystem)
    {
        for (const FString& DialogueID : Node.DialogueSequences)
        {
            DialogueSystem->PlayDialogue(DialogueID);
        }
    }
    
    // Mark node as completed
    Node.bIsCompleted = true;
    StoryProgressionScore += Node.Priority;
    
    UE_LOG(LogTemp, Log, TEXT("Triggered narrative sequence: %s"), *NodeID);
}