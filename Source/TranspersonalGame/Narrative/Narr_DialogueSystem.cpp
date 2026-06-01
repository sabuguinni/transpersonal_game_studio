#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnTriggerEnter);

    // Initialize properties
    TriggerRadius = 500.0f;
    bAutoTrigger = true;
    bDialogueActive = false;
    CurrentSequenceIndex = -1;
    CurrentLineIndex = -1;

    // Set default mesh if available
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded() && MeshComponent)
    {
        MeshComponent->SetStaticMesh(CubeMesh.Object);
        MeshComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 2.0f));
    }
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
    
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle dialogue timing and auto-progression
    if (bDialogueActive && CurrentSequenceIndex >= 0 && CurrentLineIndex >= 0)
    {
        if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
        {
            const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
            if (CurrentSequence.DialogueLines.IsValidIndex(CurrentLineIndex))
            {
                const FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
                
                // Auto-advance non-choice dialogue lines
                if (!CurrentLine.bIsPlayerChoice)
                {
                    static float DialogueTimer = 0.0f;
                    DialogueTimer += DeltaTime;
                    
                    if (DialogueTimer >= CurrentLine.Duration)
                    {
                        DialogueTimer = 0.0f;
                        NextDialogueLine();
                    }
                }
            }
        }
    }
}

void ANarr_DialogueSystem::StartDialogue(const FString& SequenceID)
{
    if (bDialogueActive)
    {
        return; // Already in dialogue
    }

    FNarr_DialogueSequence* Sequence = FindDialogueSequence(SequenceID);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Sequence '%s' not found or empty"), *SequenceID);
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

    if (CurrentSequenceIndex >= 0)
    {
        bDialogueActive = true;
        CurrentLineIndex = 0;
        
        OnDialogueStarted(SequenceID);
        OnDialogueLineChanged(Sequence->DialogueLines[0]);
        
        UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Started dialogue sequence '%s'"), *SequenceID);
    }
}

void ANarr_DialogueSystem::NextDialogueLine()
{
    if (!bDialogueActive || CurrentSequenceIndex < 0)
    {
        return;
    }

    if (!DialogueSequences.IsValidIndex(CurrentSequenceIndex))
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
    CurrentLineIndex++;

    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // End of sequence
        EndDialogue();
    }
    else
    {
        // Continue to next line
        const FNarr_DialogueLine& NextLine = CurrentSequence.DialogueLines[CurrentLineIndex];
        OnDialogueLineChanged(NextLine);
        
        UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Advanced to line %d: %s"), 
               CurrentLineIndex, *NextLine.DialogueText);
    }
}

void ANarr_DialogueSystem::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    bDialogueActive = false;
    CurrentSequenceIndex = -1;
    CurrentLineIndex = -1;
    
    OnDialogueEnded();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Dialogue ended"));
}

FNarr_DialogueLine ANarr_DialogueSystem::GetCurrentDialogueLine()
{
    if (bDialogueActive && CurrentSequenceIndex >= 0 && CurrentLineIndex >= 0)
    {
        if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
        {
            const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
            if (CurrentSequence.DialogueLines.IsValidIndex(CurrentLineIndex))
            {
                return CurrentSequence.DialogueLines[CurrentLineIndex];
            }
        }
    }

    return FNarr_DialogueLine(); // Return default empty line
}

bool ANarr_DialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

void ANarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Added dialogue sequence '%s'"), *NewSequence.SequenceID);
}

void ANarr_DialogueSystem::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                        bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoTrigger || bDialogueActive)
    {
        return;
    }

    // Check if the overlapping actor is a character (player)
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && DialogueSequences.Num() > 0)
    {
        // Start the first available dialogue sequence
        StartDialogue(DialogueSequences[0].SequenceID);
    }
}

void ANarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Create default survival-focused dialogue sequences
    
    // Danger Warning Sequence
    FNarr_DialogueSequence DangerSequence;
    DangerSequence.SequenceID = TEXT("DangerWarning");
    DangerSequence.bRepeatable = true;
    DangerSequence.Priority = 10;

    FNarr_DialogueLine DangerLine1;
    DangerLine1.SpeakerName = TEXT("Survivor's Instinct");
    DangerLine1.DialogueText = TEXT("Something is wrong here. The air is too quiet.");
    DangerLine1.Duration = 3.0f;
    DangerSequence.DialogueLines.Add(DangerLine1);

    FNarr_DialogueLine DangerLine2;
    DangerLine2.SpeakerName = TEXT("Survivor's Instinct");
    DangerLine2.DialogueText = TEXT("Predators may be nearby. Stay alert and move carefully.");
    DangerLine2.Duration = 4.0f;
    DangerSequence.DialogueLines.Add(DangerLine2);

    DialogueSequences.Add(DangerSequence);

    // Discovery Sequence
    FNarr_DialogueSequence DiscoverySequence;
    DiscoverySequence.SequenceID = TEXT("Discovery");
    DiscoverySequence.bRepeatable = false;
    DiscoverySequence.Priority = 5;

    FNarr_DialogueLine DiscoveryLine1;
    DiscoveryLine1.SpeakerName = TEXT("Explorer");
    DiscoveryLine1.DialogueText = TEXT("These markings... they're ancient. Someone was here before us.");
    DiscoveryLine1.Duration = 4.0f;
    DiscoverySequence.DialogueLines.Add(DiscoveryLine1);

    FNarr_DialogueLine DiscoveryLine2;
    DiscoveryLine2.SpeakerName = TEXT("Explorer");
    DiscoveryLine2.DialogueText = TEXT("The tools are primitive but effective. We can learn from this.");
    DiscoveryLine2.Duration = 3.5f;
    DiscoverySequence.DialogueLines.Add(DiscoveryLine2);

    DialogueSequences.Add(DiscoverySequence);

    // Resource Found Sequence
    FNarr_DialogueSequence ResourceSequence;
    ResourceSequence.SequenceID = TEXT("ResourceFound");
    ResourceSequence.bRepeatable = true;
    ResourceSequence.Priority = 3;

    FNarr_DialogueLine ResourceLine1;
    ResourceLine1.SpeakerName = TEXT("Gatherer");
    ResourceLine1.DialogueText = TEXT("Fresh water and edible plants. This location is valuable.");
    ResourceLine1.Duration = 3.0f;
    ResourceSequence.DialogueLines.Add(ResourceLine1);

    FNarr_DialogueLine ResourceLine2;
    ResourceLine2.SpeakerName = TEXT("Gatherer");
    ResourceLine2.DialogueText = TEXT("Mark this place. We'll need to return here often.");
    ResourceLine2.Duration = 3.0f;
    ResourceSequence.DialogueLines.Add(ResourceLine2);

    DialogueSequences.Add(ResourceSequence);

    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Initialized %d default dialogue sequences"), DialogueSequences.Num());
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