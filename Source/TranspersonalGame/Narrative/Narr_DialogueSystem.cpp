#include "Narr_DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ANarr_DialogueTrigger Implementation
ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visual mesh
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize properties
    TriggerCondition = ENarr_TriggerCondition::PlayerProximity;
    TriggerRadius = 500.0f;
    bCanRetrigger = false;
    RetriggerCooldown = 30.0f;
    bHasTriggered = false;
    LastTriggerTime = 0.0f;

    // Initialize dialogue data
    DialogueData.DialogueID = TEXT("DefaultTrigger");
    DialogueData.DialogueText = FText::FromString(TEXT("Dialogue trigger activated"));
    DialogueData.DialogueType = ENarr_DialogueType::FieldNarration;
    DialogueData.Duration = 5.0f;
    DialogueData.Priority = 1;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnTriggerBeginOverlap);
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update trigger conditions that require continuous checking
    if (TriggerCondition == ENarr_TriggerCondition::TimeOfDay ||
        TriggerCondition == ENarr_TriggerCondition::PlayerHealth ||
        TriggerCondition == ENarr_TriggerCondition::DinosaurPresence)
    {
        if (CanTrigger() && CheckTriggerConditions())
        {
            TriggerDialogue();
        }
    }
}

void ANarr_DialogueTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                  UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                                  bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a character (player)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (TriggerCondition == ENarr_TriggerCondition::PlayerProximity)
        {
            if (CanTrigger())
            {
                TriggerDialogue();
            }
        }
    }
}

void ANarr_DialogueTrigger::TriggerDialogue()
{
    if (!CanTrigger())
    {
        return;
    }

    // Get dialogue manager
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return;
    }

    UNarr_DialogueManager* DialogueManager = GameInstance->GetSubsystem<UNarr_DialogueManager>();
    if (DialogueManager)
    {
        DialogueManager->PlayDialogue(DialogueData);
    }

    // Update trigger state
    bHasTriggered = true;
    LastTriggerTime = World->GetTimeSeconds();

    // Call Blueprint event
    OnDialogueTriggered(DialogueData);

    UE_LOG(LogTemp, Log, TEXT("Dialogue triggered: %s"), *DialogueData.DialogueID);
}

bool ANarr_DialogueTrigger::CanTrigger() const
{
    if (!bCanRetrigger && bHasTriggered)
    {
        return false;
    }

    if (bCanRetrigger && bHasTriggered)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            float CurrentTime = World->GetTimeSeconds();
            if (CurrentTime - LastTriggerTime < RetriggerCooldown)
            {
                return false;
            }
        }
    }

    return CheckTriggerConditions();
}

bool ANarr_DialogueTrigger::CheckTriggerConditions() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    switch (TriggerCondition)
    {
        case ENarr_TriggerCondition::PlayerProximity:
            // This is handled by overlap events
            return true;

        case ENarr_TriggerCondition::TimeOfDay:
        {
            // Simple time of day check (can be expanded)
            float TimeOfDay = World->GetTimeSeconds();
            float DayLength = 1200.0f; // 20 minutes per day
            float NormalizedTime = FMath::Fmod(TimeOfDay, DayLength) / DayLength;
            
            // Trigger during evening hours (0.7 - 0.9 of day cycle)
            return (NormalizedTime >= 0.7f && NormalizedTime <= 0.9f);
        }

        case ENarr_TriggerCondition::PlayerHealth:
        {
            // Check if player health is low
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (PlayerPawn)
            {
                // This would need to be connected to actual health system
                return true; // Placeholder
            }
            return false;
        }

        case ENarr_TriggerCondition::DinosaurPresence:
        {
            // Check for nearby dinosaurs
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                    Actor->GetName().Contains(TEXT("TRex")) ||
                    Actor->GetName().Contains(TEXT("Raptor")))
                {
                    float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                    if (Distance < 2000.0f) // 20 meter radius
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        case ENarr_TriggerCondition::ResourceDiscovery:
            // This would be triggered by resource discovery events
            return true; // Placeholder

        default:
            return true;
    }
}

// UNarr_DialogueManager Implementation
void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bIsPlayingDialogue = false;
    DialogueStartTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Manager initialized"));
}

void UNarr_DialogueManager::PlayDialogue(const FNarr_DialogueEntry& Dialogue)
{
    if (bIsPlayingDialogue)
    {
        // Queue the dialogue if one is already playing
        QueueDialogue(Dialogue);
        return;
    }

    CurrentDialogue = Dialogue;
    bIsPlayingDialogue = true;
    DialogueStartTime = GetWorld()->GetTimeSeconds();

    // Call Blueprint event
    OnDialogueStarted(Dialogue);

    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), 
           *Dialogue.DialogueID, 
           *Dialogue.DialogueText.ToString());

    // Set timer to finish dialogue
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        StopCurrentDialogue();
    }, Dialogue.Duration, false);
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    if (!bIsPlayingDialogue)
    {
        return;
    }

    FNarr_DialogueEntry FinishedDialogue = CurrentDialogue;
    bIsPlayingDialogue = false;
    DialogueStartTime = 0.0f;

    // Call Blueprint event
    OnDialogueFinished(FinishedDialogue);

    UE_LOG(LogTemp, Log, TEXT("Finished dialogue: %s"), *FinishedDialogue.DialogueID);

    // Process queue
    ProcessDialogueQueue();
}

void UNarr_DialogueManager::QueueDialogue(const FNarr_DialogueEntry& Dialogue)
{
    // Insert based on priority (higher priority first)
    int32 InsertIndex = 0;
    for (int32 i = 0; i < ActiveDialogues.Num(); i++)
    {
        if (ActiveDialogues[i].Priority < Dialogue.Priority)
        {
            InsertIndex = i;
            break;
        }
        InsertIndex = i + 1;
    }

    ActiveDialogues.Insert(Dialogue, InsertIndex);
    
    UE_LOG(LogTemp, Log, TEXT("Queued dialogue: %s (Priority: %d)"), 
           *Dialogue.DialogueID, Dialogue.Priority);
}

void UNarr_DialogueManager::ProcessDialogueQueue()
{
    if (bIsPlayingDialogue || ActiveDialogues.Num() == 0)
    {
        return;
    }

    // Play the highest priority dialogue
    FNarr_DialogueEntry NextDialogue = ActiveDialogues[0];
    ActiveDialogues.RemoveAt(0);
    
    PlayDialogue(NextDialogue);
}

void UNarr_DialogueManager::Tick(float DeltaTime)
{
    if (bIsPlayingDialogue)
    {
        UpdateCurrentDialogue(DeltaTime);
    }
}

void UNarr_DialogueManager::UpdateCurrentDialogue(float DeltaTime)
{
    if (!bIsPlayingDialogue)
    {
        return;
    }

    float ElapsedTime = GetWorld()->GetTimeSeconds() - DialogueStartTime;
    
    // Auto-stop dialogue if duration exceeded
    if (ElapsedTime >= CurrentDialogue.Duration)
    {
        StopCurrentDialogue();
    }
}