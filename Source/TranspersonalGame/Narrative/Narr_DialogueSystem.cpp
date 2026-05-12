#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    GlobalDialogueRange = 1000.0f;
    DialogueCooldownTime = 5.0f;
    bEnableProximityDialogues = true;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System initialized"));
    
    // Initialize default character profiles
    LoadDialogueData();
}

void UNarr_DialogueSystem::Deinitialize()
{
    // Save progress before shutdown
    SaveDialogueProgress();
    
    // Clear data
    RegisteredCharacters.Empty();
    DialogueHistory.Empty();
    LoadedVoiceLines.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::RegisterCharacter(const FString& CharacterID, const FNarr_CharacterProfile& Profile)
{
    if (CharacterID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register character with empty ID"));
        return;
    }
    
    RegisteredCharacters.Add(CharacterID, Profile);
    DialogueHistory.Add(CharacterID, TArray<FString>());
    
    UE_LOG(LogTemp, Log, TEXT("Registered character: %s with personality: %d"), 
           *CharacterID, (int32)Profile.PersonalityType);
}

bool UNarr_DialogueSystem::GetCharacterProfile(const FString& CharacterID, FNarr_CharacterProfile& OutProfile)
{
    if (FNarr_CharacterProfile* Profile = RegisteredCharacters.Find(CharacterID))
    {
        OutProfile = *Profile;
        return true;
    }
    return false;
}

TArray<FString> UNarr_DialogueSystem::GetAllCharacterIDs()
{
    TArray<FString> CharacterIDs;
    RegisteredCharacters.GetKeys(CharacterIDs);
    return CharacterIDs;
}

bool UNarr_DialogueSystem::TriggerDialogue(const FString& CharacterID, ENarr_DialogueType DialogueType, AActor* Player, AActor* NPC)
{
    if (!Player || !NPC)
    {
        return false;
    }
    
    if (!CanTriggerDialogue(CharacterID, DialogueType))
    {
        return false;
    }
    
    float Distance = FVector::Dist(Player->GetActorLocation(), NPC->GetActorLocation());
    FNarr_DialogueLine BestDialogue = GetBestDialogue(CharacterID, DialogueType, Distance);
    
    if (!BestDialogue.DialogueText.IsEmpty())
    {
        // Play the dialogue
        PlayVoiceLine(BestDialogue, NPC);
        
        // Update history
        UpdateDialogueHistory(CharacterID, BestDialogue.DialogueText);
        
        UE_LOG(LogTemp, Log, TEXT("Triggered dialogue: %s from %s"), 
               *BestDialogue.DialogueText, *CharacterID);
        return true;
    }
    
    return false;
}

FNarr_DialogueLine UNarr_DialogueSystem::GetBestDialogue(const FString& CharacterID, ENarr_DialogueType DialogueType, float Distance)
{
    FNarr_DialogueLine BestDialogue;
    float BestPriority = -1.0f;
    
    if (FNarr_CharacterProfile* Profile = RegisteredCharacters.Find(CharacterID))
    {
        for (const FNarr_DialogueLine& Dialogue : Profile->AvailableDialogues)
        {
            if (Dialogue.DialogueType == DialogueType || DialogueType == ENarr_DialogueType::Information)
            {
                if (Distance <= Dialogue.TriggerDistance)
                {
                    float Priority = CalculateDialoguePriority(Dialogue, Distance, DialogueType);
                    if (Priority > BestPriority)
                    {
                        BestPriority = Priority;
                        BestDialogue = Dialogue;
                    }
                }
            }
        }
    }
    
    return BestDialogue;
}

void UNarr_DialogueSystem::PlayVoiceLine(const FNarr_DialogueLine& DialogueLine, AActor* SourceActor)
{
    if (!SourceActor)
    {
        return;
    }
    
    // Log the dialogue text
    UE_LOG(LogTemp, Warning, TEXT("[%s]: %s"), *DialogueLine.CharacterName, *DialogueLine.DialogueText);
    
    // Try to play audio if available
    if (DialogueLine.VoiceLine.IsValid())
    {
        USoundCue* SoundCue = DialogueLine.VoiceLine.LoadSynchronous();
        if (SoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(
                SourceActor->GetWorld(),
                SoundCue,
                SourceActor->GetActorLocation(),
                1.0f,
                1.0f,
                0.0f
            );
        }
    }
    
    // Display on screen for debugging
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *DialogueLine.CharacterName, 
                                            *DialogueLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }
}

