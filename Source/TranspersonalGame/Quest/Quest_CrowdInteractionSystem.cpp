#include "Quest_CrowdInteractionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AQuest_CrowdInteractionManager::AQuest_CrowdInteractionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    InteractionCheckRadius = 5000.0f;
    InteractionCheckInterval = 2.0f;
    MaxSimultaneousInteractions = 3;
    bDebugMode = false;
    LastInteractionCheckTime = 0.0f;
    
    // Set default location in Savana biome
    SetActorLocation(FVector(0.0f, 0.0f, 100.0f));
}

void AQuest_CrowdInteractionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Crowd Interaction Manager initialized at location: %s"), 
           *GetActorLocation().ToString());
    
    // Initialize interaction data if not set
    if (!CrowdInteractionDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdInteractionDataTable not set, using default interactions"));
    }
    
    // Clear any existing active interactions
    ActiveInteractions.Empty();
}

void AQuest_CrowdInteractionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check for new interaction opportunities periodically
    if (GetWorld()->GetTimeSeconds() - LastInteractionCheckTime > InteractionCheckInterval)
    {
        CheckForNewInteractionOpportunities();
        LastInteractionCheckTime = GetWorld()->GetTimeSeconds();
    }
    
    // Update all active interactions
    UpdateActiveInteractions(DeltaTime);
    
    // Debug visualization
    if (bDebugMode)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), InteractionCheckRadius, 32, FColor::Blue, false, 1.0f);
        
        for (int32 i = 0; i < ActiveInteractions.Num(); i++)
        {
            const FQuest_ActiveCrowdInteraction& Interaction = ActiveInteractions[i];
            if (Interaction.bIsActive)
            {
                DrawDebugSphere(GetWorld(), Interaction.TargetLocation, 
                               Interaction.InteractionData.InteractionRadius, 16, FColor::Green, false, 1.0f);
            }
        }
    }
}

bool AQuest_CrowdInteractionManager::StartCrowdInteraction(EQuest_CrowdInteractionType InteractionType, FVector Location)
{
    // Check if we can start a new interaction
    if (ActiveInteractions.Num() >= MaxSimultaneousInteractions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new crowd interaction: maximum simultaneous interactions reached"));
        return false;
    }
    
    if (!CanStartInteraction(InteractionType, Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start crowd interaction: requirements not met"));
        return false;
    }
    
    // Get interaction data
    FQuest_CrowdInteractionData InteractionData = GetInteractionDataByType(InteractionType);
    
    // Get crowd actors in range
    TArray<AActor*> CrowdActors = GetCrowdActorsInRadius(Location, InteractionData.InteractionRadius);
    
    if (CrowdActors.Num() < InteractionData.RequiredCrowdSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough crowd actors for interaction: %d required, %d found"), 
               InteractionData.RequiredCrowdSize, CrowdActors.Num());
        return false;
    }
    
    // Create new active interaction
    FQuest_ActiveCrowdInteraction NewInteraction;
    NewInteraction.InteractionData = InteractionData;
    NewInteraction.TargetLocation = Location;
    NewInteraction.TimeRemaining = InteractionData.Duration;
    NewInteraction.CurrentCrowdSize = CrowdActors.Num();
    NewInteraction.bIsActive = true;
    NewInteraction.ParticipatingCrowdActors = CrowdActors;
    
    ActiveInteractions.Add(NewInteraction);
    
    // Update crowd behavior for this interaction
    UpdateCrowdBehaviorForInteraction(NewInteraction);
    
    UE_LOG(LogTemp, Warning, TEXT("Started crowd interaction: %s at location %s with %d participants"), 
           *InteractionData.InteractionName, *Location.ToString(), CrowdActors.Num());
    
    return true;
}

