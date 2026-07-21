#include "NPCInteractionSystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UNPCInteractionSystem::UNPCInteractionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    bIsInteracting = false;
    CurrentInteractingPlayer = nullptr;
    LastInteractionTime = 0.0f;
    InteractionCooldownTimer = 0.0f;
    
    // Set default interaction data
    InteractionData.InteractionType = ENPC_InteractionType::Dialogue;
    InteractionData.InteractionText = TEXT("Hello, survivor!");
    InteractionData.InteractionRange = 300.0f;
    InteractionData.bRequiresLineOfSight = true;
    InteractionData.CooldownTime = 5.0f;
}

void UNPCInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize component
    bIsInteracting = false;
    CurrentInteractingPlayer = nullptr;
    LastInteractionTime = 0.0f;
    InteractionCooldownTimer = 0.0f;
}

void UNPCInteractionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update cooldown timer
    if (InteractionCooldownTimer > 0.0f)
    {
        InteractionCooldownTimer -= DeltaTime;
    }
    
    // Check for nearby players if not currently interacting
    if (!bIsInteracting && InteractionCooldownTimer <= 0.0f)
    {
        // Find nearby players (simplified - in full game would use proper player detection)
        UWorld* World = GetWorld();
        if (World)
        {
            // This is a simplified implementation - in a full game you'd use proper player detection
            // For now, we'll just update the interaction state
        }
    }
}

bool UNPCInteractionSystem::CanInteractWithPlayer(APawn* Player)
{
    if (!Player)
    {
        return false;
    }
    
    // Check cooldown
    if (InteractionCooldownTimer > 0.0f)
    {
        return false;
    }
    
    // Check if already interacting with someone else
    if (bIsInteracting && CurrentInteractingPlayer && CurrentInteractingPlayer != Player)
    {
        return false;
    }
    
    // Check range
    if (!IsPlayerInRange(Player, InteractionData.InteractionRange))
    {
        return false;
    }
    
    // Check line of sight if required
    if (InteractionData.bRequiresLineOfSight && !HasLineOfSightToPlayer(Player))
    {
        return false;
    }
    
    return true;
}

bool UNPCInteractionSystem::StartInteraction(APawn* Player, ENPC_InteractionType InteractionType)
{
    if (!CanInteractWithPlayer(Player))
    {
        return false;
    }
    
    // Start interaction
    bIsInteracting = true;
    CurrentInteractingPlayer = Player;
    LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    // Find matching interaction data
    FNPC_InteractionData* SelectedInteraction = &InteractionData;
    for (FNPC_InteractionData& Interaction : AvailableInteractions)
    {
        if (Interaction.InteractionType == InteractionType)
        {
            SelectedInteraction = &Interaction;
            break;
        }
    }
    
    // Trigger blueprint event
    OnInteractionStarted(Player, InteractionType);
    
    return true;
}

void UNPCInteractionSystem::EndInteraction()
{
    if (bIsInteracting)
    {
        bIsInteracting = false;
        CurrentInteractingPlayer = nullptr;
        InteractionCooldownTimer = InteractionData.CooldownTime;
        
        // Trigger blueprint event
        OnInteractionEnded();
    }
}

FString UNPCInteractionSystem::GetInteractionPrompt(APawn* Player)
{
    if (!CanInteractWithPlayer(Player))
    {
        return FString();
    }
    
    // Return appropriate prompt based on interaction type
    switch (InteractionData.InteractionType)
    {
        case ENPC_InteractionType::Dialogue:
            return TEXT("Press E to Talk");
        case ENPC_InteractionType::Trade:
            return TEXT("Press E to Trade");
        case ENPC_InteractionType::Quest:
            return TEXT("Press E for Quest");
        case ENPC_InteractionType::Information:
            return TEXT("Press E for Info");
        case ENPC_InteractionType::Warning:
            return TEXT("Press E to Listen");
        case ENPC_InteractionType::Hostile:
            return TEXT("Hostile NPC");
        default:
            return TEXT("Press E to Interact");
    }
}

bool UNPCInteractionSystem::HasLineOfSightToPlayer(APawn* Player)
{
    if (!Player)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Get start and end points for line trace
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = Player->GetActorLocation();
    
    // Set up collision parameters
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(GetOwner());
    TraceParams.AddIgnoredActor(Player);
    TraceParams.bTraceComplex = false;
    
    // Perform line trace
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        TraceParams
    );
    
    // If no hit, we have clear line of sight
    return !bHit;
}

float UNPCInteractionSystem::GetDistanceToPlayer(APawn* Player)
{
    if (!Player)
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

bool UNPCInteractionSystem::IsPlayerInRange(APawn* Player, float Range)
{
    return GetDistanceToPlayer(Player) <= Range;
}