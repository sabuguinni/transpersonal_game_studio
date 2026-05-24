#include "DialogueTrigger.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

ADialogueTrigger::ADialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default values
    EventID = TEXT("");
    bTriggerOnce = true;
    bRequirePlayerCharacter = true;
    CooldownTime = 5.0f;
    bHasTriggered = false;
    LastTriggerTime = 0.0f;
    NarrativeManager = nullptr;

    // Set up collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ADialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeNarrativeManager();
    
    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &ADialogueTrigger::OnOverlapBegin);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger initialized with EventID: %s"), *EventID);
}

void ADialogueTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!CanTrigger())
    {
        return;
    }

    // Check if it's the player character (if required)
    if (bRequirePlayerCharacter)
    {
        ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (OtherActor != PlayerCharacter)
        {
            return;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger activated by: %s"), 
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));

    // Trigger the narrative event
    if (NarrativeManager && !EventID.IsEmpty())
    {
        NarrativeManager->TriggerStoryEvent(EventID);
    }

    // Play custom dialogues if any
    if (CustomDialogues.Num() > 0 && NarrativeManager)
    {
        for (const FNarr_DialogueEntry& Dialogue : CustomDialogues)
        {
            NarrativeManager->PlayDialogue(Dialogue);
        }
    }

    // Update trigger state
    bHasTriggered = true;
    LastTriggerTime = GetWorld()->GetTimeSeconds();

    // Destroy if one-time trigger
    if (bTriggerOnce)
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        
        // Schedule destruction after a short delay
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
        {
            Destroy();
        }, 1.0f, false);
    }
}

void ADialogueTrigger::InitializeNarrativeManager()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        NarrativeManager = GameInstance->GetSubsystem<UNarrativeManager>();
        if (NarrativeManager)
        {
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager found and connected"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager not found"));
        }
    }
}

bool ADialogueTrigger::CanTrigger() const
{
    // Check if already triggered (for one-time triggers)
    if (bTriggerOnce && bHasTriggered)
    {
        return false;
    }

    // Check cooldown
    if (CooldownTime > 0.0f)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastTriggerTime < CooldownTime)
        {
            return false;
        }
    }

    // Check if narrative manager is available
    if (!NarrativeManager)
    {
        return false;
    }

    return true;
}