void AQuest_CrowdInteractionManager::EndCrowdInteraction(int32 InteractionIndex)
{
    if (InteractionIndex >= 0 && InteractionIndex < ActiveInteractions.Num())
    {
        FQuest_ActiveCrowdInteraction& Interaction = ActiveInteractions[InteractionIndex];
        
        if (Interaction.bIsActive)
        {
            // Reset crowd behavior for participating actors
            for (AActor* CrowdActor : Interaction.ParticipatingCrowdActors)
            {
                if (IsValid(CrowdActor))
                {
                    UQuest_CrowdInteractionComponent* InteractionComp = 
                        CrowdActor->FindComponentByClass<UQuest_CrowdInteractionComponent>();
                    if (InteractionComp)
                    {
                        InteractionComp->LeaveCrowdInteraction();
                    }
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Ended crowd interaction: %s"), 
                   *Interaction.InteractionData.InteractionName);
            
            // Mark as inactive
            Interaction.bIsActive = false;
        }
        
        // Remove from active interactions
        ActiveInteractions.RemoveAt(InteractionIndex);
    }
}

TArray<AActor*> AQuest_CrowdInteractionManager::GetCrowdActorsInRadius(FVector Location, float Radius)
{
    TArray<AActor*> CrowdActors;
    TArray<AActor*> AllActors;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (IsValid(Actor))
        {
            // Check if actor has crowd interaction component
            UQuest_CrowdInteractionComponent* InteractionComp = 
                Actor->FindComponentByClass<UQuest_CrowdInteractionComponent>();
            
            if (InteractionComp && InteractionComp->CanParticipateInInteraction(EQuest_CrowdInteractionType::EscortMission))
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
                if (Distance <= Radius)
                {
                    CrowdActors.Add(Actor);
                }
            }
        }
    }
    
    return CrowdActors;
}

bool AQuest_CrowdInteractionManager::CanStartInteraction(EQuest_CrowdInteractionType InteractionType, FVector Location)
{
    // Check if location is valid
    if (Location == FVector::ZeroVector)
    {
        return false;
    }
    
    // Check if there are enough crowd actors nearby
    FQuest_CrowdInteractionData InteractionData = GetInteractionDataByType(InteractionType);
    TArray<AActor*> CrowdActors = GetCrowdActorsInRadius(Location, InteractionData.InteractionRadius);
    
    return CrowdActors.Num() >= InteractionData.RequiredCrowdSize;
}

void AQuest_CrowdInteractionManager::UpdateCrowdBehaviorForInteraction(const FQuest_ActiveCrowdInteraction& Interaction)
{
    for (AActor* CrowdActor : Interaction.ParticipatingCrowdActors)
    {
        if (IsValid(CrowdActor))
        {
            UQuest_CrowdInteractionComponent* InteractionComp = 
                CrowdActor->FindComponentByClass<UQuest_CrowdInteractionComponent>();
            if (InteractionComp)
            {
                InteractionComp->JoinCrowdInteraction(Interaction.InteractionData.InteractionType, 
                                                    Interaction.TargetLocation);
            }
        }
    }
}

