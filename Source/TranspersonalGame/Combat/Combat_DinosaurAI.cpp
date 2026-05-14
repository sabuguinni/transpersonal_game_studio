#include "Combat_DinosaurAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Combat_CombatManager.h"
#include "../NPC/NPC_BehaviorTreeManager.h"

UCombat_DinosaurAI::UCombat_DinosaurAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default stats
    Stats = FCombat_DinosaurStats();
    CurrentState = ECombat_DinosaurState::Idle;
    Species = ECombat_DinosaurSpecies::Raptor;
    
    // Initialize timers
    LastAttackTime = 0.0f;
    StateChangeTime = 0.0f;
    NextPatrolTime = 0.0f;
    ThreatScanTimer = 0.0f;
    PackUpdateTimer = 0.0f;
    
    // Initialize pack data
    PackID = -1;
    bIsPackLeader = false;
    PackMembers.Empty();
    
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    PatrolTarget = FVector::ZeroVector;
    bHasValidPatrolTarget = false;
}

void UCombat_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Store home location
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
    }
    
    // Apply species-specific modifiers
    ApplySpeciesModifiers();
    
    // Find combat manager
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            CombatManager = GameInstance->GetSubsystem<UCombat_CombatManager>();
        }
        
        // Find behavior manager
        BehaviorManager = World->GetSubsystem<UNPC_BehaviorTreeManager>();
    }
    
    // Initialize state change time
    StateChangeTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAI initialized for %s (Species: %d)"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           static_cast<int32>(Species));
}

void UCombat_DinosaurAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Update AI behavior based on current state
    UpdateAIBehavior(DeltaTime);
    
    // Update timers
    ThreatScanTimer += DeltaTime;
    PackUpdateTimer += DeltaTime;
    
    // Scan for threats every 2 seconds
    if (ThreatScanTimer >= 2.0f)
    {
        ScanForThreats();
        ThreatScanTimer = 0.0f;
    }
    
    // Update pack members every 5 seconds
    if (PackUpdateTimer >= 5.0f && PackID >= 0)
    {
        UpdatePackMembers();
        PackUpdateTimer = 0.0f;
    }
}

void UCombat_DinosaurAI::UpdateAIBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ECombat_DinosaurState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Investigating:
            HandleInvestigatingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Hunting:
            HandleHuntingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Feeding:
            HandleFeedingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Socializing:
            HandleSocializingState(DeltaTime);
            break;
    }
}

void UCombat_DinosaurAI::HandleIdleState(float DeltaTime)
{
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    // After 3-8 seconds of idle, start patrolling
    float IdleTime = FMath::RandRange(3.0f, 8.0f);
    if (TimeSinceStateChange > IdleTime)
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
    }
    
    // Scan for prey while idle
    ScanForPrey();
}

void UCombat_DinosaurAI::HandlePatrollingState(float DeltaTime)
{
    if (!bHasValidPatrolTarget || GetWorld()->GetTimeSeconds() >= NextPatrolTime)
    {
        // Generate new patrol target around home location
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f; // Keep on ground level
        RandomDirection.Normalize();
        
        float PatrolRadius = Stats.TerritorialRadius * 0.7f;
        PatrolTarget = HomeLocation + (RandomDirection * FMath::RandRange(200.0f, PatrolRadius));
        bHasValidPatrolTarget = true;
        NextPatrolTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(10.0f, 20.0f);
    }
    
    // Move towards patrol target
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (PatrolTarget - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 0.5f); // Half speed patrol
        
        // Check if reached patrol target
        float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), PatrolTarget);
        if (DistanceToTarget < 100.0f)
        {
            bHasValidPatrolTarget = false;
            SetDinosaurState(ECombat_DinosaurState::Idle);
        }
    }
    
    // Continue scanning for prey
    ScanForPrey();
}

void UCombat_DinosaurAI::HandleInvestigatingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
        return;
    }
    
    // Move towards investigation target
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (CurrentTarget->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 0.7f);
        
        float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
        
        // If close enough, decide whether to hunt or return to patrol
        if (DistanceToTarget < Stats.AttackRange * 1.5f)
        {
            if (FMath::RandRange(0.0f, 1.0f) < Stats.Aggressiveness)
            {
                SetDinosaurState(ECombat_DinosaurState::Hunting);
            }
            else
            {
                SetDinosaurState(ECombat_DinosaurState::Patrolling);
            }
        }
    }
}

