#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI decisions
    
    // Initialize default values
    ThreatLevel = ECombat_ThreatLevel::Low;
    CombatState = ECombat_CombatState::Patrol;
    PackRole = ECombat_PackRole::Lone;
    
    DetectionRange = 1500.0f;
    AttackRange = 300.0f;
    FleeRange = 200.0f;
    
    Aggressiveness = 0.7f;
    Intelligence = 0.6f;
    PackLoyalty = 0.8f;
    
    LastThreatCheckTime = 0.0f;
    ThreatCheckInterval = 1.0f;
    
    CurrentTarget = nullptr;
    PackLeader = nullptr;
    
    bIsInCombat = false;
    bCanHearPlayer = false;
    bCanSeePlayer = false;
    
    CombatTimer = 0.0f;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast&lt;APawn&gt;(GetOwner());
    if (OwnerPawn)
    {
        OwnerController = Cast&lt;AAIController&gt;(OwnerPawn-&gt;GetController());
        if (OwnerController)
        {
            // Setup AI perception
            SetupAIPerception();
        }
    }
    
    // Initialize pack behavior if this is a pack animal
    if (PackRole != ECombat_PackRole::Lone)
    {
        InitializePackBehavior();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI initialized for %s"), 
           OwnerPawn ? *OwnerPawn-&gt;GetName() : TEXT("Unknown"));
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn || !OwnerController)
    {
        return;
    }
    
    CombatTimer += DeltaTime;
    
    // Periodic threat assessment
    if (CombatTimer - LastThreatCheckTime &gt; ThreatCheckInterval)
    {
        AssessThreatLevel();
        LastThreatCheckTime = CombatTimer;
    }
    
    // Update AI behavior based on current state
    UpdateCombatBehavior(DeltaTime);
    
    // Update pack coordination if in a pack
    if (PackRole != ECombat_PackRole::Lone)
    {
        UpdatePackCoordination(DeltaTime);
    }
}

void UCombat_TacticalAI::SetupAIPerception()
{
    if (!OwnerController)
    {
        return;
    }
    
    // Create perception component if it doesn't exist
    UAIPerceptionComponent* PerceptionComp = OwnerController-&gt;GetAIPerceptionComponent();
    if (!PerceptionComp)
    {
        PerceptionComp = CreateDefaultSubobject&lt;UAIPerceptionComponent&gt;(TEXT("AIPerceptionComponent"));
        OwnerController-&gt;SetPerceptionComponent(*PerceptionComp);
    }
    
    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject&lt;UAISenseConfig_Sight&gt;(TEXT("SightConfig"));
    SightConfig-&gt;SightRadius = DetectionRange;
    SightConfig-&gt;LoseSightRadius = DetectionRange * 1.2f;
    SightConfig-&gt;PeripheralVisionAngleDegrees = 90.0f;
    SightConfig-&gt;DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig-&gt;DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig-&gt;DetectionByAffiliation.bDetectEnemies = true;
    
    PerceptionComp-&gt;ConfigureSense(*SightConfig);
    
    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject&lt;UAISenseConfig_Hearing&gt;(TEXT("HearingConfig"));
    HearingConfig-&gt;HearingRange = DetectionRange * 0.8f;
    HearingConfig-&gt;DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig-&gt;DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig-&gt;DetectionByAffiliation.bDetectEnemies = true;
    
    PerceptionComp-&gt;ConfigureSense(*HearingConfig);
    
    // Bind perception events
    PerceptionComp-&gt;OnPerceptionUpdated.AddDynamic(this, &amp;UCombat_TacticalAI::OnPerceptionUpdated);
}

void UCombat_TacticalAI::OnPerceptionUpdated(const TArray&lt;AActor*&gt;&amp; UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* Character = Cast&lt;ACharacter&gt;(Actor))
        {
            // Check if this is the player
            if (Character-&gt;IsPlayerControlled())
            {
                HandlePlayerDetection(Character);
            }
        }
    }
}

