#include "Narr_DialogueTrigger.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize dialogue properties
    DialogueID = TEXT("DefaultDialogue");
    DialogueType = ENarr_DialogueType::Monologue;
    CharacterName = TEXT("Unknown");
    bIsRepeatable = false;
    bRequiresPlayerInput = true;
    TriggerRadius = 300.0f;

    // Initialize state
    bHasBeenTriggered = false;
    bPlayerInRange = false;
    TimesTriggered = 0;

    // Set up collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnTriggerEnter);
    GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnTriggerExit);

    // Set default size
    FVector BoxExtent(TriggerRadius, TriggerRadius, 200.0f);
    GetCollisionComponent()->SetBoxExtent(BoxExtent);

    InitializeDialogue();
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure dialogue is properly initialized
    if (DialogueLines.Num() == 0)
    {
        InitializeDialogue();
    }
}

void ANarr_DialogueTrigger::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    bPlayerInRange = true;
    
    if (CanTrigger())
    {
        TriggerDialogue();
    }
}

void ANarr_DialogueTrigger::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    bPlayerInRange = false;
    HideDialogueUI();
}

void ANarr_DialogueTrigger::TriggerDialogue()
{
    if (!CanTrigger())
    {
        return;
    }

    bHasBeenTriggered = true;
    TimesTriggered++;

    // Play dialogue audio if available
    PlayDialogueAudio();

    // Show dialogue UI
    ShowDialogueUI();

    // Log for debugging
    UE_LOG(LogTemp, Warning, TEXT("Dialogue triggered: %s - %s"), *DialogueID, *CharacterName);
    
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *CharacterName, 
            DialogueLines.Num() > 0 ? *DialogueLines[0] : TEXT("No dialogue available"));
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }
}

void ANarr_DialogueTrigger::ResetTrigger()
{
    bHasBeenTriggered = false;
    TimesTriggered = 0;
    HideDialogueUI();
}

bool ANarr_DialogueTrigger::CanTrigger() const
{
    if (!bPlayerInRange)
    {
        return false;
    }

    if (bHasBeenTriggered && !bIsRepeatable)
    {
        return false;
    }

    return DialogueLines.Num() > 0;
}

void ANarr_DialogueTrigger::SetDialogueLines(const TArray<FString>& NewLines)
{
    DialogueLines = NewLines;
}

void ANarr_DialogueTrigger::AddDialogueLine(const FString& NewLine)
{
    DialogueLines.Add(NewLine);
}

void ANarr_DialogueTrigger::TestDialogue()
{
    if (GEngine)
    {
        FString TestMessage = FString::Printf(TEXT("Testing dialogue: %s with %d lines"), 
            *DialogueID, DialogueLines.Num());
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TestMessage);
    }
}

void ANarr_DialogueTrigger::InitializeDialogue()
{
    // Set up default dialogue based on actor label
    FString ActorLabel = GetActorLabel();
    
    if (ActorLabel.Contains(TEXT("Campfire")))
    {
        DialogueID = TEXT("CampfireStory");
        CharacterName = TEXT("Elder");
        DialogueType = ENarr_DialogueType::Monologue;
        DialogueLines.Add(TEXT("The fire speaks of ancient times, when the great beasts ruled all lands."));
        DialogueLines.Add(TEXT("Listen to the flames, young one. They remember what we have forgotten."));
    }
    else if (ActorLabel.Contains(TEXT("Cave")))
    {
        DialogueID = TEXT("HunterWisdom");
        CharacterName = TEXT("Hunter");
        DialogueType = ENarr_DialogueType::Conversation;
        DialogueLines.Add(TEXT("These caves have sheltered our people for generations."));
        DialogueLines.Add(TEXT("The walls still bear the marks of claws... and courage."));
    }
    else if (ActorLabel.Contains(TEXT("River")))
    {
        DialogueID = TEXT("WaterWarning");
        CharacterName = TEXT("Scout");
        DialogueType = ENarr_DialogueType::Warning;
        DialogueLines.Add(TEXT("The water runs red with danger. Something stirs upstream."));
        DialogueLines.Add(TEXT("We must find another path, or risk becoming prey."));
    }
    else
    {
        DialogueID = TEXT("GenericEncounter");
        CharacterName = TEXT("Survivor");
        DialogueType = ENarr_DialogueType::Monologue;
        DialogueLines.Add(TEXT("In this harsh world, every sunrise is a victory."));
        DialogueLines.Add(TEXT("Stay alert. The wilderness shows no mercy to the unprepared."));
    }
}

void ANarr_DialogueTrigger::PlayDialogueAudio()
{
    // Audio playback will be handled by Audio Agent's system
    // This is a placeholder for integration
}

void ANarr_DialogueTrigger::ShowDialogueUI()
{
    // UI display will be handled by UI system
    // This is a placeholder for integration
}

void ANarr_DialogueTrigger::HideDialogueUI()
{
    // UI hiding will be handled by UI system
    // This is a placeholder for integration
}