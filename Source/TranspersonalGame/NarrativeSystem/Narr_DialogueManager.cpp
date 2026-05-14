#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Narrative DialogueManager initialized"));
    
    // Initialize narrative state
    NarrativeState = FNarr_NarrativeState();
    
    // Load dialogue database
    LoadDialogueDatabase();
    
    // Setup default unlocked dialogues for new players
    UnlockDialogue("INTRO_SURVIVAL_001");
    UnlockDialogue("HUNT_BASICS_001");
    UnlockDialogue("DANGER_GENERAL_001");
}

void UNarr_DialogueManager::Deinitialize()
{
    if (CurrentDialogueAudio && IsValid(CurrentDialogueAudio))
    {
        CurrentDialogueAudio->Stop();
        CurrentDialogueAudio = nullptr;
    }
    
    DialogueDatabase.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::LoadDialogueDatabase()
{
    // Create default dialogue entries for prehistoric survival game
    FNarr_DialogueEntry HuntEntry;
    HuntEntry.DialogueID = "HUNT_BASICS_001";
    HuntEntry.SpeakerRole = ENarr_CharacterRole::HuntMaster;
    HuntEntry.DialogueType = ENarr_DialogueType::HuntInstruction;
    HuntEntry.DialogueText = FText::FromString("The great hunt begins at dawn. Position yourself on the ridge and strike when the sun touches the peaks.");
    HuntEntry.Duration = 8.0f;
    DialogueDatabase.Add(HuntEntry.DialogueID, HuntEntry);
    
    FNarr_DialogueEntry SurvivalEntry;
    SurvivalEntry.DialogueID = "SURVIVAL_RAPTORS_001";
    SurvivalEntry.SpeakerRole = ENarr_CharacterRole::SurvivalGuide;
    SurvivalEntry.DialogueType = ENarr_DialogueType::SurvivalWarning;
    SurvivalEntry.DialogueText = FText::FromString("Velociraptors hunt in packs. They flank from both sides while one distracts. Keep your spear ready.");
    SurvivalEntry.Duration = 10.0f;
    DialogueDatabase.Add(SurvivalEntry.DialogueID, SurvivalEntry);
    
    FNarr_DialogueEntry DangerEntry;
    DangerEntry.DialogueID = "DANGER_TREX_001";
    DangerEntry.SpeakerRole = ENarr_CharacterRole::Scout;
    DangerEntry.DialogueType = ENarr_DialogueType::ThreatAlert;
    DangerEntry.DialogueText = FText::FromString("Thunder Lizard approaches! Retreat to higher ground immediately. Do not engage!");
    DangerEntry.Duration = 6.0f;
    DialogueDatabase.Add(DangerEntry.DialogueID, DangerEntry);
    
    FNarr_DialogueEntry WisdomEntry;
    WisdomEntry.DialogueID = "WISDOM_CAVES_001";
    WisdomEntry.SpeakerRole = ENarr_CharacterRole::Elder;
    WisdomEntry.DialogueType = ENarr_DialogueType::TribeWisdom;
    WisdomEntry.DialogueText = FText::FromString("Ancient caves hold secrets. Study the markings - they guide to water, shelter, or warn of danger.");
    WisdomEntry.Duration = 9.0f;
    DialogueDatabase.Add(WisdomEntry.DialogueID, WisdomEntry);
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue database loaded with %d entries"), DialogueDatabase.Num());
}

void UNarr_DialogueManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!IsDialogueUnlocked(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s is not unlocked"), *DialogueID);
        return;
    }
    
    const FNarr_DialogueEntry* DialogueEntry = DialogueDatabase.Find(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue %s not found in database"), *DialogueID);
        return;
    }
    
    if (!CheckDialogueConditions(*DialogueEntry))
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue %s conditions not met"), *DialogueID);
        return;
    }
    
    // Play the dialogue
    PlayDialogueAudio(*DialogueEntry);
    
    // Log the event
    LogDialogueEvent(DialogueID, Speaker ? Speaker->GetName() : "System");
    
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s"), *DialogueEntry->DialogueText.ToString());
}

