#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    DefaultCooldownTime = 30.0f;
    bEnableDebugOutput = true;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System initialized with %d entries"), DialogueEntries.Num());
    }
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateCooldowns(DeltaTime);
}

void UNarr_DialogueSystem::TriggerDialogue(ENarr_DialogueTrigger TriggerType, float HealthPercent, float HungerLevel, float ThirstLevel)
{
    if (IsDialogueOnCooldown(TriggerType))
    {
        return;
    }

    FNarr_DialogueEntry* BestDialogue = FindBestDialogue(TriggerType, HealthPercent, HungerLevel, ThirstLevel);
    if (BestDialogue)
    {
        PlayDialogue(BestDialogue->DialogueText, BestDialogue->Priority);
        SetDialogueCooldown(TriggerType, BestDialogue->CooldownTime);
        
        LogDialogue(BestDialogue->DialogueText, TriggerType);
    }
}

void UNarr_DialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueEntries.Add(NewEntry);
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("Added dialogue entry: %s"), *NewEntry.DialogueText);
    }
}

void UNarr_DialogueSystem::PlayDialogue(const FString& DialogueText, ENarr_DialoguePriority Priority)
{
    if (GEngine)
    {
        FColor DisplayColor = FColor::White;
        switch (Priority)
        {
            case ENarr_DialoguePriority::Critical:
                DisplayColor = FColor::Red;
                break;
            case ENarr_DialoguePriority::High:
                DisplayColor = FColor::Orange;
                break;
            case ENarr_DialoguePriority::Normal:
                DisplayColor = FColor::Yellow;
                break;
            case ENarr_DialoguePriority::Low:
                DisplayColor = FColor::Green;
                break;
        }
        
        float DisplayTime = 5.0f + (static_cast<int32>(Priority) * 2.0f);
        GEngine->AddOnScreenDebugMessage(-1, DisplayTime, DisplayColor, DialogueText);
    }
}

bool UNarr_DialogueSystem::IsDialogueOnCooldown(ENarr_DialogueTrigger TriggerType) const
{
    if (const float* CooldownTime = DialogueCooldowns.Find(TriggerType))
    {
        return *CooldownTime > 0.0f;
    }
    return false;
}