int32 AQuest_CrowdInteractionManager::GetActiveCrowdInteractionCount() const
{
    int32 ActiveCount = 0;
    for (const FQuest_ActiveCrowdInteraction& Interaction : ActiveInteractions)
    {
        if (Interaction.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

FQuest_CrowdInteractionData AQuest_CrowdInteractionManager::GetInteractionDataByType(EQuest_CrowdInteractionType InteractionType)
{
    FQuest_CrowdInteractionData DefaultData;
    
    // Set default data based on interaction type
    switch (InteractionType)
    {
        case EQuest_CrowdInteractionType::EscortMission:
            DefaultData.InteractionName = TEXT("Escort Mission");
            DefaultData.Description = TEXT("Guide a group of people to safety");
            DefaultData.RequiredCrowdSize = 5;
            DefaultData.InteractionRadius = 1500.0f;
            DefaultData.Duration = 120.0f;
            DefaultData.ExperienceReward = 200;
            break;
            
        case EQuest_CrowdInteractionType::CrowdControl:
            DefaultData.InteractionName = TEXT("Crowd Control");
            DefaultData.Description = TEXT("Manage a large gathering of people");
            DefaultData.RequiredCrowdSize = 15;
            DefaultData.InteractionRadius = 2000.0f;
            DefaultData.Duration = 90.0f;
            DefaultData.ExperienceReward = 150;
            break;
            
        case EQuest_CrowdInteractionType::CelebrationEvent:
            DefaultData.InteractionName = TEXT("Celebration Event");
            DefaultData.Description = TEXT("Organize a tribal celebration");
            DefaultData.RequiredCrowdSize = 20;
            DefaultData.InteractionRadius = 2500.0f;
            DefaultData.Duration = 180.0f;
            DefaultData.ExperienceReward = 300;
            break;
            
        case EQuest_CrowdInteractionType::PanicResponse:
            DefaultData.InteractionName = TEXT("Panic Response");
            DefaultData.Description = TEXT("Calm panicked crowd during emergency");
            DefaultData.RequiredCrowdSize = 10;
            DefaultData.InteractionRadius = 1000.0f;
            DefaultData.Duration = 60.0f;
            DefaultData.ExperienceReward = 250;
            break;
            
        default:
            DefaultData.InteractionName = TEXT("Generic Interaction");
            DefaultData.Description = TEXT("Basic crowd interaction");
            DefaultData.RequiredCrowdSize = 5;
            DefaultData.InteractionRadius = 1000.0f;
            DefaultData.Duration = 60.0f;
            DefaultData.ExperienceReward = 100;
            break;
    }
    
    DefaultData.InteractionType = InteractionType;
    DefaultData.bIsRepeatable = true;
    
    return DefaultData;
}

void AQuest_CrowdInteractionManager::CheckForNewInteractionOpportunities()
{
    // Get player location for proximity checks
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!IsValid(PlayerPawn))
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check if player is within interaction check radius
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerLocation);
    if (DistanceToPlayer > InteractionCheckRadius)
    {
        return;
    }
    
    // Check for escort mission opportunities
    if (CanStartInteraction(EQuest_CrowdInteractionType::EscortMission, PlayerLocation))
    {
        // Could trigger escort mission here based on game conditions
        UE_LOG(LogTemp, Log, TEXT("Escort mission opportunity detected near player"));
    }
    
    // Check for crowd control opportunities
    if (CanStartInteraction(EQuest_CrowdInteractionType::CrowdControl, PlayerLocation))
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd control opportunity detected near player"));
    }
}

void AQuest_CrowdInteractionManager::UpdateActiveInteractions(float DeltaTime)
{
    for (int32 i = ActiveInteractions.Num() - 1; i >= 0; i--)
    {
        FQuest_ActiveCrowdInteraction& Interaction = ActiveInteractions[i];
        
        if (Interaction.bIsActive)
        {
            // Update time remaining
            Interaction.TimeRemaining -= DeltaTime;
            
            // Check if interaction should end
            if (Interaction.TimeRemaining <= 0.0f)
            {
                ProcessInteractionCompletion(i);
            }
            else
            {
                // Update crowd behavior periodically
                TriggerCrowdResponse(Interaction);
            }
        }
    }
}

void AQuest_CrowdInteractionManager::ProcessInteractionCompletion(int32 InteractionIndex)
{
    if (InteractionIndex >= 0 && InteractionIndex < ActiveInteractions.Num())
    {
        const FQuest_ActiveCrowdInteraction& Interaction = ActiveInteractions[InteractionIndex];
        
        UE_LOG(LogTemp, Warning, TEXT("Completed crowd interaction: %s. Awarding %d experience."), 
               *Interaction.InteractionData.InteractionName, 
               Interaction.InteractionData.ExperienceReward);
        
        // Award experience to player
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (IsValid(PlayerPawn))
        {
            // Could integrate with character progression system here
        }
        
        EndCrowdInteraction(InteractionIndex);
    }
}