void UCombat_DinosaurAI::HandleHuntingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
        return;
    }
    
    // Move towards target aggressively
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (CurrentTarget->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 1.0f); // Full speed hunt
        
        float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
        
        // Attack if in range
        if (IsInAttackRange(CurrentTarget))
        {
            SetDinosaurState(ECombat_DinosaurState::Attacking);
        }
        // Give up hunt if target is too far
        else if (DistanceToTarget > Stats.SightRange * 1.5f)
        {
            CurrentTarget = nullptr;
            SetDinosaurState(ECombat_DinosaurState::Patrolling);
        }
    }
}

void UCombat_DinosaurAI::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float AttackCooldown = 2.0f; // 2 seconds between attacks
    
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        if (IsInAttackRange(CurrentTarget))
        {
            AttackTarget();
            LastAttackTime = CurrentTime;
            
            // Issue pack attack command if leader
            if (bIsPackLeader && PackMembers.Num() > 0)
            {
                IssuePackCommand(ECombat_DinosaurState::Attacking, CurrentTarget);
            }
        }
        else
        {
            // Target moved away, return to hunting
            SetDinosaurState(ECombat_DinosaurState::Hunting);
        }
    }
    
    // Continue moving towards target during attack state
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (CurrentTarget->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 0.8f);
    }
}

void UCombat_DinosaurAI::HandleFleeingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
        return;
    }
    
    // Move away from threat
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (OwnerPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 1.0f); // Full speed flee
        
        float DistanceToThreat = GetDistanceToTarget(CurrentTarget);
        
        // Stop fleeing if far enough away
        if (DistanceToThreat > Stats.SightRange * 2.0f)
        {
            CurrentTarget = nullptr;
            SetDinosaurState(ECombat_DinosaurState::Patrolling);
        }
    }
}

void UCombat_DinosaurAI::HandleFeedingState(float DeltaTime)
{
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    // Feed for 5-10 seconds
    float FeedingTime = FMath::RandRange(5.0f, 10.0f);
    if (TimeSinceStateChange > FeedingTime)
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
    }
}

void UCombat_DinosaurAI::HandleSocializingState(float DeltaTime)
{
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    // Socialize for 3-7 seconds
    float SocializeTime = FMath::RandRange(3.0f, 7.0f);
    if (TimeSinceStateChange > SocializeTime)
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
    }
}

void UCombat_DinosaurAI::SetDinosaurState(ECombat_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_DinosaurState OldState = CurrentState;
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("%s state changed from %d to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               static_cast<int32>(OldState), 
               static_cast<int32>(NewState));
    }
}

void UCombat_DinosaurAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("%s acquired target: %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               *NewTarget->GetName());
    }
}

void UCombat_DinosaurAI::AttackTarget()
{
    if (!CurrentTarget || !GetOwner())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s attacks %s for %.1f damage!"), 
           *GetOwner()->GetName(), 
           *CurrentTarget->GetName(), 
           Stats.AttackDamage);
    
    // Apply damage to target if it has a DinosaurAI component
    if (UCombat_DinosaurAI* TargetAI = CurrentTarget->FindComponentByClass<UCombat_DinosaurAI>())
    {
        TargetAI->TakeDamage(Stats.AttackDamage, GetOwner());
    }
    
    // Register attack with combat manager
    if (CombatManager)
    {
        CombatManager->RegisterCombatEvent(GetOwner(), CurrentTarget, Stats.AttackDamage);
    }
}

void UCombat_DinosaurAI::FleeFromThreat(AActor* ThreatActor)
{
    SetTarget(ThreatActor);
    SetDinosaurState(ECombat_DinosaurState::Fleeing);
    
    UE_LOG(LogTemp, Warning, TEXT("%s flees from threat: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           ThreatActor ? *ThreatActor->GetName() : TEXT("Unknown"));
}

void UCombat_DinosaurAI::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);
    
    UE_LOG(LogTemp, Warning, TEXT("%s takes %.1f damage from %s (Health: %.1f/%.1f)"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           DamageAmount,
           DamageSource ? *DamageSource->GetName() : TEXT("Unknown"),
           Stats.Health, Stats.MaxHealth);
    
    // React to damage based on species and health
    if (Stats.Health <= 0.0f)
    {
        // Death logic
        SetDinosaurState(ECombat_DinosaurState::Idle);
        if (GetOwner())
        {
            GetOwner()->Destroy();
        }
    }
    else if (Stats.Health < Stats.MaxHealth * 0.3f)
    {
        // Low health - flee
        if (DamageSource)
        {
            FleeFromThreat(DamageSource);
        }
    }
    else if (DamageSource && FMath::RandRange(0.0f, 1.0f) < Stats.Aggressiveness)
    {
        // Fight back
        SetTarget(DamageSource);
        SetDinosaurState(ECombat_DinosaurState::Attacking);
    }
}

bool UCombat_DinosaurAI::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= Stats.AttackRange;
}

