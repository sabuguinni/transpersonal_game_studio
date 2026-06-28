#include "TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    // T-Rex species stats — apex predator configuration
    Stats.MaxHealth = 800.0f;
    Stats.CurrentHealth = 800.0f;
    Stats.MaxHunger = 300.0f;
    Stats.CurrentHunger = 300.0f;
    Stats.MaxStamina = 200.0f;
    Stats.CurrentStamina = 200.0f;
    Stats.AttackDamage = 120.0f;
    Stats.DetectionRadius = 3500.0f;
    Stats.AttackRadius = 350.0f;
    Stats.WalkSpeed = 300.0f;
    Stats.RunSpeed = 650.0f;
    Stats.bIsCarnivore = true;

    Species = EDB_DinosaurSpecies::TRex;

    // T-Rex movement — heavy, slow turning, no jump
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->RotationRate = FRotator(0.0f, 80.0f, 0.0f); // Slow turn rate
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->JumpZVelocity = 0.0f;
        MoveComp->GravityScale = 1.5f; // Heavy
        MoveComp->MaxAcceleration = 800.0f;
        MoveComp->BrakingDecelerationWalking = 600.0f;
    }

    // Patrol radius — T-Rex has large territory
    PatrolRadius = 3000.0f;
    AttackCooldown = 2.5f; // Slower attack rate but devastating
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();
    RoarTimer = FMath::RandRange(5.0f, 10.0f); // Random initial roar delay
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Roar cooldown countdown
    if (RoarTimer > 0.0f)
    {
        RoarTimer -= DeltaTime;
        if (RoarTimer <= 0.0f)
        {
            // Only roar when hunting or idle (not fleeing)
            if (CurrentState == EDB_BehaviourState::Hunting ||
                CurrentState == EDB_BehaviourState::Idle ||
                CurrentState == EDB_BehaviourState::Patrolling)
            {
                PerformRoar();
            }
            RoarTimer = RoarCooldown;
        }
    }
}

void ATRexDinosaur::PerformAttack(AActor* Target)
{
    if (!Target) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

    // Use stomp if target is very close (in melee range)
    if (DistToTarget < StompRadius * 0.5f)
    {
        PerformStomp();
    }
    else
    {
        // Standard bite attack via parent
        Super::PerformAttack(Target);
    }
}

void ATRexDinosaur::PerformRoar()
{
    // Trigger Blueprint VFX/SFX event
    OnRoar();

    // Find all actors within roar radius and apply flee behaviour
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* NearActor : NearbyActors)
    {
        if (NearActor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), NearActor->GetActorLocation());
        if (Dist > RoarRadius) continue;

        // Apply roar effect to other dinosaurs — trigger flee state
        ADinosaurBase* OtherDino = Cast<ADinosaurBase>(NearActor);
        if (OtherDino && OtherDino->Species != EDB_DinosaurSpecies::TRex)
        {
            OtherDino->CurrentState = EDB_BehaviourState::Fleeing;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROAR — radius %.0f"), *GetName(), RoarRadius);
}

void ATRexDinosaur::PerformStomp()
{
    // Trigger Blueprint VFX/SFX event
    OnStomp();

    // AoE damage in stomp radius
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        GetActorLocation(),
        StompRadius,
        UDamageType::StaticClass(),
        IgnoreActors,
        this,
        GetController(),
        true // Full damage (no falloff)
    );

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] STOMP — radius %.0f, damage %.0f"), *GetName(), StompRadius, StompDamage);
}
