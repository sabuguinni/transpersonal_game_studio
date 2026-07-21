#include "Narr_TribalDialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_TribalDialogueComponent::UNarr_TribalDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize tribal identity
    TribalRole = ENarr_TribalRole::Elder;
    TribalName = TEXT("Unknown");
    Age = 30;
    
    // Initialize relationships
    PlayerTrust = 0;
    PlayerRespect = 0;
    
    // Initialize survival state
    CurrentFear = 0.0f;
    HungerLevel = 0.5f;
}

void UNarr_TribalDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalDialogue();
    InitializeTribalKnowledge();
}

void UNarr_TribalDialogueComponent::InitializeTribalDialogue()
{
    AvailableDialogue.Empty();
    
    // Elder dialogue
    FNarr_TribalDialogueEntry ElderWisdom;
    ElderWisdom.DialogueText = TEXT("The old paths teach us much about survival. Listen well, young hunter.");
    ElderWisdom.RequiredRole = ENarr_TribalRole::Elder;
    ElderWisdom.Context = ENarr_SurvivalContext::Settlement;
    ElderWisdom.RelationshipThreshold = 0;
    ElderWisdom.PlayerResponses.Add(TEXT("Tell me about the old ways"));
    ElderWisdom.PlayerResponses.Add(TEXT("What dangers should I know about?"));
    ElderWisdom.ThreatLevel = 0.0f;
    AvailableDialogue.Add(ElderWisdom);
    
    // Hunter dialogue
    FNarr_TribalDialogueEntry HunterAdvice;
    HunterAdvice.DialogueText = TEXT("The great beasts move differently when storms approach. Watch the sky, track the wind.");
    HunterAdvice.RequiredRole = ENarr_TribalRole::Hunter;
    HunterAdvice.Context = ENarr_SurvivalContext::Hunting;
    HunterAdvice.RelationshipThreshold = 10;
    HunterAdvice.PlayerResponses.Add(TEXT("Show me the hunting signs"));
    HunterAdvice.PlayerResponses.Add(TEXT("Where are the best hunting grounds?"));
    HunterAdvice.ThreatLevel = 0.3f;
    AvailableDialogue.Add(HunterAdvice);
    
    // Scout dialogue
    FNarr_TribalDialogueEntry ScoutWarning;
    ScoutWarning.DialogueText = TEXT("I've seen fresh tracks beyond the ridge. Something large moves in the darkness.");
    ScoutWarning.RequiredRole = ENarr_TribalRole::Scout;
    ScoutWarning.Context = ENarr_SurvivalContext::Dangerous;
    ScoutWarning.RelationshipThreshold = 5;
    ScoutWarning.PlayerResponses.Add(TEXT("What kind of tracks?"));
    ScoutWarning.PlayerResponses.Add(TEXT("Should we avoid that area?"));
    ScoutWarning.ThreatLevel = 0.7f;
    AvailableDialogue.Add(ScoutWarning);
    
    // Gatherer dialogue
    FNarr_TribalDialogueEntry GathererKnowledge;
    GathererKnowledge.DialogueText = TEXT("These berries heal wounds, but those leaves bring fever. Learn to see the difference.");
    GathererKnowledge.RequiredRole = ENarr_TribalRole::Gatherer;
    GathererKnowledge.Context = ENarr_SurvivalContext::Gathering;
    GathererKnowledge.RelationshipThreshold = 0;
    GathererKnowledge.PlayerResponses.Add(TEXT("Teach me about healing plants"));
    GathererKnowledge.PlayerResponses.Add(TEXT("Where do you find the best resources?"));
    GathererKnowledge.ThreatLevel = 0.1f;
    AvailableDialogue.Add(GathererKnowledge);
    
    // Warrior dialogue
    FNarr_TribalDialogueEntry WarriorTactics;
    WarriorTactics.DialogueText = TEXT("Face a raptor alone and you die. Face a pack and you run. Face a rex and you pray.");
    WarriorTactics.RequiredRole = ENarr_TribalRole::Warrior;
    WarriorTactics.Context = ENarr_SurvivalContext::Dangerous;
    WarriorTactics.RelationshipThreshold = 15;
    WarriorTactics.PlayerResponses.Add(TEXT("Teach me combat tactics"));
    WarriorTactics.PlayerResponses.Add(TEXT("How do you survive predator attacks?"));
    WarriorTactics.ThreatLevel = 0.8f;
    AvailableDialogue.Add(WarriorTactics);
}

