#include "Narr_DialogueComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default settings
    TriggerDistance = 500.0f;
    bRequireLineOfSight = true;
    CurrentLineIndex = 0;
    bIsPlayingDialogue = false;
    DialogueTimer = 0.0f;
    CachedPlayerActor = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player actor reference
    CachedPlayerActor = FindPlayerActor();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent initialized on %s"), *GetOwner()->GetName());
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsPlayingDialogue)
    {
        UpdateDialogueTimer(DeltaTime);
    }
    else
    {
        CheckPlayerProximity();
    }
}

bool UNarr_DialogueComponent::StartDialogueSequence(const FString& SequenceID)
{
    // Find the sequence by ID
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            CurrentSequence = Sequence;
            CurrentLineIndex = 0;
            bIsPlayingDialogue = true;
            DialogueTimer = 0.0f;
            
            // Broadcast start event
            OnDialogueStarted.Broadcast(SequenceID);
            
            // Start first line
            if (CurrentSequence.DialogueLines.Num() > 0)
            {
                OnDialogueLineChanged.Broadcast(CurrentSequence.DialogueLines[0]);
                UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
                return true;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    return false;
}

void UNarr_DialogueComponent::StopDialogue()
{
    bIsPlayingDialogue = false;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    
    OnDialogueEnded.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

void UNarr_DialogueComponent::NextDialogueLine()
{
    if (!bIsPlayingDialogue || CurrentSequence.DialogueLines.Num() == 0)
    {
        return;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // End of sequence
        StopDialogue();
    }
    else
    {
        // Move to next line
        DialogueTimer = 0.0f;
        OnDialogueLineChanged.Broadcast(CurrentSequence.DialogueLines[CurrentLineIndex]);
    }
}

bool UNarr_DialogueComponent::CanTriggerDialogue(AActor* PlayerActor)
{
    if (!PlayerActor || DialogueSequences.Num() == 0)
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    if (Distance > TriggerDistance)
    {
        return false;
    }
    
    // Check line of sight if required
    if (bRequireLineOfSight)
    {
        FHitResult HitResult;
        FVector Start = GetOwner()->GetActorLocation();
        FVector End = PlayerActor->GetActorLocation();
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Visibility
        );
        
        if (bHit && HitResult.GetActor() != PlayerActor)
        {
            return false; // Something is blocking line of sight
        }
    }
    
    return true;
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentDialogueLine()
{
    if (bIsPlayingDialogue && 
        CurrentLineIndex >= 0 && 
        CurrentLineIndex < CurrentSequence.DialogueLines.Num())
    {
        return CurrentSequence.DialogueLines[CurrentLineIndex];
    }
    
    return FNarr_DialogueLine(); // Return default empty line
}

void UNarr_DialogueComponent::UpdateDialogueTimer(float DeltaTime)
{
    DialogueTimer += DeltaTime;
    
    if (CurrentLineIndex >= 0 && CurrentLineIndex < CurrentSequence.DialogueLines.Num())
    {
        const FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
        
        if (DialogueTimer >= CurrentLine.Duration)
        {
            NextDialogueLine();
        }
    }
}

void UNarr_DialogueComponent::CheckPlayerProximity()
{
    if (!CachedPlayerActor)
    {
        CachedPlayerActor = FindPlayerActor();
        return;
    }
    
    if (CanTriggerDialogue(CachedPlayerActor) && !bIsPlayingDialogue)
    {
        // Find first available sequence based on trigger condition
        for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
        {
            if (Sequence.TriggerCondition == ENarr_TriggerCondition::PlayerProximity)
            {
                StartDialogueSequence(Sequence.SequenceID);
                break;
            }
        }
    }
}

AActor* UNarr_DialogueComponent::FindPlayerActor()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}