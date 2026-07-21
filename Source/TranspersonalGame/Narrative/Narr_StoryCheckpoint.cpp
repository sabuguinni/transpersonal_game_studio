#include "Narr_StoryCheckpoint.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ANarr_StoryCheckpoint::ANarr_StoryCheckpoint()
{
    PrimaryActorTick.bCanEverTick = false;

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
    CheckpointID = TEXT("DefaultCheckpoint");
    StoryPhase = ENarr_StoryPhase::Beginning;
    ChapterTitle = TEXT("Unknown Chapter");
    ChapterDescription = TEXT("A moment in the journey of survival.");
    bIsMainStoryCheckpoint = true;
    bTriggerOnce = true;
    TriggerRadius = 500.0f;

    // Initialize state
    bHasBeenActivated = false;
    ActivationCount = 0;

    // Bind overlap event
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_StoryCheckpoint::OnCheckpointEnter);

    InitializeCheckpoint();
}

void ANarr_StoryCheckpoint::BeginPlay()
{
    Super::BeginPlay();
    
    // Update trigger radius
    TriggerSphere->SetSphereRadius(TriggerRadius);
    
    // Initialize checkpoint based on label if not already set
    if (CheckpointID == TEXT("DefaultCheckpoint"))
    {
        InitializeCheckpoint();
    }
}

void ANarr_StoryCheckpoint::OnCheckpointEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    if (CanActivate())
    {
        ActivateCheckpoint();
    }
}

void ANarr_StoryCheckpoint::ActivateCheckpoint()
{
    if (!CanActivate())
    {
        return;
    }

    bHasBeenActivated = true;
    ActivationCount++;
    ActivationTime = FDateTime::Now();

    // Broadcast story progress
    BroadcastStoryProgress();

    // Update game state
    UpdateGameState();

    // Play effects
    PlayCheckpointEffects();

    // Log activation
    UE_LOG(LogTemp, Warning, TEXT("Story checkpoint activated: %s - %s"), *CheckpointID, *ChapterTitle);
    
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("Chapter Unlocked: %s"), *ChapterTitle);
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Gold, DisplayText);
    }
}

void ANarr_StoryCheckpoint::ResetCheckpoint()
{
    bHasBeenActivated = false;
    ActivationCount = 0;
    CompletedObjectives.Empty();
}

bool ANarr_StoryCheckpoint::CanActivate() const
{
    if (bHasBeenActivated && bTriggerOnce)
    {
        return false;
    }

    return true;
}

void ANarr_StoryCheckpoint::SetStoryPhase(ENarr_StoryPhase NewPhase)
{
    StoryPhase = NewPhase;
}

void ANarr_StoryCheckpoint::AddObjective(const FString& ObjectiveID)
{
    if (!UnlockedObjectives.Contains(ObjectiveID))
    {
        UnlockedObjectives.Add(ObjectiveID);
    }
}

void ANarr_StoryCheckpoint::CompleteObjective(const FString& ObjectiveID)
{
    if (!CompletedObjectives.Contains(ObjectiveID))
    {
        CompletedObjectives.Add(ObjectiveID);
    }
}

bool ANarr_StoryCheckpoint::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    return CompletedObjectives.Contains(ObjectiveID);
}

void ANarr_StoryCheckpoint::TestCheckpoint()
{
    if (GEngine)
    {
        FString TestMessage = FString::Printf(TEXT("Testing checkpoint: %s - Phase: %d"), 
            *CheckpointID, (int32)StoryPhase);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TestMessage);
    }
}

void ANarr_StoryCheckpoint::ShowCheckpointInfo()
{
    if (GEngine)
    {
        FString InfoMessage = FString::Printf(TEXT("Checkpoint: %s | Activated: %s | Count: %d"), 
            *ChapterTitle, bHasBeenActivated ? TEXT("Yes") : TEXT("No"), ActivationCount);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, InfoMessage);
    }
}

void ANarr_StoryCheckpoint::InitializeCheckpoint()
{
    FString ActorLabel = GetActorLabel();
    
    if (ActorLabel.Contains(TEXT("Beginning")))
    {
        CheckpointID = TEXT("StoryBeginning");
        StoryPhase = ENarr_StoryPhase::Beginning;
        ChapterTitle = TEXT("First Light");
        ChapterDescription = TEXT("You awaken in a world where survival is the only law. The journey begins.");
        UnlockedObjectives.Add(TEXT("FindShelter"));
        UnlockedObjectives.Add(TEXT("GatherBasicResources"));
    }
    else if (ActorLabel.Contains(TEXT("FirstHunt")))
    {
        CheckpointID = TEXT("FirstHunt");
        StoryPhase = ENarr_StoryPhase::Rising;
        ChapterTitle = TEXT("The Hunt Begins");
        ChapterDescription = TEXT("You have learned to survive. Now you must learn to hunt or become the hunted.");
        UnlockedObjectives.Add(TEXT("CraftWeapon"));
        UnlockedObjectives.Add(TEXT("TrackPrey"));
        UnlockedObjectives.Add(TEXT("FirstKill"));
    }
    else if (ActorLabel.Contains(TEXT("Tribal")))
    {
        CheckpointID = TEXT("TribalEncounter");
        StoryPhase = ENarr_StoryPhase::Climax;
        ChapterTitle = TEXT("The Others");
        ChapterDescription = TEXT("You are not alone. Other survivors have formed tribes. Will you join them or stand apart?");
        UnlockedObjectives.Add(TEXT("MeetTribalLeader"));
        UnlockedObjectives.Add(TEXT("ProveWorthiness"));
        UnlockedObjectives.Add(TEXT("ChooseAlliance"));
    }
    else if (ActorLabel.Contains(TEXT("Alpha")))
    {
        CheckpointID = TEXT("AlphaConfrontation");
        StoryPhase = ENarr_StoryPhase::Resolution;
        ChapterTitle = TEXT("Alpha's Challenge");
        ChapterDescription = TEXT("The apex predator has marked you as a threat. This confrontation will define your legacy.");
        UnlockedObjectives.Add(TEXT("PrepareForAlpha"));
        UnlockedObjectives.Add(TEXT("FaceTheAlpha"));
        UnlockedObjectives.Add(TEXT("SurviveOrPerish"));
    }
}

void ANarr_StoryCheckpoint::BroadcastStoryProgress()
{
    // Story progress broadcasting will be handled by Quest system integration
    // This is a placeholder for cross-system communication
}

void ANarr_StoryCheckpoint::UpdateGameState()
{
    // Game state updates will be handled by GameState integration
    // This is a placeholder for state management
}

void ANarr_StoryCheckpoint::PlayCheckpointEffects()
{
    // Visual and audio effects will be handled by VFX and Audio agents
    // This is a placeholder for effect integration
}