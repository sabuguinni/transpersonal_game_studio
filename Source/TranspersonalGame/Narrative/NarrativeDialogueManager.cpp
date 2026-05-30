#include "NarrativeDialogueManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Initializing dialogue system"));
    
    LoadDialogueDatabase();
    InitializeDefaultDialogues();
    CreateNPCDialogueSets();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Loaded %d dialogue entries"), DialogueDatabase.Num());
}

FNarr_DialogueEntry UNarrativeDialogueManager::GetDialogueEntry(const FString& DialogueID)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        return DialogueDatabase[DialogueID];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Dialogue ID not found: %s"), *DialogueID);
    return FNarr_DialogueEntry();
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueManager::GetNPCDialogues(const FString& NPCName, ENarr_DialogueContext Context)
{
    TArray<FNarr_DialogueEntry> Result;
    
    if (NPCDialogueSets.Contains(NPCName))
    {
        const FNarr_NPCDialogueSet& DialogueSet = NPCDialogueSets[NPCName];
        
        switch (Context)
        {
            case ENarr_DialogueContext::Greeting:
                Result = DialogueSet.GreetingDialogues;
                break;
            case ENarr_DialogueContext::Quest:
                Result = DialogueSet.QuestDialogues;
                break;
            case ENarr_DialogueContext::Trading:
                Result = DialogueSet.TradingDialogues;
                break;
            case ENarr_DialogueContext::Combat:
                Result = DialogueSet.CombatDialogues;
                break;
            default:
                Result = DialogueSet.GreetingDialogues;
                break;
        }
    }
    
    return Result;
}

void UNarrativeDialogueManager::RegisterDialogueEntry(const FNarr_DialogueEntry& DialogueEntry)
{
    DialogueDatabase.Add(DialogueEntry.DialogueID, DialogueEntry);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Registered dialogue: %s"), *DialogueEntry.DialogueID);
}

void UNarrativeDialogueManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (!Speaker || !Listener)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Invalid speaker or listener for dialogue: %s"), *DialogueID);
        return;
    }
    
    FNarr_DialogueEntry DialogueEntry = GetDialogueEntry(DialogueID);
    if (DialogueEntry.DialogueID.IsEmpty())
    {
        return;
    }
    
    // Log dialogue trigger
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Triggering dialogue '%s' between %s and %s"), 
           *DialogueID, *Speaker->GetName(), *Listener->GetName());
    
    // Here you would integrate with UI system to display dialogue
    // For now, we'll just log the dialogue text
    UE_LOG(LogTemp, Warning, TEXT("DIALOGUE [%s]: %s"), *DialogueEntry.SpeakerName, *DialogueEntry.DialogueText);
}

bool UNarrativeDialogueManager::IsDialogueAvailable(const FString& DialogueID, ENarr_BiomeType CurrentBiome, ENarr_StoryPhase CurrentPhase)
{
    FNarr_DialogueEntry DialogueEntry = GetDialogueEntry(DialogueID);
    if (DialogueEntry.DialogueID.IsEmpty())
    {
        return false;
    }
    
    // Check biome requirement
    if (DialogueEntry.RequiredBiome != ENarr_BiomeType::Savanna && DialogueEntry.RequiredBiome != CurrentBiome)
    {
        return false;
    }
    
    // Check story phase requirement
    if (DialogueEntry.RequiredStoryPhase != ENarr_StoryPhase::Awakening && DialogueEntry.RequiredStoryPhase != CurrentPhase)
    {
        return false;
    }
    
    return true;
}

void UNarrativeDialogueManager::LoadDialogueDatabase()
{
    DialogueDatabase.Empty();
    NPCDialogueSets.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Loading dialogue database"));
}

