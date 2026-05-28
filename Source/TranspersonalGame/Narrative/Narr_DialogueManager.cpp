#include "Narr_DialogueManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    CurrentContext = FNarr_DialogueContext();
    DialogueDatabase.Empty();
    LastPlayedTimes.Empty();
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Initializing narrative dialogue system"));
    
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Loaded %d dialogue lines"), DialogueDatabase.Num());
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // Survival warning dialogues
    FNarr_DialogueLine LowHealthWarning;
    LowHealthWarning.DialogueText = TEXT("Your wounds bleed freely. Find shelter and rest, or death will claim you.");
    LowHealthWarning.Speaker = ENarr_SpeakerType::Narrator;
    LowHealthWarning.TriggerCondition = ENarr_DialogueTrigger::LowHealth;
    LowHealthWarning.Priority = 10.0f;
    LowHealthWarning.bIsRepeatable = true;
    LowHealthWarning.CooldownTime = 60.0f;
    RegisterDialogueLine(LowHealthWarning);

    FNarr_DialogueLine HungerWarning;
    HungerWarning.DialogueText = TEXT("Your stomach gnaws with hunger. Hunt or gather food before weakness takes hold.");
    HungerWarning.Speaker = ENarr_SpeakerType::Narrator;
    HungerWarning.TriggerCondition = ENarr_DialogueTrigger::Survival_Hunger;
    HungerWarning.Priority = 8.0f;
    HungerWarning.bIsRepeatable = true;
    HungerWarning.CooldownTime = 45.0f;
    RegisterDialogueLine(HungerWarning);

    FNarr_DialogueLine ThirstWarning;
    ThirstWarning.DialogueText = TEXT("Your throat burns with thirst. Seek water or perish under the merciless sun.");
    ThirstWarning.Speaker = ENarr_SpeakerType::Narrator;
    ThirstWarning.TriggerCondition = ENarr_DialogueTrigger::Survival_Thirst;
    ThirstWarning.Priority = 9.0f;
    ThirstWarning.bIsRepeatable = true;
    ThirstWarning.CooldownTime = 40.0f;
    RegisterDialogueLine(ThirstWarning);

    // Dinosaur encounter dialogues
    FNarr_DialogueLine TRexWarning;
    TRexWarning.DialogueText = TEXT("The earth trembles. A massive predator approaches. Hide or prepare for the fight of your life.");
    TRexWarning.Speaker = ENarr_SpeakerType::Warning;
    TRexWarning.TriggerCondition = ENarr_DialogueTrigger::DinosaurNearby;
    TRexWarning.Priority = 15.0f;
    TRexWarning.bIsRepeatable = true;
    TRexWarning.CooldownTime = 120.0f;
    RegisterDialogueLine(TRexWarning);

    FNarr_DialogueLine RaptorWarning;
    RaptorWarning.DialogueText = TEXT("Clever hunters stalk through the undergrowth. They hunt in packs. Watch your back.");
    RaptorWarning.Speaker = ENarr_SpeakerType::Warning;
    RaptorWarning.TriggerCondition = ENarr_DialogueTrigger::DinosaurNearby;
    RaptorWarning.Priority = 12.0f;
    RaptorWarning.bIsRepeatable = true;
    RaptorWarning.CooldownTime = 90.0f;
    RegisterDialogueLine(RaptorWarning);

    // Tribal elder wisdom
    FNarr_DialogueLine ElderWisdom1;
    ElderWisdom1.DialogueText = TEXT("Fire keeps the darkness at bay. Sharp stone cuts deep. These truths have kept our people alive.");
    ElderWisdom1.Speaker = ENarr_SpeakerType::TribalElder;
    ElderWisdom1.TriggerCondition = ENarr_DialogueTrigger::FirstEncounter;
    ElderWisdom1.Priority = 5.0f;
    ElderWisdom1.bIsRepeatable = false;
    ElderWisdom1.CooldownTime = 0.0f;
    RegisterDialogueLine(ElderWisdom1);

    FNarr_DialogueLine ElderWisdom2;
    ElderWisdom2.DialogueText = TEXT("The great beasts have ruled this land since time began. Respect their power or become their prey.");
    ElderWisdom2.Speaker = ENarr_SpeakerType::TribalElder;
    ElderWisdom2.TriggerCondition = ENarr_DialogueTrigger::Discovery;
    ElderWisdom2.Priority = 6.0f;
    ElderWisdom2.bIsRepeatable = true;
    ElderWisdom2.CooldownTime = 300.0f;
    RegisterDialogueLine(ElderWisdom2);

    // Discovery dialogues
    FNarr_DialogueLine ResourceDiscovery;
    ResourceDiscovery.DialogueText = TEXT("Useful materials lie scattered here. Gather what you can - survival depends on preparation.");
    ResourceDiscovery.Speaker = ENarr_SpeakerType::PlayerThought;
    ResourceDiscovery.TriggerCondition = ENarr_DialogueTrigger::Discovery;
    ResourceDiscovery.Priority = 4.0f;
    ResourceDiscovery.bIsRepeatable = true;
    ResourceDiscovery.CooldownTime = 30.0f;
    RegisterDialogueLine(ResourceDiscovery);
}

