#include "Narr_EnvironmentalNarrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"

UNarr_EnvironmentalNarrator::UNarr_EnvironmentalNarrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CheckInterval = 2.0f;
    bEnableEnvironmentalNarration = true;
    NarrationVolume = 0.8f;
    NarrationDuration = 3.0f;
    TimeSinceLastCheck = 0.0f;
    PlayerPawn = nullptr;

    // Initialize default environmental triggers
    FNarr_EnvironmentalTrigger ForestTrigger;
    ForestTrigger.EventType = ENarr_EnvironmentalEvent::EnteringForest;
    ForestTrigger.NarrativeText = TEXT("The dense canopy above blocks out the sun. Ancient trees whisper of dangers that lurk in shadows.");
    ForestTrigger.TriggerRadius = 300.0f;
    ForestTrigger.CooldownTime = 45.0f;
    EnvironmentalTriggers.Add(ForestTrigger);

    FNarr_EnvironmentalTrigger WaterTrigger;
    WaterTrigger.EventType = ENarr_EnvironmentalEvent::NearWater;
    WaterTrigger.NarrativeText = TEXT("The sound of flowing water reaches your ears. Life gathers here, but so do predators seeking easy prey.");
    WaterTrigger.TriggerRadius = 400.0f;
    WaterTrigger.CooldownTime = 60.0f;
    EnvironmentalTriggers.Add(WaterTrigger);

    FNarr_EnvironmentalTrigger HighGroundTrigger;
    HighGroundTrigger.EventType = ENarr_EnvironmentalEvent::HighGround;
    HighGroundTrigger.NarrativeText = TEXT("From this elevated position, you can survey the land. The view reveals both opportunities and threats.");
    HighGroundTrigger.TriggerRadius = 200.0f;
    HighGroundTrigger.CooldownTime = 90.0f;
    EnvironmentalTriggers.Add(HighGroundTrigger);

    FNarr_EnvironmentalTrigger DangerTrigger;
    DangerTrigger.EventType = ENarr_EnvironmentalEvent::DangerousArea;
    DangerTrigger.NarrativeText = TEXT("The air grows thick with tension. Broken bones and claw marks warn of apex predators nearby.");
    DangerTrigger.TriggerRadius = 500.0f;
    DangerTrigger.CooldownTime = 30.0f;
    EnvironmentalTriggers.Add(DangerTrigger);
}

void UNarr_EnvironmentalNarrator::BeginPlay()
{
    Super::BeginPlay();
    
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }

    // Initialize cooldown tracking
    for (const FNarr_EnvironmentalTrigger& Trigger : EnvironmentalTriggers)
    {
        LastTriggerTimes.Add(Trigger.EventType, -1000.0f);
    }
}

void UNarr_EnvironmentalNarrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableEnvironmentalNarration || !PlayerPawn)
    {
        return;
    }

    TimeSinceLastCheck += DeltaTime;
    
    if (TimeSinceLastCheck >= CheckInterval)
    {
        CheckEnvironmentalConditions();
        TimeSinceLastCheck = 0.0f;
    }
}

void UNarr_EnvironmentalNarrator::CheckEnvironmentalConditions()
{
    if (!PlayerPawn)
    {
        return;
    }

    for (const FNarr_EnvironmentalTrigger& Trigger : EnvironmentalTriggers)
    {
        if (!Trigger.bIsActive || IsEventOnCooldown(Trigger.EventType))
        {
            continue;
        }

        bool bShouldTrigger = false;

        switch (Trigger.EventType)
        {
            case ENarr_EnvironmentalEvent::EnteringForest:
                bShouldTrigger = IsPlayerInForest();
                break;
            case ENarr_EnvironmentalEvent::NearWater:
                bShouldTrigger = IsPlayerNearWater();
                break;
            case ENarr_EnvironmentalEvent::HighGround:
                bShouldTrigger = IsPlayerOnHighGround();
                break;
            case ENarr_EnvironmentalEvent::DangerousArea:
                bShouldTrigger = IsPlayerInDangerousArea();
                break;
            default:
                break;
        }

        if (bShouldTrigger)
        {
            ProcessEnvironmentalEvent(Trigger);
        }
    }
}

void UNarr_EnvironmentalNarrator::ProcessEnvironmentalEvent(const FNarr_EnvironmentalTrigger& Trigger)
{
    if (UWorld* World = GetWorld())
    {
        float CurrentTime = World->GetTimeSeconds();
        LastTriggerTimes[Trigger.EventType] = CurrentTime;

        // Display narrative text
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 
                NarrationDuration, 
                FColor::Yellow, 
                FString::Printf(TEXT("[Environmental] %s"), *Trigger.NarrativeText)
            );
        }

        UE_LOG(LogTemp, Log, TEXT("Environmental Narrator: %s"), *Trigger.NarrativeText);
    }
}

