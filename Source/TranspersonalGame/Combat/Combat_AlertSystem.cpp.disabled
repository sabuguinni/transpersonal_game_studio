#include "Combat_AlertSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"

UCombat_AlertSystem::UCombat_AlertSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5 seconds
    
    AlertRadius = 3000.0f;
    AlertDuration = 10.0f;
    AlertLevel = ECombat_AlertLevel::None;
    LastAlertTime = 0.0f;
    
    bIsAlerted = false;
    bCanReceiveAlerts = true;
}

void UCombat_AlertSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize alert system
    AlertLevel = ECombat_AlertLevel::None;
    bIsAlerted = false;
    
    UE_LOG(LogTemp, Log, TEXT("Combat Alert System initialized for %s"), *GetOwner()->GetName());
}

void UCombat_AlertSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update alert state
    UpdateAlertState(DeltaTime);
    
    // Check for nearby threats if we're not already alerted
    if (!bIsAlerted && bCanReceiveAlerts)
    {
        CheckForThreats();
    }
    
    // Propagate alerts to nearby allies
    if (bIsAlerted)
    {
        PropagateAlert();
    }
}

void UCombat_AlertSystem::TriggerAlert(ECombat_AlertLevel NewAlertLevel, AActor* ThreatActor, FVector ThreatLocation)
{
    if (!bCanReceiveAlerts)
    {
        return;
    }
    
    // Only escalate alert level, never reduce it during active alert
    if (NewAlertLevel > AlertLevel || !bIsAlerted)
    {
        AlertLevel = NewAlertLevel;
        CurrentThreat = ThreatActor;
        ThreatPosition = ThreatLocation;
        LastAlertTime = GetWorld()->GetTimeSeconds();
        bIsAlerted = true;
        
        // Broadcast alert event
        OnAlertTriggered.Broadcast(AlertLevel, ThreatActor, ThreatLocation);
        
        UE_LOG(LogTemp, Warning, TEXT("%s triggered alert level %d due to threat at %s"), 
               *GetOwner()->GetName(), 
               (int32)AlertLevel, 
               *ThreatLocation.ToString());
    }
}

void UCombat_AlertSystem::ClearAlert()
{
    if (bIsAlerted)
    {
        AlertLevel = ECombat_AlertLevel::None;
        CurrentThreat = nullptr;
        ThreatPosition = FVector::ZeroVector;
        bIsAlerted = false;
        
        OnAlertCleared.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("%s alert cleared"), *GetOwner()->GetName());
    }
}

void UCombat_AlertSystem::UpdateAlertState(float DeltaTime)
{
    if (!bIsAlerted)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceAlert = CurrentTime - LastAlertTime;
    
    // Check if alert should expire
    if (TimeSinceAlert > AlertDuration)
    {
        // Verify threat is still valid
        if (!IsValid(CurrentThreat) || !IsThreatStillPresent())
        {
            ClearAlert();
            return;
        }
    }
    
    // Update threat position if threat is still valid
    if (IsValid(CurrentThreat))
    {
        ThreatPosition = CurrentThreat->GetActorLocation();
    }
}

void UCombat_AlertSystem::CheckForThreats()
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors within alert radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (!IsValid(Actor) || Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= AlertRadius)
        {
            // Check if this is a threat (player or hostile actor)
            if (IsThreatActor(Actor))
            {
                ECombat_AlertLevel ThreatLevel = CalculateThreatLevel(Actor, Distance);
                TriggerAlert(ThreatLevel, Actor, Actor->GetActorLocation());
                break; // Only handle one threat at a time
            }
        }
    }
}

void UCombat_AlertSystem::PropagateAlert()
{
    if (!bIsAlerted || !GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find nearby allies with alert systems
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (!IsValid(Actor) || Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= AlertRadius)
        {
            // Check if this is an ally
            if (IsAllyActor(Actor))
            {
                UCombat_AlertSystem* AllyAlertSystem = Actor->FindComponentByClass<UCombat_AlertSystem>();
                if (AllyAlertSystem && !AllyAlertSystem->bIsAlerted)
                {
                    // Propagate alert with slightly reduced level
                    ECombat_AlertLevel PropagatedLevel = AlertLevel;
                    if (AlertLevel == ECombat_AlertLevel::High)
                    {
                        PropagatedLevel = ECombat_AlertLevel::Medium;
                    }
                    else if (AlertLevel == ECombat_AlertLevel::Medium)
                    {
                        PropagatedLevel = ECombat_AlertLevel::Low;
                    }
                    
                    AllyAlertSystem->TriggerAlert(PropagatedLevel, CurrentThreat, ThreatPosition);
                }
            }
        }
    }
}

bool UCombat_AlertSystem::IsThreatActor(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    // Check for player character
    if (Actor->IsA<APawn>())
    {
        APawn* Pawn = Cast<APawn>(Actor);
        if (Pawn && Pawn->IsPlayerControlled())
        {
            return true;
        }
    }
    
    // Check actor tags for threat indicators
    if (Actor->Tags.Contains("Player") || 
        Actor->Tags.Contains("Threat") || 
        Actor->Tags.Contains("Enemy"))
    {
        return true;
    }
    
    return false;
}

bool UCombat_AlertSystem::IsAllyActor(AActor* Actor) const
{
    if (!IsValid(Actor) || !GetOwner())
    {
        return false;
    }
    
    // Check if both actors have similar tags (same species/faction)
    TArray<FName> OwnerTags = GetOwner()->Tags;
    TArray<FName> ActorTags = Actor->Tags;
    
    // Look for common faction/species tags
    for (const FName& OwnerTag : OwnerTags)
    {
        FString TagString = OwnerTag.ToString();
        if (TagString.Contains("COMBAT_AI") || 
            TagString.Contains("DINOSAUR") || 
            TagString.Contains("GROUP_"))
        {
            if (ActorTags.Contains(OwnerTag))
            {
                return true;
            }
        }
    }
    
    return false;
}

ECombat_AlertLevel UCombat_AlertSystem::CalculateThreatLevel(AActor* ThreatActor, float Distance) const
{
    if (!IsValid(ThreatActor))
    {
        return ECombat_AlertLevel::None;
    }
    
    // Base threat level on distance
    float ThreatRatio = Distance / AlertRadius;
    
    if (ThreatRatio <= 0.3f) // Very close
    {
        return ECombat_AlertLevel::High;
    }
    else if (ThreatRatio <= 0.6f) // Medium distance
    {
        return ECombat_AlertLevel::Medium;
    }
    else // Far but still within range
    {
        return ECombat_AlertLevel::Low;
    }
}

bool UCombat_AlertSystem::IsThreatStillPresent() const
{
    if (!IsValid(CurrentThreat) || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentThreat->GetActorLocation());
    return Distance <= AlertRadius * 1.2f; // Allow 20% buffer to prevent flickering
}

void UCombat_AlertSystem::SetAlertRadius(float NewRadius)
{
    AlertRadius = FMath::Max(NewRadius, 100.0f); // Minimum 100 units
}

void UCombat_AlertSystem::SetAlertDuration(float NewDuration)
{
    AlertDuration = FMath::Max(NewDuration, 1.0f); // Minimum 1 second
}

void UCombat_AlertSystem::EnableAlerts()
{
    bCanReceiveAlerts = true;
}

void UCombat_AlertSystem::DisableAlerts()
{
    bCanReceiveAlerts = false;
    ClearAlert();
}