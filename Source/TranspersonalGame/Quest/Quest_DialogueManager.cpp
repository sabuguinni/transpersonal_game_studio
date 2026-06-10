#include "Quest_DialogueManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UQuest_DialogueManager::UQuest_DialogueManager()
{
    // Constructor
}

void UQuest_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Initializing dialogue system"));
    
    InitializeDialogueDatabase();
    LoadNPCDialogues();
    SetupQuestGivers();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Dialogue system initialized with %d NPCs"), 
           NPCDialogues.Num());
}

void UQuest_DialogueManager::Deinitialize()
{
    // Clear all dialogue data
    NPCDialogues.Empty();
    DialogueDatabase.Empty();
    QuestGivers.Empty();
    ActiveDialogues.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Dialogue system deinitialized"));
    
    Super::Deinitialize();
}

FQuest_DialogueEntry UQuest_DialogueManager::GetDialogueEntry(const FString& DialogueID)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        return DialogueDatabase[DialogueID];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: Dialogue entry not found: %s"), *DialogueID);
    return FQuest_DialogueEntry();
}

TArray<FQuest_DialogueEntry> UQuest_DialogueManager::GetNPCDialogues(const FString& NPCName)
{
    if (NPCDialogues.Contains(NPCName))
    {
        return NPCDialogues[NPCName].DialogueEntries;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: NPC dialogues not found: %s"), *NPCName);
    return TArray<FQuest_DialogueEntry>();
}

FString UQuest_DialogueManager::GetNPCGreeting(const FString& NPCName)
{
    if (NPCDialogues.Contains(NPCName))
    {
        return NPCDialogues[NPCName].DefaultGreeting;
    }
    
    return TEXT("Hello, traveler.");
}

bool UQuest_DialogueManager::StartDialogue(const FString& NPCName, const FString& PlayerID)
{
    if (!NPCDialogues.Contains(NPCName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: Cannot start dialogue with unknown NPC: %s"), *NPCName);
        return false;
    }
    
    if (ActiveDialogues.Contains(PlayerID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: Player %s already in dialogue"), *PlayerID);
        return false;
    }
    
    ActiveDialogues.Add(PlayerID, NPCName);
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Started dialogue between %s and %s"), *PlayerID, *NPCName);
    
    return true;
}

void UQuest_DialogueManager::EndDialogue(const FString& PlayerID)
{
    if (ActiveDialogues.Contains(PlayerID))
    {
        FString NPCName = ActiveDialogues[PlayerID];
        ActiveDialogues.Remove(PlayerID);
        UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Ended dialogue between %s and %s"), *PlayerID, *NPCName);
    }
}

bool UQuest_DialogueManager::IsInDialogue(const FString& PlayerID) const
{
    return ActiveDialogues.Contains(PlayerID);
}

FString UQuest_DialogueManager::GetQuestDialogue(const FString& QuestID, const FString& DialogueType)
{
    FString DialogueKey = FString::Printf(TEXT("%s_%s"), *QuestID, *DialogueType);
    
    if (DialogueDatabase.Contains(DialogueKey))
    {
        return DialogueDatabase[DialogueKey].DialogueText;
    }
    
    return TEXT("Quest dialogue not found.");
}

void UQuest_DialogueManager::RegisterQuestGiver(const FString& NPCName, const FString& QuestID)
{
    QuestGivers.Add(NPCName, QuestID);
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Registered %s as quest giver for %s"), *NPCName, *QuestID);
}

bool UQuest_DialogueManager::IsQuestGiver(const FString& NPCName) const
{
    return QuestGivers.Contains(NPCName);
}

FString UQuest_DialogueManager::GetDialogueAudioPath(const FString& DialogueID)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        return DialogueDatabase[DialogueID].AudioFilePath;
    }
    
    return TEXT("");
}

void UQuest_DialogueManager::PlayDialogueAudio(const FString& DialogueID)
{
    FString AudioPath = GetDialogueAudioPath(DialogueID);
    if (!AudioPath.IsEmpty())
    {
        // Audio playback would be handled by Audio Agent
        UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Playing audio for dialogue: %s"), *DialogueID);
    }
}

void UQuest_DialogueManager::InitializeDialogueDatabase()
{
    // Hunt Master Dialogues
    DialogueDatabase.Add(TEXT("hunt_master_greeting"), 
        CreateDialogueEntry(TEXT("hunt_master_greeting"), TEXT("Hunt Master"),
        TEXT("Greetings, survivor. I am the Hunt Master. The great beasts of this land hold secrets that only the brave can uncover."),
        TEXT("/Game/Audio/Dialogue/HuntMaster_Greeting.wav")));
    
    DialogueDatabase.Add(TEXT("hunt_master_quest_offer"), 
        CreateDialogueEntry(TEXT("hunt_master_quest_offer"), TEXT("Hunt Master"),
        TEXT("Bring me proof of your prowess against the apex predators, and I will share the ancient knowledge of tracking and hunting."),
        TEXT("/Game/Audio/Dialogue/HuntMaster_QuestOffer.wav")));
    
    DialogueDatabase.Add(TEXT("hunt_master_warning"), 
        CreateDialogueEntry(TEXT("hunt_master_warning"), TEXT("Hunt Master"),
        TEXT("But beware - the T-Rex that roams the eastern cliffs has claimed many hunters before you."),
        TEXT("/Game/Audio/Dialogue/HuntMaster_Warning.wav")));
    
    // Resource Collector Dialogues
    DialogueDatabase.Add(TEXT("resource_collector_greeting"), 
        CreateDialogueEntry(TEXT("resource_collector_greeting"), TEXT("Resource Collector"),
        TEXT("Welcome, young gatherer. The forest provides all we need to survive, but only those who understand its gifts can thrive."),
        TEXT("/Game/Audio/Dialogue/ResourceCollector_Greeting.wav")));
    
    DialogueDatabase.Add(TEXT("resource_collector_quest"), 
        CreateDialogueEntry(TEXT("resource_collector_quest"), TEXT("Resource Collector"),
        TEXT("I seek one who can collect the sacred stones, the fallen branches, and the healing herbs."),
        TEXT("/Game/Audio/Dialogue/ResourceCollector_Quest.wav")));
    
    DialogueDatabase.Add(TEXT("resource_collector_reward"), 
        CreateDialogueEntry(TEXT("resource_collector_reward"), TEXT("Resource Collector"),
        TEXT("Complete this task, and I will teach you the art of crafting tools that can mean the difference between life and death."),
        TEXT("/Game/Audio/Dialogue/ResourceCollector_Reward.wav")));
    
    // Survival Expert Dialogues
    DialogueDatabase.Add(TEXT("survival_expert_greeting"), 
        CreateDialogueEntry(TEXT("survival_expert_greeting"), TEXT("Survival Expert"),
        TEXT("Ah, another soul seeking to master the harsh ways of this ancient world. Survival is not just about strength - it is about wisdom."),
        TEXT("/Game/Audio/Dialogue/SurvivalExpert_Greeting.wav")));
    
    DialogueDatabase.Add(TEXT("survival_expert_challenge"), 
        CreateDialogueEntry(TEXT("survival_expert_challenge"), TEXT("Survival Expert"),
        TEXT("I will test your endurance, your resourcefulness, and your will to live. Are you prepared for the ultimate survival challenge?"),
        TEXT("/Game/Audio/Dialogue/SurvivalExpert_Challenge.wav")));
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Initialized dialogue database with %d entries"), 
           DialogueDatabase.Num());
}