void UNarr_DialogueManager::PlayDialogueByType(ENarr_DialogueType DialogueType, ENarr_CharacterRole SpeakerRole)
{
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Entry = DialoguePair.Value;
        if (Entry.DialogueType == DialogueType && Entry.SpeakerRole == SpeakerRole)
        {
            if (IsDialogueUnlocked(Entry.DialogueID))
            {
                TriggerDialogue(Entry.DialogueID);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No unlocked dialogue found for type %d, role %d"), 
           (int32)DialogueType, (int32)SpeakerRole);
}

bool UNarr_DialogueManager::IsDialogueUnlocked(const FString& DialogueID) const
{
    return NarrativeState.UnlockedDialogues.Contains(DialogueID);
}

void UNarr_DialogueManager::UnlockDialogue(const FString& DialogueID)
{
    if (!NarrativeState.UnlockedDialogues.Contains(DialogueID))
    {
        NarrativeState.UnlockedDialogues.Add(DialogueID);
        UE_LOG(LogTemp, Log, TEXT("Unlocked dialogue: %s"), *DialogueID);
    }
}

void UNarr_DialogueManager::AdvanceChapter(int32 NewChapter)
{
    if (NewChapter > NarrativeState.CurrentChapter)
    {
        NarrativeState.CurrentChapter = NewChapter;
        UE_LOG(LogTemp, Log, TEXT("Advanced to chapter %d"), NewChapter);
        
        // Unlock chapter-specific dialogues
        switch (NewChapter)
        {
        case 2:
            UnlockDialogue("HUNT_ADVANCED_001");
            UnlockDialogue("TRIBE_MEETING_001");
            break;
        case 3:
            UnlockDialogue("ELDER_WISDOM_001");
            UnlockDialogue("DANGEROUS_TERRITORY_001");
            break;
        }
    }
}

void UNarr_DialogueManager::RegisterQuestCompletion(const FString& QuestID)
{
    if (!NarrativeState.CompletedQuests.Contains(QuestID))
    {
        NarrativeState.CompletedQuests.Add(QuestID);
        UE_LOG(LogTemp, Log, TEXT("Quest completed: %s"), *QuestID);
        
        // Unlock related dialogues based on quest completion
        if (QuestID.Contains("HUNT"))
        {
            NarrativeState.SuccessfulHunts++;
            UnlockDialogue("HUNT_SUCCESS_001");
        }
        else if (QuestID.Contains("SURVIVAL"))
        {
            UnlockDialogue("SURVIVAL_MASTERY_001");
        }
    }
}

void UNarr_DialogueManager::UpdateSurvivalExperience(float ExperienceGained)
{
    NarrativeState.SurvivalExperience += ExperienceGained;
    
    // Unlock dialogues based on experience milestones
    if (NarrativeState.SurvivalExperience >= 100.0f && !IsDialogueUnlocked("EXPERIENCED_SURVIVOR_001"))
    {
        UnlockDialogue("EXPERIENCED_SURVIVOR_001");
    }
    
    if (NarrativeState.SurvivalExperience >= 500.0f && !IsDialogueUnlocked("MASTER_SURVIVOR_001"))
    {
        UnlockDialogue("MASTER_SURVIVOR_001");
    }
}

void UNarr_DialogueManager::TriggerHuntDialogue(const FString& DinosaurType, int32 PackSize)
{
    FString DialogueContext = FString::Printf(TEXT("Hunt_%s_Pack%d"), *DinosaurType, PackSize);
    
    if (DinosaurType.Contains("Raptor") && PackSize > 1)
    {
        TriggerDialogue("SURVIVAL_RAPTORS_001");
    }
    else if (DinosaurType.Contains("TRex"))
    {
        TriggerDialogue("DANGER_TREX_001");
    }
    else
    {
        PlayDialogueByType(ENarr_DialogueType::HuntInstruction, ENarr_CharacterRole::HuntMaster);
    }
    
    LogDialogueEvent("HUNT_TRIGGERED", DialogueContext);
}

void UNarr_DialogueManager::TriggerDangerWarning(const FString& ThreatType, float Distance)
{
    FString DialogueContext = FString::Printf(TEXT("Danger_%s_Dist%.0f"), *ThreatType, Distance);
    
    if (ThreatType.Contains("TRex") || ThreatType.Contains("Thunder"))
    {
        TriggerDialogue("DANGER_TREX_001");
    }
    else
    {
        PlayDialogueByType(ENarr_DialogueType::ThreatAlert, ENarr_CharacterRole::Scout);
    }
    
    LogDialogueEvent("DANGER_WARNING", DialogueContext);
}

void UNarr_DialogueManager::TriggerResourceGuidance(const FString& ResourceType, bool bFound)
{
    FString DialogueContext = FString::Printf(TEXT("Resource_%s_%s"), *ResourceType, bFound ? TEXT("Found") : TEXT("Seeking"));
    
    if (ResourceType.Contains("Water") || ResourceType.Contains("Shelter"))
    {
        TriggerDialogue("WISDOM_CAVES_001");
    }
    else
    {
        PlayDialogueByType(ENarr_DialogueType::ResourceGuidance, ENarr_CharacterRole::Elder);
    }
    
    LogDialogueEvent("RESOURCE_GUIDANCE", DialogueContext);
}

TArray<FNarr_DialogueEntry> UNarr_DialogueManager::GetAvailableDialogues() const
{
    TArray<FNarr_DialogueEntry> AvailableDialogues;
    
    for (const auto& DialoguePair : DialogueDatabase)
    {
        if (IsDialogueUnlocked(DialoguePair.Key))
        {
            AvailableDialogues.Add(DialoguePair.Value);
        }
    }
    
    return AvailableDialogues;
}

void UNarr_DialogueManager::PlayDialogueAudio(const FNarr_DialogueEntry& DialogueEntry)
{
    // Stop current dialogue if playing
    if (CurrentDialogueAudio && IsValid(CurrentDialogueAudio))
    {
        CurrentDialogueAudio->Stop();
    }
    
    // For now, just log the dialogue text
    // In a full implementation, this would load and play the audio asset
    UE_LOG(LogTemp, Log, TEXT("DIALOGUE [%s]: %s"), 
           *UEnum::GetValueAsString(DialogueEntry.SpeakerRole),
           *DialogueEntry.DialogueText.ToString());
    
    // Display dialogue text on screen for debugging
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), 
                                            *UEnum::GetValueAsString(DialogueEntry.SpeakerRole),
                                            *DialogueEntry.DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, DialogueEntry.Duration, FColor::Yellow, DisplayText);
    }
}

