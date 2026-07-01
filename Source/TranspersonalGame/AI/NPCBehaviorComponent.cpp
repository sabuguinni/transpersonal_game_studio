// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation of dinosaur/NPC state machine with patrol, chase, attack, flee logic

#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for AI

    BehaviorState = ENPC_BehaviorState::Idle;
    Species = ENPC_DinoSpecies::Raptor;

    // Default stats
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    DetectionRadius = 2000.0f;
    AttackRadius = 300.0f;
    FleeHealthThreshold = 0.25f;
    PatrolRadius = 1500.0f;
    MoveSpeed_Patrol = 200.0f;
    MoveSpeed_Chase = 600.0f;
    MoveSpeed_Flee = 700.0f;

    // Memory
    MemoryDuration = 15.0f;
    TimeSinceLastPlayerSighting = 0.0f;
    bPlayerInMemory = false;

    // Daily routine
    bHasSchedule = true;
    CurrentSchedulePhase = ENPC_SchedulePhase::Patrol;

    // Pack
    bIsPackMember = false;
    PackAlertRadius = 3000.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorComponent: Owner is not an ACharacter!"));
        return;
    }

    // Cache home location for patrol
    HomeLocation = OwnerCharacter->GetActorLocation();

    // Apply species-specific defaults
    ApplySpeciesDefaults();

    // Start in patrol state
    SetBehaviorState(ENPC_BehaviorState::Patrol);

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent initialized on %s (Species: %d)"),
        *GetOwner()->GetName(), (int32)Species);
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter) return;

    // Update memory timer
    if (bPlayerInMemory)
    {
        TimeSinceLastPlayerSighting += DeltaTime;
        if (TimeSinceLastPlayerSighting >= MemoryDuration)
        {
            bPlayerInMemory = false;
            TimeSinceLastPlayerSighting = 0.0f;
            UE_LOG(LogTemp, Log, TEXT("%s: Player memory expired — returning to patrol"), *GetOwner()->GetName());
        }
    }

    // Scan for player
    AActor* Player = ScanForPlayer();

    // State machine update
    UpdateStateMachine(Player, DeltaTime);

    // Debug visualization
