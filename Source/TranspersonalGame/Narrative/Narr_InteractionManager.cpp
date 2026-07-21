#include "Narr_InteractionManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ANarr_InteractionManager::ANarr_InteractionManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize visual marker
    VisualMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMarker"));
    RootComponent = VisualMarker;

    // Set default values
    InteractionRange = 300.0f;
    bIsActive = true;
    bShowMarker = true;
    bPlayerInRange = false;
    CurrentPlayer = nullptr;

    // Set up collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ANarr_InteractionManager::OnOverlapBegin);
    GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &ANarr_InteractionManager::OnOverlapEnd);

    // Set default interaction data
    InteractionData.InteractionText = TEXT("A mysterious presence emanates from this place...");
    InteractionData.InteractionType = ENarr_InteractionType::Dialogue;
    InteractionData.bRequiresQuestCompletion = false;
    InteractionData.MinPlayerLevel = 1;
}

void ANarr_InteractionManager::BeginPlay()
{
    Super::BeginPlay();

    // Configure visual marker visibility
    if (VisualMarker)
    {
        VisualMarker->SetVisibility(bShowMarker);
    }

    // Set initial state
    SetActive(bIsActive);
}

void ANarr_InteractionManager::TriggerInteraction(AActor* InteractingActor)
{
    if (!bIsActive || !CanInteract(InteractingActor))
    {
        return;
    }

    // Display interaction text
    if (GEngine && !InteractionData.InteractionText.IsEmpty())
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), 
            UEnum::GetValueAsString(InteractionData.InteractionType), 
            *InteractionData.InteractionText);
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }

    // Play audio if available
    PlayInteractionAudio();

    // Log interaction for debugging
    UE_LOG(LogTemp, Warning, TEXT("Narrative Interaction: %s triggered by %s"), 
           *GetName(), 
           InteractingActor ? *InteractingActor->GetName() : TEXT("Unknown"));
}

bool ANarr_InteractionManager::CanInteract(AActor* InteractingActor) const
{
    if (!InteractingActor || !bIsActive)
    {
        return false;
    }

    // Check if it's a player character
    ACharacter* Character = Cast<ACharacter>(InteractingActor);
    if (!Character)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetActorLocation(), InteractingActor->GetActorLocation());
    if (Distance > InteractionRange)
    {
        return false;
    }

    // Additional checks can be added here (quest completion, level, etc.)
    if (InteractionData.bRequiresQuestCompletion && !InteractionData.RequiredQuestID.IsEmpty())
    {
        // TODO: Check quest completion status
        // For now, assume quest is completed
    }

    return true;
}

void ANarr_InteractionManager::SetInteractionData(const FNarr_InteractionData& NewData)
{
    InteractionData = NewData;
}

void ANarr_InteractionManager::SetActive(bool bActive)
{
    bIsActive = bActive;
    
    if (VisualMarker)
    {
        VisualMarker->SetVisibility(bActive && bShowMarker);
    }

    GetCollisionComponent()->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ANarr_InteractionManager::PlayInteractionAudio()
{
    if (InteractionData.AudioPath.IsEmpty())
    {
        return;
    }

    // Try to load and play audio
    USoundBase* AudioClip = LoadObject<USoundBase>(nullptr, *InteractionData.AudioPath);
    if (AudioClip)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AudioClip, GetActorLocation());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load audio: %s"), *InteractionData.AudioPath);
    }
}

void ANarr_InteractionManager::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                            bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || !OtherActor)
    {
        return;
    }

    // Check if it's a player character
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        bPlayerInRange = true;
        CurrentPlayer = OtherActor;

        // Show interaction prompt
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                TEXT("Press E to interact"));
        }

        // Auto-trigger interaction for some types
        if (InteractionData.InteractionType == ENarr_InteractionType::Storyteller)
        {
            TriggerInteraction(OtherActor);
        }
    }
}

void ANarr_InteractionManager::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == CurrentPlayer)
    {
        bPlayerInRange = false;
        CurrentPlayer = nullptr;

        // Hide interaction prompt
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, 
                TEXT("Interaction area left"));
        }
    }
}