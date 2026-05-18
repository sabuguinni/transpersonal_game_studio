#include "Narr_DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize components
    SetupInteractionComponents();

    // Initialize default values
    InteractionRange = 300.0f;
    bIsActive = true;
    CurrentSequenceIndex = -1;
    CurrentLineIndex = -1;
    bIsPlayingDialogue = false;

    // Initialize default context
    CurrentContext.LocationName = TEXT("Ancient Valley");
    CurrentContext.BiomeType = TEXT("Savana");
    CurrentContext.DangerLevel = 0.3f;
    CurrentContext.AvailableResources.Add(TEXT("Fresh Water"));
    CurrentContext.AvailableResources.Add(TEXT("Stone Tools"));
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogue();
    ProcessDialogueTriggers();
}

void ANarr_DialogueSystem::SetupInteractionComponents()
{
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(InteractionRange);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create dialogue marker mesh
    DialogueMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DialogueMarker"));
    DialogueMarker->SetupAttachment(RootComponent);
    DialogueMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnInteractionSphereEndOverlap);
}

void ANarr_DialogueSystem::InitializeDefaultDialogue()
{
    // Create default dialogue sequences for different contexts
    FNarr_DialogueSequence ValleyIntroSequence;
    ValleyIntroSequence.SequenceID = TEXT("valley_introduction");
    ValleyIntroSequence.bRepeatable = false;
    ValleyIntroSequence.TriggerCondition = TEXT("first_visit");

    // Add dialogue lines
    FNarr_DialogueLine IntroLine1;
    IntroLine1.SpeakerName = TEXT("Valley Narrator");
    IntroLine1.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators and the promise of discovery."));
    IntroLine1.Duration = 8.0f;
    ValleyIntroSequence.DialogueLines.Add(IntroLine1);

    FNarr_DialogueLine IntroLine2;
    IntroLine2.SpeakerName = TEXT("Valley Narrator");
    IntroLine2.DialogueText = FText::FromString(TEXT("Your survival depends on understanding the language of this primal world. Every sound, every movement tells a story."));
    IntroLine2.Duration = 6.0f;
    ValleyIntroSequence.DialogueLines.Add(IntroLine2);

    DialogueSequences.Add(ValleyIntroSequence);

    // Create warning sequence for dangerous areas
    FNarr_DialogueSequence DangerWarningSequence;
    DangerWarningSequence.SequenceID = TEXT("danger_warning");
    DangerWarningSequence.bRepeatable = true;
    DangerWarningSequence.TriggerCondition = TEXT("high_danger_area");

    FNarr_DialogueLine WarningLine;
    WarningLine.SpeakerName = TEXT("Survival Guide");
    WarningLine.DialogueText = FText::FromString(TEXT("Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey. Stay low, move quietly."));
    WarningLine.Duration = 7.0f;
    DangerWarningSequence.DialogueLines.Add(WarningLine);

    DialogueSequences.Add(DangerWarningSequence);

    // Create discovery sequence for lore points
    FNarr_DialogueSequence DiscoverySequence;
    DiscoverySequence.SequenceID = TEXT("ancient_discovery");
    DiscoverySequence.bRepeatable = false;
    DiscoverySequence.TriggerCondition = TEXT("lore_interaction");

    FNarr_DialogueLine DiscoveryLine;
    DiscoveryLine.SpeakerName = TEXT("Ancient Chronicler");
    DiscoveryLine.DialogueText = FText::FromString(TEXT("You have discovered ancient markings on this stone. The symbols speak of a time when giants walked the earth and survival meant understanding their ways."));
    DiscoveryLine.Duration = 9.0f;
    DiscoverySequence.DialogueLines.Add(DiscoveryLine);

    DialogueSequences.Add(DiscoverySequence);
}

void ANarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsPlayingDialogue)
    {
        return; // Already playing dialogue
    }

    FNarr_DialogueSequence* Sequence = FindDialogueSequence(SequenceID);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found or empty: %s"), *SequenceID);
        return;
    }

    // Find sequence index
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceID == SequenceID)
        {
            CurrentSequenceIndex = i;
            break;
        }
    }

    CurrentLineIndex = 0;
    bIsPlayingDialogue = true;

    PlayNextDialogueLine();
}

