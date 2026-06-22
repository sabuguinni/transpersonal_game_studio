#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreat = ENPC_ThreatLevel::None;
    CurrentWaypointIndex = 0;
    StateTimer = 0.0f;
    DialogueCooldownTimer = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Seed default patrol waypoints around spawn location
    if (Waypoints.Num() == 0 && GetOwner())
    {
        FVector Origin = GetOwner()->GetActorLocation();
        float R = PatrolRadius * 0.5f;

        FNPC_WaypointData WP0, WP1, WP2, WP3;
        WP0.Location = Origin + FVector(R, 0, 0);   WP0.WaitDuration = 3.0f;
        WP1.Location = Origin + FVector(0, R, 0);   WP1.WaitDuration = 2.0f;
        WP2.Location = Origin + FVector(-R, 0, 0);  WP2.WaitDuration = 4.0f;
        WP3.Location = Origin + FVector(0, -R, 0);  WP3.WaitDuration = 2.5f;

        Waypoints.Add(WP0);
        Waypoints.Add(WP1);
        Waypoints.Add(WP2);
        Waypoints.Add(WP3);
    }

    // Seed default dialogue lines
    if (DialogueLines.Num() == 0)
    {
        AddDialogueLine(TEXT("Stay quiet. Something is out there."), ENPC_BehaviorState::Alert, 20.0f);
        AddDialogueLine(TEXT("The herd moved north this morning."), ENPC_BehaviorState::Patrol, 45.0f);
        AddDialogueLine(TEXT("Keep your spear ready."), ENPC_BehaviorState::Idle, 60.0f);
        AddDialogueLine(TEXT("Run! Do not look back!"), ENPC_BehaviorState::Flee, 5.0f);
    }

    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    if (DialogueCooldownTimer > 0.0f)
    {
        DialogueCooldownTimer -= DeltaTime;
    }

    // Threat detection every 2 seconds
    if (FMath::Fmod(StateTimer, 2.0f) < DeltaTime)
    {
        bool bThreatDetected = DetectThreatInRadius(DetectionRadius);
        if (bThreatDetected && CurrentState != ENPC_BehaviorState::Flee)
        {
            if (CurrentThreat >= ENPC_ThreatLevel::High)
            {
                SetBehaviorState(ENPC_BehaviorState::Flee);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Alert);
            }
        }
        else if (!bThreatDetected && CurrentState == ENPC_BehaviorState::Alert)
        {
            // Calm down after 10s with no threat
            if (StateTimer > 10.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
        }
    }

    // State tick
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Patrol:    TickPatrol(DeltaTime);  break;
        case ENPC_BehaviorState::Alert:     TickAlert(DeltaTime);   break;
        case ENPC_BehaviorState::Flee:      TickFlee(DeltaTime);    break;
        case ENPC_BehaviorState::Idle:      TickIdle(DeltaTime);    break;
        default: break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;
    SelectDialogueForState();

    UE_LOG(LogTemp, Log, TEXT("NPC [%s] State -> %d"),
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
        (int32)NewState);
}

void UNPCBehaviorComponent::UpdateThreatLevel(ENPC_ThreatLevel NewThreat)
{
    CurrentThreat = NewThreat;

    if (NewThreat >= ENPC_ThreatLevel::High)
    {
        AlertNearbyNPCs(AlertRadius);
    }
}

void UNPCBehaviorComponent::AddWaypoint(FVector Location, float WaitTime)
{
    FNPC_WaypointData WP;
    WP.Location = Location;
    WP.WaitDuration = WaitTime;
    WP.bLookAround = true;
    Waypoints.Add(WP);
}

void UNPCBehaviorComponent::ClearWaypoints()
{
    Waypoints.Empty();
    CurrentWaypointIndex = 0;
}

void UNPCBehaviorComponent::AddDialogueLine(FString Text, ENPC_BehaviorState TriggerState, float Cooldown)
{
    FNPC_DialogueLine Line;
    Line.LineText = Text;
    Line.TriggerState = TriggerState;
    Line.Cooldown = Cooldown;
    DialogueLines.Add(Line);
}

FString UNPCBehaviorComponent::GetCurrentDialogue() const
{
    return ActiveDialogueLine;
}

bool UNPCBehaviorComponent::DetectThreatInRadius(float Radius)
{
    if (!GetOwner() || !GetWorld()) return false;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        Radius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>(),
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == GetOwner()) continue;

        FString Label = Actor->GetActorLabel();
        // Dinosaurs are threats
        if (Label.Contains(TEXT("TRex")) || Label.Contains(TEXT("Raptor")) ||
            Label.Contains(TEXT("Rex"))  || Label.Contains(TEXT("Dino")))
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Dist < Radius * 0.4f)
            {
                UpdateThreatLevel(ENPC_ThreatLevel::Critical);
            }
            else if (Dist < Radius * 0.7f)
            {
                UpdateThreatLevel(ENPC_ThreatLevel::High);
            }
            else
            {
                UpdateThreatLevel(ENPC_ThreatLevel::Medium);
            }
            return true;
        }
    }

    if (CurrentThreat != ENPC_ThreatLevel::None)
    {
        UpdateThreatLevel(ENPC_ThreatLevel::None);
    }
    return false;
}

void UNPCBehaviorComponent::AlertNearbyNPCs(float Radius)
{
    if (!GetOwner() || !GetWorld()) return;

    TArray<AActor*> NearbyActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        Radius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>(),
        NearbyActors
    );

    int32 AlertCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == GetOwner()) continue;

        UNPCBehaviorComponent* OtherNPC = Actor->FindComponentByClass<UNPCBehaviorComponent>();
        if (OtherNPC && OtherNPC->CurrentState != ENPC_BehaviorState::Flee)
        {
            OtherNPC->SetBehaviorState(ENPC_BehaviorState::Alert);
            OtherNPC->UpdateThreatLevel(ENPC_ThreatLevel::High);
            AlertCount++;
        }
    }

    if (AlertCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC [%s] alerted %d nearby NPCs"),
            *GetOwner()->GetActorLabel(), AlertCount);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (Waypoints.Num() == 0) return;

    // Advance waypoint index every 8 seconds (movement handled by AI controller)
    if (StateTimer > 8.0f)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
        StateTimer = 0.0f;
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert: stop moving, scan for threats
    // After 15s with no escalation, return to patrol
    if (StateTimer > 15.0f && CurrentThreat <= ENPC_ThreatLevel::Low)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Flee: run away from threat
    // After 20s, check if threat is gone
    if (StateTimer > 20.0f)
    {
        bool bStillThreat = DetectThreatInRadius(DetectionRadius * 1.5f);
        if (!bStillThreat)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
        else
        {
            StateTimer = 10.0f; // Keep fleeing, reset partial timer
        }
    }
}

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // Idle: occasionally start patrolling
    if (StateTimer > 12.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::SelectDialogueForState()
{
    if (DialogueCooldownTimer > 0.0f) return;

    for (const FNPC_DialogueLine& Line : DialogueLines)
    {
        if (Line.TriggerState == CurrentState)
        {
            ActiveDialogueLine = Line.LineText;
            DialogueCooldownTimer = Line.Cooldown;
            UE_LOG(LogTemp, Log, TEXT("NPC [%s] says: %s"),
                GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
                *ActiveDialogueLine);
            return;
        }
    }
}
