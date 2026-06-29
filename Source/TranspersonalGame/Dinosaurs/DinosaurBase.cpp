// DinosaurBase.cpp
// Engine Architect #02 — Cycle AUTO_20260629_011
// Base class implementation for all dinosaur types in the prehistoric survival game.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 120.0f);

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->MaxWalkSpeedCrouched = 200.0f;
        MoveComp->JumpZVelocity = 500.0f;
        MoveComp->GravityScale = 1.2f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->NavAgentProps.bCanCrouch = false;
        MoveComp->NavAgentProps.bCanJump = false;
    }

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
        PerceptionStimuliSource->bAutoRegister = true;
    }

    // Default species stats
    Species = EDinosaurSpecies::TyrannosaurusRex;
    BehaviorState = EDinosaurBehavior::Idle;
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 75.0f;
    AttackRange = 200.0f;
    DetectionRadius = 2000.0f;
    HearingRadius = 3000.0f;
    MoveSpeed_Walk = 300.0f;
    MoveSpeed_Run = 700.0f;
    MoveSpeed_Chase = 900.0f;
    bIsCarnivore = true;
    bIsPackHunter = false;
    bIsNocturnal = false;
    HungerLevel = 0.0f;
    MaxHunger = 100.0f;
    HungerDecayRate = 1.0f;
    TerritoryRadius = 5000.0f;
    bIsTerritory = true;
    LastKnownPlayerLocation = FVector::ZeroVector;
    bIsAwareOfPlayer = false;
    bIsDead = false;
    DamageResistance = 0.0f;
    RoarCooldown = 30.0f;
    LastRoarTime = -999.0f;

    // Auto possess by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    HungerLevel = FMath::RandRange(0.0f, MaxHunger * 0.5f);

    // Store spawn location as territory center
    TerritoryCenter = GetActorLocation();

    // Start hunger decay timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::UpdateHunger,
        5.0f,
        true
    );

    // Start behavior update timer
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase BeginPlay: %s spawned at %s"),
        *GetName(), *GetActorLocation().ToString());
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    // Update awareness of player
    DetectPlayer();
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = FMath::Max(0.0f, DamageAmount - DamageResistance);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage (%.1f actual). HP: %.1f/%.1f"),
        *GetName(), DamageAmount, ActualDamage, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // React to damage — become aggressive
        if (BehaviorState == EDinosaurBehavior::Idle || BehaviorState == EDinosaurBehavior::Patrol)
        {
            BehaviorState = EDinosaurBehavior::Alert;
            bIsAwareOfPlayer = true;
            if (DamageCauser)
            {
                LastKnownPlayerLocation = DamageCauser->GetActorLocation();
            }
        }
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    BehaviorState = EDinosaurBehavior::Dead;

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Disable AI
    AAIController* AICtrl = Cast<AAIController>(GetController());
    if (AICtrl)
    {
        AICtrl->StopMovement();
        AICtrl->UnPossess();
    }

    UE_LOG(LogTemp, Log, TEXT("%s has died."), *GetName());

    // Destroy after 30 seconds (corpse decay)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::Roar()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoarTime < RoarCooldown) return;

    LastRoarTime = CurrentTime;
    BehaviorState = EDinosaurBehavior::Roaring;

    UE_LOG(LogTemp, Log, TEXT("%s ROARS!"), *GetName());

    // Reset behavior after roar
    FTimerHandle RoarEndHandle;
    GetWorldTimerManager().SetTimer(RoarEndHandle, [this]()
    {
        if (!bIsDead && BehaviorState == EDinosaurBehavior::Roaring)
        {
            BehaviorState = EDinosaurBehavior::Alert;
        }
    }, 3.0f, false);
}

void ADinosaurBase::DetectPlayer()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= DetectionRadius)
    {
        if (!bIsAwareOfPlayer)
        {
            bIsAwareOfPlayer = true;
            UE_LOG(LogTemp, Log, TEXT("%s detected player at distance %.1f"), *GetName(), DistToPlayer);
        }
        LastKnownPlayerLocation = PlayerPawn->GetActorLocation();

        // Transition to chase if carnivore and close enough
        if (bIsCarnivore && DistToPlayer <= DetectionRadius * 0.7f)
        {
            if (BehaviorState != EDinosaurBehavior::Chasing && BehaviorState != EDinosaurBehavior::Attacking)
            {
                BehaviorState = EDinosaurBehavior::Chasing;
                GetCharacterMovement()->MaxWalkSpeed = MoveSpeed_Chase;
            }
        }
    }
    else if (bIsAwareOfPlayer && DistToPlayer > DetectionRadius * 1.5f)
    {
        // Lost player — return to territory
        bIsAwareOfPlayer = false;
        BehaviorState = EDinosaurBehavior::Returning;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed_Walk;
    }
}

void ADinosaurBase::UpdateHunger()
{
    if (bIsDead) return;

    HungerLevel = FMath::Clamp(HungerLevel + HungerDecayRate, 0.0f, MaxHunger);

    if (HungerLevel >= MaxHunger * 0.8f && bIsCarnivore)
    {
        // Hungry — become more aggressive
        if (BehaviorState == EDinosaurBehavior::Idle)
        {
            BehaviorState = EDinosaurBehavior::Patrol;
            GetCharacterMovement()->MaxWalkSpeed = MoveSpeed_Walk;
        }
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (bIsDead) return;

    switch (BehaviorState)
    {
    case EDinosaurBehavior::Returning:
    {
        float DistToTerritory = FVector::Dist(GetActorLocation(), TerritoryCenter);
        if (DistToTerritory < 200.0f)
        {
            BehaviorState = EDinosaurBehavior::Idle;
            GetCharacterMovement()->MaxWalkSpeed = MoveSpeed_Walk;
        }
        break;
    }
    case EDinosaurBehavior::Idle:
    {
        // Occasionally start patrolling
        if (FMath::RandRange(0, 4) == 0)
        {
            BehaviorState = EDinosaurBehavior::Patrol;
        }
        break;
    }
    default:
        break;
    }
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

FVector ADinosaurBase::GetTerritoryCenter() const
{
    return TerritoryCenter;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (bIsDead) return;
    BehaviorState = NewState;
}
