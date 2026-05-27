#include "NarrativeEventTrigger.h"
#include "../Character/TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNarrativeEventTrigger::UNarrativeEventTrigger()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    TriggerCondition = FNarr_TriggerCondition();
    NarrativeEventID = TEXT("DefaultEvent");
    DialogueText = TEXT("Something stirs in the ancient lands...");
    SpeakerName = TEXT("AncientNarrator");
    AudioDuration = 5.0f;
    bHasTriggered = false;
    LastCheckTime = 0.0f;
    CheckInterval = 1.0f;
}

void UNarrativeEventTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    LastCheckTime = GetWorld()->GetTimeSeconds();
}

void UNarrativeEventTrigger::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bHasTriggered && TriggerCondition.bOnceOnly)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCheckTime >= CheckInterval)
    {
        LastCheckTime = CurrentTime;
        
        if (CheckTriggerConditions())
        {
            TriggerNarrativeEvent();
        }
    }
}

bool UNarrativeEventTrigger::CheckTriggerConditions()
{
    switch (TriggerCondition.TriggerType)
    {
        case ENarr_TriggerType::LocationBased:
            return IsPlayerInRange();
            
        case ENarr_TriggerType::SurvivalStatus:
            return CheckSurvivalStatus();
            
        case ENarr_TriggerType::DinosaurEncounter:
            return CheckDinosaurPresence();
            
        case ENarr_TriggerType::ResourceDiscovery:
            return IsPlayerInRange(); // Simplified for now
            
        case ENarr_TriggerType::TimeOfDay:
            return true; // Simplified - always trigger for time-based events
            
        case ENarr_TriggerType::WeatherChange:
            return true; // Simplified - always trigger for weather events
            
        default:
            return false;
    }
}

void UNarrativeEventTrigger::TriggerNarrativeEvent()
{
    if (bHasTriggered && TriggerCondition.bOnceOnly)
    {
        return;
    }

    bHasTriggered = true;
    
    // Broadcast the narrative event
    OnNarrativeTriggered.Broadcast(NarrativeEventID, DialogueText);
    
    // Log the event
    UE_LOG(LogTemp, Warning, TEXT("Narrative Event Triggered: %s - %s"), *NarrativeEventID, *DialogueText);
    
    // Debug visual feedback
    if (GetWorld())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        DrawDebugSphere(GetWorld(), OwnerLocation, 100.0f, 12, FColor::Yellow, false, 3.0f);
    }
}

void UNarrativeEventTrigger::ResetTrigger()
{
    bHasTriggered = false;
    LastCheckTime = GetWorld()->GetTimeSeconds();
}

bool UNarrativeEventTrigger::IsPlayerInRange()
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        return false;
    }

    float Distance = GetDistanceToPlayer();
    bool bInRange = Distance <= TriggerCondition.TriggerRadius;
    
    if (bInRange && TriggerCondition.bRequiresLineOfSight)
    {
        return HasLineOfSightToPlayer();
    }
    
    return bInRange;
}

bool UNarrativeEventTrigger::CheckSurvivalStatus()
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        return false;
    }

    // Check if player is in critical survival state
    bool bLowHealth = Player->GetHealth() <= TriggerCondition.HealthThreshold;
    bool bHighHunger = Player->GetHunger() >= TriggerCondition.HungerThreshold;
    bool bHighFear = Player->GetFear() >= TriggerCondition.FearThreshold;
    
    return bLowHealth || bHighHunger || bHighFear;
}

bool UNarrativeEventTrigger::CheckDinosaurPresence()
{
    if (!GetWorld())
    {
        return false;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find all actors within trigger radius
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(TriggerCondition.TriggerRadius),
        QueryParams
    );
    
    if (bHasOverlaps)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor() && Result.GetActor()->GetName().Contains(TriggerCondition.RequiredDinosaurType))
            {
                return true;
            }
        }
    }
    
    return false;
}

ATranspersonalCharacter* UNarrativeEventTrigger::GetPlayerCharacter()
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    return Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

float UNarrativeEventTrigger::GetDistanceToPlayer()
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player || !GetOwner())
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

bool UNarrativeEventTrigger::HasLineOfSightToPlayer()
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player || !GetOwner() || !GetWorld())
    {
        return false;
    }
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Player->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Player);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No obstruction means clear line of sight
}