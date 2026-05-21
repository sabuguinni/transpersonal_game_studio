#include "DialogueTrigger.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"

ADialogueTrigger::ADialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default values
    DialogueID = TEXT("");
    StoryBeatID = TEXT("");
    TriggerType = ENarr_TriggerType::OnEnter;
    bOneTimeOnly = true;
    bRequiresPlayerOnly = true;
    DelayBeforeTrigger = 0.0f;
    bHasTriggered = false;

    // Create visual indicator component
    VisualIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualIndicator"));
    if (VisualIndicator)
    {
        VisualIndicator->SetupAttachment(RootComponent);
        VisualIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualIndicator->SetVisibility(true);
    }

    // Setup collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ADialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Get narrative manager
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        NarrativeManager = GameInstance->GetSubsystem<UNarrativeManager>();
    }

    // Setup overlap events
    OnActorBeginOverlap.AddDynamic(this, &ADialogueTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &ADialogueTrigger::OnOverlapEnd);

    // Setup visual indicator
    SetupVisualIndicator();

    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: %s initialized with DialogueID: %s"), 
           *GetActorLabel(), *DialogueID);
}

void ADialogueTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                     bool bFromSweep, const FHitResult& SweepResult)
{
    if (TriggerType == ENarr_TriggerType::OnEnter)
    {
        ExecuteTrigger(OtherActor);
    }
}

void ADialogueTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (TriggerType == ENarr_TriggerType::OnExit)
    {
        ExecuteTrigger(OtherActor);
    }
}

void ADialogueTrigger::ExecuteTrigger(AActor* TriggeringActor)
{
    // Check if already triggered and one-time only
    if (bHasTriggered && bOneTimeOnly)
    {
        return;
    }

    // Check trigger conditions
    if (!CheckTriggerConditions(TriggeringActor))
    {
        return;
    }

    // Handle delay
    if (DelayBeforeTrigger > 0.0f)
    {
        GetWorldTimerManager().SetTimer(DelayTimerHandle, this, 
                                       &ADialogueTrigger::DelayedTriggerExecution, 
                                       DelayBeforeTrigger, false);
    }
    else
    {
        DelayedTriggerExecution();
    }
}

void ADialogueTrigger::DelayedTriggerExecution()
{
    if (!NarrativeManager)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueTrigger: NarrativeManager not found"));
        return;
    }

    // Trigger dialogue
    if (!DialogueID.IsEmpty())
    {
        NarrativeManager->TriggerDialogue(DialogueID, this);
        UE_LOG(LogTemp, Warning, TEXT("DialogueTrigger: Triggered dialogue - %s"), *DialogueID);
    }

    // Trigger story beat
    if (!StoryBeatID.IsEmpty())
    {
        NarrativeManager->TriggerStoryBeat(StoryBeatID);
        UE_LOG(LogTemp, Warning, TEXT("DialogueTrigger: Triggered story beat - %s"), *StoryBeatID);
    }

    // Mark as triggered
    bHasTriggered = true;

    // Hide visual indicator if one-time only
    if (bOneTimeOnly && VisualIndicator)
    {
        VisualIndicator->SetVisibility(false);
    }
}

bool ADialogueTrigger::CheckTriggerConditions(AActor* TriggeringActor) const
{
    // Check if player only requirement
    if (bRequiresPlayerOnly)
    {
        ACharacter* Character = Cast<ACharacter>(TriggeringActor);
        if (!Character || !Character->IsPlayerControlled())
        {
            return false;
        }
    }

    // Check required conditions
    if (NarrativeManager)
    {
        for (const FString& Condition : RequiredConditions)
        {
            // Simple condition checking - extend as needed
            if (Condition == TEXT("first_time") && bHasTriggered)
            {
                return false;
            }
            // Add more condition checks here
        }
    }

    return true;
}

void ADialogueTrigger::SetupVisualIndicator()
{
    if (!VisualIndicator)
    {
        return;
    }

    // Try to load a simple mesh for the indicator
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        VisualIndicator->SetStaticMesh(SphereMesh.Object);
        VisualIndicator->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
        VisualIndicator->SetRelativeLocation(FVector(0, 0, 100));
    }

    // Set material color based on trigger type
    UMaterialInterface* Material = nullptr;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (DefaultMaterial.Succeeded())
    {
        Material = DefaultMaterial.Object;
        VisualIndicator->SetMaterial(0, Material);
    }
}