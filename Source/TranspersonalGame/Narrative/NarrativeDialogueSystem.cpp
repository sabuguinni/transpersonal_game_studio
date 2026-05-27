#include "NarrativeDialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    TriggerRadius = 1000.0f;
    bAutoTrigger = true;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize trigger tracking
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        LastTriggerTimes.Add(Entry.TriggerType, -999.0f);
        TriggerCounts.Add(Entry.TriggerType, 0);
    }
}

void UNarr_DialogueComponent::TriggerDialogue(ENarr_DialogueTrigger TriggerType)
{
    if (!CanTriggerDialogue(TriggerType))
    {
        return;
    }

    // Find matching dialogue entry
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.TriggerType == TriggerType)
        {
            // Update tracking
            float CurrentTime = GetWorld()->GetTimeSeconds();
            LastTriggerTimes[TriggerType] = CurrentTime;
            TriggerCounts[TriggerType]++;

            // Play dialogue lines
            for (const FNarr_DialogueLine& Line : Entry.DialogueLines)
            {
                if (UNarrativeDialogueSubsystem* DialogueSystem = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeDialogueSubsystem>())
                {
                    DialogueSystem->PlayDialogue(Line);
                }
            }
            break;
        }
    }
}

bool UNarr_DialogueComponent::CanTriggerDialogue(ENarr_DialogueTrigger TriggerType)
{
    // Find the dialogue entry
    const FNarr_DialogueEntry* Entry = nullptr;
    for (const FNarr_DialogueEntry& DialogueEntry : DialogueEntries)
    {
        if (DialogueEntry.TriggerType == TriggerType)
        {
            Entry = &DialogueEntry;
            break;
        }
    }

    if (!Entry)
    {
        return false;
    }

    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float* LastTriggerTime = LastTriggerTimes.Find(TriggerType);
    if (LastTriggerTime && (CurrentTime - *LastTriggerTime) < Entry->CooldownTime)
    {
        return false;
    }

    // Check max triggers
    int32* TriggerCount = TriggerCounts.Find(TriggerType);
    if (Entry->MaxTriggers > 0 && TriggerCount && *TriggerCount >= Entry->MaxTriggers)
    {
        return false;
    }

    return true;
}

void UNarrativeDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadDefaultDialogueLibrary();
}

void UNarrativeDialogueSubsystem::PlayDialogue(const FNarr_DialogueLine& DialogueLine)
{
    // Log dialogue for debugging
    UE_LOG(LogTemp, Warning, TEXT("Playing Dialogue: %s (Speaker: %d)"), 
           *DialogueLine.DialogueText, 
           (int32)DialogueLine.Speaker);

    // Display on screen for testing
    if (GEngine)
    {
        FString SpeakerName;
        switch (DialogueLine.Speaker)
        {
            case ENarr_SpeakerType::Narrator: SpeakerName = TEXT("Narrator"); break;
            case ENarr_SpeakerType::TribalScout: SpeakerName = TEXT("Tribal Scout"); break;
            case ENarr_SpeakerType::ElderHunter: SpeakerName = TEXT("Elder Hunter"); break;
            case ENarr_SpeakerType::FireKeeper: SpeakerName = TEXT("Fire Keeper"); break;
            case ENarr_SpeakerType::PlayerThought: SpeakerName = TEXT("Player Thought"); break;
        }

        FString DisplayText = FString::Printf(TEXT("%s: %s"), *SpeakerName, *DialogueLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.Duration, FColor::Yellow, DisplayText);
    }

    // TODO: Integrate with audio system to play DialogueLine.AudioURL
}

void UNarrativeDialogueSubsystem::RegisterDialogueZone(AActor* ZoneActor, ENarr_DialogueTrigger TriggerType)
{
    if (ZoneActor && !RegisteredDialogueZones.Contains(ZoneActor))
    {
        RegisteredDialogueZones.Add(ZoneActor);
        UE_LOG(LogTemp, Log, TEXT("Registered dialogue zone: %s for trigger type %d"), 
               *ZoneActor->GetName(), (int32)TriggerType);
    }
}

void UNarrativeDialogueSubsystem::TriggerContextualDialogue(ENarr_DialogueTrigger TriggerType, const FVector& Location)
{
    if (IsOnCooldown(TriggerType))
    {
        return;
    }

    // Set cooldown
    GlobalCooldowns.Add(TriggerType, GetWorld()->GetTimeSeconds());

    // Find nearby dialogue components and trigger them
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (UNarr_DialogueComponent* DialogueComp = Actor->FindComponentByClass<UNarr_DialogueComponent>())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
            if (Distance <= DialogueComp->TriggerRadius)
            {
                DialogueComp->TriggerDialogue(TriggerType);
            }
        }
    }
}

void UNarrativeDialogueSubsystem::LoadDefaultDialogueLibrary()
{
    // Load default speaker audio URLs from generated TTS
    SpeakerAudioURLs.Add(ENarr_SpeakerType::Narrator, 
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779897729503_AncientNarrator.mp3"));
    
    SpeakerAudioURLs.Add(ENarr_SpeakerType::TribalScout, 
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779897737298_TribalScout.mp3"));
    
    SpeakerAudioURLs.Add(ENarr_SpeakerType::ElderHunter, 
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779897743188_ElderHunter.mp3"));
    
    SpeakerAudioURLs.Add(ENarr_SpeakerType::FireKeeper, 
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779897749215_FireKeeper.mp3"));

    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized with %d speaker audio URLs"), 
           SpeakerAudioURLs.Num());
}

bool UNarrativeDialogueSubsystem::IsOnCooldown(ENarr_DialogueTrigger TriggerType)
{
    float* LastTriggerTime = GlobalCooldowns.Find(TriggerType);
    if (!LastTriggerTime)
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - *LastTriggerTime) < 30.0f; // 30 second global cooldown
}