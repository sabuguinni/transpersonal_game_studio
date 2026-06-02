#include "Narr_NPCInteractionManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UNarr_NPCInteractionManager::UNarr_NPCInteractionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentInteractionIndex = 0;
    bCanInteract = true;
    LastInteractionTime = 0.0f;
    InteractionCooldown = 3.0f;

    // Default interaction setup
    FNarr_NPCInteraction DefaultInteraction;
    DefaultInteraction.InteractionText = TEXT("Greetings, hunter. The beasts grow restless in the valley.");
    DefaultInteraction.InteractionRange = 250.0f;
    DefaultInteraction.bRequiresItem = false;
    AvailableInteractions.Add(DefaultInteraction);
}

void UNarr_NPCInteractionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize interaction system
    LastInteractionTime = GetWorld()->GetTimeSeconds();
}

void UNarr_NPCInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update interaction cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (!bCanInteract && (CurrentTime - LastInteractionTime) > InteractionCooldown)
    {
        bCanInteract = true;
    }
}

bool UNarr_NPCInteractionManager::CanPlayerInteract(AActor* PlayerActor)
{
    if (!PlayerActor || !bCanInteract || AvailableInteractions.Num() == 0)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    FNarr_NPCInteraction CurrentInteraction = GetCurrentInteraction();
    
    if (Distance > CurrentInteraction.InteractionRange)
    {
        return false;
    }

    // Check item requirements
    if (CurrentInteraction.bRequiresItem)
    {
        // TODO: Check player inventory for required item
        // For now, assume player has required items
    }

    return true;
}

FNarr_NPCInteraction UNarr_NPCInteractionManager::GetCurrentInteraction()
{
    if (AvailableInteractions.Num() == 0)
    {
        return FNarr_NPCInteraction();
    }

    int32 SafeIndex = FMath::Clamp(CurrentInteractionIndex, 0, AvailableInteractions.Num() - 1);
    return AvailableInteractions[SafeIndex];
}

void UNarr_NPCInteractionManager::TriggerInteraction(AActor* PlayerActor)
{
    if (!CanPlayerInteract(PlayerActor))
    {
        return;
    }

    FNarr_NPCInteraction CurrentInteraction = GetCurrentInteraction();
    
    // Log interaction for debugging
    UE_LOG(LogTemp, Warning, TEXT("NPC Interaction: %s"), *CurrentInteraction.InteractionText);

    // Play audio if available
    if (!CurrentInteraction.AudioPath.IsEmpty())
    {
        PlayInteractionAudio(CurrentInteraction.AudioPath);
    }

    // Set cooldown
    bCanInteract = false;
    LastInteractionTime = GetWorld()->GetTimeSeconds();

    // Advance to next interaction if available
    if (AvailableInteractions.Num() > 1)
    {
        CurrentInteractionIndex = (CurrentInteractionIndex + 1) % AvailableInteractions.Num();
    }
}

void UNarr_NPCInteractionManager::AddInteraction(const FNarr_NPCInteraction& NewInteraction)
{
    AvailableInteractions.Add(NewInteraction);
}

void UNarr_NPCInteractionManager::SetInteractionEnabled(bool bEnabled)
{
    bCanInteract = bEnabled;
}

void UNarr_NPCInteractionManager::PlayInteractionAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty())
    {
        return;
    }

    // Load and play audio
    USoundBase* Sound = LoadObject<USoundBase>(nullptr, *AudioPath);
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetOwner()->GetActorLocation());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load audio: %s"), *AudioPath);
    }
}