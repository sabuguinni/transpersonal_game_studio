#include "Combat_CombatManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UCombat_CombatManager::UCombat_CombatManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Default combat parameters
    DetectionRange = 2000.0f;
    AttackRange = 300.0f;
    AttackDamage = 25.0f;
    AttackCooldown = 2.0f;
    
    // Initialize state
    CurrentCombatState = ECombat_CombatState::Idle;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    bIsPackLeader = false;
    PackLeader = nullptr;
}

void UCombat_CombatManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat system
    SetCombatState(ECombat_CombatState::Idle);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Manager initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_CombatManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    // Update threat tracking
    UpdateThreatTracking(DeltaTime);
    
    // Process current combat state
    ProcessCombatState(DeltaTime);
    
    // Scan for new threats periodically
    static float ThreatScanTimer = 0.0f;
    ThreatScanTimer += DeltaTime;
    if (ThreatScanTimer >= 1.0f) // Scan every second
    {
        ScanForThreats();
        ThreatScanTimer = 0.0f;
    }
}

void UCombat_CombatManager::SetCombatState(ECombat_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombat_CombatState OldState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s: Combat state changed from %d to %d"), 
               *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
        
        // State transition logic
        switch (NewState)
        {
            case ECombat_CombatState::Idle:
                CurrentTarget = nullptr;
                break;
                
            case ECombat_CombatState::Searching:
                // Start actively looking for threats
                break;
                
            case ECombat_CombatState::Stalking:
                // Begin stalking behavior
                break;
                
            case ECombat_CombatState::Attacking:
                // Prepare for attack
                break;
                
            case ECombat_CombatState::Fleeing:
                // Begin flee behavior
                break;
                
            case ECombat_CombatState::Defending:
                // Defensive posture
                break;
        }
    }
}

void UCombat_CombatManager::ScanForThreats()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors in detection range
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        OwnerLocation,
        DetectionRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>{GetOwner()},
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (IsValidThreat(Actor))
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            AddThreat(Actor, ThreatLevel);
        }
    }
}

void UCombat_CombatManager::AddThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor || !IsValidThreat(ThreatActor))
    {
        return;
    }

    // Check if threat already exists
    for (FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor == ThreatActor)
        {
            // Update existing threat
            Threat.ThreatLevel = ThreatLevel;
            Threat.Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
            Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
            return;
        }
    }

    // Add new threat
    FCombat_ThreatData NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLevel = ThreatLevel;
    NewThreat.Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
    
    KnownThreats.Add(NewThreat);
    
    UE_LOG(LogTemp, Warning, TEXT("%s: New threat detected - %s (Level: %.1f)"), 
           *GetOwner()->GetName(), *ThreatActor->GetName(), ThreatLevel);
}

void UCombat_CombatManager::RemoveThreat(AActor* ThreatActor)
{
    KnownThreats.RemoveAll([ThreatActor](const FCombat_ThreatData& Threat)
    {
        return Threat.ThreatActor == ThreatActor;
    });
    
    if (CurrentTarget == ThreatActor)
    {
        CurrentTarget = nullptr;
        SetCombatState(ECombat_CombatState::Searching);
    }
}

AActor* UCombat_CombatManager::GetPrimaryThreat() const
{
    AActor* PrimaryThreat = nullptr;
    float HighestThreat = 0.0f;

    for (const FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor && Threat.ThreatLevel > HighestThreat)
        {
            HighestThreat = Threat.ThreatLevel;
            PrimaryThreat = Threat.ThreatActor;
        }
    }

    return PrimaryThreat;
}

float UCombat_CombatManager::GetThreatLevel(AActor* Actor) const
{
    for (const FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor == Actor)
        {
            return Threat.ThreatLevel;
        }
    }
    return 0.0f;
}

bool UCombat_CombatManager::CanAttackTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
    
    return Distance <= AttackRange && TimeSinceLastAttack >= AttackCooldown;
}

