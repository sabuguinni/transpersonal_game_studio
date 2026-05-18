#include "Narr_DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visual marker
    VisualMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMarker"));
    VisualMarker->SetupAttachment(RootComponent);
    VisualMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize properties
    bHasBeenTriggered = false;
    DialogueRowName = TEXT("Default");
    DialogueTable = nullptr;
    DialogueSound = nullptr;

    // Bind overlap event
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnTriggerBeginOverlap);
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Set visual marker visibility based on dialogue availability
    if (VisualMarker)
    {
        VisualMarker->SetVisibility(IsDialogueAvailable());
    }
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Optional: Add floating animation or other visual feedback
    if (VisualMarker && IsDialogueAvailable())
    {
        FVector CurrentLocation = VisualMarker->GetRelativeLocation();
        float FloatOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 10.0f;
        VisualMarker->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, FloatOffset));
    }
}

void ANarr_DialogueSystem::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        TriggerDialogue(PlayerCharacter);
    }
}

void ANarr_DialogueSystem::TriggerDialogue(AActor* PlayerActor)
{
    if (!IsDialogueAvailable())
    {
        return;
    }

    FNarr_DialogueEntry DialogueEntry = GetDialogueEntry();
    
    // Check if this is a one-time dialogue and has already been triggered
    if (DialogueEntry.bOneTimeOnly && bHasBeenTriggered)
    {
        return;
    }

    // Mark as triggered
    bHasBeenTriggered = true;

    // Play dialogue sound if available
    if (DialogueSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DialogueSound, GetActorLocation());
    }

    // Display dialogue text (in a real game, this would show UI)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *DialogueEntry.SpeakerName, 
                                            *DialogueEntry.DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }

    // Call Blueprint event
    OnDialogueTriggered(DialogueEntry);

    // Hide visual marker if one-time only
    if (DialogueEntry.bOneTimeOnly && VisualMarker)
    {
        VisualMarker->SetVisibility(false);
    }
}

FNarr_DialogueEntry ANarr_DialogueSystem::GetDialogueEntry()
{
    if (DialogueTable && !DialogueRowName.IsNone())
    {
        FNarr_DialogueEntry* FoundEntry = DialogueTable->FindRow<FNarr_DialogueEntry>(DialogueRowName, TEXT(""));
        if (FoundEntry)
        {
            return *FoundEntry;
        }
    }

    // Return default entry if not found
    FNarr_DialogueEntry DefaultEntry;
    DefaultEntry.SpeakerName = TEXT("Valley Guide");
    DefaultEntry.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor."));
    DefaultEntry.DialogueType = ENarr_DialogueType::Information;
    return DefaultEntry;
}

void ANarr_DialogueSystem::SetDialogueEntry(FName RowName)
{
    DialogueRowName = RowName;
    bHasBeenTriggered = false;
    
    if (VisualMarker)
    {
        VisualMarker->SetVisibility(IsDialogueAvailable());
    }
}

bool ANarr_DialogueSystem::IsDialogueAvailable() const
{
    if (!DialogueTable || DialogueRowName.IsNone())
    {
        return false;
    }

    FNarr_DialogueEntry* FoundEntry = DialogueTable->FindRow<FNarr_DialogueEntry>(DialogueRowName, TEXT(""));
    if (!FoundEntry)
    {
        return false;
    }

    // If it's one-time only and already triggered, not available
    if (FoundEntry->bOneTimeOnly && bHasBeenTriggered)
    {
        return false;
    }

    return true;
}