#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), OwnerCharacter->GetActorLocation(), DetectionRadius, 12,
        FColor::Yellow, false, 0.15f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), OwnerCharacter->GetActorLocation(), AttackRadius, 8,
        FColor::Red, false, 0.15f, 0, 2.0f);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// State Machine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateStateMachine(AActor* Player, float DeltaTime)
{
    // Check flee condition first (highest priority)
    if (BehaviorState != ENPC_BehaviorState::Dead &&
        BehaviorState != ENPC_BehaviorState::Flee &&
        CurrentHealth / MaxHealth <= FleeHealthThreshold)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    switch (BehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            TickIdle(Player, DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            TickPatrol(Player, DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            TickAlert(Player, DeltaTime);
            break;
        case ENPC_BehaviorState::Chase:
            TickChase(Player, DeltaTime);
            break;
        case ENPC_BehaviorState::Attack:
            TickAttack(Player, DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(Player, DeltaTime);
            break;
        case ENPC_BehaviorState::Feed:
            TickFeed(DeltaTime);
            break;
        case ENPC_BehaviorState::Rest:
            TickRest(DeltaTime);
            break;
        case ENPC_BehaviorState::Dead:
            // No updates
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Tick Implementations
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickIdle(AActor* Player, float DeltaTime)
{
    if (Player)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    // After 5s idle, resume patrol
    IdleTimer += DeltaTime;
    if (IdleTimer >= 5.0f)
    {
        IdleTimer = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(AActor* Player, float DeltaTime)
{
    if (Player)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    // Move toward current waypoint
    if (PatrolWaypoints.Num() > 0)
    {
        FVector Target = PatrolWaypoints[CurrentWaypointIndex];
        float DistToWaypoint = FVector::Dist(OwnerCharacter->GetActorLocation(), Target);

        if (DistToWaypoint < 150.0f)
        {
            // Reached waypoint — advance
            CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        }
        else
        {
            MoveToward(Target, MoveSpeed_Patrol);
        }
    }
    else
    {
        // No waypoints — wander around home
        WanderAroundHome(DeltaTime);
    }
}

void UNPCBehaviorComponent::TickAlert(AActor* Player, float DeltaTime)
{
    if (!Player)
    {
        // Lost sight — go back to patrol after brief investigation
        AlertTimer += DeltaTime;
        if (AlertTimer >= 3.0f)
        {
            AlertTimer = 0.0f;
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        return;
    }

    AlertTimer = 0.0f;
    LastKnownPlayerLocation = Player->GetActorLocation();
    bPlayerInMemory = true;
    TimeSinceLastPlayerSighting = 0.0f;

    // Alert pack members
    if (bIsPackMember)
    {
        AlertPackMembers();
    }

    // Transition to chase
    SetBehaviorState(ENPC_BehaviorState::Chase);
}

void UNPCBehaviorComponent::TickChase(AActor* Player, float DeltaTime)
{
    if (!Player && !bPlayerInMemory)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }

    FVector Target = Player ? Player->GetActorLocation() : LastKnownPlayerLocation;
    float DistToPlayer = Player ? FVector::Dist(OwnerCharacter->GetActorLocation(), Target) : 9999.0f;

    if (Player)
    {
        LastKnownPlayerLocation = Target;
        TimeSinceLastPlayerSighting = 0.0f;
    }

    // Within attack range?
    if (DistToPlayer <= AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
        return;
    }

    MoveToward(Target, MoveSpeed_Chase);
}

void UNPCBehaviorComponent::TickAttack(AActor* Player, float DeltaTime)
{
    if (!Player)
    {
        SetBehaviorState(ENPC_BehaviorState::Chase);
        return;
    }

    float DistToPlayer = FVector::Dist(OwnerCharacter->GetActorLocation(), Player->GetActorLocation());

    if (DistToPlayer > AttackRadius * 1.5f)
    {
        // Player escaped melee range — chase again
        SetBehaviorState(ENPC_BehaviorState::Chase);
        return;
    }

    // Attack cooldown
    AttackCooldownTimer += DeltaTime;
    if (AttackCooldownTimer >= AttackCooldown)
    {
        AttackCooldownTimer = 0.0f;
        ExecuteAttack(Player);
    }

    // Face player
    FVector Dir = (Player->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
    FRotator LookRot = Dir.Rotation();
    OwnerCharacter->SetActorRotation(FRotator(0, LookRot.Yaw, 0));
}

void UNPCBehaviorComponent::TickFlee(AActor* Player, float DeltaTime)
{
    // Move away from player or threat
    FVector FleeDir = FVector::ZeroVector;
    if (Player)
    {
        FleeDir = (OwnerCharacter->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
    }
    else
    {
        FleeDir = (OwnerCharacter->GetActorLocation() - LastKnownPlayerLocation).GetSafeNormal();
    }

    FVector FleeTarget = OwnerCharacter->GetActorLocation() + FleeDir * 2000.0f;
    MoveToward(FleeTarget, MoveSpeed_Flee);

    // After fleeing far enough, rest
    FleeTimer += DeltaTime;
    if (FleeTimer >= 8.0f)
    {
        FleeTimer = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Rest);
    }
}

void UNPCBehaviorComponent::TickFeed(float DeltaTime)
{
    // Feeding — recover health slowly
    FeedTimer += DeltaTime;
    CurrentHealth = FMath::Min(CurrentHealth + 5.0f * DeltaTime, MaxHealth);
    if (FeedTimer >= 10.0f)
    {
        FeedTimer = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickRest(float DeltaTime)
{
    // Resting — recover health
    RestTimer += DeltaTime;
    CurrentHealth = FMath::Min(CurrentHealth + 2.0f * DeltaTime, MaxHealth);
    if (RestTimer >= 15.0f)
    {
        RestTimer = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper Methods
// ─────────────────────────────────────────────────────────────────────────────

AActor* UNPCBehaviorComponent::ScanForPlayer() const
{
    if (!OwnerCharacter) return nullptr;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar) return nullptr;

    float Dist = FVector::Dist(OwnerCharacter->GetActorLocation(), PlayerChar->GetActorLocation());
    if (Dist > DetectionRadius) return nullptr;

    // Line of sight check
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    FVector EyeLocation = OwnerCharacter->GetActorLocation() + FVector(0, 0, 80.0f);
    FVector PlayerLocation = PlayerChar->GetActorLocation() + FVector(0, 0, 50.0f);

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, PlayerLocation,
        ECC_Visibility, Params);

    if (bBlocked && Hit.GetActor() != PlayerChar)
    {
        return nullptr; // Obstructed
    }

    return PlayerChar;
}

void UNPCBehaviorComponent::MoveToward(const FVector& Target, float Speed)
{
    if (!OwnerCharacter) return;

    FVector Dir = (Target - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
    OwnerCharacter->AddMovementInput(Dir, 1.0f);

    // Set movement speed via CharacterMovement
    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Speed;
    }
}

void UNPCBehaviorComponent::WanderAroundHome(float DeltaTime)
{
    WanderTimer += DeltaTime;
    if (WanderTimer >= 3.0f)
    {
        WanderTimer = 0.0f;
        // Pick a random point within patrol radius
        FVector RandOffset = FVector(
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            0.0f
        );
        WanderTarget = HomeLocation + RandOffset;
    }

    if (!WanderTarget.IsZero())
    {
        MoveToward(WanderTarget, MoveSpeed_Patrol);
    }
}

void UNPCBehaviorComponent::ExecuteAttack(AActor* Target)
{
    if (!Target) return;

    // Broadcast attack event — Combat Agent (#12) will handle damage application
    OnNPCAttack.Broadcast(Target, AttackDamage);

    UE_LOG(LogTemp, Log, TEXT("%s attacks %s for %.1f damage"),
        *GetOwner()->GetName(), *Target->GetName(), AttackDamage);
}

void UNPCBehaviorComponent::AlertPackMembers()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor == OwnerCharacter) continue;

        float Dist = FVector::Dist(OwnerCharacter->GetActorLocation(), Actor->GetActorLocation());
        if (Dist > PackAlertRadius) continue;

        UNPCBehaviorComponent* PackMate = Actor->FindComponentByClass<UNPCBehaviorComponent>();
        if (PackMate && PackMate->bIsPackMember && PackMate->Species == Species)
        {
            PackMate->OnPackAlert(LastKnownPlayerLocation);
        }
    }
}

void UNPCBehaviorComponent::OnPackAlert(const FVector& ThreatLocation)
{
    if (BehaviorState == ENPC_BehaviorState::Dead) return;

    LastKnownPlayerLocation = ThreatLocation;
    bPlayerInMemory = true;
    TimeSinceLastPlayerSighting = 0.0f;

    if (BehaviorState == ENPC_BehaviorState::Idle || BehaviorState == ENPC_BehaviorState::Rest)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    ENPC_BehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    OnBehaviorStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogTemp, Verbose, TEXT("%s: State %d -> %d"),
        *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
}

void UNPCBehaviorComponent::ApplySpeciesDefaults()
{
    switch (Species)
    {
        case ENPC_DinoSpecies::TRex:
            MaxHealth = 500.0f;
            CurrentHealth = 500.0f;
            DetectionRadius = 3000.0f;
            AttackRadius = 350.0f;
            AttackDamage = 80.0f;
            AttackCooldown = 2.0f;
            MoveSpeed_Patrol = 180.0f;
            MoveSpeed_Chase = 550.0f;
            MoveSpeed_Flee = 400.0f;
            PatrolRadius = 5000.0f;
            bIsPackMember = false;
            break;

        case ENPC_DinoSpecies::Raptor:
            MaxHealth = 120.0f;
            CurrentHealth = 120.0f;
            DetectionRadius = 2500.0f;
            AttackRadius = 200.0f;
            AttackDamage = 30.0f;
            AttackCooldown = 0.8f;
            MoveSpeed_Patrol = 300.0f;
            MoveSpeed_Chase = 700.0f;
            MoveSpeed_Flee = 750.0f;
            PatrolRadius = 1500.0f;
            bIsPackMember = true;
            PackAlertRadius = 2000.0f;
            break;

        case ENPC_DinoSpecies::Brachiosaurus:
            MaxHealth = 1000.0f;
            CurrentHealth = 1000.0f;
            DetectionRadius = 1000.0f;
            AttackRadius = 500.0f;
            AttackDamage = 50.0f;
            AttackCooldown = 3.0f;
            MoveSpeed_Patrol = 150.0f;
            MoveSpeed_Chase = 250.0f;
            MoveSpeed_Flee = 300.0f;
            PatrolRadius = 3000.0f;
            FleeHealthThreshold = 0.1f; // Brachios rarely flee
            bIsPackMember = true;
            PackAlertRadius = 4000.0f;
            break;

        case ENPC_DinoSpecies::Triceratops:
            MaxHealth = 400.0f;
            CurrentHealth = 400.0f;
            DetectionRadius = 1500.0f;
            AttackRadius = 300.0f;
            AttackDamage = 60.0f;
            AttackCooldown = 1.5f;
            MoveSpeed_Patrol = 200.0f;
            MoveSpeed_Chase = 500.0f;
            MoveSpeed_Flee = 450.0f;
            PatrolRadius = 2000.0f;
            bIsPackMember = true;
            PackAlertRadius = 2500.0f;
            break;

        case ENPC_DinoSpecies::Pterodactyl:
            MaxHealth = 80.0f;
            CurrentHealth = 80.0f;
            DetectionRadius = 4000.0f; // High aerial detection
            AttackRadius = 150.0f;
            AttackDamage = 20.0f;
            AttackCooldown = 1.2f;
            MoveSpeed_Patrol = 400.0f;
            MoveSpeed_Chase = 900.0f;
            MoveSpeed_Flee = 1000.0f;
            PatrolRadius = 6000.0f;
            bIsPackMember = false;
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TakeDamage_NPC(float DamageAmount, AActor* DamageSource)
{
    if (BehaviorState == ENPC_BehaviorState::Dead) return;

    CurrentHealth = FMath::Max(CurrentHealth - DamageAmount, 0.0f);

    if (DamageSource)
    {
        LastKnownPlayerLocation = DamageSource->GetActorLocation();
        bPlayerInMemory = true;
        TimeSinceLastPlayerSighting = 0.0f;
    }

    if (CurrentHealth <= 0.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Dead);
        OnNPCDeath.Broadcast(GetOwner());
        return;
    }

    // Damage reaction
    if (BehaviorState == ENPC_BehaviorState::Idle || BehaviorState == ENPC_BehaviorState::Patrol)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }

    if (bIsPackMember)
    {
        AlertPackMembers();
    }
}

void UNPCBehaviorComponent::SetSchedulePhase(ENPC_SchedulePhase NewPhase)
{
    CurrentSchedulePhase = NewPhase;

    switch (NewPhase)
    {
        case ENPC_SchedulePhase::Patrol:
            SetBehaviorState(ENPC_BehaviorState::Patrol);
            break;
        case ENPC_SchedulePhase::Feeding:
            SetBehaviorState(ENPC_BehaviorState::Feed);
            break;
        case ENPC_SchedulePhase::Resting:
            SetBehaviorState(ENPC_BehaviorState::Rest);
            break;
        case ENPC_SchedulePhase::Hunting:
            SetBehaviorState(ENPC_BehaviorState::Patrol);
            break;
    }
}

float UNPCBehaviorComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

bool UNPCBehaviorComponent::IsHostile() const
{
    return BehaviorState == ENPC_BehaviorState::Chase ||
           BehaviorState == ENPC_BehaviorState::Attack ||
           BehaviorState == ENPC_BehaviorState::Alert;
}