void UCombat_CombatManager::InitiateAttack(AActor* Target)
{
    if (!CanAttackTarget(Target))
    {
        return;
    }

    CurrentTarget = Target;
    SetCombatState(ECombat_CombatState::Attacking);
    
    UE_LOG(LogTemp, Warning, TEXT("%s: Initiating attack on %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombat_CombatManager::ExecuteAttack()
{
    if (!CurrentTarget || !CanAttackTarget(CurrentTarget))
    {
        return;
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Apply damage if target has health component
    if (APawn* TargetPawn = Cast<APawn>(CurrentTarget))
    {
        // For now, just log the attack - damage system would be implemented separately
        UE_LOG(LogTemp, Warning, TEXT("%s: Attacking %s for %.1f damage"), 
               *GetOwner()->GetName(), *CurrentTarget->GetName(), AttackDamage);
    }
    
    // Return to stalking after attack
    SetCombatState(ECombat_CombatState::Stalking);
}

void UCombat_CombatManager::DefendPosition()
{
    SetCombatState(ECombat_CombatState::Defending);
    UE_LOG(LogTemp, Log, TEXT("%s: Defending position"), *GetOwner()->GetName());
}

void UCombat_CombatManager::FleeFromThreat()
{
    SetCombatState(ECombat_CombatState::Fleeing);
    CurrentTarget = nullptr;
    UE_LOG(LogTemp, Warning, TEXT("%s: Fleeing from threat"), *GetOwner()->GetName());
}

void UCombat_CombatManager::CoordinatePackAttack(const TArray<APawn*>& PackMembers, AActor* Target)
{
    if (!bIsPackLeader || !Target)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("%s: Coordinating pack attack with %d members on %s"), 
           *GetOwner()->GetName(), PackMembers.Num(), *Target->GetName());

    // Coordinate attack positions around target
    FVector TargetLocation = Target->GetActorLocation();
    float AngleStep = 360.0f / FMath::Max(PackMembers.Num(), 1);
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i])
        {
            // Calculate attack position around target
            float Angle = AngleStep * i;
            FVector AttackPosition = TargetLocation + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * AttackRange,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * AttackRange,
                0.0f
            );
            
            // Command pack member to attack from this position
            if (UCombat_CombatManager* MemberCombat = PackMembers[i]->FindComponentByClass<UCombat_CombatManager>())
            {
                MemberCombat->InitiateAttack(Target);
            }
        }
    }
}

void UCombat_CombatManager::SetPackLeader(APawn* Leader)
{
    PackLeader = Leader;
    bIsPackLeader = (Leader == GetOwner());
    
    UE_LOG(LogTemp, Log, TEXT("%s: Pack leader set to %s"), 
           *GetOwner()->GetName(), Leader ? *Leader->GetName() : TEXT("None"));
}

void UCombat_CombatManager::UpdateThreatTracking(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old threats that haven't been seen recently
    KnownThreats.RemoveAll([CurrentTime](const FCombat_ThreatData& Threat)
    {
        return !Threat.ThreatActor || (CurrentTime - Threat.LastSeenTime) > 10.0f;
    });
    
    // Update distances for remaining threats
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    for (FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor)
        {
            Threat.Distance = FVector::Dist(OwnerLocation, Threat.ThreatActor->GetActorLocation());
        }
    }
}

void UCombat_CombatManager::ProcessCombatState(float DeltaTime)
{
    switch (CurrentCombatState)
    {
        case ECombat_CombatState::Idle:
            if (KnownThreats.Num() > 0)
            {
                SetCombatState(ECombat_CombatState::Searching);
            }
            break;
            
        case ECombat_CombatState::Searching:
            {
                AActor* PrimaryThreat = GetPrimaryThreat();
                if (PrimaryThreat)
                {
                    CurrentTarget = PrimaryThreat;
                    SetCombatState(ECombat_CombatState::Stalking);
                }
                else if (KnownThreats.Num() == 0)
                {
                    SetCombatState(ECombat_CombatState::Idle);
                }
            }
            break;
            
        case ECombat_CombatState::Stalking:
            if (CurrentTarget)
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
                if (Distance <= AttackRange)
                {
                    InitiateAttack(CurrentTarget);
                }
                else if (Distance > DetectionRange)
                {
                    SetCombatState(ECombat_CombatState::Searching);
                }
            }
            else
            {
                SetCombatState(ECombat_CombatState::Searching);
            }
            break;
            
        case ECombat_CombatState::Attacking:
            if (CanAttackTarget(CurrentTarget))
            {
                ExecuteAttack();
            }
            break;
            
        case ECombat_CombatState::Fleeing:
            // Flee logic would be implemented here
            if (KnownThreats.Num() == 0)
            {
                SetCombatState(ECombat_CombatState::Idle);
            }
            break;
            
        case ECombat_CombatState::Defending:
            // Defensive logic would be implemented here
            break;
    }
}

bool UCombat_CombatManager::IsValidThreat(AActor* Actor) const
{
    if (!Actor || Actor == GetOwner())
    {
        return false;
    }

    // Check if it's a player character
    if (APawn* Pawn = Cast<APawn>(Actor))
    {
        if (Pawn->IsPlayerControlled())
        {
            return true;
        }
    }

    // Check if it's another dinosaur (different species)
    // This would need more sophisticated logic based on species relationships
    return false;
}

float UCombat_CombatManager::CalculateThreatLevel(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }

    float ThreatLevel = 1.0f;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
    
    // Player characters are high threat
    if (APawn* Pawn = Cast<APawn>(Actor))
    {
        if (Pawn->IsPlayerControlled())
        {
            ThreatLevel = 10.0f;
        }
    }
    
    // Closer threats are more dangerous
    ThreatLevel *= FMath::Max(0.1f, (DetectionRange - Distance) / DetectionRange);
    
    return ThreatLevel;
}