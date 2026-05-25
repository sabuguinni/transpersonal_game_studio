#include "Combat_DinosaurCombatSystem.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_DinosaurCombatSystem::ACombat_DinosaurCombatSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create detection sphere component
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_DinosaurCombatSystem::OnDetectionSphereBeginOverlap);

    // Initialize default values
    CurrentState = ECombat_DinosaurState::Idle;
    TargetActor = nullptr;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;

    // Set default dinosaur stats
    DinosaurStats.Health = 100.0f;
    DinosaurStats.AttackDamage = 25.0f;
    DinosaurStats.AttackRange = 300.0f;
    DinosaurStats.DetectionRadius = 2000.0f;
    DinosaurStats.MovementSpeed = 400.0f;
}

void ACombat_DinosaurCombatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Set detection sphere radius from stats
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(DinosaurStats.DetectionRadius);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem initialized at %s"), *GetActorLocation().ToString());
}

void ACombat_DinosaurCombatSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // State machine logic
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
        {
            // Look for nearby players
            AActor* Player = DetectNearbyPlayer();
            if (Player)
            {
                TargetActor = Player;
                SetCombatState(ECombat_DinosaurState::Hunting);
            }
            break;
        }
        
        case ECombat_DinosaurState::Hunting:
        {
            if (TargetActor && !TargetActor->IsPendingKill())
            {
                float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
                
                if (DistanceToTarget <= DinosaurStats.AttackRange)
                {
                    SetCombatState(ECombat_DinosaurState::Attacking);
                }
                else if (DistanceToTarget > DinosaurStats.DetectionRadius * 1.5f)
                {
                    // Lost target
                    TargetActor = nullptr;
                    SetCombatState(ECombat_DinosaurState::Idle);
                }
                else
                {
                    // Move towards target
                    MoveTowardsTarget(DeltaTime);
                }
            }
            else
            {
                TargetActor = nullptr;
                SetCombatState(ECombat_DinosaurState::Idle);
            }
            break;
        }
        
        case ECombat_DinosaurState::Attacking:
        {
            if (TargetActor && !TargetActor->IsPendingKill())
            {
                float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
                
                if (DistanceToTarget <= DinosaurStats.AttackRange && CanAttack())
                {
                    PerformAttack();
                }
                else if (DistanceToTarget > DinosaurStats.AttackRange)
                {
                    SetCombatState(ECombat_DinosaurState::Hunting);
                }
            }
            else
            {
                TargetActor = nullptr;
                SetCombatState(ECombat_DinosaurState::Idle);
            }
            break;
        }
        
        case ECombat_DinosaurState::Dead:
        {
            // Do nothing when dead
            break;
        }
    }
}

void ACombat_DinosaurCombatSystem::SetCombatState(ECombat_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_DinosaurState OldState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur state changed from %d to %d"), (int32)OldState, (int32)NewState);
    }
}

bool ACombat_DinosaurCombatSystem::CanAttack() const
{
    if (CurrentState == ECombat_DinosaurState::Dead)
        return false;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void ACombat_DinosaurCombatSystem::PerformAttack()
{
    if (!CanAttack() || !TargetActor)
        return;
        
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Apply damage to target if it's a character
    if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
    {
        // Simple damage application - in a full system this would be more sophisticated
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur attacking %s for %f damage!"), 
               *TargetCharacter->GetName(), DinosaurStats.AttackDamage);
               
        // Visual feedback
        DrawDebugSphere(GetWorld(), TargetActor->GetActorLocation(), 100.0f, 12, FColor::Red, false, 1.0f);
    }
}

AActor* ACombat_DinosaurCombatSystem::DetectNearbyPlayer()
{
    if (!GetWorld())
        return nullptr;
        
    // Find all characters within detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && !Actor->IsPendingKill())
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= DinosaurStats.DetectionRadius)
            {
                return Actor;
            }
        }
    }
    
    return nullptr;
}

void ACombat_DinosaurCombatSystem::MoveTowardsTarget(float DeltaTime)
{
    if (!TargetActor)
        return;
        
    FVector CurrentLocation = GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    // Simple movement towards target
    FVector NewLocation = CurrentLocation + (Direction * DinosaurStats.MovementSpeed * DeltaTime);
    SetActorLocation(NewLocation);
    
    // Face the target
    FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookRotation, DeltaTime, 2.0f));
}

void ACombat_DinosaurCombatSystem::TakeDamage(float Damage)
{
    if (CurrentState == ECombat_DinosaurState::Dead)
        return;
        
    DinosaurStats.Health -= Damage;
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur took %f damage, health now: %f"), Damage, DinosaurStats.Health);
    
    if (DinosaurStats.Health <= 0.0f)
    {
        DinosaurStats.Health = 0.0f;
        SetCombatState(ECombat_DinosaurState::Dead);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur died!"));
    }
}

void ACombat_DinosaurCombatSystem::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentState == ECombat_DinosaurState::Dead)
        return;
        
    // Check if the overlapping actor is a character (player)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (CurrentState == ECombat_DinosaurState::Idle && !TargetActor)
        {
            TargetActor = Character;
            SetCombatState(ECombat_DinosaurState::Hunting);
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur detected player: %s"), *Character->GetName());
        }
    }
}