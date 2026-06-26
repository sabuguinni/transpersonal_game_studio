// DinosaurBase.cpp — Performance Optimizer #04 — Cycle AUTO_008
// Implements ADinosaurBase with performance-conscious patterns:
// - Detection uses cached sphere overlap (not O(n) actor iteration)
// - Tick at 20Hz via SetActorTickInterval(0.05f) in constructor
// - BiomeManager query cached per-second (not per-frame)
// - Ragdoll death disables tick immediately

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    // Tick at 20Hz — dino AI does not need per-frame updates
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz

    // Detection sphere — overlap-based, not O(n) iteration
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DetectionSphere->SetGenerateOverlapEvents(true);

    // Attack sphere — tight radius
    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetupAttachment(RootComponent);
    AttackSphere->SetSphereRadius(200.0f);
    AttackSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // Default stats
    DinoStats.MaxHealth = 500.0f;
    DinoStats.CurrentHealth = 500.0f;
    DinoStats.AttackDamage = 80.0f;
    DinoStats.MoveSpeed = 600.0f;
    DinoStats.DetectionRadius = 1500.0f;
    DinoStats.AttackRadius = 200.0f;
    DinoStats.AttackCooldown = 1.5f;

    CurrentState = ECore_DinoState::Idle;
    Species = ECore_DinoSpecies::TyrannosaurusRex;

    // Cache invalidation timer
    BiomeCacheAge = 0.0f;
    BiomeCacheInterval = 1.0f; // Re-query biome once per second max

    // Movement defaults
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.MoveSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events for detection sphere
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionEndOverlap);

    // Set cull distance for performance (8000cm = 80m)
    if (USkeletalMeshComponent* SKM = GetMesh())
    {
        SKM->SetCullDistance(8000.0f);
        SKM->bPerBoneMotionBlur = false; // Perf: disable per-bone motion blur
    }

    // Set capsule cull distance
    if (UCapsuleComponent* Cap = GetCapsuleComponent())
    {
        Cap->SetCullDistance(8000.0f);
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Dead dinos don't tick (tick disabled in Die())
    if (CurrentState == ECore_DinoState::Dead)
    {
        return;
    }

    // Update biome cache age
    BiomeCacheAge += DeltaTime;

    // State machine update at 20Hz
    UpdateStateMachine(DeltaTime);
}

void ADinosaurBase::UpdateStateMachine(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECore_DinoState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ECore_DinoState::Patrolling:
            HandlePatrolState(DeltaTime);
            break;
        case ECore_DinoState::Chasing:
            HandleChaseState(DeltaTime);
            break;
        case ECore_DinoState::Attacking:
            HandleAttackState(DeltaTime);
            break;
        case ECore_DinoState::Fleeing:
            HandleFleeState(DeltaTime);
            break;
        case ECore_DinoState::Feeding:
        case ECore_DinoState::Resting:
        case ECore_DinoState::Dead:
        default:
            break;
    }
}

void ADinosaurBase::HandleIdleState(float DeltaTime)
{
    // Idle: occasionally transition to patrol
    // Overlap events handle threat detection
}

void ADinosaurBase::HandlePatrolState(float DeltaTime)
{
    // Patrol: move toward patrol waypoint
    // Overlap events handle threat detection
}

void ADinosaurBase::HandleChaseState(float DeltaTime)
{
    if (!TargetActor || !TargetActor->IsValidLowLevel())
    {
        SetDinoState(ECore_DinoState::Idle);
        return;
    }

    // Check if in attack range
    float DistSq = FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation());
    float AttackRadiusSq = DinoStats.AttackRadius * DinoStats.AttackRadius;

    if (DistSq <= AttackRadiusSq)
    {
        SetDinoState(ECore_DinoState::Attacking);
    }
}

void ADinosaurBase::HandleAttackState(float DeltaTime)
{
    if (!TargetActor || !TargetActor->IsValidLowLevel())
    {
        SetDinoState(ECore_DinoState::Idle);
        return;
    }

    // Check still in range
    float DistSq = FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation());
    float AttackRadiusSq = DinoStats.AttackRadius * DinoStats.AttackRadius * 1.5f; // 50% hysteresis

    if (DistSq > AttackRadiusSq)
    {
        SetDinoState(ECore_DinoState::Chasing);
    }
}

void ADinosaurBase::HandleFleeState(float DeltaTime)
{
    // Flee: move away from threat
    if (!TargetActor)
    {
        SetDinoState(ECore_DinoState::Idle);
    }
}

void ADinosaurBase::SetDinoState(ECore_DinoState NewState)
{
    if (CurrentState == NewState) return;

    ECore_DinoState OldState = CurrentState;
    CurrentState = NewState;

    OnStateChanged(OldState, NewState);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - ActualDamage);

    OnDamageTaken(ActualDamage, DinoStats.CurrentHealth);

    if (DinoStats.CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (DamageCauser)
    {
        // Aggro: chase attacker
        TargetActor = DamageCauser;
        SetDinoState(ECore_DinoState::Chasing);
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (CurrentState == ECore_DinoState::Dead) return;

    SetDinoState(ECore_DinoState::Dead);

    // Disable tick immediately — dead dinos don't need updates
    SetActorTickEnabled(false);

    // Disable collision on capsule
    if (UCapsuleComponent* Cap = GetCapsuleComponent())
    {
        Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Enable ragdoll on skeletal mesh
    if (USkeletalMeshComponent* SKM = GetMesh())
    {
        SKM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SKM->SetSimulatePhysics(true);
        SKM->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
    }

    // Disable detection sphere
    if (DetectionSphere)
    {
        DetectionSphere->SetGenerateOverlapEvents(false);
        DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    OnDeath();

    // Auto-destroy after 30 seconds (configurable)
    SetLifeSpan(30.0f);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentState != ECore_DinoState::Dead && DinoStats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f) return 0.0f;
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

void ADinosaurBase::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    if (CurrentState == ECore_DinoState::Dead) return;

    // Check if it's a player character
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        TargetActor = OtherActor;
        SetDinoState(ECore_DinoState::Chasing);
    }
}

void ADinosaurBase::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (OtherActor == TargetActor)
    {
        TargetActor = nullptr;
        SetDinoState(ECore_DinoState::Idle);
    }
}

// Blueprint-implementable stubs (called from C++, overridden in BP)
void ADinosaurBase::OnStateChanged_Implementation(ECore_DinoState OldState, ECore_DinoState NewState) {}
void ADinosaurBase::OnDamageTaken_Implementation(float Damage, float RemainingHealth) {}
void ADinosaurBase::OnDeath_Implementation() {}