bool UCombat_DinosaurAI::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return false;
    }
    
    float Distance = GetDistanceToTarget(Target);
    if (Distance > Stats.SightRange)
    {
        return false;
    }
    
    // Simple line of sight check
    FHitResult HitResult;
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, 
        Start, 
        End, 
        ECC_Visibility
    );
    
    return !bHit || HitResult.GetActor() == Target;
}

float UCombat_DinosaurAI::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

void UCombat_DinosaurAI::ScanForThreats()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Find all actors within sight range
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = GetDistanceToTarget(Actor);
        if (Distance <= Stats.SightRange && CanSeeTarget(Actor))
        {
            // Check if this is a player character (threat)
            if (Actor->IsA<ACharacter>())
            {
                // Player detected - react based on species and aggressiveness
                if (FMath::RandRange(0.0f, 1.0f) < Stats.Aggressiveness)
                {
                    SetTarget(Actor);
                    SetDinosaurState(ECombat_DinosaurState::Investigating);
                }
                break;
            }
        }
    }
}

void UCombat_DinosaurAI::ScanForPrey()
{
    // For now, only scan for player characters as potential prey/threats
    ScanForThreats();
}

void UCombat_DinosaurAI::InitializeForSpecies(ECombat_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    Stats = GetDefaultStatsForSpecies(NewSpecies);
    ApplySpeciesModifiers();
}

FCombat_DinosaurStats UCombat_DinosaurAI::GetDefaultStatsForSpecies(ECombat_DinosaurSpecies InSpecies) const
{
    FCombat_DinosaurStats SpeciesStats;
    
    switch (InSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            SpeciesStats.MaxHealth = 300.0f;
            SpeciesStats.AttackDamage = 75.0f;
            SpeciesStats.AttackRange = 400.0f;
            SpeciesStats.SightRange = 2000.0f;
            SpeciesStats.MovementSpeed = 600.0f;
            SpeciesStats.Aggressiveness = 0.8f;
            SpeciesStats.PackLoyalty = 0.3f;
            SpeciesStats.TerritorialRadius = 3000.0f;
            break;
            
        case ECombat_DinosaurSpecies::Raptor:
            SpeciesStats.MaxHealth = 80.0f;
            SpeciesStats.AttackDamage = 35.0f;
            SpeciesStats.AttackRange = 250.0f;
            SpeciesStats.SightRange = 1500.0f;
            SpeciesStats.MovementSpeed = 800.0f;
            SpeciesStats.Aggressiveness = 0.9f;
            SpeciesStats.PackLoyalty = 0.9f;
            SpeciesStats.TerritorialRadius = 2000.0f;
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            SpeciesStats.MaxHealth = 250.0f;
            SpeciesStats.AttackDamage = 50.0f;
            SpeciesStats.AttackRange = 300.0f;
            SpeciesStats.SightRange = 1200.0f;
            SpeciesStats.MovementSpeed = 400.0f;
            SpeciesStats.Aggressiveness = 0.4f;
            SpeciesStats.PackLoyalty = 0.6f;
            SpeciesStats.TerritorialRadius = 1500.0f;
            break;
            
        case ECombat_DinosaurSpecies::Brachiosaurus:
            SpeciesStats.MaxHealth = 500.0f;
            SpeciesStats.AttackDamage = 20.0f;
            SpeciesStats.AttackRange = 200.0f;
            SpeciesStats.SightRange = 1000.0f;
            SpeciesStats.MovementSpeed = 200.0f;
            SpeciesStats.Aggressiveness = 0.1f;
            SpeciesStats.PackLoyalty = 0.5f;
            SpeciesStats.TerritorialRadius = 1000.0f;
            break;
            
        case ECombat_DinosaurSpecies::Stegosaurus:
            SpeciesStats.MaxHealth = 200.0f;
            SpeciesStats.AttackDamage = 40.0f;
            SpeciesStats.AttackRange = 350.0f;
            SpeciesStats.SightRange = 1100.0f;
            SpeciesStats.MovementSpeed = 300.0f;
            SpeciesStats.Aggressiveness = 0.3f;
            SpeciesStats.PackLoyalty = 0.4f;
            SpeciesStats.TerritorialRadius = 1200.0f;
            break;
    }
    
    SpeciesStats.Health = SpeciesStats.MaxHealth;
    return SpeciesStats;
}

