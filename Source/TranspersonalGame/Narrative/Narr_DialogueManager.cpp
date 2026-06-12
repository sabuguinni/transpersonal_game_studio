#include "Narr_DialogueManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visual marker
    VisualMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMarker"));
    VisualMarker->SetupAttachment(RootComponent);
    VisualMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize properties
    InteractionRadius = 300.0f;
    bIsActive = true;
    bPlayerInRange = false;
    CurrentPlayer = nullptr;
    NPCPersonality = ENarr_NPCPersonality::Wise;

    // Initialize dialogue tree
    DialogueTree.TreeName = TEXT("DefaultDialogue");
    DialogueTree.CurrentDialogueID = 0;

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueManager::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueManager::OnInteractionSphereEndOverlap);
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateVisualMarker();
    
    // Create default dialogue if none exists
    if (DialogueTree.DialogueEntries.Num() == 0)
    {
        FNarr_DialogueEntry DefaultEntry;
        DefaultEntry.SpeakerName = TEXT("Elder");
        DefaultEntry.DialogueText = FText::FromString(TEXT("Greetings, survivor. The path ahead is treacherous."));
        DefaultEntry.RequiredQuestStage = ENarr_QuestStage::Tutorial;
        DefaultEntry.bIsQuestDialogue = false;
        DefaultEntry.EmotionalIntensity = 0.6f;
        
        DialogueTree.DialogueEntries.Add(DefaultEntry);
    }
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bPlayerInRange && CurrentPlayer)
    {
        // Update visual marker based on player proximity
        float Distance = FVector::Dist(GetActorLocation(), CurrentPlayer->GetActorLocation());
        if (Distance <= InteractionRadius * 0.5f)
        {
            // Player very close - brighten marker
            UpdateVisualMarker();
        }
    }
}

void ANarr_DialogueManager::StartDialogue(AActor* PlayerActor)
{
    if (!bIsActive || !PlayerActor || !CanStartDialogue(PlayerActor))
    {
        return;
    }

    CurrentPlayer = PlayerActor;
    DialogueTree.CurrentDialogueID = 0;
    
    // Log dialogue start
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("Starting dialogue with %s"), *DialogueTree.TreeName);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, LogMessage);
    }

    OnDialogueStarted();
}

void ANarr_DialogueManager::EndDialogue()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Dialogue ended"));
    }

    CurrentPlayer = nullptr;
    DialogueTree.CurrentDialogueID = 0;
    
    OnDialogueEnded();
}

FNarr_DialogueEntry ANarr_DialogueManager::GetCurrentDialogue()
{
    if (DialogueTree.DialogueEntries.IsValidIndex(DialogueTree.CurrentDialogueID))
    {
        return DialogueTree.DialogueEntries[DialogueTree.CurrentDialogueID];
    }
    
    // Return empty dialogue if invalid
    FNarr_DialogueEntry EmptyEntry;
    EmptyEntry.DialogueText = FText::FromString(TEXT("..."));
    return EmptyEntry;
}

void ANarr_DialogueManager::SelectResponse(int32 ResponseIndex)
{
    FNarr_DialogueEntry CurrentEntry = GetCurrentDialogue();
    
    if (CurrentEntry.NextDialogueIDs.IsValidIndex(ResponseIndex))
    {
        int32 NextID = CurrentEntry.NextDialogueIDs[ResponseIndex];
        if (DialogueTree.DialogueEntries.IsValidIndex(NextID))
        {
            DialogueTree.CurrentDialogueID = NextID;
            
            if (GEngine)
            {
                FString LogMessage = FString::Printf(TEXT("Selected response %d, moving to dialogue %d"), ResponseIndex, NextID);
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, LogMessage);
            }
            
            OnResponseSelected(ResponseIndex);
        }
        else
        {
            // End dialogue if no valid next ID
            EndDialogue();
        }
    }
    else
    {
        // Invalid response - end dialogue
        EndDialogue();
    }
}

bool ANarr_DialogueManager::CanStartDialogue(AActor* PlayerActor)
{
    if (!PlayerActor || !bIsActive)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetActorLocation(), PlayerActor->GetActorLocation());
    if (Distance > InteractionRadius)
    {
        return false;
    }

    // Check quest requirements for current dialogue
    FNarr_DialogueEntry CurrentEntry = GetCurrentDialogue();
    return CheckQuestRequirements(CurrentEntry);
}

void ANarr_DialogueManager::SetDialogueTree(const FNarr_DialogueTree& NewTree)
{
    DialogueTree = NewTree;
    DialogueTree.CurrentDialogueID = 0;
    
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("Dialogue tree set: %s with %d entries"), 
                                           *NewTree.TreeName, NewTree.DialogueEntries.Num());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, LogMessage);
    }
}

void ANarr_DialogueManager::AddVoiceLine(const FString& VoiceURL)
{
    VoiceLineURLs.Add(VoiceURL);
    
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("Added voice line: %s"), *VoiceURL);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, LogMessage);
    }
}

void ANarr_DialogueManager::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInRange = true;
        CurrentPlayer = OtherActor;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Player entered dialogue range"));
        }
        
        UpdateVisualMarker();
    }
}

void ANarr_DialogueManager::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == CurrentPlayer)
    {
        bPlayerInRange = false;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Player left dialogue range"));
        }
        
        // End dialogue if active
        if (CurrentPlayer)
        {
            EndDialogue();
        }
        
        CurrentPlayer = nullptr;
        UpdateVisualMarker();
    }
}

void ANarr_DialogueManager::UpdateVisualMarker()
{
    if (VisualMarker)
    {
        // Scale marker based on interaction state
        float Scale = bPlayerInRange ? 1.5f : 1.0f;
        VisualMarker->SetWorldScale3D(FVector(Scale, Scale, Scale));
        
        // Change color based on availability
        if (bIsActive && CanStartDialogue(CurrentPlayer))
        {
            // Green for available dialogue
            VisualMarker->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(0.0f, 1.0f, 0.0f));
        }
        else if (bPlayerInRange)
        {
            // Yellow for player in range but dialogue unavailable
            VisualMarker->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 1.0f, 0.0f));
        }
        else
        {
            // Blue for default state
            VisualMarker->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(0.0f, 0.0f, 1.0f));
        }
    }
}

bool ANarr_DialogueManager::CheckQuestRequirements(const FNarr_DialogueEntry& Entry)
{
    // For now, always return true - quest integration will be handled by Quest system
    // TODO: Integrate with Quest_ProgressionManager to check actual quest stage
    return true;
}