void UNarr_TribalDialogueComponent::InitializeTribalKnowledge()
{
    TribalKnowledge.Empty();
    
    // Hunting knowledge
    FNarr_TribalKnowledge HuntingLore;
    HuntingLore.KnowledgeType = TEXT("Hunting Techniques");
    HuntingLore.Description = TEXT("Ancient methods for tracking and hunting prehistoric creatures safely.");
    HuntingLore.RequiredTrust = 20;
    HuntingLore.Prerequisites.Add(TEXT("Proven in combat"));
    TribalKnowledge.Add(HuntingLore);
    
    // Survival knowledge
    FNarr_TribalKnowledge SurvivalLore;
    SurvivalLore.KnowledgeType = TEXT("Survival Wisdom");
    SurvivalLore.Description = TEXT("How to find water, shelter, and food in the harsh prehistoric world.");
    SurvivalLore.RequiredTrust = 10;
    SurvivalLore.Prerequisites.Add(TEXT("Helped the tribe"));
    TribalKnowledge.Add(SurvivalLore);
    
    // Sacred knowledge
    FNarr_TribalKnowledge SacredLore;
    SacredLore.KnowledgeType = TEXT("Sacred Sites");
    SacredLore.Description = TEXT("Locations of power and danger that must be respected or avoided.");
    SacredLore.RequiredTrust = 50;
    SacredLore.Prerequisites.Add(TEXT("Elder status"));
    TribalKnowledge.Add(SacredLore);
}

FString UNarr_TribalDialogueComponent::GetContextualDialogue(ENarr_SurvivalContext Context, float ThreatLevel)
{
    for (const FNarr_TribalDialogueEntry& Entry : AvailableDialogue)
    {
        if (Entry.RequiredRole == TribalRole && 
            Entry.Context == Context && 
            PlayerTrust >= Entry.RelationshipThreshold &&
            ThreatLevel >= Entry.ThreatLevel - 0.2f && 
            ThreatLevel <= Entry.ThreatLevel + 0.2f)
        {
            return Entry.DialogueText;
        }
    }
    
    // Fallback dialogue based on role
    return SelectDialogueByRole(Context);
}

FString UNarr_TribalDialogueComponent::SelectDialogueByRole(ENarr_SurvivalContext Context)
{
    switch (TribalRole)
    {
        case ENarr_TribalRole::Elder:
            return TEXT("The ancestors guide us through these trials. Stay strong.");
            
        case ENarr_TribalRole::Hunter:
            return TEXT("The hunt calls, but wisdom keeps us alive.");
            
        case ENarr_TribalRole::Scout:
            return TEXT("I watch the horizon for signs of danger.");
            
        case ENarr_TribalRole::Gatherer:
            return TEXT("Nature provides, if you know where to look.");
            
        case ENarr_TribalRole::Warrior:
            return TEXT("Strength protects the tribe, but courage protects the soul.");
            
        case ENarr_TribalRole::Shaman:
            return TEXT("The spirits whisper of change coming to our lands.");
            
        case ENarr_TribalRole::Crafter:
            return TEXT("Good tools mean the difference between life and death.");
            
        case ENarr_TribalRole::Child:
            return TEXT("Will you tell me a story about the great beasts?");
            
        default:
            return TEXT("Greetings, traveler.");
    }
}

TArray<FString> UNarr_TribalDialogueComponent::GetPlayerResponses(const FString& DialogueKey)
{
    for (const FNarr_TribalDialogueEntry& Entry : AvailableDialogue)
    {
        if (Entry.DialogueText.Contains(DialogueKey))
        {
            return Entry.PlayerResponses;
        }
    }
    
    TArray<FString> DefaultResponses;
    DefaultResponses.Add(TEXT("Tell me more"));
    DefaultResponses.Add(TEXT("I understand"));
    DefaultResponses.Add(TEXT("Thank you"));
    return DefaultResponses;
}

void UNarr_TribalDialogueComponent::ModifyTrust(int32 TrustChange)
{
    PlayerTrust = FMath::Clamp(PlayerTrust + TrustChange, -100, 100);
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Trust with %s: %d"), *TribalName, PlayerTrust);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, Message);
    }
}

void UNarr_TribalDialogueComponent::ModifyRespect(int32 RespectChange)
{
    PlayerRespect = FMath::Clamp(PlayerRespect + RespectChange, -100, 100);
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Respect with %s: %d"), *TribalName, PlayerRespect);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Message);
    }
}

