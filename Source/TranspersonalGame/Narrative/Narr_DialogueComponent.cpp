#include "Narr_DialogueComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    NPCName = TEXT("Unknown");
    InteractionRange = 300.0f;
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    PlayerPawn = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player reference
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Initialize default dialogue if empty
    if (DialogueLines.Num() == 0)
    {
        FNarr_DialogueLine DefaultLine;
        DefaultLine.SpeakerName = NPCName;
        DefaultLine.DialogueText = TEXT("The winds carry strange scents today. Danger lurks beyond the ridge.");
        DefaultLine.Duration = 4.0f;
        DefaultLine.bIsPlayerChoice = false;
        DialogueLines.Add(DefaultLine);
    }
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ENarr_DialogueState::Speaking)
    {
        UpdateDialogueTimer(DeltaTime);
    }
}

void UNarr_DialogueComponent::StartDialogue()
{
    if (!IsPlayerInRange() || DialogueLines.Num() == 0)
    {
        return;
    }
    
    CurrentState = ENarr_DialogueState::Speaking;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    
    DisplayCurrentLine();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue started with %s"), *NPCName);
}

void UNarr_DialogueComponent::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended with %s"), *NPCName);
}

void UNarr_DialogueComponent::NextDialogueLine()
{
    if (CurrentState != ENarr_DialogueState::Speaking)
    {
        return;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    CurrentLineTimer = 0.0f;
    DisplayCurrentLine();
}

bool UNarr_DialogueComponent::IsDialogueActive() const
{
    return CurrentState != ENarr_DialogueState::Idle;
}

void UNarr_DialogueComponent::UpdateDialogueTimer(float DeltaTime)
{
    if (CurrentLineIndex >= DialogueLines.Num())
    {
        return;
    }
    
    CurrentLineTimer += DeltaTime;
    
    const FNarr_DialogueLine& CurrentLine = DialogueLines[CurrentLineIndex];
    
    if (CurrentLineTimer >= CurrentLine.Duration)
    {
        if (CurrentLine.bIsPlayerChoice)
        {
            CurrentState = ENarr_DialogueState::WaitingForResponse;
        }
        else
        {
            NextDialogueLine();
        }
    }
}

bool UNarr_DialogueComponent::IsPlayerInRange() const
{
    if (!PlayerPawn || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= InteractionRange;
}

void UNarr_DialogueComponent::DisplayCurrentLine()
{
    if (CurrentLineIndex >= DialogueLines.Num())
    {
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = DialogueLines[CurrentLineIndex];
    
    // Display dialogue on screen
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, FColor::Yellow, DisplayText);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue: %s - %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
}