void UQuest_DialogueManager::LoadNPCDialogues()
{
    // Hunt Master NPC
    FQuest_NPCDialogueSet HuntMasterSet;
    HuntMasterSet.NPCName = TEXT("HuntMaster");
    HuntMasterSet.DefaultGreeting = TEXT("Greetings, survivor. I am the Hunt Master.");
    HuntMasterSet.QuestCompleteResponse = TEXT("Excellent work, hunter! You have proven yourself worthy.");
    HuntMasterSet.QuestInProgressResponse = TEXT("The hunt continues. Show me your dedication.");
    
    HuntMasterSet.DialogueEntries.Add(DialogueDatabase[TEXT("hunt_master_greeting")]);
    HuntMasterSet.DialogueEntries.Add(DialogueDatabase[TEXT("hunt_master_quest_offer")]);
    HuntMasterSet.DialogueEntries.Add(DialogueDatabase[TEXT("hunt_master_warning")]);
    
    NPCDialogues.Add(TEXT("HuntMaster"), HuntMasterSet);
    
    // Resource Collector NPC
    FQuest_NPCDialogueSet ResourceCollectorSet;
    ResourceCollectorSet.NPCName = TEXT("ResourceCollector");
    ResourceCollectorSet.DefaultGreeting = TEXT("Welcome, young gatherer.");
    ResourceCollectorSet.QuestCompleteResponse = TEXT("Well done! You understand the forest's gifts.");
    ResourceCollectorSet.QuestInProgressResponse = TEXT("Keep searching. The forest has more to offer.");
    
    ResourceCollectorSet.DialogueEntries.Add(DialogueDatabase[TEXT("resource_collector_greeting")]);
    ResourceCollectorSet.DialogueEntries.Add(DialogueDatabase[TEXT("resource_collector_quest")]);
    ResourceCollectorSet.DialogueEntries.Add(DialogueDatabase[TEXT("resource_collector_reward")]);
    
    NPCDialogues.Add(TEXT("ResourceCollector"), ResourceCollectorSet);
    
    // Survival Expert NPC
    FQuest_NPCDialogueSet SurvivalExpertSet;
    SurvivalExpertSet.NPCName = TEXT("SurvivalExpert");
    SurvivalExpertSet.DefaultGreeting = TEXT("Ah, another soul seeking to master the harsh ways of this ancient world.");
    SurvivalExpertSet.QuestCompleteResponse = TEXT("Impressive! You have the heart of a true survivor.");
    SurvivalExpertSet.QuestInProgressResponse = TEXT("The challenge continues. Prove your worth.");
    
    SurvivalExpertSet.DialogueEntries.Add(DialogueDatabase[TEXT("survival_expert_greeting")]);
    SurvivalExpertSet.DialogueEntries.Add(DialogueDatabase[TEXT("survival_expert_challenge")]);
    
    NPCDialogues.Add(TEXT("SurvivalExpert"), SurvivalExpertSet);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Loaded dialogues for %d NPCs"), NPCDialogues.Num());
}

void UQuest_DialogueManager::SetupQuestGivers()
{
    RegisterQuestGiver(TEXT("HuntMaster"), TEXT("prehistoric_hunt"));
    RegisterQuestGiver(TEXT("ResourceCollector"), TEXT("resource_gathering"));
    RegisterQuestGiver(TEXT("SurvivalExpert"), TEXT("survival_challenge"));
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Setup %d quest givers"), QuestGivers.Num());
}

FQuest_DialogueEntry UQuest_DialogueManager::CreateDialogueEntry(const FString& ID, const FString& Speaker, 
                                                               const FString& Text, const FString& AudioPath)
{
    FQuest_DialogueEntry Entry;
    Entry.DialogueID = ID;
    Entry.SpeakerName = Speaker;
    Entry.DialogueText = Text;
    Entry.AudioFilePath = AudioPath;
    Entry.bIsQuestGiver = true;
    
    return Entry;
}

void UQuest_DialogueManager::AddNPCDialogue(const FString& NPCName, const FQuest_DialogueEntry& Entry)
{
    if (NPCDialogues.Contains(NPCName))
    {
        NPCDialogues[NPCName].DialogueEntries.Add(Entry);
    }
}