bool UNarr_TribalDialogueComponent::CanShareKnowledge(const FString& KnowledgeType)
{
    for (const FNarr_TribalKnowledge& Knowledge : TribalKnowledge)
    {
        if (Knowledge.KnowledgeType == KnowledgeType)
        {
            return PlayerTrust >= Knowledge.RequiredTrust;
        }
    }
    return false;
}

void UNarr_TribalDialogueComponent::UpdateSurvivalState(float Fear, float Hunger)
{
    CurrentFear = FMath::Clamp(Fear, 0.0f, 1.0f);
    HungerLevel = FMath::Clamp(Hunger, 0.0f, 1.0f);
}

// Tribal Dialogue Manager Implementation
ANarr_TribalDialogueManager::ANarr_TribalDialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    CurrentContext = ENarr_SurvivalContext::Settlement;
    AreaThreatLevel = 0.0f;
}

void ANarr_TribalDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalDialogue();
}

void ANarr_TribalDialogueManager::InitializeTribalDialogue()
{
    LoadTribalDialogueDatabase();
    SetupContextualResponses();
}

void ANarr_TribalDialogueManager::LoadTribalDialogueDatabase()
{
    RoleBasedDialogue.Empty();
    
    // Initialize dialogue for each role
    TArray<FNarr_TribalDialogueEntry> ElderDialogue;
    TArray<FNarr_TribalDialogueEntry> HunterDialogue;
    TArray<FNarr_TribalDialogueEntry> ScoutDialogue;
    TArray<FNarr_TribalDialogueEntry> GathererDialogue;
    TArray<FNarr_TribalDialogueEntry> WarriorDialogue;
    
    // Add comprehensive dialogue entries for each role
    RoleBasedDialogue.Add(ENarr_TribalRole::Elder, ElderDialogue);
    RoleBasedDialogue.Add(ENarr_TribalRole::Hunter, HunterDialogue);
    RoleBasedDialogue.Add(ENarr_TribalRole::Scout, ScoutDialogue);
    RoleBasedDialogue.Add(ENarr_TribalRole::Gatherer, GathererDialogue);
    RoleBasedDialogue.Add(ENarr_TribalRole::Warrior, WarriorDialogue);
}

void ANarr_TribalDialogueManager::SetupContextualResponses()
{
    // Setup context-sensitive dialogue responses
    // This would typically load from data tables in a production environment
}

FString ANarr_TribalDialogueManager::ProcessTribalInteraction(AActor* NPC, AActor* Player)
{
    if (!NPC || !Player)
    {
        return TEXT("No interaction possible");
    }
    
    UNarr_TribalDialogueComponent* DialogueComp = NPC->FindComponentByClass<UNarr_TribalDialogueComponent>();
    if (!DialogueComp)
    {
        return TEXT("This person doesn't seem interested in talking");
    }
    
    return DialogueComp->GetContextualDialogue(CurrentContext, AreaThreatLevel);
}

void ANarr_TribalDialogueManager::UpdateSurvivalContext(ENarr_SurvivalContext NewContext, float ThreatLevel)
{
    CurrentContext = NewContext;
    AreaThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    if (GEngine)
    {
        FString ContextName;
        switch (CurrentContext)
        {
            case ENarr_SurvivalContext::Safe: ContextName = TEXT("Safe"); break;
            case ENarr_SurvivalContext::Dangerous: ContextName = TEXT("Dangerous"); break;
            case ENarr_SurvivalContext::Hunting: ContextName = TEXT("Hunting"); break;
            case ENarr_SurvivalContext::Gathering: ContextName = TEXT("Gathering"); break;
            case ENarr_SurvivalContext::Settlement: ContextName = TEXT("Settlement"); break;
            case ENarr_SurvivalContext::Unknown: ContextName = TEXT("Unknown"); break;
            case ENarr_SurvivalContext::Sacred: ContextName = TEXT("Sacred"); break;
            case ENarr_SurvivalContext::Forbidden: ContextName = TEXT("Forbidden"); break;
        }
        
        FString Message = FString::Printf(TEXT("Context: %s (Threat: %.2f)"), *ContextName, AreaThreatLevel);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }
}

void ANarr_TribalDialogueManager::UpdateTribalRelationships(AActor* NPC, int32 TrustChange, int32 RespectChange)
{
    if (!NPC)
    {
        return;
    }
    
    UNarr_TribalDialogueComponent* DialogueComp = NPC->FindComponentByClass<UNarr_TribalDialogueComponent>();
    if (DialogueComp)
    {
        DialogueComp->ModifyTrust(TrustChange);
        DialogueComp->ModifyRespect(RespectChange);
    }
}