void UCombat_DinosaurAI::ApplySpeciesModifiers()
{
    Stats = GetDefaultStatsForSpecies(Species);
    
    // Add some randomization to make each individual unique
    float HealthVariation = FMath::RandRange(0.8f, 1.2f);
    float DamageVariation = FMath::RandRange(0.9f, 1.1f);
    float SpeedVariation = FMath::RandRange(0.9f, 1.1f);
    
    Stats.MaxHealth *= HealthVariation;
    Stats.Health = Stats.MaxHealth;
    Stats.AttackDamage *= DamageVariation;
    Stats.MovementSpeed *= SpeedVariation;
}

void UCombat_DinosaurAI::JoinPack(int32 NewPackID, bool bAsLeader)
{
    PackID = NewPackID;
    bIsPackLeader = bAsLeader;
    
    UE_LOG(LogTemp, Log, TEXT("%s joined pack %d as %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           PackID,
           bAsLeader ? TEXT("leader") : TEXT("member"));
}

void UCombat_DinosaurAI::LeavePack()
{
    PackID = -1;
    bIsPackLeader = false;
    PackMembers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("%s left pack"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_DinosaurAI::IssuePackCommand(ECombat_DinosaurState CommandState, AActor* CommandTarget)
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
    {
        return;
    }
    
    for (AActor* Member : PackMembers)
    {
        if (Member && Member != GetOwner())
        {
            if (UCombat_DinosaurAI* MemberAI = Member->FindComponentByClass<UCombat_DinosaurAI>())
            {
                MemberAI->ReceivePackCommand(CommandState, CommandTarget);
            }
        }
    }
}

void UCombat_DinosaurAI::ReceivePackCommand(ECombat_DinosaurState CommandState, AActor* CommandTarget)
{
    if (PackID < 0 || bIsPackLeader)
    {
        return; // Not in pack or is leader
    }
    
    // Follow pack leader's command with some pack loyalty influence
    if (FMath::RandRange(0.0f, 1.0f) < Stats.PackLoyalty)
    {
        SetTarget(CommandTarget);
        SetDinosaurState(CommandState);
    }
}

FVector UCombat_DinosaurAI::GetPackFormationPosition() const
{
    // Simple formation logic - could be expanded
    if (!GetOwner() || PackMembers.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    
    // Find pack leader position
    FVector LeaderPos = FVector::ZeroVector;
    for (AActor* Member : PackMembers)
    {
        if (Member)
        {
            if (UCombat_DinosaurAI* MemberAI = Member->FindComponentByClass<UCombat_DinosaurAI>())
            {
                if (MemberAI->bIsPackLeader)
                {
                    LeaderPos = Member->GetActorLocation();
                    break;
                }
            }
        }
    }
    
    // Position around leader in a circle
    int32 MemberIndex = PackMembers.Find(GetOwner());
    if (MemberIndex >= 0)
    {
        float Angle = (2.0f * PI * MemberIndex) / PackMembers.Num();
        FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * 300.0f;
        return LeaderPos + Offset;
    }
    
    return LeaderPos;
}

void UCombat_DinosaurAI::UpdatePackMembers()
{
    if (PackID < 0)
    {
        return;
    }
    
    // Find all dinosaurs with the same pack ID
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
    
    PackMembers.Empty();
    
    for (AActor* Actor : AllActors)
    {
        if (UCombat_DinosaurAI* OtherAI = Actor->FindComponentByClass<UCombat_DinosaurAI>())
        {
            if (OtherAI->PackID == PackID)
            {
                PackMembers.Add(Actor);
            }
        }
    }
}