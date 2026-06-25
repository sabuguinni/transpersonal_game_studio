// DinosaurBase.cpp
// Transpersonal Game Studio — Agent #03 Core Systems / #04 Performance Optimizer
// Prehistoric survival dinosaur base class implementation

#include "DinosaurBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f; // 30Hz default — performance budget

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(80.f, 120.f);

    // Skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -120.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Species defaults
    DinosaurSpecies = EPerf_DinoSpecies::TRex;
    CurrentBehaviorState = EPerf_DinoBehavior::Idle;
    MaxHealth = 1000.f;
    CurrentHealth = 1000.f;
    AttackDamage = 150.f;
    AttackRange = 300.f;
    DetectionRadius = 3000.f;
    bIsAlpha = false;
    PackID = -1;
    CurrentBiome = EPerf_DinoHomeRange::Savanna;

    // LOD tick distances (performance)
    LODTickIntervals[0] = 0.016f;  // <2000 units: 60Hz
    LODTickIntervals[1] = 0.033f;  // 2000-5000: 30Hz
    LODTickIntervals[2] = 0.1f;    // 5000-10000: 10Hz
    LODTickIntervals[3] = 0.5f;    // >10000: 2Hz (nearly culled)
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentBehaviorState = EPerf_DinoBehavior::Idle;

    // Start behavior tick loop
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehavior,
        1.0f,
        true
    );

    // Start LOD distance check
    GetWorldTimerManager().SetTimer(
        LODCheckTimerHandle,
        this,
        &ADinosaurBase::UpdateLODTickRate,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Behavior logic handled by timer — Tick is minimal for performance
}

void ADinosaurBase::UpdateBehavior()
{
    if (CurrentHealth <= 0.f)
    {
        SetBehaviorState(EPerf_DinoBehavior::Dead);
        return;
    }

    // Basic state machine — expanded by Combat AI Agent #12
    switch (CurrentBehaviorState)
    {
        case EPerf_DinoBehavior::Idle:
            // Transition to patrol after idle period
            SetBehaviorState(EPerf_DinoBehavior::Patrol);
            break;

        case EPerf_DinoBehavior::Patrol:
            // Scan for threats — AI Agent #12 will override
            break;

        case EPerf_DinoBehavior::Fleeing:
            // Flee logic handled by BehaviorTree
            break;

        default:
            break;
    }
}

void ADinosaurBase::UpdateLODTickRate()
{
    // Adjust tick rate based on distance to nearest player camera
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    float NewInterval = LODTickIntervals[3]; // Default: far
    if (Dist < 2000.f)       NewInterval = LODTickIntervals[0];
    else if (Dist < 5000.f)  NewInterval = LODTickIntervals[1];
    else if (Dist < 10000.f) NewInterval = LODTickIntervals[2];

    SetActorTickInterval(NewInterval);
}

void ADinosaurBase::SetBehaviorState(EPerf_DinoBehavior NewState)
{
    if (CurrentBehaviorState == NewState) return;
    CurrentBehaviorState = NewState;
    OnBehaviorStateChanged(NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EPerf_DinoBehavior NewState)
{
    // Blueprint-implementable event — Animation Agent #10 hooks here
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.f)
    {
        SetBehaviorState(EPerf_DinoBehavior::Dead);
        HandleDeath();
    }
    else if (CurrentHealth < MaxHealth * 0.3f)
    {
        // Low health — flee behavior
        SetBehaviorState(EPerf_DinoBehavior::Fleeing);
    }
    else
    {
        SetBehaviorState(EPerf_DinoBehavior::Attacking);
    }

    return ActualDamage;
}

void ADinosaurBase::HandleDeath()
{
    // Disable collision and tick
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetActorTickEnabled(false);
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);
    GetWorldTimerManager().ClearTimer(LODCheckTimerHandle);

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 30 seconds (cleanup)
    SetLifeSpan(30.f);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.f) return 0.f;
    return CurrentHealth / MaxHealth;
}