void UNarr_DialogueSystem::CheckProximityDialogues(AActor* Player, const TArray<AActor*>& NPCs)
{
    if (!Player || !bEnableProximityDialogues)
    {
        return;
    }
    
    for (AActor* NPC : NPCs)
    {
        if (!NPC)
        {
            continue;
        }
        
        FString NPCName = NPC->GetName();
        float Distance = FVector::Dist(Player->GetActorLocation(), NPC->GetActorLocation());
        
        if (Distance <= GlobalDialogueRange)
        {
            // Check if this NPC has registered dialogues
            for (const auto& CharacterPair : RegisteredCharacters)
            {
                if (CharacterPair.Key.Contains(NPCName) || NPCName.Contains(CharacterPair.Key))
                {
                    // Try to trigger a greeting or information dialogue
                    TriggerDialogue(CharacterPair.Key, ENarr_DialogueType::Greeting, Player, NPC);
                    break;
                }
            }
        }
    }
}

void UNarr_DialogueSystem::BroadcastEmergencyWarning(const FString& WarningText, const FVector& DangerLocation)
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY WARNING: %s at location %s"), 
           *WarningText, *DangerLocation.ToString());
    
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("EMERGENCY: %s"), *WarningText);
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, DisplayText);
    }
    
    // Find all scout characters and trigger emergency dialogues
    for (const auto& CharacterPair : RegisteredCharacters)
    {
        if (CharacterPair.Value.PersonalityType == ENarr_NPCPersonality::Scout_Alert)
        {
            // Create emergency dialogue line
            FNarr_DialogueLine EmergencyLine;
            EmergencyLine.DialogueText = WarningText;
            EmergencyLine.DialogueType = ENarr_DialogueType::Emergency;
            EmergencyLine.CharacterName = CharacterPair.Value.CharacterName;
            EmergencyLine.Priority = 10; // Maximum priority
            
            // This would need an NPC actor reference in a real implementation
            UE_LOG(LogTemp, Warning, TEXT("Emergency broadcast from %s: %s"), 
                   *CharacterPair.Value.CharacterName, *WarningText);
        }
    }
}

void UNarr_DialogueSystem::TriggerStorytellingSequence(const FString& CharacterID, const FString& StoryTheme)
{
    if (FNarr_CharacterProfile* Profile = RegisteredCharacters.Find(CharacterID))
    {
        if (Profile->PersonalityType == ENarr_NPCPersonality::Elder_Wise)
        {
            // Find storytelling dialogues
            for (const FNarr_DialogueLine& Dialogue : Profile->AvailableDialogues)
            {
                if (Dialogue.DialogueType == ENarr_DialogueType::Storytelling)
                {
                    UE_LOG(LogTemp, Log, TEXT("Elder %s begins storytelling: %s"), 
                           *Profile->CharacterName, *Dialogue.DialogueText);
                    
                    if (GEngine)
                    {
                        FString DisplayText = FString::Printf(TEXT("[Story] %s: %s"), 
                                                            *Profile->CharacterName, 
                                                            *Dialogue.DialogueText);
                        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, DisplayText);
                    }
                    break;
                }
            }
        }
    }
}

