#include "Narr_DialogueManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bDialogueActive = false;
    CurrentCharacterID = TEXT("");
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadDialogueData();
    InitializeTribalCharacters();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue Manager initialized with %d characters and %d dialogue nodes"), 
           CharacterDatabase.Num(), DialogueDatabase.Num());
}

void UNarr_DialogueManager::Deinitialize()
{
    DialogueDatabase.Empty();
    CharacterDatabase.Empty();
    CompletedStoryEvents.Empty();
    StoryVariables.Empty();
    
    Super::Deinitialize();
}

bool UNarr_DialogueManager::StartDialogue(const FString& CharacterID, const FString& InitialDialogueID)
{
    if (!CharacterDatabase.Contains(CharacterID))
    {
        UE_LOG(LogTemp, Error, TEXT("Character %s not found in database"), *CharacterID);
        return false;
    }

    if (!DialogueDatabase.Contains(InitialDialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue %s not found in database"), *InitialDialogueID);
        return false;
    }

    FNarr_DialogueNode* DialogueNode = DialogueDatabase.Find(InitialDialogueID);
    if (!DialogueNode)
    {
        return false;
    }

    // Check quest requirements
    if (DialogueNode->bRequiresQuestCompletion && !DialogueNode->RequiredQuestID.IsEmpty())
    {
        if (!HasStoryEventOccurred(DialogueNode->RequiredQuestID))
        {
            UE_LOG(LogTemp, Warning, TEXT("Dialogue %s requires quest %s to be completed"), 
                   *InitialDialogueID, *DialogueNode->RequiredQuestID);
            return false;
        }
    }

    CurrentDialogue = *DialogueNode;
    CurrentCharacterID = CharacterID;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Started dialogue %s with character %s"), *InitialDialogueID, *CharacterID);
    return true;
}

bool UNarr_DialogueManager::SelectDialogueResponse(int32 ResponseIndex)
{
    if (!bDialogueActive)
    {
        return false;
    }

    if (!CurrentDialogue.ResponseOptions.IsValidIndex(ResponseIndex) || 
        !CurrentDialogue.NextDialogueIDs.IsValidIndex(ResponseIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid response index %d"), ResponseIndex);
        return false;
    }

    FString NextDialogueID = CurrentDialogue.NextDialogueIDs[ResponseIndex];
    
    if (NextDialogueID.IsEmpty() || NextDialogueID == TEXT("END"))
    {
        EndDialogue();
        return true;
    }

    if (!DialogueDatabase.Contains(NextDialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Next dialogue %s not found"), *NextDialogueID);
        EndDialogue();
        return false;
    }

    CurrentDialogue = DialogueDatabase[NextDialogueID];
    return true;
}

void UNarr_DialogueManager::EndDialogue()
{
    bDialogueActive = false;
    CurrentCharacterID = TEXT("");
    CurrentDialogue = FNarr_DialogueNode();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueNode UNarr_DialogueManager::GetCurrentDialogueNode() const
{
    return CurrentDialogue;
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueManager::RegisterCharacter(const FNarr_CharacterProfile& Character)
{
    CharacterDatabase.Add(Character.CharacterID, Character);
    UE_LOG(LogTemp, Log, TEXT("Registered character %s (%s)"), *Character.CharacterName, *Character.CharacterID);
}

FNarr_CharacterProfile UNarr_DialogueManager::GetCharacterProfile(const FString& CharacterID) const
{
    if (CharacterDatabase.Contains(CharacterID))
    {
        return CharacterDatabase[CharacterID];
    }
    
    return FNarr_CharacterProfile();
}

void UNarr_DialogueManager::UpdateCharacterTrust(const FString& CharacterID, float TrustDelta)
{
    if (CharacterDatabase.Contains(CharacterID))
    {
        FNarr_CharacterProfile& Character = CharacterDatabase[CharacterID];
        Character.TrustLevel = FMath::Clamp(Character.TrustLevel + TrustDelta, 0.0f, 100.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Updated trust for %s: %f"), *CharacterID, Character.TrustLevel);
    }
}

TArray<FString> UNarr_DialogueManager::GetNearbyCharacters(const FVector& PlayerLocation, float SearchRadius) const
{
    TArray<FString> NearbyCharacters;
    
    for (const auto& CharacterPair : CharacterDatabase)
    {
        const FNarr_CharacterProfile& Character = CharacterPair.Value;
        if (Character.bIsAlive)
        {
            float Distance = FVector::Dist(PlayerLocation, Character.WorldLocation);
            if (Distance <= SearchRadius)
            {
                NearbyCharacters.Add(Character.CharacterID);
            }
        }
    }
    
    return NearbyCharacters;
}

void UNarr_DialogueManager::MarkStoryEvent(const FString& EventID)
{
    CompletedStoryEvents.Add(EventID);
    UE_LOG(LogTemp, Log, TEXT("Story event marked: %s"), *EventID);
}

bool UNarr_DialogueManager::HasStoryEventOccurred(const FString& EventID) const
{
    return CompletedStoryEvents.Contains(EventID);
}

void UNarr_DialogueManager::SetStoryVariable(const FString& VariableName, const FString& Value)
{
    StoryVariables.Add(VariableName, Value);
    UE_LOG(LogTemp, Log, TEXT("Story variable set: %s = %s"), *VariableName, *Value);
}

FString UNarr_DialogueManager::GetStoryVariable(const FString& VariableName) const
{
    if (StoryVariables.Contains(VariableName))
    {
        return StoryVariables[VariableName];
    }
    
    return TEXT("");
}

void UNarr_DialogueManager::LoadDialogueData()
{
    // Elder introduction dialogue
    FNarr_DialogueNode ElderIntro;
    ElderIntro.DialogueID = TEXT("ELDER_INTRO_01");
    ElderIntro.SpeakerName = TEXT("Tribal Elder");
    ElderIntro.DialogueText = TEXT("Young one, you have survived your first night in the valley. The ancestors smile upon you.");
    ElderIntro.ResponseOptions.Add(TEXT("Tell me about this place."));
    ElderIntro.ResponseOptions.Add(TEXT("What dangers should I know about?"));
    ElderIntro.NextDialogueIDs.Add(TEXT("ELDER_LORE_01"));
    ElderIntro.NextDialogueIDs.Add(TEXT("ELDER_DANGER_01"));
    DialogueDatabase.Add(ElderIntro.DialogueID, ElderIntro);

    // Elder lore dialogue
    FNarr_DialogueNode ElderLore;
    ElderLore.DialogueID = TEXT("ELDER_LORE_01");
    ElderLore.SpeakerName = TEXT("Tribal Elder");
    ElderLore.DialogueText = TEXT("This valley has been our home for many generations. The Thunder Lizards rule the day, but we have learned their ways.");
    ElderLore.ResponseOptions.Add(TEXT("How do we survive among such beasts?"));
    ElderLore.NextDialogueIDs.Add(TEXT("ELDER_SURVIVAL_01"));
    DialogueDatabase.Add(ElderLore.DialogueID, ElderLore);

    // Elder danger dialogue
    FNarr_DialogueNode ElderDanger;
    ElderDanger.DialogueID = TEXT("ELDER_DANGER_01");
    ElderDanger.SpeakerName = TEXT("Tribal Elder");
    ElderDanger.DialogueText = TEXT("The pack hunters are your greatest threat. They hunt in groups and show no mercy to the isolated.");
    ElderDanger.ResponseOptions.Add(TEXT("How can I defend myself?"));
    ElderDanger.NextDialogueIDs.Add(TEXT("ELDER_DEFENSE_01"));
    DialogueDatabase.Add(ElderDanger.DialogueID, ElderDanger);

    // Scout warning dialogue
    FNarr_DialogueNode ScoutWarning;
    ScoutWarning.DialogueID = TEXT("SCOUT_WARNING_01");
    ScoutWarning.SpeakerName = TEXT("Tribal Scout");
    ScoutWarning.DialogueText = TEXT("I've seen fresh tracks near the eastern cliffs. Large predators, moving in formation.");
    ScoutWarning.ResponseOptions.Add(TEXT("How many were there?"));
    ScoutWarning.ResponseOptions.Add(TEXT("Should we avoid that area?"));
    ScoutWarning.NextDialogueIDs.Add(TEXT("SCOUT_COUNT_01"));
    ScoutWarning.NextDialogueIDs.Add(TEXT("SCOUT_ADVICE_01"));
    DialogueDatabase.Add(ScoutWarning.DialogueID, ScoutWarning);

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d dialogue nodes"), DialogueDatabase.Num());
}

void UNarr_DialogueManager::InitializeTribalCharacters()
{
    // Tribal Elder
    FNarr_CharacterProfile Elder;
    Elder.CharacterID = TEXT("CHAR_ELDER_01");
    Elder.CharacterName = TEXT("Kael the Wise");
    Elder.TribalRole = ENarr_TribalRole::Elder;
    Elder.WorldLocation = FVector(1000.0f, 0.0f, 100.0f);
    Elder.TrustLevel = 75.0f;
    Elder.AvailableDialogues.Add(TEXT("ELDER_INTRO_01"));
    RegisterCharacter(Elder);

    // Tribal Scout
    FNarr_CharacterProfile Scout;
    Scout.CharacterID = TEXT("CHAR_SCOUT_01");
    Scout.CharacterName = TEXT("Vera the Swift");
    Scout.TribalRole = ENarr_TribalRole::Scout;
    Scout.WorldLocation = FVector(-500.0f, 800.0f, 150.0f);
    Scout.TrustLevel = 60.0f;
    Scout.AvailableDialogues.Add(TEXT("SCOUT_WARNING_01"));
    RegisterCharacter(Scout);

    // Tribal Hunter
    FNarr_CharacterProfile Hunter;
    Hunter.CharacterID = TEXT("CHAR_HUNTER_01");
    Hunter.CharacterName = TEXT("Thane the Bold");
    Hunter.TribalRole = ENarr_TribalRole::Hunter;
    Hunter.WorldLocation = FVector(300.0f, -600.0f, 80.0f);
    Hunter.TrustLevel = 50.0f;
    RegisterCharacter(Hunter);

    // Tribal Crafter
    FNarr_CharacterProfile Crafter;
    Crafter.CharacterID = TEXT("CHAR_CRAFTER_01");
    Crafter.CharacterName = TEXT("Mira the Maker");
    Crafter.TribalRole = ENarr_TribalRole::Crafter;
    Crafter.WorldLocation = FVector(-200.0f, -400.0f, 120.0f);
    Crafter.TrustLevel = 70.0f;
    RegisterCharacter(Crafter);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d tribal characters"), CharacterDatabase.Num());
}

bool UNarr_DialogueManager::ValidateDialogueTransition(const FString& FromDialogueID, const FString& ToDialogueID) const
{
    if (!DialogueDatabase.Contains(FromDialogueID) || !DialogueDatabase.Contains(ToDialogueID))
    {
        return false;
    }

    const FNarr_DialogueNode& FromDialogue = DialogueDatabase[FromDialogueID];
    return FromDialogue.NextDialogueIDs.Contains(ToDialogueID);
}