void UCombat_TacticalAI::HandlePlayerDetection(ACharacter* Player)
{
    if (!Player)
    {
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(OwnerPawn-&gt;GetActorLocation(), Player-&gt;GetActorLocation());
    
    // Update detection flags
    bCanSeePlayer = CanSeeTarget(Player);
    bCanHearPlayer = DistanceToPlayer &lt; DetectionRange * 0.8f;
    
    if (bCanSeePlayer || bCanHearPlayer)
    {
        CurrentTarget = Player;
        
        // Escalate threat level based on distance
        if (DistanceToPlayer &lt; AttackRange)
        {
            ThreatLevel = ECombat_ThreatLevel::Critical;
            CombatState = ECombat_CombatState::Attack;
        }
        else if (DistanceToPlayer &lt; DetectionRange * 0.5f)
        {
            ThreatLevel = ECombat_ThreatLevel::High;
            CombatState = ECombat_CombatState::Stalk;
        }
        else
        {
            ThreatLevel = ECombat_ThreatLevel::Medium;
            CombatState = ECombat_CombatState::Investigate;
        }
        
        bIsInCombat = true;
        
        // Alert pack members if in a pack
        if (PackRole != ECombat_PackRole::Lone)
        {
            AlertPackMembers(Player);
        }
    }
}

void UCombat_TacticalAI::AssessThreatLevel()
{
    if (!CurrentTarget)
    {
        // No target, reduce threat level
        if (ThreatLevel &gt; ECombat_ThreatLevel::None)
        {
            ThreatLevel = static_cast&lt;ECombat_ThreatLevel&gt;(static_cast&lt;int32&gt;(ThreatLevel) - 1);
        }
        
        if (ThreatLevel == ECombat_ThreatLevel::None)
        {
            CombatState = ECombat_CombatState::Patrol;
            bIsInCombat = false;
        }
        return;
    }
    
    float DistanceToTarget = FVector::Dist(OwnerPawn-&gt;GetActorLocation(), CurrentTarget-&gt;GetActorLocation());
    
    // Assess threat based on distance and target behavior
    if (DistanceToTarget &lt; FleeRange &amp;&amp; ShouldFlee())
    {
        ThreatLevel = ECombat_ThreatLevel::Critical;
        CombatState = ECombat_CombatState::Flee;
    }
    else if (DistanceToTarget &lt; AttackRange &amp;&amp; ShouldAttack())
    {
        ThreatLevel = ECombat_ThreatLevel::High;
        CombatState = ECombat_CombatState::Attack;
    }
    else if (DistanceToTarget &lt; DetectionRange)
    {
        ThreatLevel = ECombat_ThreatLevel::Medium;
        CombatState = ECombat_CombatState::Stalk;
    }
    else
    {
        ThreatLevel = ECombat_ThreatLevel::Low;
        CombatState = ECombat_CombatState::Patrol;
    }
}

void UCombat_TacticalAI::UpdateCombatBehavior(float DeltaTime)
{
    switch (CombatState)
    {
        case ECombat_CombatState::Patrol:
            ExecutePatrolBehavior();
            break;
            
        case ECombat_CombatState::Investigate:
            ExecuteInvestigateBehavior();
            break;
            
        case ECombat_CombatState::Stalk:
            ExecuteStalkBehavior();
            break;
            
        case ECombat_CombatState::Attack:
            ExecuteAttackBehavior();
            break;
            
        case ECombat_CombatState::Flee:
            ExecuteFleeBehavior();
            break;
            
        case ECombat_CombatState::Guard:
            ExecuteGuardBehavior();
            break;
    }
}

void UCombat_TacticalAI::ExecutePatrolBehavior()
{
    // Simple patrol behavior - move to random nearby location
    if (!OwnerController)
    {
        return;
    }
    
    FVector CurrentLocation = OwnerPawn-&gt;GetActorLocation();
    FVector RandomDirection = FMath::VRand() * 1000.0f;
    RandomDirection.Z = 0; // Keep on ground level
    
    FVector TargetLocation = CurrentLocation + RandomDirection;
    
    OwnerController-&gt;MoveToLocation(TargetLocation, 100.0f);
}

void UCombat_TacticalAI::ExecuteInvestigateBehavior()
{
    if (!CurrentTarget || !OwnerController)
    {
        return;
    }
    
    // Move towards last known target location
    FVector TargetLocation = CurrentTarget-&gt;GetActorLocation();
    OwnerController-&gt;MoveToLocation(TargetLocation, AttackRange * 1.5f);
}

void UCombat_TacticalAI::ExecuteStalkBehavior()
{
    if (!CurrentTarget || !OwnerController)
    {
        return;
    }
    
    // Maintain distance while following target
    FVector TargetLocation = CurrentTarget-&gt;GetActorLocation();
    FVector OwnerLocation = OwnerPawn-&gt;GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Stay at stalking distance
    FVector StalkPosition = TargetLocation - (DirectionToTarget * AttackRange * 1.2f);
    
    OwnerController-&gt;MoveToLocation(StalkPosition, 50.0f);
}

void UCombat_TacticalAI::ExecuteAttackBehavior()
{
    if (!CurrentTarget || !OwnerController)
    {
        return;
    }
    
    float CurrentTime = GetWorld()-&gt;GetTimeSeconds();
    if (CurrentTime - LastAttackTime &lt; AttackCooldown)
    {
        return;
    }
    
    // Move to attack range and execute attack
    FVector TargetLocation = CurrentTarget-&gt;GetActorLocation();
    float DistanceToTarget = FVector::Dist(OwnerPawn-&gt;GetActorLocation(), TargetLocation);
    
    if (DistanceToTarget &lt; AttackRange)
    {
        // Execute attack
        PerformAttack();
        LastAttackTime = CurrentTime;
    }
    else
    {
        // Move closer to target
        OwnerController-&gt;MoveToLocation(TargetLocation, AttackRange * 0.8f);
    }
}

void UCombat_TacticalAI::ExecuteFleeBehavior()
{
    if (!CurrentTarget || !OwnerController)
    {
        return;
    }
    
    // Run away from target
    FVector TargetLocation = CurrentTarget-&gt;GetActorLocation();
    FVector OwnerLocation = OwnerPawn-&gt;GetActorLocation();
    FVector FleeDirection = (OwnerLocation - TargetLocation).GetSafeNormal();
    
    FVector FleeLocation = OwnerLocation + (FleeDirection * 2000.0f);
    
    OwnerController-&gt;MoveToLocation(FleeLocation, 100.0f);
}

void UCombat_TacticalAI::ExecuteGuardBehavior()
{
    // Stay in position and watch for threats
    if (CurrentTarget &amp;&amp; OwnerController)
    {
        // Face the target
        FVector TargetLocation = CurrentTarget-&gt;GetActorLocation();
        FVector OwnerLocation = OwnerPawn-&gt;GetActorLocation();
        FVector LookDirection = (TargetLocation - OwnerLocation).GetSafeNormal();
        
        FRotator TargetRotation = LookDirection.Rotation();
        OwnerPawn-&gt;SetActorRotation(TargetRotation);
    }
}

bool UCombat_TacticalAI::CanSeeTarget(AActor* Target) const
{
    if (!Target || !OwnerPawn)
    {
        return false;
    }
    
    FVector Start = OwnerPawn-&gt;GetActorLocation();
    FVector End = Target-&gt;GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()-&gt;LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstruction
}

bool UCombat_TacticalAI::ShouldAttack() const
{
    if (!CurrentTarget)
    {
        return false;
    }
    
    // Base attack decision on aggressiveness and threat level
    float AttackChance = Aggressiveness;
    
    if (ThreatLevel == ECombat_ThreatLevel::Critical)
    {
        AttackChance += 0.3f;
    }
    
    // Pack animals are more likely to attack
    if (PackRole != ECombat_PackRole::Lone)
    {
        AttackChance += 0.2f;
    }
    
    return FMath::RandRange(0.0f, 1.0f) &lt; AttackChance;
}

bool UCombat_TacticalAI::ShouldFlee() const
{
    if (!CurrentTarget)
    {
        return false;
    }
    
    // Base flee decision on low aggressiveness and high threat
    float FleeChance = 1.0f - Aggressiveness;
    
    if (ThreatLevel == ECombat_ThreatLevel::Critical)
    {
        FleeChance += 0.4f;
    }
    
    // Injured animals are more likely to flee
    if (OwnerPawn)
    {
        // Assume health component exists
        FleeChance += 0.3f; // Placeholder for health check
    }
    
    return FMath::RandRange(0.0f, 1.0f) &lt; FleeChance;
}

void UCombat_TacticalAI::PerformAttack()
{
    if (!CurrentTarget)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s attacking %s!"), 
           *OwnerPawn-&gt;GetName(), *CurrentTarget-&gt;GetName());
    
    // Placeholder for actual attack implementation
    // This would trigger animations, apply damage, etc.
    
    // For now, just log the attack
    if (GEngine)
    {
        FString AttackMessage = FString::Printf(TEXT("%s attacks!"), *OwnerPawn-&gt;GetName());
        GEngine-&gt;AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, AttackMessage);
    }
}

