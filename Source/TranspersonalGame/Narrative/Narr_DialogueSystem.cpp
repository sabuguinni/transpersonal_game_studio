#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = ENarr_DialogueState::Idle;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    DialogueRange = 500.0f;
    bAutoAdvance = true;
    CurrentLineTimer = 0.0f;
    PlayerActor = nullptr;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UpdatePlayerReference();
    
    // Initialize with default tribal dialogue sequences
    FNarr_DialogueSequence WelcomeSequence;
    WelcomeSequence.SequenceID = TEXT("tribal_welcome");
    WelcomeSequence.bIsQuestRelated = false;
    
    FNarr_DialogueLine WelcomeLine1;
    WelcomeLine1.SpeakerName = TEXT("Tribal Elder");
    WelcomeLine1.DialogueText = TEXT("Welcome to our valley, survivor. The ancient ways still guide us here.");
    WelcomeLine1.Duration = 4.0f;
    
    FNarr_DialogueLine WelcomeLine2;
    WelcomeLine2.SpeakerName = TEXT("Tribal Elder");
    WelcomeLine2.DialogueText = TEXT("Learn our customs, respect the great beasts, and you may find sanctuary among us.");
    WelcomeLine2.Duration = 5.0f;
    
    WelcomeSequence.DialogueLines.Add(WelcomeLine1);
    WelcomeSequence.DialogueLines.Add(WelcomeLine2);
    
    DialogueSequences.Add(WelcomeSequence);
    
    // Hunt briefing sequence
    FNarr_DialogueSequence HuntSequence;
    HuntSequence.SequenceID = TEXT("hunt_briefing");
    HuntSequence.bIsQuestRelated = true;
    HuntSequence.QuestID = TEXT("velociraptor_hunt");
    
    FNarr_DialogueLine HuntLine1;
    HuntLine1.SpeakerName = TEXT("Hunt Master");
    HuntLine1.DialogueText = TEXT("The pack hunters grow bold. They threaten our territory and our young.");
    HuntLine1.Duration = 4.5f;
    
    FNarr_DialogueLine HuntLine2;
    HuntLine2.SpeakerName = TEXT("Hunt Master");
    HuntLine2.DialogueText = TEXT("Prove your worth - eliminate the alpha and scatter the pack. The tribe depends on it.");
    HuntLine2.Duration = 5.5f;
    
    HuntSequence.DialogueLines.Add(HuntLine1);
    HuntSequence.DialogueLines.Add(HuntLine2);
    
    DialogueSequences.Add(HuntSequence);
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue System initialized with %d sequences"), DialogueSequences.Num());
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ENarr_DialogueState::Speaking)
    {
        ProcessCurrentLine(DeltaTime);
    }
    
    // Update player reference if lost
    if (!PlayerActor)
    {
        UpdatePlayerReference();
    }
}

bool UNarr_DialogueSystem::StartDialogue(const FString& SequenceID)
{
    if (CurrentState != ENarr_DialogueState::Idle)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - system is busy"));
        return false;
    }
    
    // Find the sequence
    FNarr_DialogueSequence* FoundSequence = nullptr;
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            FoundSequence = &Sequence;
            break;
        }
    }
    
    if (!FoundSequence || FoundSequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue sequence not found or empty: %s"), *SequenceID);
        return false;
    }
    
    if (!IsPlayerInRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player not in range for dialogue"));
        return false;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Speaking;
    CurrentLineTimer = 0.0f;
    
    OnDialogueStarted(SequenceID);
    OnDialogueLineChanged(FoundSequence->DialogueLines[0]);
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    return true;
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (CurrentState != ENarr_DialogueState::Speaking)
    {
        return;
    }
    
    FNarr_DialogueSequence* CurrentSequence = nullptr;
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            CurrentSequence = &Sequence;
            break;
        }
    }
    
    if (!CurrentSequence)
    {
        StopDialogue();
        return;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        // Dialogue completed
        CurrentState = ENarr_DialogueState::Completed;
        OnDialogueCompleted(CurrentSequenceID);
        
        // Reset to idle after a brief delay
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { CurrentState = ENarr_DialogueState::Idle; },
            1.0f,
            false
        );
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue sequence completed: %s"), *CurrentSequenceID);
    }
    else
    {
        // Continue to next line
        CurrentLineTimer = 0.0f;
        OnDialogueLineChanged(CurrentSequence->DialogueLines[CurrentLineIndex]);
        UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue line %d"), CurrentLineIndex);
    }
}

void UNarr_DialogueSystem::StopDialogue()
{
    CurrentState = ENarr_DialogueState::Idle;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

bool UNarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Check for duplicate sequence IDs
    for (const FNarr_DialogueSequence& ExistingSequence : DialogueSequences)
    {
        if (ExistingSequence.SequenceID == NewSequence.SequenceID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence ID already exists: %s"), *NewSequence.SequenceID);
            return false;
        }
    }
    
    DialogueSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue sequence: %s"), *NewSequence.SequenceID);
    return true;
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (CurrentState == ENarr_DialogueState::Idle)
    {
        return FNarr_DialogueLine();
    }
    
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            if (CurrentLineIndex < Sequence.DialogueLines.Num())
            {
                return Sequence.DialogueLines[CurrentLineIndex];
            }
            break;
        }
    }
    
    return FNarr_DialogueLine();
}

bool UNarr_DialogueSystem::IsPlayerInRange() const
{
    if (!PlayerActor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= DialogueRange;
}

void UNarr_DialogueSystem::UpdatePlayerReference()
{
    if (UWorld* World = GetWorld())
    {
        PlayerActor = UGameplayStatics::GetPlayerPawn(World, 0);
    }
}

void UNarr_DialogueSystem::ProcessCurrentLine(float DeltaTime)
{
    CurrentLineTimer += DeltaTime;
    
    if (bAutoAdvance)
    {
        FNarr_DialogueLine CurrentLine = GetCurrentDialogueLine();
        if (CurrentLineTimer >= CurrentLine.Duration)
        {
            AdvanceDialogue();
        }
    }
}