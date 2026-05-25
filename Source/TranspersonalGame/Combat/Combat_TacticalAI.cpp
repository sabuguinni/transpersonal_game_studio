#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = ECombat_TacticalState::Idle;
    DetectionRadius = 5000.0f;
    AttackRange = 800.0f;
    RetreatThreshold = 200.0f;
    LastThreatCheck = 0.0f;
    ThreatCheckInterval = 1.0f;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat Tactical AI initialized for: %s"), *GetOwner()->GetName());
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Periodic threat detection
    if (CurrentTime - LastThreatCheck >= ThreatCheckInterval)
    {
        DetectThreats();
        UpdateTacticalState();
        LastThreatCheck = CurrentTime;
    }

    // Execute current state behavior
    switch (CurrentState)
    {
        case ECombat_TacticalState::Attack:
            ExecuteAttackBehavior();
            break;
        case ECombat_TacticalState::Retreat:
            ExecuteRetreatBehavior();
            break;
        default:
            break;
    }
}

void UCombat_TacticalAI::DetectThreats()
{
    if (!GetOwner()) return;

    ThreatList.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<AActor*> FoundActors;
    
    // Find all pawns in detection radius
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= DetectionRadius)
        {
            FCombat_ThreatAssessment Threat;
            Threat.ThreatActor = Actor;
            Threat.Distance = Distance;
            
            // Calculate threat level based on distance and actor type
            if (Actor->IsA<APawn>() && Actor->GetName().Contains("Character"))
            {
                Threat.ThreatLevel = FMath::Clamp(1.0f - (Distance / DetectionRadius), 0.1f, 1.0f);
            }
            else
            {
                Threat.ThreatLevel = 0.3f;
            }
            
            ThreatList.Add(Threat);
        }
    }
    
    // Sort by threat level
    ThreatList.Sort([](const FCombat_ThreatAssessment& A, const FCombat_ThreatAssessment& B) {
        return A.ThreatLevel > B.ThreatLevel;
    });
}

void UCombat_TacticalAI::UpdateTacticalState()
{
    AActor* PrimaryThreat = GetPrimaryThreat();
    
    if (!PrimaryThreat)
    {
        CurrentState = ECombat_TacticalState::Idle;
        return;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryThreat->GetActorLocation());
    
    if (Distance <= AttackRange)
    {
        CurrentState = ECombat_TacticalState::Attack;
    }
    else if (Distance <= DetectionRadius)
    {
        CurrentState = ECombat_TacticalState::Hunt;
    }
    else
    {
        CurrentState = ECombat_TacticalState::Patrol;
    }
    
    // Check retreat conditions
    if (ThreatList.Num() > 3 || (PrimaryThreat && Distance < RetreatThreshold))
    {
        CurrentState = ECombat_TacticalState::Retreat;
    }
}

void UCombat_TacticalAI::ExecuteAttackBehavior()
{
    AActor* Target = GetPrimaryThreat();
    if (!Target) return;
    
    // Move towards target
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Simple movement towards target
    FVector NewLocation = OwnerLocation + Direction * 200.0f * GetWorld()->GetDeltaSeconds();
    GetOwner()->SetActorLocation(NewLocation);
    
    // Face the target
    FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    GetOwner()->SetActorRotation(LookRotation);
    
    UE_LOG(LogTemp, Log, TEXT("Combat AI: %s attacking %s"), *GetOwner()->GetName(), *Target->GetName());
}

void UCombat_TacticalAI::ExecuteRetreatBehavior()
{
    AActor* Threat = GetPrimaryThreat();
    if (!Threat) return;
    
    // Move away from threat
    FVector ThreatLocation = Threat->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector Direction = (OwnerLocation - ThreatLocation).GetSafeNormal();
    
    FVector NewLocation = OwnerLocation + Direction * 300.0f * GetWorld()->GetDeltaSeconds();
    GetOwner()->SetActorLocation(NewLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Combat AI: %s retreating from %s"), *GetOwner()->GetName(), *Threat->GetName());
}

AActor* UCombat_TacticalAI::GetPrimaryThreat()
{
    if (ThreatList.Num() > 0)
    {
        return ThreatList[0].ThreatActor;
    }
    return nullptr;
}