FString UNarrativeDialogueManager::GetRandomDialogueByContext(ENarr_DialogueContext Context, ENarr_BiomeType Biome)
{
    TArray<FString> MatchingDialogues;
    
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Entry = DialoguePair.Value;
        
        // Check if dialogue matches context and biome
        bool bContextMatch = false;
        switch (Context)
        {
            case ENarr_DialogueContext::Danger:
                bContextMatch = Entry.DialogueText.Contains(TEXT("danger")) || Entry.DialogueText.Contains(TEXT("warning"));
                break;
            case ENarr_DialogueContext::Exploration:
                bContextMatch = Entry.DialogueText.Contains(TEXT("explore")) || Entry.DialogueText.Contains(TEXT("discover"));
                break;
            case ENarr_DialogueContext::Survival:
                bContextMatch = Entry.DialogueText.Contains(TEXT("survive")) || Entry.DialogueText.Contains(TEXT("endure"));
                break;
            default:
                bContextMatch = true;
                break;
        }
        
        if (bContextMatch && (Entry.RequiredBiome == Biome || Entry.RequiredBiome == ENarr_BiomeType::Savanna))
        {
            MatchingDialogues.Add(Entry.DialogueID);
        }
    }
    
    if (MatchingDialogues.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, MatchingDialogues.Num() - 1);
        return MatchingDialogues[RandomIndex];
    }
    
    return TEXT("");
}

void UNarrativeDialogueManager::InitializeDefaultDialogues()
{
    CreateSurvivalDialogues();
    CreateDangerWarningDialogues();
    CreateExplorationDialogues();
}

void UNarrativeDialogueManager::CreateSurvivalDialogues()
{
    // Savanna survival dialogues
    FNarr_DialogueEntry SavannaSurvival;
    SavannaSurvival.DialogueID = TEXT("SAVANNA_SURVIVAL_01");
    SavannaSurvival.SpeakerName = TEXT("Survival Guide");
    SavannaSurvival.DialogueText = TEXT("The savanna grasslands test your endurance. Find water sources and avoid the territorial herbivores during their feeding times.");
    SavannaSurvival.RequiredBiome = ENarr_BiomeType::Savanna;
    SavannaSurvival.RequiredStoryPhase = ENarr_StoryPhase::Awakening;
    RegisterDialogueEntry(SavannaSurvival);
    
    // Forest survival dialogues
    FNarr_DialogueEntry ForestSurvival;
    ForestSurvival.DialogueID = TEXT("FOREST_SURVIVAL_01");
    ForestSurvival.SpeakerName = TEXT("Forest Guide");
    ForestSurvival.DialogueText = TEXT("The dense forest provides shelter but hides many dangers. Watch for predator tracks and listen for the calls of the great Brachiosaurus herds.");
    ForestSurvival.RequiredBiome = ENarr_BiomeType::Forest;
    ForestSurvival.RequiredStoryPhase = ENarr_StoryPhase::Exploration;
    RegisterDialogueEntry(ForestSurvival);
    
    // Desert survival dialogues
    FNarr_DialogueEntry DesertSurvival;
    DesertSurvival.DialogueID = TEXT("DESERT_SURVIVAL_01");
    DesertSurvival.SpeakerName = TEXT("Desert Wanderer");
    DesertSurvival.DialogueText = TEXT("The desert heat is merciless. Travel during dawn and dusk. The sand hides ancient bones and precious resources for those brave enough to dig.");
    DesertSurvival.RequiredBiome = ENarr_BiomeType::Desert;
    DesertSurvival.RequiredStoryPhase = ENarr_StoryPhase::Adaptation;
    RegisterDialogueEntry(DesertSurvival);
}

void UNarrativeDialogueManager::CreateDangerWarningDialogues()
{
    // T-Rex danger warning
    FNarr_DialogueEntry TRexWarning;
    TRexWarning.DialogueID = TEXT("DANGER_TREX_01");
    TRexWarning.SpeakerName = TEXT("Danger Alert");
    TRexWarning.DialogueText = TEXT("Massive predator detected! The T-Rex has caught your scent. Move slowly, avoid sudden movements, and find cover immediately.");
    TRexWarning.RequiredBiome = ENarr_BiomeType::Savanna;
    TRexWarning.RequiredStoryPhase = ENarr_StoryPhase::Awakening;
    RegisterDialogueEntry(TRexWarning);
    
    // Velociraptor pack warning
    FNarr_DialogueEntry RaptorWarning;
    RaptorWarning.DialogueID = TEXT("DANGER_RAPTOR_01");
    RaptorWarning.SpeakerName = TEXT("Pack Alert");
    RaptorWarning.DialogueText = TEXT("Velociraptor pack nearby! They hunt in coordinated groups. Watch your flanks and never turn your back on them.");
    RaptorWarning.RequiredBiome = ENarr_BiomeType::Forest;
    RaptorWarning.RequiredStoryPhase = ENarr_StoryPhase::Conflict;
    RegisterDialogueEntry(RaptorWarning);
}