void UNarr_EnvironmentalNarrator::TriggerEnvironmentalNarration(ENarr_EnvironmentalEvent EventType, const FString& CustomText)
{
    if (!bEnableEnvironmentalNarration)
    {
        return;
    }

    FString NarrativeText = CustomText;
    if (NarrativeText.IsEmpty())
    {
        // Find default text for this event type
        for (const FNarr_EnvironmentalTrigger& Trigger : EnvironmentalTriggers)
        {
            if (Trigger.EventType == EventType)
            {
                NarrativeText = Trigger.NarrativeText;
                break;
            }
        }
    }

    if (GEngine && !NarrativeText.IsEmpty())
    {
        GEngine->AddOnScreenDebugMessage(
            -1, 
            NarrationDuration, 
            FColor::Cyan, 
            FString::Printf(TEXT("[Manual Environmental] %s"), *NarrativeText)
        );
    }

    if (UWorld* World = GetWorld())
    {
        LastTriggerTimes[EventType] = World->GetTimeSeconds();
    }
}

void UNarr_EnvironmentalNarrator::AddEnvironmentalTrigger(ENarr_EnvironmentalEvent EventType, const FString& NarrativeText, float Radius, float Cooldown)
{
    FNarr_EnvironmentalTrigger NewTrigger;
    NewTrigger.EventType = EventType;
    NewTrigger.NarrativeText = NarrativeText;
    NewTrigger.TriggerRadius = Radius;
    NewTrigger.CooldownTime = Cooldown;
    NewTrigger.bIsActive = true;

    EnvironmentalTriggers.Add(NewTrigger);
    LastTriggerTimes.Add(EventType, -1000.0f);
}

void UNarr_EnvironmentalNarrator::SetEnvironmentalNarrationEnabled(bool bEnabled)
{
    bEnableEnvironmentalNarration = bEnabled;
}

bool UNarr_EnvironmentalNarrator::IsEventOnCooldown(ENarr_EnvironmentalEvent EventType) const
{
    if (const float* LastTriggerTime = LastTriggerTimes.Find(EventType))
    {
        if (UWorld* World = GetWorld())
        {
            float CurrentTime = World->GetTimeSeconds();
            
            for (const FNarr_EnvironmentalTrigger& Trigger : EnvironmentalTriggers)
            {
                if (Trigger.EventType == EventType)
                {
                    return (CurrentTime - *LastTriggerTime) < Trigger.CooldownTime;
                }
            }
        }
    }
    
    return false;
}

bool UNarr_EnvironmentalNarrator::IsPlayerInForest() const
{
    if (!PlayerPawn)
    {
        return false;
    }

    // Check for nearby tree actors or forest-tagged actors
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<AActor*> NearbyActors;
    
    // Simple forest detection - look for multiple StaticMeshActors nearby (trees)
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && FVector::Dist(Actor->GetActorLocation(), PlayerLocation) < 300.0f)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("forest")) || ActorName.Contains(TEXT("wood")))
            {
                NearbyActors.Add(Actor);
            }
        }
    }

    return NearbyActors.Num() >= 3; // Consider it forest if 3+ tree-like objects nearby
}

bool UNarr_EnvironmentalNarrator::IsPlayerNearWater() const
{
    if (!PlayerPawn)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check for water-tagged actors or low elevation areas
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && FVector::Dist(Actor->GetActorLocation(), PlayerLocation) < 400.0f)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("water")) || ActorName.Contains(TEXT("river")) || ActorName.Contains(TEXT("lake")))
            {
                return true;
            }
        }
    }

    return false;
}

bool UNarr_EnvironmentalNarrator::IsPlayerOnHighGround() const
{
    if (!PlayerPawn)
    {
        return false;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Simple high ground detection - check if player is above a certain Z threshold
    return PlayerLocation.Z > 500.0f; // Arbitrary high ground threshold
}

bool UNarr_EnvironmentalNarrator::IsPlayerInDangerousArea() const
{
    if (!PlayerPawn)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check for nearby predator actors or danger-tagged areas
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn != PlayerPawn && FVector::Dist(Pawn->GetActorLocation(), PlayerLocation) < 500.0f)
        {
            FString PawnName = Pawn->GetName().ToLower();
            if (PawnName.Contains(TEXT("trex")) || PawnName.Contains(TEXT("raptor")) || PawnName.Contains(TEXT("predator")))
            {
                return true;
            }
        }
    }

    return false;
}