bool UNarr_DialogueManager::CheckDialogueConditions(const FNarr_DialogueEntry& DialogueEntry) const
{
    // Check basic narrative state conditions
    for (const FString& Condition : DialogueEntry.TriggerConditions)
    {
        if (Condition.Contains("Chapter") && !Condition.Contains(FString::FromInt(NarrativeState.CurrentChapter)))
        {
            return false;
        }
        
        if (Condition.Contains("Quest") && !NarrativeState.CompletedQuests.ContainsByPredicate([&Condition](const FString& Quest)
        {
            return Condition.Contains(Quest);
        }))
        {
            return false;
        }
        
        if (Condition.Contains("Experience"))
        {
            FString ExpString = Condition.RightChop(Condition.Find("Experience") + 10);
            float RequiredExp = FCString::Atof(*ExpString);
            if (NarrativeState.SurvivalExperience < RequiredExp)
            {
                return false;
            }
        }
    }
    
    return true;
}

void UNarr_DialogueManager::LogDialogueEvent(const FString& DialogueID, const FString& Context)
{
    FString LogEntry = FString::Printf(TEXT("NARRATIVE_EVENT: %s | Context: %s | Chapter: %d | Experience: %.1f"), 
                                     *DialogueID, *Context, NarrativeState.CurrentChapter, NarrativeState.SurvivalExperience);
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogEntry);
}