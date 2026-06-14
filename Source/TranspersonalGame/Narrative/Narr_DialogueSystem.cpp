#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentDialogueIndex = 0;
    bIsDialogueActive = false;
    DialogueRange = 500.0f;

    // Add default survival dialogue entries
    FNarr_DialogueEntry WelcomeEntry;
    WelcomeEntry.SpeakerName = TEXT("Tribal Elder");
    WelcomeEntry.DialogueText = FText::FromString(TEXT("Welcome to the ancient hunting grounds. Survival depends on your wits and courage."));
    WelcomeEntry.DialogueType = ENarr_DialogueType::Information;
    WelcomeEntry.DisplayDuration = 4.0f;
    DialogueEntries.Add(WelcomeEntry);

    FNarr_DialogueEntry WarningEntry;
    WarningEntry.SpeakerName = TEXT("Scout");
    WarningEntry.DialogueText = FText::FromString(TEXT("Beware the great predators! They hunt in packs and show no mercy."));
    WarningEntry.DialogueType = ENarr_DialogueType::Warning;
    WarningEntry.DisplayDuration = 3.5f;
    DialogueEntries.Add(WarningEntry);

    FNarr_DialogueEntry SurvivalEntry;
    SurvivalEntry.SpeakerName = TEXT("Craftsman");
    SurvivalEntry.DialogueText = FText::FromString(TEXT("Stone and wood are your allies. Craft tools to increase your chances of survival."));
    SurvivalEntry.DialogueType = ENarr_DialogueType::Survival;
    SurvivalEntry.DisplayDuration = 4.5f;
    DialogueEntries.Add(SurvivalEntry);
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDialogueActive && IsPlayerInRange())
    {
        // Auto-advance dialogue after display duration
        static float DialogueTimer = 0.0f;
        DialogueTimer += DeltaTime;
        
        if (DialogueTimer >= GetCurrentDialogue().DisplayDuration)
        {
            NextDialogue();
            DialogueTimer = 0.0f;
        }
    }
}

void UNarr_DialogueSystem::StartDialogue()
{
    if (DialogueEntries.Num() > 0 && IsPlayerInRange())
    {
        CurrentDialogueIndex = 0;
        bIsDialogueActive = true;
        
        FNarr_DialogueEntry CurrentEntry = GetCurrentDialogue();
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
            *CurrentEntry.SpeakerName, 
            *CurrentEntry.DialogueText.ToString());
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, CurrentEntry.DisplayDuration, FColor::Yellow, DisplayText);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue Started: %s"), *DisplayText);
    }
}

void UNarr_DialogueSystem::NextDialogue()
{
    if (bIsDialogueActive && DialogueEntries.Num() > 0)
    {
        CurrentDialogueIndex++;
        
        if (CurrentDialogueIndex >= DialogueEntries.Num())
        {
            EndDialogue();
            return;
        }
        
        FNarr_DialogueEntry CurrentEntry = GetCurrentDialogue();
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
            *CurrentEntry.SpeakerName, 
            *CurrentEntry.DialogueText.ToString());
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, CurrentEntry.DisplayDuration, FColor::Yellow, DisplayText);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Next Dialogue: %s"), *DisplayText);
    }
}

void UNarr_DialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentDialogueIndex = 0;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Dialogue Ended"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue System: Conversation ended"));
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetCurrentDialogue() const
{
    if (DialogueEntries.IsValidIndex(CurrentDialogueIndex))
    {
        return DialogueEntries[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueEntry();
}

void UNarr_DialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueEntries.Add(NewEntry);
    UE_LOG(LogTemp, Log, TEXT("Dialogue System: Added new entry from %s"), *NewEntry.SpeakerName);
}

bool UNarr_DialogueSystem::IsPlayerInRange() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= DialogueRange;
}