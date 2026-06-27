#include "DialogueSystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bSequenceActive = false;
    ActiveSequenceIndex = -1;
    CurrentLineIndex = 0;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueComponent::TriggerSequence(FName SequenceID)
{
    for (int32 i = 0; i < DialogueSequences.Num(); ++i)
    {
        FNarr_DialogueSequence& Seq = DialogueSequences[i];
        if (Seq.SequenceID == SequenceID)
        {
            if (Seq.bTriggeredOnce)
            {
                UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Sequence %s already triggered once — skipping."), *SequenceID.ToString());
                return;
            }
            ActiveSequenceID = SequenceID;
            ActiveSequenceIndex = i;
            CurrentLineIndex = 0;
            bSequenceActive = true;
            UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Sequence %s started (%d lines)."), *SequenceID.ToString(), Seq.Lines.Num());
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Sequence %s not found."), *SequenceID.ToString());
}

void UNarr_DialogueComponent::AdvanceLine()
{
    if (!bSequenceActive || ActiveSequenceIndex < 0)
    {
        return;
    }

    FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];
    CurrentLineIndex++;

    if (CurrentLineIndex >= Seq.Lines.Num())
    {
        EndSequence();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Line %d/%d — %s: %s"),
            CurrentLineIndex + 1,
            Seq.Lines.Num(),
            *Seq.Lines[CurrentLineIndex].SpeakerName,
            *Seq.Lines[CurrentLineIndex].LineText);
    }
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (!bSequenceActive || ActiveSequenceIndex < 0)
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];
    if (Seq.Lines.IsValidIndex(CurrentLineIndex))
    {
        return Seq.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool UNarr_DialogueComponent::IsSequenceActive() const
{
    return bSequenceActive;
}

void UNarr_DialogueComponent::EndSequence()
{
    if (ActiveSequenceIndex >= 0 && ActiveSequenceIndex < DialogueSequences.Num())
    {
        DialogueSequences[ActiveSequenceIndex].bTriggeredOnce = true;
    }
    bSequenceActive = false;
    ActiveSequenceIndex = -1;
    CurrentLineIndex = 0;
    ActiveSequenceID = NAME_None;
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Sequence ended."));
}

// ============================================================
// ANarr_DialogueTrigger
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    TriggerRadius = 400.0f;
    bTriggerOnce = true;
    bHasTriggered = false;
    CachedPlayer = nullptr;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bHasTriggered && bTriggerOnce)
    {
        return;
    }

    if (!CachedPlayer)
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        return;
    }

    float Dist = FVector::Dist(GetActorLocation(), CachedPlayer->GetActorLocation());
    if (Dist <= TriggerRadius)
    {
        OnPlayerEnterRange();
    }
}

void ANarr_DialogueTrigger::OnPlayerEnterRange()
{
    if (bHasTriggered && bTriggerOnce)
    {
        return;
    }

    bHasTriggered = true;
    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: Player entered range — firing sequence %s"), *DialogueSequenceID.ToString());

    // Broadcast to any UNarr_DialogueComponent in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        UNarr_DialogueComponent* DC = Actor->FindComponentByClass<UNarr_DialogueComponent>();
        if (DC)
        {
            DC->TriggerSequence(DialogueSequenceID);
            break;
        }
    }
}