void UNarrativeDialogueManager::CreateExplorationDialogues()
{
    // Brachiosaurus migration
    FNarr_DialogueEntry BrachioMigration;
    BrachioMigration.DialogueID = TEXT("EXPLORE_BRACHIO_01");
    BrachioMigration.SpeakerName = TEXT("Migration Observer");
    BrachioMigration.DialogueText = TEXT("The ancient Brachiosaurus herds are on the move. Follow their trails to discover new feeding grounds and fresh water sources.");
    BrachioMigration.RequiredBiome = ENarr_BiomeType::Forest;
    BrachioMigration.RequiredStoryPhase = ENarr_StoryPhase::Exploration;
    RegisterDialogueEntry(BrachioMigration);
    
    // Resource discovery
    FNarr_DialogueEntry ResourceDiscovery;
    ResourceDiscovery.DialogueID = TEXT("EXPLORE_RESOURCES_01");
    ResourceDiscovery.SpeakerName = TEXT("Resource Scout");
    ResourceDiscovery.DialogueText = TEXT("Ancient bone deposits lie buried in this area. The minerals here could strengthen your tools and weapons if you know how to extract them.");
    ResourceDiscovery.RequiredBiome = ENarr_BiomeType::Desert;
    ResourceDiscovery.RequiredStoryPhase = ENarr_StoryPhase::Mastery;
    RegisterDialogueEntry(ResourceDiscovery);
}

void UNarrativeDialogueManager::CreateNPCDialogueSets()
{
    // Tribal Elder NPC
    FNarr_NPCDialogueSet TribalElder;
    TribalElder.NPCName = TEXT("Tribal_Elder");
    TribalElder.NPCType = ENarr_NPCType::QuestGiver;
    
    FNarr_DialogueEntry ElderGreeting;
    ElderGreeting.DialogueID = TEXT("ELDER_GREETING_01");
    ElderGreeting.SpeakerName = TEXT("Tribal Elder");
    ElderGreeting.DialogueText = TEXT("Young hunter, you have survived your first days in this ancient world. But greater challenges await. Are you ready to prove yourself?");
    ElderGreeting.RequiredStoryPhase = ENarr_StoryPhase::Awakening;
    TribalElder.GreetingDialogues.Add(ElderGreeting);
    
    FNarr_DialogueEntry ElderQuest;
    ElderQuest.DialogueID = TEXT("ELDER_QUEST_01");
    ElderQuest.SpeakerName = TEXT("Tribal Elder");
    ElderQuest.DialogueText = TEXT("The five sacred biomes must be explored. Each holds trials that will forge you into a true survivor. Begin with the savanna grasslands.");
    ElderQuest.bIsQuestDialogue = true;
    ElderQuest.QuestID = TEXT("BIOME_TRIALS");
    TribalElder.QuestDialogues.Add(ElderQuest);
    
    NPCDialogueSets.Add(TribalElder.NPCName, TribalElder);
    
    // Trader NPC
    FNarr_NPCDialogueSet Trader;
    Trader.NPCName = TEXT("Resource_Trader");
    Trader.NPCType = ENarr_NPCType::Trader;
    
    FNarr_DialogueEntry TraderGreeting;
    TraderGreeting.DialogueID = TEXT("TRADER_GREETING_01");
    TraderGreeting.SpeakerName = TEXT("Resource Trader");
    TraderGreeting.DialogueText = TEXT("Ah, another survivor! I trade in rare materials and crafting components. What treasures have you found in your travels?");
    Trader.GreetingDialogues.Add(TraderGreeting);
    
    FNarr_DialogueEntry TraderTrade;
    TraderTrade.DialogueID = TEXT("TRADER_TRADE_01");
    TraderTrade.SpeakerName = TEXT("Resource Trader");
    TraderTrade.DialogueText = TEXT("Dinosaur bones, rare minerals, medicinal plants - I'll trade fairly for quality materials. What do you need for your next expedition?");
    Trader.TradingDialogues.Add(TraderTrade);
    
    NPCDialogueSets.Add(Trader.NPCName, Trader);
}