void ANarr_DialogueSystem::PlayNextDialogueLine()
{
    if (!bIsPlayingDialogue || CurrentSequenceIndex < 0 || CurrentSequenceIndex >= DialogueSequences.Num())
    {
        return;
    }

    FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // End of sequence
        StopDialogue();
        return;
    }

    FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    
    // Display dialogue (in a real game, this would trigger UI)
    UE_LOG(LogTemp, Log, TEXT("[%s]: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
    
    // In a real implementation, you would:
    // 1. Display UI with dialogue text
    // 2. Play audio file if available
    // 3. Set up timer for auto-advance or wait for player input
    
    CurrentLineIndex++;
    
    // Auto-advance for now (in real game, this would be player-controlled)
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ANarr_DialogueSystem::PlayNextDialogueLine, CurrentLine.Duration, false);
}

void ANarr_DialogueSystem::StopDialogue()
{
    bIsPlayingDialogue = false;
    CurrentSequenceIndex = -1;
    CurrentLineIndex = -1;
    
    // Clear any active timers
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

bool ANarr_DialogueSystem::CanInteract() const
{
    return bIsActive && !bIsPlayingDialogue;
}

void ANarr_DialogueSystem::UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext)
{
    CurrentContext = NewContext;
    ProcessDialogueTriggers();
}

FNarr_NarrativeContext ANarr_DialogueSystem::GetCurrentContext() const
{
    return CurrentContext;
}

void ANarr_DialogueSystem::AddDiscoveredLore(const FString& LoreEntry)
{
    if (!CurrentContext.DiscoveredLore.Contains(LoreEntry))
    {
        CurrentContext.DiscoveredLore.Add(LoreEntry);
        UE_LOG(LogTemp, Log, TEXT("New lore discovered: %s"), *LoreEntry);
    }
}

void ANarr_DialogueSystem::TriggerEnvironmentalNarrative(const FString& EventType)
{
    if (EventType == TEXT("predator_nearby"))
    {
        StartDialogueSequence(TEXT("danger_warning"));
    }
    else if (EventType == TEXT("ancient_site"))
    {
        StartDialogueSequence(TEXT("ancient_discovery"));
    }
    else if (EventType == TEXT("first_visit"))
    {
        StartDialogueSequence(TEXT("valley_introduction"));
    }
}

TArray<FString> ANarr_DialogueSystem::GetContextualHints() const
{
    TArray<FString> Hints;
    
    if (CurrentContext.DangerLevel > 0.7f)
    {
        Hints.Add(TEXT("This area feels dangerous. Predators may be nearby."));
    }
    
    if (CurrentContext.AvailableResources.Num() > 0)
    {
        Hints.Add(FString::Printf(TEXT("Resources available: %s"), *FString::Join(CurrentContext.AvailableResources, TEXT(", "))));
    }
    
    if (CurrentContext.DiscoveredLore.Num() > 0)
    {
        Hints.Add(TEXT("Ancient knowledge has been uncovered in this location."));
    }
    
    return Hints;
}

void ANarr_DialogueSystem::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || !OtherActor)
    {
        return;
    }

    // Check if it's the player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        UE_LOG(LogTemp, Log, TEXT("Player entered dialogue area: %s"), *GetName());
        
        // Trigger contextual dialogue based on current narrative context
        if (CurrentContext.DangerLevel > 0.5f)
        {
            TriggerEnvironmentalNarrative(TEXT("predator_nearby"));
        }
        else
        {
            TriggerEnvironmentalNarrative(TEXT("first_visit"));
        }
    }
}

void ANarr_DialogueSystem::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        UE_LOG(LogTemp, Log, TEXT("Player left dialogue area: %s"), *GetName());
        // Could stop dialogue or fade it out here
    }
}

FNarr_DialogueSequence* ANarr_DialogueSystem::FindDialogueSequence(const FString& SequenceID)
{
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return &Sequence;
        }
    }
    return nullptr;
}

void ANarr_DialogueSystem::ProcessDialogueTriggers()
{
    // Process any automatic dialogue triggers based on current context
    if (CurrentContext.BiomeType == TEXT("Savana") && CurrentContext.DangerLevel < 0.3f)
    {
        // Safe savana area - could trigger exploration dialogue
    }
    else if (CurrentContext.DangerLevel > 0.7f)
    {
        // High danger area - could trigger warning dialogue
    }
}