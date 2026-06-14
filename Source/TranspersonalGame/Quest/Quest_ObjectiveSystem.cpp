#include "Quest_ObjectiveSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UQuest_ObjectiveComponent::UQuest_ObjectiveComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    bAutoActivateOnBeginPlay = false;
}

void UQuest_ObjectiveComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoActivateOnBeginPlay)
    {
        ActivateObjective();
    }
    
    BindToGameEvents();
}

void UQuest_ObjectiveComponent::BindToGameEvents()
{
    if (UQuest_ObjectiveSubsystem* ObjectiveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuest_ObjectiveSubsystem>())
    {
        ObjectiveSubsystem->RegisterObjective(this);
    }
}

void UQuest_ObjectiveComponent::ActivateObjective()
{
    ObjectiveData.Status = EQuest_ObjectiveStatus::Active;
    ObjectiveData.CurrentCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Quest Objective Activated: %s"), *ObjectiveData.Description);
    
    if (ObjectiveData.ObjectiveType == EQuest_ObjectiveType::Reach)
    {
        SetComponentTickEnabled(true);
    }
}

void UQuest_ObjectiveComponent::CompleteObjective()
{
    if (ObjectiveData.Status == EQuest_ObjectiveStatus::Active)
    {
        ObjectiveData.Status = EQuest_ObjectiveStatus::Completed;
        ObjectiveData.CurrentCount = ObjectiveData.RequiredCount;
        
        UE_LOG(LogTemp, Log, TEXT("Quest Objective Completed: %s"), *ObjectiveData.Description);
        
        SetComponentTickEnabled(false);
    }
}

void UQuest_ObjectiveComponent::FailObjective()
{
    ObjectiveData.Status = EQuest_ObjectiveStatus::Failed;
    UE_LOG(LogTemp, Warning, TEXT("Quest Objective Failed: %s"), *ObjectiveData.Description);
    SetComponentTickEnabled(false);
}

void UQuest_ObjectiveComponent::UpdateProgress(int32 ProgressAmount)
{
    if (ObjectiveData.Status != EQuest_ObjectiveStatus::Active)
    {
        return;
    }
    
    ObjectiveData.CurrentCount = FMath::Min(ObjectiveData.CurrentCount + ProgressAmount, ObjectiveData.RequiredCount);
    
    UE_LOG(LogTemp, Log, TEXT("Quest Objective Progress: %s (%d/%d)"), 
           *ObjectiveData.Description, ObjectiveData.CurrentCount, ObjectiveData.RequiredCount);
    
    if (ObjectiveData.CurrentCount >= ObjectiveData.RequiredCount)
    {
        CompleteObjective();
    }
}

bool UQuest_ObjectiveComponent::IsObjectiveComplete() const
{
    return ObjectiveData.Status == EQuest_ObjectiveStatus::Completed;
}

float UQuest_ObjectiveComponent::GetCompletionPercentage() const
{
    if (ObjectiveData.RequiredCount <= 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(ObjectiveData.CurrentCount) / static_cast<float>(ObjectiveData.RequiredCount);
}

void UQuest_ObjectiveComponent::OnActorKilled(AActor* KilledActor, AActor* Killer)
{
    if (ObjectiveData.Status != EQuest_ObjectiveStatus::Active || ObjectiveData.ObjectiveType != EQuest_ObjectiveType::Kill)
    {
        return;
    }
    
    if (KilledActor && KilledActor->Tags.Contains(FName(*ObjectiveData.TargetTag)))
    {
        UpdateProgress(1);
    }
}

void UQuest_ObjectiveComponent::OnItemCollected(const FString& ItemTag, int32 Amount)
{
    if (ObjectiveData.Status != EQuest_ObjectiveStatus::Active || ObjectiveData.ObjectiveType != EQuest_ObjectiveType::Collect)
    {
        return;
    }
    
    if (ItemTag == ObjectiveData.TargetTag)
    {
        UpdateProgress(Amount);
    }
}

void UQuest_ObjectiveComponent::OnLocationReached(const FVector& Location, AActor* ReachingActor)
{
    if (ObjectiveData.Status != EQuest_ObjectiveStatus::Active || ObjectiveData.ObjectiveType != EQuest_ObjectiveType::Reach)
    {
        return;
    }
    
    float Distance = FVector::Dist(Location, ObjectiveData.TargetLocation);
    if (Distance <= ObjectiveData.InteractionRadius)
    {
        CompleteObjective();
    }
}

void UQuest_ObjectiveComponent::CheckLocationObjective()
{
    if (ObjectiveData.ObjectiveType != EQuest_ObjectiveType::Reach || ObjectiveData.Status != EQuest_ObjectiveStatus::Active)
    {
        return;
    }
    
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), ObjectiveData.TargetLocation);
        if (Distance <= ObjectiveData.InteractionRadius)
        {
            CompleteObjective();
        }
    }
}

