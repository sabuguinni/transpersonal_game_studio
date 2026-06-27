// DialogueManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// CYCLE: PROD_CYCLE_AUTO_20260627_012

#include "DialogueManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ADialogueManager::ADialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AssignedSpeaker = ENarr_DialogueSpeaker::Unknown;
    TriggerRadius = 300.0f;
    bHasTriggered = false;
    bDialogueActive = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;

    // Default dialogue lines per speaker type
    FNarr_DialogueLine DefaultLine;
    DefaultLine.LineText = TEXT("Stay alert. This land does not forgive mistakes.");
    DefaultLine.Speaker = ENarr_DialogueSpeaker::Unknown;
    DefaultLine.DisplayDuration = 5.0f;
    DialogueLines.Add(DefaultLine);
}

void ADialogueManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap event
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ADialogueManager::OnPlayerEnterTrigger);

    // Set sphere radius from property
    TriggerSphere->SetSphereRadius(TriggerRadius);

    UE_LOG(LogTemp, Log, TEXT("DialogueManager BeginPlay — Speaker: %d, Lines: %d"),
        (int32)AssignedSpeaker, DialogueLines.Num());
}

void ADialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDialogueActive) return;

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        AdvanceDialogue();
    }
}

void ADialogueManager::OnPlayerEnterTrigger(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Only trigger once per session unless reset
    if (bHasTriggered) return;

    if (DialogueLines.Num() == 0) return;

    bHasTriggered = true;
    bDialogueActive = true;
    CurrentLineIndex = 0;

    // Start first line timer
    LineTimer = DialogueLines[0].DisplayDuration;

    // Play audio if available
    if (DialogueLines[0].AudioCue.IsValid())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DialogueLines[0].AudioCue.Get());
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager triggered — Speaker: %d, Line: %s"),
        (int32)AssignedSpeaker, *DialogueLines[0].LineText);
}

void ADialogueManager::AdvanceDialogue()
{
    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueLines.Num())
    {
        // End of dialogue
        bDialogueActive = false;
        CurrentLineIndex = DialogueLines.Num() - 1;
        UE_LOG(LogTemp, Log, TEXT("DialogueManager — dialogue complete for Speaker: %d"), (int32)AssignedSpeaker);
        return;
    }

    // Start next line
    LineTimer = DialogueLines[CurrentLineIndex].DisplayDuration;

    // Play audio if available
    if (DialogueLines[CurrentLineIndex].AudioCue.IsValid())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DialogueLines[CurrentLineIndex].AudioCue.Get());
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager — advancing to line %d: %s"),
        CurrentLineIndex, *DialogueLines[CurrentLineIndex].LineText);
}

void ADialogueManager::ResetDialogue()
{
    bHasTriggered = false;
    bDialogueActive = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

FNarr_DialogueLine ADialogueManager::GetCurrentLine() const
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueLines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool ADialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}