void UNarr_DialogueManager::TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context)
{
    UpdateContext(Context);
    
    FNarr_DialogueLine* BestDialogue = FindBestDialogue(TriggerType, Context);
    
    if (BestDialogue && CanPlayDialogue(*BestDialogue))
    {
        PlayDialogue(*BestDialogue);
    }
}

void UNarr_DialogueManager::RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    DialogueDatabase.Add(DialogueLine);
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Registered dialogue line: %s"), *DialogueLine.DialogueText);
}

void UNarr_DialogueManager::PlayDialogue(const FNarr_DialogueLine& DialogueLine)
{
    FString DialogueKey = FString::Printf(TEXT("%s_%s"), 
        *UEnum::GetValueAsString(DialogueLine.Speaker),
        *DialogueLine.DialogueText.Left(20));
    
    LastPlayedTimes.Add(DialogueKey, GetWorld()->GetTimeSeconds());
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Playing dialogue - %s: %s"), 
        *UEnum::GetValueAsString(DialogueLine.Speaker), 
        *DialogueLine.DialogueText);
    
    OnDialogueTriggered(DialogueLine);
    
    // Play audio if available
    if (DialogueLine.VoiceAudio.IsValid())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DialogueLine.VoiceAudio.LoadSynchronous());
    }
}

bool UNarr_DialogueManager::CanPlayDialogue(const FNarr_DialogueLine& DialogueLine) const
{
    if (!DialogueLine.bIsRepeatable)
    {
        FString DialogueKey = FString::Printf(TEXT("%s_%s"), 
            *UEnum::GetValueAsString(DialogueLine.Speaker),
            *DialogueLine.DialogueText.Left(20));
        
        if (LastPlayedTimes.Contains(DialogueKey))
        {
            return false;
        }
    }
    
    if (DialogueLine.CooldownTime > 0.0f)
    {
        FString DialogueKey = FString::Printf(TEXT("%s_%s"), 
            *UEnum::GetValueAsString(DialogueLine.Speaker),
            *DialogueLine.DialogueText.Left(20));
        
        if (const float* LastPlayTime = LastPlayedTimes.Find(DialogueKey))
        {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            if (CurrentTime - *LastPlayTime < DialogueLine.CooldownTime)
            {
                return false;
            }
        }
    }
    
    return EvaluateTriggerCondition(DialogueLine, CurrentContext);
}

void UNarr_DialogueManager::UpdateContext(const FNarr_DialogueContext& NewContext)
{
    CurrentContext = NewContext;
    OnContextChanged(NewContext);
}

bool UNarr_DialogueManager::EvaluateTriggerCondition(const FNarr_DialogueLine& DialogueLine, const FNarr_DialogueContext& Context) const
{
    switch (DialogueLine.TriggerCondition)
    {
        case ENarr_DialogueTrigger::LowHealth:
            return Context.PlayerHealthPercent < 30.0f;
            
        case ENarr_DialogueTrigger::Survival_Hunger:
            return Context.PlayerHungerLevel > 70.0f;
            
        case ENarr_DialogueTrigger::Survival_Thirst:
            return Context.PlayerThirstLevel > 80.0f;
            
        case ENarr_DialogueTrigger::DinosaurNearby:
            return Context.bDinosaurNearby && Context.DinosaurDistance < 2000.0f;
            
        case ENarr_DialogueTrigger::Combat_Warning:
            return Context.bDinosaurNearby && Context.DinosaurDistance < 1000.0f;
            
        case ENarr_DialogueTrigger::Discovery:
        case ENarr_DialogueTrigger::FirstEncounter:
        case ENarr_DialogueTrigger::QuestComplete:
        case ENarr_DialogueTrigger::PlayerApproach:
            return true;
            
        default:
            return false;
    }
}

FNarr_DialogueLine* UNarr_DialogueManager::FindBestDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context)
{
    FNarr_DialogueLine* BestDialogue = nullptr;
    float HighestPriority = -1.0f;
    
    for (FNarr_DialogueLine& DialogueLine : DialogueDatabase)
    {
        if (DialogueLine.TriggerCondition == TriggerType && 
            EvaluateTriggerCondition(DialogueLine, Context) &&
            DialogueLine.Priority > HighestPriority)
        {
            BestDialogue = &DialogueLine;
            HighestPriority = DialogueLine.Priority;
        }
    }
    
    return BestDialogue;
}