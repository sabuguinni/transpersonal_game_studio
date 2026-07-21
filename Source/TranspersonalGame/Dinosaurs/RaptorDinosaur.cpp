// RaptorDinosaur.cpp
// Transpersonal Game Studio — Agent #05 Procedural World Generator
// Velociraptor pack hunter AI implementation

#include "RaptorDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ARaptorDinosaur::ARaptorDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // Raptor physical dimensions — 1.8m tall, 2.5m long
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

    // Movement — fast, agile pack hunter
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 700.0f;       // ~25 km/h sprint
        MoveComp->MaxAcceleration = 2048.0f;
        MoveComp->JumpZVelocity = 600.0f;
        MoveComp->GravityScale = 1.2f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }

    bUseControllerRotationYaw = false;

    // Vital stats
    Health = 120.0f;
    MaxHealth = 120.0f;
    AttackDamage = 35.0f;
    AttackRange = 180.0f;
    DetectionRange = 2200.0f;
    PackRole = EWorld_RaptorRole::Scout;
    bIsAlpha = false;
    bIsInPackChase = false;
    PackLeader = nullptr;
    CurrentState = EWorld_DinoState::Idle;
    PatrolRadius = 1200.0f;
    HomeLocation = FVector::ZeroVector;
    TimeSinceLastAttack = 0.0f;
    AttackCooldown = 1.2f;
}

void ARaptorDinosaur::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();

    // Start AI tick loop
    GetWorld()->GetTimerManager().SetTimer(
        AITickHandle,
        this,
        &ARaptorDinosaur::UpdateAI,
        0.25f,
        true
    );
}

void ARaptorDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastAttack += DeltaTime;

    // Update pack coordination
    if (bIsInPackChase && PackLeader && PackLeader != this)
    {
        CoordinateWithPack();
    }
}

void ARaptorDinosaur::UpdateAI()
{
    if (Health <= 0.0f) return;

    AActor* Target = FindNearestThreat();

    if (Target)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

        if (DistToTarget <= AttackRange && TimeSinceLastAttack >= AttackCooldown)
        {
            CurrentState = EWorld_DinoState::Attacking;
            PerformAttack(Target);
        }
        else if (DistToTarget <= DetectionRange)
        {
            CurrentState = EWorld_DinoState::Chasing;
            ChaseTarget(Target);

            // Alert pack members
            if (!bIsInPackChase)
            {
                AlertPackMembers(Target);
            }
        }
    }
    else
    {
        bIsInPackChase = false;

        float DistFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
        if (DistFromHome > PatrolRadius)
        {
            CurrentState = EWorld_DinoState::Returning;
            ReturnToHome();
        }
        else
        {
            CurrentState = EWorld_DinoState::Patrolling;
            Patrol();
        }
    }
}

AActor* ARaptorDinosaur::FindNearestThreat()
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Find player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return nullptr;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer <= DetectionRange)
    {
        return PlayerPawn;
    }

    return nullptr;
}

void ARaptorDinosaur::ChaseTarget(AActor* Target)
{
    if (!Target) return;

    FVector TargetLoc = Target->GetActorLocation();
    FVector MyLoc = GetActorLocation();
    FVector Direction = (TargetLoc - MyLoc).GetSafeNormal();

    // Flanking behaviour — raptors circle around target
    if (PackRole == EWorld_RaptorRole::Flanker)
    {
        FVector FlankOffset = FVector(-Direction.Y, Direction.X, 0.0f) * 300.0f;
        TargetLoc += FlankOffset;
    }

    AddMovementInput(Direction, 1.0f);
}

void ARaptorDinosaur::PerformAttack(AActor* Target)
{
    if (!Target || TimeSinceLastAttack < AttackCooldown) return;

    TimeSinceLastAttack = 0.0f;

    // Apply damage
    UGameplayStatics::ApplyDamage(
        Target,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    unreal_log_attack(Target);
}

void ARaptorDinosaur::AlertPackMembers(AActor* Target)
{
    if (!Target) return;

    bIsInPackChase = true;

    // Find nearby raptors within 1500 units
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorDinosaur::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        ARaptorDinosaur* OtherRaptor = Cast<ARaptorDinosaur>(Actor);
        if (OtherRaptor && OtherRaptor != this)
        {
            float Dist = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
            if (Dist <= 1500.0f)
            {
                OtherRaptor->bIsInPackChase = true;
                OtherRaptor->PackLeader = this;

                // Assign flanker role to pack members
                if (OtherRaptor->PackRole == EWorld_RaptorRole::Scout)
                {
                    OtherRaptor->PackRole = EWorld_RaptorRole::Flanker;
                }
            }
        }
    }
}

void ARaptorDinosaur::CoordinateWithPack()
{
    if (!PackLeader) return;

    // Follow leader's target with offset based on role
    FVector LeaderLoc = PackLeader->GetActorLocation();
    FVector MyLoc = GetActorLocation();
    FVector ToLeader = (LeaderLoc - MyLoc).GetSafeNormal();

    // Flankers spread out to surround prey
    if (PackRole == EWorld_RaptorRole::Flanker)
    {
        FVector FlankDir = FVector(-ToLeader.Y, ToLeader.X, 0.0f);
        AddMovementInput(ToLeader + FlankDir * 0.5f, 1.0f);
    }
}

void ARaptorDinosaur::Patrol()
{
    // Random patrol within radius
    if (FVector::Dist(GetActorLocation(), PatrolTarget) < 100.0f || PatrolTarget.IsZero())
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-PatrolRadius * 0.5f, PatrolRadius * 0.5f),
            FMath::RandRange(-PatrolRadius * 0.5f, PatrolRadius * 0.5f),
            0.0f
        );
        PatrolTarget = HomeLocation + RandomOffset;
    }

    FVector Direction = (PatrolTarget - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Direction, 0.4f); // Slow patrol speed
}

void ARaptorDinosaur::ReturnToHome()
{
    FVector Direction = (HomeLocation - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Direction, 0.6f);
}

float ARaptorDinosaur::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health -= ActualDamage;

    if (Health <= 0.0f)
    {
        Health = 0.0f;
        CurrentState = EWorld_DinoState::Dead;
        OnDeath();
    }
    else if (Health < MaxHealth * 0.3f)
    {
        // Flee when critically wounded
        CurrentState = EWorld_DinoState::Fleeing;
    }

    return ActualDamage;
}

void ARaptorDinosaur::OnDeath()
{
    // Disable collision and AI
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    GetWorld()->GetTimerManager().ClearTimer(AITickHandle);

    // Destroy after 10 seconds
    SetLifeSpan(10.0f);
}

void ARaptorDinosaur::unreal_log_attack(AActor* Target)
{
    if (Target)
    {
        UE_LOG(LogTemp, Log, TEXT("RaptorDinosaur [%s] attacked [%s] for %.1f damage"),
            *GetName(), *Target->GetName(), AttackDamage);
    }
}
