#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    CurrentLocation = TEXT("");
    NearbyDinosaur = TEXT("");
    PlayerHealth = 100.0f;
    PlayerHunger = 100.0f;
    PlayerThirst = 100.0f;
    PlayerFear = 0.0f;
    CurrentTimeOfDay = 12.0f;
    DialogueAudioComponent = nullptr;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Initializing dialogue system"));
    
    // Start dialogue update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueUpdateTimer, this, &UNarr_DialogueManager::TickDialogueSystem, 1.0f, true);
    }
    
    // Initialize default dialogue lines
    FNarr_DialogueLine WelcomeDialogue;
    WelcomeDialogue.DialogueID = TEXT("WELCOME_001");
    WelcomeDialogue.CharacterType = ENarr_CharacterType::TribalElder;
    WelcomeDialogue.DialogueText = TEXT("Welcome to the ancient lands, young hunter. The great beasts roam these territories.");
    WelcomeDialogue.TriggerType = ENarr_DialogueTrigger::LocationEnter;
    WelcomeDialogue.TriggerCondition = TEXT("StartingArea");
    WelcomeDialogue.Priority = 10;
    WelcomeDialogue.Duration = 8.0f;
    RegisterDialogueLine(WelcomeDialogue);
    
    FNarr_DialogueLine DinosaurWarning;
    DinosaurWarning.DialogueID = TEXT("DINO_WARNING_001");
    DinosaurWarning.CharacterType = ENarr_CharacterType::TribalScout;
    DinosaurWarning.DialogueText = TEXT("Massive predator spotted ahead. Approach with extreme caution.");
    DinosaurWarning.TriggerType = ENarr_DialogueTrigger::DinosaurSighted;
    DinosaurWarning.TriggerCondition = TEXT("TRex");
    DinosaurWarning.Priority = 15;
    DinosaurWarning.Duration = 6.0f;
    RegisterDialogueLine(DinosaurWarning);
    
    FNarr_DialogueLine SurvivalCritical;
    SurvivalCritical.DialogueID = TEXT("SURVIVAL_CRITICAL_001");
    SurvivalCritical.CharacterType = ENarr_CharacterType::PlayerThought;
    SurvivalCritical.DialogueText = TEXT("My strength fades... I must find food and water soon or I will not survive.");
    SurvivalCritical.TriggerType = ENarr_DialogueTrigger::SurvivalCritical;
    SurvivalCritical.TriggerCondition = TEXT("LowHealth");
    SurvivalCritical.Priority = 20;
    SurvivalCritical.Duration = 7.0f;
    RegisterDialogueLine(SurvivalCritical);
}

void UNarr_DialogueManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueUpdateTimer);
    }
    
    StopCurrentDialogue();
    DialogueDatabase.Empty();
    DialogueCooldowns.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FString& Condition)
{
    if (IsDialoguePlaying())
    {
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Dialogue already playing, ignoring trigger"));
        return;
    }
    
    FNarr_DialogueLine* BestDialogue = FindBestDialogue(TriggerType, Condition);
    if (BestDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Triggering dialogue: %s"), *BestDialogue->DialogueID);
        PlayDialogueLine(*BestDialogue);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: No suitable dialogue found for trigger"));
    }
}

void UNarr_DialogueManager::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    if (IsDialoguePlaying())
    {
        StopCurrentDialogue();
    }
    
    CurrentDialogue.DialogueLine = DialogueLine;
    CurrentDialogue.TimeStarted = GetWorld()->GetTimeSeconds();
    CurrentDialogue.bIsPlaying = true;
    
    // Create audio component if needed
    if (!DialogueAudioComponent && GetWorld())
    {
        DialogueAudioComponent = NewObject<UAudioComponent>(this);
        if (DialogueAudioComponent)
        {
            DialogueAudioComponent->RegisterComponent();
        }
    }
    
    // Play audio if available
    if (DialogueAudioComponent && DialogueLine.AudioAsset.LoadSynchronous())
    {
        DialogueAudioComponent->SetSound(DialogueLine.AudioAsset.Get());
        DialogueAudioComponent->Play();
        CurrentDialogue.AudioComponent = DialogueAudioComponent;
    }
    
    // Add cooldown
    AddDialogueCooldown(DialogueLine.DialogueID, DialogueLine.Cooldown);
    
    // Set timer to end dialogue
    if (UWorld* World = GetWorld())
    {
        FTimerHandle DialogueEndTimer;
        World->GetTimerManager().SetTimer(DialogueEndTimer, this, &UNarr_DialogueManager::OnDialogueFinished, DialogueLine.Duration, false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Playing dialogue: %s"), *DialogueLine.DialogueText);
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    if (CurrentDialogue.bIsPlaying)
    {
        CurrentDialogue.bIsPlaying = false;
        
        if (CurrentDialogue.AudioComponent)
        {
            CurrentDialogue.AudioComponent->Stop();
            CurrentDialogue.AudioComponent = nullptr;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Stopped current dialogue"));
    }
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return CurrentDialogue.bIsPlaying;
}

void UNarr_DialogueManager::RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    DialogueDatabase.Add(DialogueLine);
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Registered dialogue line: %s"), *DialogueLine.DialogueID);
}

void UNarr_DialogueManager::LoadDialogueDatabase(UDataTable* DialogueTable)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Invalid dialogue table"));
        return;
    }
    
    DialogueDatabase.Empty();
    
    TArray<FName> RowNames = DialogueTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        if (FNarr_DialogueLine* Row = DialogueTable->FindRow<FNarr_DialogueLine>(RowName, TEXT("")))
        {
            RegisterDialogueLine(*Row);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Loaded %d dialogue lines from table"), DialogueDatabase.Num());
}