void UNarr_DialogueSystem::SetDialogueCooldown(ENarr_DialogueTrigger TriggerType, float CooldownTime)
{
    DialogueCooldowns.Add(TriggerType, CooldownTime);
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Critical survival warnings
    FNarr_DialogueEntry CriticalHealth;
    CriticalHealth.DialogueText = TEXT("Your life force weakens! Find shelter and sustenance before the darkness claims you!");
    CriticalHealth.TriggerType = ENarr_DialogueTrigger::LowHealth;
    CriticalHealth.Priority = ENarr_DialoguePriority::Critical;
    CriticalHealth.CooldownTime = 45.0f;
    CriticalHealth.bIsRepeatable = true;
    DialogueEntries.Add(CriticalHealth);

    // Hunger warnings
    FNarr_DialogueEntry HungerWarning;
    HungerWarning.DialogueText = TEXT("Your stomach gnaws with emptiness. Hunt or gather, or weakness will make you prey.");
    HungerWarning.TriggerType = ENarr_DialogueTrigger::Hunger;
    HungerWarning.Priority = ENarr_DialoguePriority::High;
    HungerWarning.CooldownTime = 60.0f;
    HungerWarning.bIsRepeatable = true;
    DialogueEntries.Add(HungerWarning);

    // Thirst warnings
    FNarr_DialogueEntry ThirstWarning;
    ThirstWarning.DialogueText = TEXT("Your throat burns like desert sand. Seek water before your strength fails completely.");
    ThirstWarning.TriggerType = ENarr_DialogueTrigger::Thirst;
    ThirstWarning.Priority = ENarr_DialoguePriority::High;
    ThirstWarning.CooldownTime = 50.0f;
    ThirstWarning.bIsRepeatable = true;
    DialogueEntries.Add(ThirstWarning);

    // Dinosaur proximity warnings
    FNarr_DialogueEntry DinosaurNear;
    DinosaurNear.DialogueText = TEXT("Ancient predator stalks nearby! Move with caution or become its next meal.");
    DinosaurNear.TriggerType = ENarr_DialogueTrigger::DinosaurNearby;
    DinosaurNear.Priority = ENarr_DialoguePriority::Critical;
    DinosaurNear.CooldownTime = 30.0f;
    DinosaurNear.bIsRepeatable = true;
    DialogueEntries.Add(DinosaurNear);

    // Combat warnings
    FNarr_DialogueEntry CombatAlert;
    CombatAlert.DialogueText = TEXT("Battle approaches! Ready your primitive weapons and fight for survival!");
    CombatAlert.TriggerType = ENarr_DialogueTrigger::CombatWarning;
    CombatAlert.Priority = ENarr_DialoguePriority::Critical;
    CombatAlert.CooldownTime = 20.0f;
    CombatAlert.bIsRepeatable = true;
    DialogueEntries.Add(CombatAlert);

    // Discovery celebrations
    FNarr_DialogueEntry Discovery;
    Discovery.DialogueText = TEXT("You have uncovered ancient secrets! This knowledge will aid your survival.");
    Discovery.TriggerType = ENarr_DialogueTrigger::Discovery;
    Discovery.Priority = ENarr_DialoguePriority::Normal;
    Discovery.CooldownTime = 15.0f;
    Discovery.bIsRepeatable = true;
    DialogueEntries.Add(Discovery);

    // Fire discovery
    FNarr_DialogueEntry FireLit;
    FireLit.DialogueText = TEXT("Fire! The sacred flame burns bright, keeping the darkness and beasts at bay.");
    FireLit.TriggerType = ENarr_DialogueTrigger::FireLit;
    FireLit.Priority = ENarr_DialoguePriority::High;
    FireLit.CooldownTime = 120.0f;
    FireLit.bIsRepeatable = false;
    DialogueEntries.Add(FireLit);

    // Day/Night cycle
    FNarr_DialogueEntry NightFall;
    NightFall.DialogueText = TEXT("Darkness falls upon the prehistoric world. Predators hunt in shadow - stay vigilant.");
    NightFall.TriggerType = ENarr_DialogueTrigger::NightFall;
    NightFall.Priority = ENarr_DialoguePriority::Normal;
    NightFall.CooldownTime = 600.0f; // 10 minutes
    NightFall.bIsRepeatable = true;
    DialogueEntries.Add(NightFall);

    FNarr_DialogueEntry DayBreak;
    DayBreak.DialogueText = TEXT("Dawn breaks over the ancient lands. A new day of survival begins.");
    DayBreak.TriggerType = ENarr_DialogueTrigger::DayBreak;
    DayBreak.Priority = ENarr_DialoguePriority::Low;
    DayBreak.CooldownTime = 600.0f; // 10 minutes
    DayBreak.bIsRepeatable = true;
    DialogueEntries.Add(DayBreak);
}

FNarr_DialogueEntry* UNarr_DialogueSystem::FindBestDialogue(ENarr_DialogueTrigger TriggerType, float HealthPercent, float HungerLevel, float ThirstLevel)
{
    TArray<FNarr_DialogueEntry*> MatchingDialogues;
    
    for (FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.TriggerType == TriggerType)
        {
            // Additional context-based filtering
            bool bContextMatch = true;
            
            if (TriggerType == ENarr_DialogueTrigger::LowHealth && HealthPercent > 0.3f)
            {
                bContextMatch = false;
            }
            else if (TriggerType == ENarr_DialogueTrigger::Hunger && HungerLevel < 0.7f)
            {
                bContextMatch = false;
            }
            else if (TriggerType == ENarr_DialogueTrigger::Thirst && ThirstLevel < 0.7f)
            {
                bContextMatch = false;
            }
            
            if (bContextMatch)
            {
                MatchingDialogues.Add(&Entry);
            }
        }
    }
    
    if (MatchingDialogues.Num() == 0)
    {
        return nullptr;
    }
    
    // Sort by priority (highest first)
    MatchingDialogues.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B) {
        return static_cast<int32>(A.Priority) > static_cast<int32>(B.Priority);
    });
    
    return MatchingDialogues[0];
}

void UNarr_DialogueSystem::UpdateCooldowns(float DeltaTime)
{
    for (auto& CooldownPair : DialogueCooldowns)
    {
        if (CooldownPair.Value > 0.0f)
        {
            CooldownPair.Value = FMath::Max(0.0f, CooldownPair.Value - DeltaTime);
        }
    }
}

void UNarr_DialogueSystem::LogDialogue(const FString& DialogueText, ENarr_DialogueTrigger TriggerType) const
{
    if (bEnableDebugOutput)
    {
        FString TriggerName = UEnum::GetValueAsString(TriggerType);
        UE_LOG(LogTemp, Warning, TEXT("NARRATIVE [%s]: %s"), *TriggerName, *DialogueText);
    }
}