void AQuest_CrowdInteractionManager::TriggerCrowdResponse(const FQuest_ActiveCrowdInteraction& Interaction)
{
    // Update behavior of participating crowd actors based on interaction type
    for (AActor* CrowdActor : Interaction.ParticipatingCrowdActors)
    {
        if (IsValid(CrowdActor))
        {
            UQuest_CrowdInteractionComponent* InteractionComp = 
                CrowdActor->FindComponentByClass<UQuest_CrowdInteractionComponent>();
            if (InteractionComp)
            {
                InteractionComp->SetInteractionBehavior(Interaction.InteractionData.InteractionType);
            }
        }
    }
}

// UQuest_CrowdInteractionComponent Implementation

UQuest_CrowdInteractionComponent::UQuest_CrowdInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    bCanParticipateInInteractions = true;
    InteractionInfluenceRadius = 500.0f;
    InteractionPriority = 1;
    bIsCurrentlyParticipating = false;
    CurrentInteractionType = EQuest_CrowdInteractionType::EscortMission;
    
    // Add all interaction types as supported by default
    SupportedInteractionTypes.Add(EQuest_CrowdInteractionType::EscortMission);
    SupportedInteractionTypes.Add(EQuest_CrowdInteractionType::CrowdControl);
    SupportedInteractionTypes.Add(EQuest_CrowdInteractionType::CelebrationEvent);
    SupportedInteractionTypes.Add(EQuest_CrowdInteractionType::PanicResponse);
}

void UQuest_CrowdInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest Crowd Interaction Component initialized on actor: %s"), 
           *GetOwner()->GetName());
}

void UQuest_CrowdInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update interaction behavior if currently participating
    if (bIsCurrentlyParticipating)
    {
        // Could update movement or behavior based on current interaction type
        SetInteractionBehavior(CurrentInteractionType);
    }
}

bool UQuest_CrowdInteractionComponent::CanParticipateInInteraction(EQuest_CrowdInteractionType InteractionType) const
{
    if (!bCanParticipateInInteractions || bIsCurrentlyParticipating)
    {
        return false;
    }
    
    return SupportedInteractionTypes.Contains(InteractionType);
}

void UQuest_CrowdInteractionComponent::JoinCrowdInteraction(EQuest_CrowdInteractionType InteractionType, 
                                                           FVector InteractionLocation)
{
    if (CanParticipateInInteraction(InteractionType))
    {
        bIsCurrentlyParticipating = true;
        CurrentInteractionType = InteractionType;
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s joined crowd interaction: %d"), 
               *GetOwner()->GetName(), (int32)InteractionType);
        
        SetInteractionBehavior(InteractionType);
    }
}

void UQuest_CrowdInteractionComponent::LeaveCrowdInteraction()
{
    if (bIsCurrentlyParticipating)
    {
        bIsCurrentlyParticipating = false;
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s left crowd interaction"), *GetOwner()->GetName());
        
        // Reset to default behavior
        SetInteractionBehavior(EQuest_CrowdInteractionType::EscortMission);
    }
}

void UQuest_CrowdInteractionComponent::SetInteractionBehavior(EQuest_CrowdInteractionType InteractionType)
{
    // Modify actor behavior based on interaction type
    switch (InteractionType)
    {
        case EQuest_CrowdInteractionType::EscortMission:
            // Set following behavior
            break;
            
        case EQuest_CrowdInteractionType::CrowdControl:
            // Set orderly movement behavior
            break;
            
        case EQuest_CrowdInteractionType::CelebrationEvent:
            // Set celebration behavior (dancing, cheering)
            break;
            
        case EQuest_CrowdInteractionType::PanicResponse:
            // Set calm/fleeing behavior
            break;
            
        default:
            // Default behavior
            break;
    }
}

bool UQuest_CrowdInteractionComponent::IsParticipatingInInteraction() const
{
    return bIsCurrentlyParticipating;
}

EQuest_CrowdInteractionType UQuest_CrowdInteractionComponent::GetCurrentInteractionType() const
{
    return CurrentInteractionType;
}