void UNarr_DialogueManager::SetPlayerLocation(const FString& LocationName)
{
    if (CurrentLocation != LocationName)
    {
        FString PreviousLocation = CurrentLocation;
        CurrentLocation = LocationName;
        
        // Trigger location enter dialogue
        TriggerDialogue(ENarr_DialogueTrigger::LocationEnter, LocationName);
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Player location changed to: %s"), *LocationName);
    }
}

void UNarr_DialogueManager::SetNearbyDinosaur(const FString& DinosaurType)
{
    if (NearbyDinosaur != DinosaurType && !DinosaurType.IsEmpty())
    {
        NearbyDinosaur = DinosaurType;
        
        // Trigger dinosaur sighted dialogue
        TriggerDialogue(ENarr_DialogueTrigger::DinosaurSighted, DinosaurType);
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Dinosaur sighted: %s"), *DinosaurType);
    }
}

void UNarr_DialogueManager::SetSurvivalState(float Health, float Hunger, float Thirst, float Fear)
{
    PlayerHealth = Health;
    PlayerHunger = Hunger;
    PlayerThirst = Thirst;
    PlayerFear = Fear;
    
    // Check for critical survival states
    if (Health < 25.0f || Hunger < 20.0f || Thirst < 20.0f)
    {
        TriggerDialogue(ENarr_DialogueTrigger::SurvivalCritical, TEXT("LowHealth"));
    }
    else if (Fear > 80.0f)
    {
        TriggerDialogue(ENarr_DialogueTrigger::SurvivalCritical, TEXT("HighFear"));
    }
}

void UNarr_DialogueManager::SetTimeOfDay(float TimeHours)
{
    float PreviousTime = CurrentTimeOfDay;
    CurrentTimeOfDay = TimeHours;
    
    // Trigger time-based dialogue
    if ((PreviousTime < 6.0f && TimeHours >= 6.0f) || (PreviousTime < 18.0f && TimeHours >= 18.0f))
    {
        FString TimeCondition = (TimeHours >= 6.0f && TimeHours < 18.0f) ? TEXT("Dawn") : TEXT("Dusk");
        TriggerDialogue(ENarr_DialogueTrigger::TimeOfDay, TimeCondition);
    }
}

bool UNarr_DialogueManager::CanPlayDialogue(const FString& DialogueID) const
{
    if (const float* CooldownTime = DialogueCooldowns.Find(DialogueID))
    {
        return GetWorld()->GetTimeSeconds() >= *CooldownTime;
    }
    return true;
}

void UNarr_DialogueManager::AddDialogueCooldown(const FString& DialogueID, float CooldownTime)
{
    float EndTime = GetWorld()->GetTimeSeconds() + CooldownTime;
    DialogueCooldowns.Add(DialogueID, EndTime);
}

FNarr_DialogueLine* UNarr_DialogueManager::FindBestDialogue(ENarr_DialogueTrigger TriggerType, const FString& Condition)
{
    FNarr_DialogueLine* BestDialogue = nullptr;
    int32 BestPriority = -1;
    
    for (FNarr_DialogueLine& DialogueLine : DialogueDatabase)
    {
        if (DialogueLine.TriggerType == TriggerType)
        {
            if (CanPlayDialogue(DialogueLine.DialogueID))
            {
                if (EvaluateDialogueCondition(DialogueLine, Condition))
                {
                    if (DialogueLine.Priority > BestPriority)
                    {
                        BestPriority = DialogueLine.Priority;
                        BestDialogue = &DialogueLine;
                    }
                }
            }
        }
    }
    
    return BestDialogue;
}

bool UNarr_DialogueManager::EvaluateDialogueCondition(const FNarr_DialogueLine& DialogueLine, const FString& Condition)
{
    if (DialogueLine.TriggerCondition.IsEmpty() || Condition.IsEmpty())
    {
        return true;
    }
    
    return DialogueLine.TriggerCondition.Equals(Condition, ESearchCase::IgnoreCase);
}

void UNarr_DialogueManager::UpdateCooldowns()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    TArray<FString> ExpiredCooldowns;
    for (const auto& Cooldown : DialogueCooldowns)
    {
        if (CurrentTime >= Cooldown.Value)
        {
            ExpiredCooldowns.Add(Cooldown.Key);
        }
    }
    
    for (const FString& ExpiredID : ExpiredCooldowns)
    {
        DialogueCooldowns.Remove(ExpiredID);
    }
}

void UNarr_DialogueManager::OnDialogueFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Dialogue finished"));
    StopCurrentDialogue();
}

void UNarr_DialogueManager::TickDialogueSystem()
{
    UpdateCooldowns();
    
    // Check if current dialogue has exceeded its duration
    if (CurrentDialogue.bIsPlaying)
    {
        float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentDialogue.TimeStarted;
        if (ElapsedTime >= CurrentDialogue.DialogueLine.Duration)
        {
            OnDialogueFinished();
        }
    }
}