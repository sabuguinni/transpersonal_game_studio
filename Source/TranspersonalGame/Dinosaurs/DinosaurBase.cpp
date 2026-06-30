// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of the DinosaurBase actor class.
// All dinosaur species inherit from this class.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Skeletal Mesh ---
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    MoveComp->MaxWalkSpeed = 400.0f;
    MoveComp->JumpZVelocity = 500.0f;
    MoveComp->AirControl = 0.2f;
    MoveComp->NavAgentProps.AgentRadius = 60.0f;
    MoveComp->NavAgentProps.AgentHeight = 240.0f;

    // --- AI Perception Stimuli Source ---
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
    PerceptionStimuliSource->bAutoRegister = true;

    // --- Default species stats ---
    SpeciesName = TEXT("Unknown");
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    DetectionRadius = 1500.0f;
    AggroRadius = 800.0f;
    WalkSpeed = 250.0f;
    RunSpeed = 600.0f;
    bIsCarnivore = true;
    bIsPackHunter = false;
    PackSize = 1;
    TerritoryRadius = 3000.0f;
    CurrentState = EDinoState::Idle;
    bIsDead = false;
    bIsAggressive = false;
    HungerLevel = 50.0f;
    MaxHunger = 100.0f;
    HungerDecayRate = 1.0f;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    HungerLevel = FMath::RandRange(20.0f, 80.0f);

    // Apply movement speeds
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }

    // Start hunger decay timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::UpdateHunger,
        5.0f,   // every 5 seconds
        true    // looping
    );

    // Start behaviour update timer
    GetWorldTimerManager().SetTimer(
        BehaviourTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviour,
        2.0f,   // every 2 seconds
        true
    );

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spawned. HP=%.0f, Species=%s"),
        *GetName(), CurrentHealth, *SpeciesName);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Debug draw territory radius in editor
#if WITH_EDITOR
    if (bDrawDebugSpheres)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), DetectionRadius, 16,
            FColor::Yellow, false, -1.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), GetActorLocation(), AggroRadius, 16,
            FColor::Red, false, -1.0f, 0, 2.0f);
    }
#endif
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s took %.1f damage. HP=%.1f/%.1f"),
        *GetName(), ActualDamage, CurrentHealth, MaxHealth);

    OnDinosaurDamaged(ActualDamage, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // Become aggressive when hit
        if (!bIsAggressive && DamageCauser)
        {
            bIsAggressive = true;
            CurrentState = EDinoState::Aggressive;
            SetCurrentTarget(DamageCauser);
        }
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    CurrentState = EDinoState::Dead;

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviourTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    OnDinosaurDied();

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s has died."), *GetName());

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::SetCurrentTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (NewTarget)
    {
        CurrentState = EDinoState::Chasing;
        bIsAggressive = true;
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = RunSpeed;
        }
    }
    else
    {
        bIsAggressive = false;
        CurrentState = EDinoState::Idle;
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = WalkSpeed;
        }
    }
}

void ADinosaurBase::PerformAttack()
{
    if (bIsDead || !CurrentTarget) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget <= AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            AttackDamage,
            GetController(),
            this,
            nullptr
        );

        CurrentState = EDinoState::Attacking;
        OnDinosaurAttack(CurrentTarget);

        UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s attacked %s for %.1f damage"),
            *GetName(), *CurrentTarget->GetName(), AttackDamage);
    }
}

void ADinosaurBase::UpdateHunger()
{
    if (bIsDead) return;

    HungerLevel = FMath::Clamp(HungerLevel - HungerDecayRate, 0.0f, MaxHunger);

    // Very hungry carnivores become more aggressive
    if (bIsCarnivore && HungerLevel < 20.0f && CurrentState == EDinoState::Idle)
    {
        CurrentState = EDinoState::Hunting;
    }
}

void ADinosaurBase::UpdateBehaviour()
{
    if (bIsDead) return;

    switch (CurrentState)
    {
    case EDinoState::Idle:
        // Occasionally patrol
        if (FMath::RandBool())
        {
            CurrentState = EDinoState::Patrolling;
        }
        break;

    case EDinoState::Patrolling:
        // Check if player is nearby
        DetectNearbyThreats();
        break;

    case EDinoState::Hunting:
        DetectNearbyThreats();
        break;

    case EDinoState::Chasing:
        if (CurrentTarget)
        {
            float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
            if (Dist <= AttackRange)
            {
                PerformAttack();
            }
            else if (Dist > DetectionRadius * 1.5f)
            {
                // Lost target
                SetCurrentTarget(nullptr);
                CurrentState = EDinoState::Idle;
            }
        }
        else
        {
            CurrentState = EDinoState::Idle;
        }
        break;

    case EDinoState::Fleeing:
        // Herbivores flee from threats
        break;

    case EDinoState::Aggressive:
        if (CurrentTarget)
        {
            PerformAttack();
        }
        break;

    default:
        break;
    }
}

void ADinosaurBase::DetectNearbyThreats()
{
    if (bIsDead) return;

    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);

    // Sphere overlap for detection
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    TArray<AActor*> OutActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        DetectionRadius,
        ObjectTypes,
        nullptr,
        ActorsToIgnore,
        OutActors
    );

    for (AActor* DetectedActor : OutActors)
    {
        if (!DetectedActor || DetectedActor == this) continue;

        // Check if it's the player character
        APawn* DetectedPawn = Cast<APawn>(DetectedActor);
        if (DetectedPawn && DetectedPawn->IsPlayerControlled())
        {
            float Dist = FVector::Dist(GetActorLocation(), DetectedActor->GetActorLocation());

            if (bIsCarnivore && Dist <= AggroRadius)
            {
                // Carnivore attacks player within aggro range
                SetCurrentTarget(DetectedActor);
                UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s detected player at dist=%.0f — AGGRO"),
                    *GetName(), Dist);
                return;
            }
            else if (!bIsCarnivore && Dist <= AggroRadius * 0.5f)
            {
                // Herbivore flees
                CurrentState = EDinoState::Fleeing;
                SetCurrentTarget(nullptr);
            }
        }
    }
}

void ADinosaurBase::OnDinosaurDamaged_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Blueprint-overridable: play hit reaction, sound, VFX
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Blueprint-overridable: play death animation, drop loot, spawn VFX
}

void ADinosaurBase::OnDinosaurAttack_Implementation(AActor* Target)
{
    // Blueprint-overridable: play attack animation, sound
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

float ADinosaurBase::GetHungerPercent() const
{
    if (MaxHunger <= 0.0f) return 0.0f;
    return HungerLevel / MaxHunger;
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

EDinoState ADinosaurBase::GetCurrentState() const
{
    return CurrentState;
}
