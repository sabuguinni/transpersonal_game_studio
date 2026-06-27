#include "Dinosaurs/VelociraptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVelociraptorCharacter::AVelociraptorCharacter()
{
    // ── Species identity ──────────────────────────────────────────────────
    Species = EEng_DinoSpecies::Velociraptor;

    // ── Raptor stats — fast, fragile, pack hunter ─────────────────────────
    DinoStats.MaxHealth         = 150.0f;
    DinoStats.CurrentHealth     = 150.0f;
    DinoStats.AttackDamage      = 30.0f;
    DinoStats.DetectionRadius   = 2500.0f;
    DinoStats.AttackRange       = 200.0f;
    DinoStats.WalkSpeed         = 400.0f;
    DinoStats.RunSpeed          = 1200.0f;
    DinoStats.Hunger            = 100.0f;
    DinoStats.HungerDecayRate   = 1.0f;   // Raptors burn energy fast
    DinoStats.bIsCarnivore      = true;
    DinoStats.bIsPackHunter     = true;

    // ── Capsule — lean, agile body ────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 80.0f);

    // ── Movement — fastest dino in the game ──────────────────────────────
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed          = DinoStats.RunSpeed;
        MoveComp->MaxAcceleration       = 3000.0f;
        MoveComp->BrakingDecelerationWalking = 2000.0f;
        MoveComp->JumpZVelocity         = 600.0f;  // Raptors can jump
        MoveComp->AirControl            = 0.4f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate          = FRotator(0.0f, 720.0f, 0.0f);
    }

    // ── Behavior tick — raptors react faster than large dinos ────────────
    BehaviorTickInterval = 0.3f;

    PrimaryActorTick.bCanEverTick = true;
}

void AVelociraptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Determine pack leadership — first raptor spawned in a group becomes leader
    // Simple heuristic: check if any other raptor is already a leader nearby
    int32 NearbyCount = CountNearbyPackMembers();
    if (NearbyCount == 0)
    {
        bIsLeader = true;
        UE_LOG(LogTemp, Log, TEXT("VelociraptorCharacter: %s designated as pack leader"), *GetName());
    }

    // Start in idle, will transition to hunting when player detected
    SetBehaviorState(EEng_DinoBehaviorState::Idle);
}

void AVelociraptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update pack coordination on behavior tick cadence
    // (BehaviorTickAccumulator is managed by parent Tick)
    // We piggyback on the parent's behavior tick by checking the accumulator indirectly
    // Pack coordination runs every 0.5s (slightly less frequent than behavior tick)
    static float PackTickAccumulator = 0.0f;
    PackTickAccumulator += DeltaTime;
    if (PackTickAccumulator >= 0.5f)
    {
        PackTickAccumulator = 0.0f;
        UpdatePackCoordination();
    }
}

// ── Pack Coordination ─────────────────────────────────────────────────────

void AVelociraptorCharacter::UpdatePackCoordination()
{
    NearbyPackMemberCount = CountNearbyPackMembers();

    // If we have enough pack members and a target, trigger coordinated attack
    if (NearbyPackMemberCount >= FlankThreshold && BehaviorState == EEng_DinoBehaviorState::Hunting)
    {
        // Screech to coordinate if not on cooldown
        if (!bScreechCooldown)
        {
            PerformScreech();
        }
    }
}

int32 AVelociraptorCharacter::CountNearbyPackMembers() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVelociraptorCharacter::StaticClass(), FoundActors);

    int32 Count = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor == this) continue;
        AVelociraptorCharacter* OtherRaptor = Cast<AVelociraptorCharacter>(Actor);
        if (OtherRaptor && OtherRaptor->IsAlive())
        {
            float Distance = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
            if (Distance <= PackCoordinationRadius)
            {
                Count++;
            }
        }
    }
    return Count;
}

// ── Raptor Abilities ──────────────────────────────────────────────────────

void AVelociraptorCharacter::PerformPounce(AActor* Target)
{
    if (!Target || !IsAlive() || bPounceCooldown) return;

    // Launch toward target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector PounceVelocity = ToTarget * 1500.0f + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(PounceVelocity, true, true);

    // Apply bonus damage on pounce
    float TotalDamage = DinoStats.AttackDamage + PounceBonusDamage;
    TotalDamage *= GetEffectiveAttackDamage() / FMath::Max(DinoStats.AttackDamage, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Raptor %s pouncing on %s for %.0f damage"),
        *GetName(), *Target->GetName(), TotalDamage);

    // Start cooldown
    bPounceCooldown = true;
    GetWorldTimerManager().SetTimer(
        PounceTimerHandle,
        this,
        &AVelociraptorCharacter::ResetPounceCooldown,
        PounceCooldownDuration,
        false
    );

    // Fire Blueprint event
    OnPounce(Target);
}

void AVelociraptorCharacter::PerformScreech()
{
    if (bScreechCooldown) return;

    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Log, TEXT("Raptor %s screeching — alerting pack"), *GetName());

    // Alert all nearby raptors — set them to Aggressive state
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVelociraptorCharacter::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == this) continue;
        AVelociraptorCharacter* OtherRaptor = Cast<AVelociraptorCharacter>(Actor);
        if (OtherRaptor && OtherRaptor->IsAlive())
        {
            float Distance = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
            if (Distance <= PackCoordinationRadius)
            {
                // Propagate hunting state to pack members
                if (OtherRaptor->BehaviorState == EEng_DinoBehaviorState::Idle ||
                    OtherRaptor->BehaviorState == EEng_DinoBehaviorState::Patrolling)
                {
                    OtherRaptor->SetBehaviorState(EEng_DinoBehaviorState::Aggressive);
                }
            }
        }
    }

    // Start screech cooldown
    bScreechCooldown = true;
    GetWorldTimerManager().SetTimer(
        ScreechTimerHandle,
        this,
        &AVelociraptorCharacter::ResetScreechCooldown,
        ScreechCooldownDuration,
        false
    );

    OnScreech();
}

float AVelociraptorCharacter::GetEffectiveAttackDamage() const
{
    // Base damage + pack bonus (capped at 3 additional members)
    int32 ClampedPackCount = FMath::Clamp(NearbyPackMemberCount, 0, 3);
    float Multiplier = FMath::Pow(PackDamageMultiplier, static_cast<float>(ClampedPackCount));
    return DinoStats.AttackDamage * Multiplier;
}

bool AVelociraptorCharacter::IsPackLeader() const
{
    return bIsLeader;
}

// ── Cooldown Resets ───────────────────────────────────────────────────────

void AVelociraptorCharacter::ResetPounceCooldown()
{
    bPounceCooldown = false;
}

void AVelociraptorCharacter::ResetScreechCooldown()
{
    bScreechCooldown = false;
}