void UQuest_ObjectiveComponent::CheckInteractionObjective()
{
    // Implementation for interaction-based objectives
    if (ObjectiveData.ObjectiveType == EQuest_ObjectiveType::Interact && ObjectiveData.Status == EQuest_ObjectiveStatus::Active)
    {
        // Check if player is within interaction range
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
            if (Distance <= ObjectiveData.InteractionRadius)
            {
                // Objective can be completed via external trigger
                UE_LOG(LogTemp, Log, TEXT("Player in interaction range for objective: %s"), *ObjectiveData.Description);
            }
        }
    }
}

// Subsystem Implementation
void UQuest_ObjectiveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("Quest Objective Subsystem Initialized"));
}

void UQuest_ObjectiveSubsystem::RegisterObjective(UQuest_ObjectiveComponent* Objective)
{
    if (Objective && !ActiveObjectives.Contains(Objective))
    {
        ActiveObjectives.Add(Objective);
        UE_LOG(LogTemp, Log, TEXT("Objective registered: %s"), *Objective->ObjectiveData.Description);
    }
}

void UQuest_ObjectiveSubsystem::UnregisterObjective(UQuest_ObjectiveComponent* Objective)
{
    if (Objective)
    {
        ActiveObjectives.Remove(Objective);
        UE_LOG(LogTemp, Log, TEXT("Objective unregistered: %s"), *Objective->ObjectiveData.Description);
    }
}

TArray<UQuest_ObjectiveComponent*> UQuest_ObjectiveSubsystem::GetActiveObjectives() const
{
    TArray<UQuest_ObjectiveComponent*> ActiveOnly;
    for (UQuest_ObjectiveComponent* Objective : ActiveObjectives)
    {
        if (Objective && Objective->ObjectiveData.Status == EQuest_ObjectiveStatus::Active)
        {
            ActiveOnly.Add(Objective);
        }
    }
    return ActiveOnly;
}

UQuest_ObjectiveComponent* UQuest_ObjectiveSubsystem::FindObjectiveByID(const FString& ObjectiveID) const
{
    for (UQuest_ObjectiveComponent* Objective : ActiveObjectives)
    {
        if (Objective && Objective->ObjectiveData.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    return nullptr;
}

void UQuest_ObjectiveSubsystem::NotifyActorKilled(AActor* KilledActor, AActor* Killer)
{
    for (UQuest_ObjectiveComponent* Objective : ActiveObjectives)
    {
        if (Objective)
        {
            Objective->OnActorKilled(KilledActor, Killer);
        }
    }
}

void UQuest_ObjectiveSubsystem::NotifyItemCollected(const FString& ItemTag, int32 Amount)
{
    for (UQuest_ObjectiveComponent* Objective : ActiveObjectives)
    {
        if (Objective)
        {
            Objective->OnItemCollected(ItemTag, Amount);
        }
    }
}

void UQuest_ObjectiveSubsystem::NotifyLocationReached(const FVector& Location, AActor* ReachingActor)
{
    for (UQuest_ObjectiveComponent* Objective : ActiveObjectives)
    {
        if (Objective)
        {
            Objective->OnLocationReached(Location, ReachingActor);
        }
    }
}

void UQuest_ObjectiveSubsystem::ProcessKillObjectives(AActor* KilledActor, AActor* Killer)
{
    NotifyActorKilled(KilledActor, Killer);
}

void UQuest_ObjectiveSubsystem::ProcessCollectionObjectives(const FString& ItemTag, int32 Amount)
{
    NotifyItemCollected(ItemTag, Amount);
}

void UQuest_ObjectiveSubsystem::ProcessLocationObjectives(const FVector& Location, AActor* ReachingActor)
{
    NotifyLocationReached(Location, ReachingActor);
}