void UNarr_DialogueSystem::LoadDialogueData()
{
    // Create default character profiles
    
    // Elder Storyteller
    FNarr_CharacterProfile ElderProfile;
    ElderProfile.CharacterName = TEXT("Elder Storyteller");
    ElderProfile.PersonalityType = ENarr_NPCPersonality::Elder_Wise;
    ElderProfile.BackgroundStory = TEXT("An ancient survivor who has seen many seasons pass. Keeper of tribal knowledge and stories.");
    ElderProfile.SpecialtyKnowledge.Add(TEXT("Ancient History"));
    ElderProfile.SpecialtyKnowledge.Add(TEXT("Dinosaur Behavior"));
    ElderProfile.SpecialtyKnowledge.Add(TEXT("Survival Wisdom"));
    
    FNarr_DialogueLine StoryLine;
    StoryLine.DialogueText = TEXT("The ancient stories speak of the time before time, when the great lizards ruled the earth...");
    StoryLine.DialogueType = ENarr_DialogueType::Storytelling;
    StoryLine.CharacterName = ElderProfile.CharacterName;
    StoryLine.PersonalityType = ENarr_NPCPersonality::Elder_Wise;
    StoryLine.Priority = 5;
    ElderProfile.AvailableDialogues.Add(StoryLine);
    
    RegisterCharacter(TEXT("Elder_001"), ElderProfile);
    
    // Scout Marcus
    FNarr_CharacterProfile ScoutProfile;
    ScoutProfile.CharacterName = TEXT("Scout Marcus");
    ScoutProfile.PersonalityType = ENarr_NPCPersonality::Scout_Alert;
    ScoutProfile.BackgroundStory = TEXT("A keen-eyed scout who watches for predator movements and dangers.");
    ScoutProfile.SpecialtyKnowledge.Add(TEXT("Predator Tracking"));
    ScoutProfile.SpecialtyKnowledge.Add(TEXT("Territory Mapping"));
    
    FNarr_DialogueLine WarningLine;
    WarningLine.DialogueText = TEXT("Warning! Massive predator approaching from the north ridge!");
    WarningLine.DialogueType = ENarr_DialogueType::Emergency;
    WarningLine.CharacterName = ScoutProfile.CharacterName;
    WarningLine.PersonalityType = ENarr_NPCPersonality::Scout_Alert;
    WarningLine.Priority = 10;
    ScoutProfile.AvailableDialogues.Add(WarningLine);
    
    RegisterCharacter(TEXT("Scout_001"), ScoutProfile);
    
    // Tracker Elena
    FNarr_CharacterProfile TrackerProfile;
    TrackerProfile.CharacterName = TEXT("Tracker Elena");
    TrackerProfile.PersonalityType = ENarr_NPCPersonality::Tracker_Analytical;
    TrackerProfile.BackgroundStory = TEXT("An analytical tracker who studies migration patterns and animal behavior.");
    TrackerProfile.SpecialtyKnowledge.Add(TEXT("Migration Patterns"));
    TrackerProfile.SpecialtyKnowledge.Add(TEXT("Herbivore Behavior"));
    
    FNarr_DialogueLine InfoLine;
    InfoLine.DialogueText = TEXT("The herbivore migration patterns have shifted again this season...");
    InfoLine.DialogueType = ENarr_DialogueType::Information;
    InfoLine.CharacterName = TrackerProfile.CharacterName;
    InfoLine.PersonalityType = ENarr_NPCPersonality::Tracker_Analytical;
    InfoLine.Priority = 3;
    TrackerProfile.AvailableDialogues.Add(InfoLine);
    
    RegisterCharacter(TEXT("Tracker_001"), TrackerProfile);
    
    // Veteran Hunter
    FNarr_CharacterProfile HunterProfile;
    HunterProfile.CharacterName = TEXT("Veteran Hunter");
    HunterProfile.PersonalityType = ENarr_NPCPersonality::Veteran_Cautious;
    HunterProfile.BackgroundStory = TEXT("A seasoned hunter with years of experience facing the deadliest predators.");
    HunterProfile.SpecialtyKnowledge.Add(TEXT("Combat Tactics"));
    HunterProfile.SpecialtyKnowledge.Add(TEXT("Predator Weaknesses"));
    
    FNarr_DialogueLine TeachingLine;
    TeachingLine.DialogueText = TEXT("Listen to me, rookie. In this world, knowledge is survival...");
    TeachingLine.DialogueType = ENarr_DialogueType::Information;
    TeachingLine.CharacterName = HunterProfile.CharacterName;
    TeachingLine.PersonalityType = ENarr_NPCPersonality::Veteran_Cautious;
    TeachingLine.Priority = 4;
    HunterProfile.AvailableDialogues.Add(TeachingLine);
    
    RegisterCharacter(TEXT("Hunter_001"), HunterProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded %d character profiles with dialogue data"), RegisteredCharacters.Num());
}

void UNarr_DialogueSystem::SaveDialogueProgress()
{
    // In a full implementation, this would save to a file or database
    UE_LOG(LogTemp, Log, TEXT("Saving dialogue progress for %d characters"), RegisteredCharacters.Num());
    
    for (const auto& HistoryPair : DialogueHistory)
    {
        UE_LOG(LogTemp, Log, TEXT("Character %s has %d dialogue entries"), 
               *HistoryPair.Key, HistoryPair.Value.Num());
    }
}

bool UNarr_DialogueSystem::CanTriggerDialogue(const FString& CharacterID, ENarr_DialogueType DialogueType)
{
    // Check if character exists
    if (!RegisteredCharacters.Contains(CharacterID))
    {
        return false;
    }
    
    // Emergency dialogues always allowed
    if (DialogueType == ENarr_DialogueType::Emergency)
    {
        return true;
    }
    
    // Check cooldown (simplified - in full implementation would track per-character cooldowns)
    return true;
}

void UNarr_DialogueSystem::UpdateDialogueHistory(const FString& CharacterID, const FString& DialogueText)
{
    if (TArray<FString>* History = DialogueHistory.Find(CharacterID))
    {
        History->Add(DialogueText);
        
        // Keep only last 10 dialogues per character
        if (History->Num() > 10)
        {
            History->RemoveAt(0);
        }
    }
}

float UNarr_DialogueSystem::CalculateDialoguePriority(const FNarr_DialogueLine& DialogueLine, float Distance, ENarr_DialogueType RequestedType)
{
    float Priority = DialogueLine.Priority;
    
    // Distance factor (closer = higher priority)
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / DialogueLine.TriggerDistance), 0.1f, 1.0f);
    Priority *= DistanceFactor;
    
    // Type match bonus
    if (DialogueLine.DialogueType == RequestedType)
    {
        Priority *= 1.5f;
    }
    
    // Emergency dialogues get massive priority boost
    if (DialogueLine.DialogueType == ENarr_DialogueType::Emergency)
    {
        Priority *= 3.0f;
    }
    
    return Priority;
}