void UCombat_TacticalAI::InitializePackBehavior()
{
    // Find other pack members in the area
    TArray&lt;AActor*&gt; FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (APawn* OtherPawn = Cast&lt;APawn&gt;(Actor))
        {
            if (OtherPawn != OwnerPawn)
            {
                UCombat_TacticalAI* OtherAI = OtherPawn-&gt;FindComponentByClass&lt;UCombat_TacticalAI&gt;();
                if (OtherAI &amp;&amp; OtherAI-&gt;PackRole != ECombat_PackRole::Lone)
                {
                    // Check if they're the same species (simplified)
                    if (OtherPawn-&gt;GetClass() == OwnerPawn-&gt;GetClass())
                    {
                        PackMembers.Add(OtherPawn);
                        
                        // Establish pack hierarchy
                        if (PackRole == ECombat_PackRole::Alpha || 
                            (OtherAI-&gt;PackRole != ECombat_PackRole::Alpha &amp;&amp; !PackLeader))
                        {
                            PackLeader = OwnerPawn;
                            PackRole = ECombat_PackRole::Alpha;
                        }
                        else if (!PackLeader &amp;&amp; OtherAI-&gt;PackRole == ECombat_PackRole::Alpha)
                        {
                            PackLeader = OtherPawn;
                            PackRole = ECombat_PackRole::Beta;
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s initialized pack with %d members"), 
           *OwnerPawn-&gt;GetName(), PackMembers.Num());
}

void UCombat_TacticalAI::UpdatePackCoordination(float DeltaTime)
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Pack coordination logic
    if (PackRole == ECombat_PackRole::Alpha &amp;&amp; CurrentTarget)
    {
        // Alpha coordinates the pack attack
        CoordinatePackAttack();
    }
    else if (PackLeader &amp;&amp; PackRole != ECombat_PackRole::Alpha)
    {
        // Follow pack leader's commands
        FollowPackLeader();
    }
}

void UCombat_TacticalAI::CoordinatePackAttack()
{
    if (!CurrentTarget || PackMembers.Num() == 0)
    {
        return;
    }
    
    // Simple pack coordination - spread out around target
    FVector TargetLocation = CurrentTarget-&gt;GetActorLocation();
    
    for (int32 i = 0; i &lt; PackMembers.Num(); ++i)
    {
        if (APawn* PackMember = PackMembers[i])
        {
            UCombat_TacticalAI* MemberAI = PackMember-&gt;FindComponentByClass&lt;UCombat_TacticalAI&gt;();
            if (MemberAI)
            {
                // Assign flanking positions
                float Angle = (360.0f / PackMembers.Num()) * i;
                FVector FlankPosition = TargetLocation + FVector(
                    FMath::Cos(FMath::DegreesToRadians(Angle)) * AttackRange * 1.5f,
                    FMath::Sin(FMath::DegreesToRadians(Angle)) * AttackRange * 1.5f,
                    0.0f
                );
                
                // Command pack member to move to flanking position
                if (AAIController* MemberController = Cast&lt;AAIController&gt;(PackMember-&gt;GetController()))
                {
                    MemberController-&gt;MoveToLocation(FlankPosition, 100.0f);
                }
                
                // Set member's target
                MemberAI-&gt;CurrentTarget = CurrentTarget;
                MemberAI-&gt;CombatState = ECombat_CombatState::Stalk;
            }
        }
    }
}

void UCombat_TacticalAI::FollowPackLeader()
{
    if (!PackLeader)
    {
        return;
    }
    
    UCombat_TacticalAI* LeaderAI = PackLeader-&gt;FindComponentByClass&lt;UCombat_TacticalAI&gt;();
    if (LeaderAI)
    {
        // Copy leader's target and threat assessment
        CurrentTarget = LeaderAI-&gt;CurrentTarget;
        ThreatLevel = LeaderAI-&gt;ThreatLevel;
        
        // Adjust own combat state based on pack loyalty
        if (PackLoyalty &gt; 0.7f)
        {
            CombatState = LeaderAI-&gt;CombatState;
        }
    }
}

void UCombat_TacticalAI::AlertPackMembers(AActor* Threat)
{
    for (APawn* PackMember : PackMembers)
    {
        if (PackMember)
        {
            UCombat_TacticalAI* MemberAI = PackMember-&gt;FindComponentByClass&lt;UCombat_TacticalAI&gt;();
            if (MemberAI)
            {
                MemberAI-&gt;CurrentTarget = Threat;
                MemberAI-&gt;ThreatLevel = ECombat_ThreatLevel::High;
                MemberAI-&gt;CombatState = ECombat_CombatState::Investigate;
                MemberAI-&gt;bIsInCombat = true;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s alerted %d pack members of threat"), 
           *OwnerPawn-&gt;GetName(), PackMembers.Num());
}