#include "NPC_DinosaurPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ANPC_DinosaurPawn::ANPC_DinosaurPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root collision component
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetSphereRadius(100.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create detection sphere for AI perception
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANPC_DinosaurPawn::OnDetectionSphereBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ANPC_DinosaurPawn::OnDetectionSphereEndOverlap);

    // Initialize default values
    TerritoryCenter = GetActorLocation();
    CurrentState = ENPC_DinosaurState::Idle;
    Species = ENPC_DinosaurSpecies::TRex;
    
    // Apply default species modifiers
    ApplySpeciesModifiers();
}

void ANPC_DinosaurPawn::BeginPlay()
{
    Super::BeginPlay();
    
    TerritoryCenter = GetActorLocation();
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Generate patrol points around territory
    if (PatrolPoints.Num() == 0)
    {
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) * PI / 180.0f;
            float Distance = TerritoryRadius * 0.7f;
            FVector PatrolPoint = TerritoryCenter + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                0.0f
            );
            PatrolPoints.Add(PatrolPoint);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s spawned at %s with species %d"), 
        *GetName(), *GetActorLocation().ToString(), (int32)Species);
}

void ANPC_DinosaurPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateStats(DeltaTime);
    CheckForThreats();
    UpdateMovement(DeltaTime);
    
    StateTimer += DeltaTime;
}

void ANPC_DinosaurPawn::SetSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    ApplySpeciesModifiers();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s species changed to %d"), *GetName(), (int32)Species);
}

void ANPC_DinosaurPawn::SetState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurState PreviousState = CurrentState;
        CurrentState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s state changed from %d to %d"), 
            *GetName(), (int32)PreviousState, (int32)CurrentState);
    }
}

void ANPC_DinosaurPawn::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Clamp(Stats.Health - DamageAmount, 0.0f, Stats.MaxHealth);
    
    if (DamageSource && DamageSource != this)
    {
        SetTarget(DamageSource);
        
        if (Stats.Health > Stats.MaxHealth * 0.3f)
        {
            SetState(ENPC_DinosaurState::Attacking);
            AggressionLevel = FMath::Min(AggressionLevel + 0.3f, 1.0f);
        }
        else
        {
            SetState(ENPC_DinosaurState::Fleeing);
            FearLevel = FMath::Min(FearLevel + 0.5f, 1.0f);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s took %f damage, health now %f"), 
        *GetName(), DamageAmount, Stats.Health);
}

void ANPC_DinosaurPawn::Heal(float HealAmount)
{
    Stats.Health = FMath::Clamp(Stats.Health + HealAmount, 0.0f, Stats.MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s healed %f, health now %f"), 
        *GetName(), HealAmount, Stats.Health);
}

bool ANPC_DinosaurPawn::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetWorld())
    {
        return false;
    }
    
    FVector StartLocation = GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, TargetLocation);
    
    if (Distance > Stats.SightRange)
    {
        return false;
    }
    
    // Line trace to check for obstacles
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TargetLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstacles
}

bool ANPC_DinosaurPawn::IsInTerritory(FVector Location) const
{
    float Distance = FVector::Dist(Location, TerritoryCenter);
    return Distance <= TerritoryRadius;
}

void ANPC_DinosaurPawn::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        LastKnownPlayerLocation = NewTarget->GetActorLocation();
        bPlayerDetected = true;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s acquired target: %s"), 
            *GetName(), *NewTarget->GetName());
    }
}

void ANPC_DinosaurPawn::ClearTarget()
{
    CurrentTarget = nullptr;
    bPlayerDetected = false;
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s cleared target"), *GetName());
}

FVector ANPC_DinosaurPawn::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetActorLocation();
    }
    
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex];
}

void ANPC_DinosaurPawn::JoinPack(ANPC_DinosaurPawn* Leader)
{
    if (Leader && Leader != this)
    {
        PackLeader = Leader;
        bIsPackLeader = false;
        
        if (Leader->PackMembers.Find(this) == INDEX_NONE)
        {
            Leader->PackMembers.Add(this);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s joined pack led by %s"), 
            *GetName(), *Leader->GetName());
    }
}

void ANPC_DinosaurPawn::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    PackMembers.Empty();
    bIsPackLeader = false;
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s left pack"), *GetName());
}

void ANPC_DinosaurPawn::CallPackToTarget(AActor* Target)
{
    if (!bIsPackLeader || !Target)
    {
        return;
    }
    
    for (ANPC_DinosaurPawn* Member : PackMembers)
    {
        if (Member && IsValid(Member))
        {
            Member->SetTarget(Target);
            Member->SetState(ENPC_DinosaurState::Hunting);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pack leader %s called pack to target %s"), 
        *GetName(), *Target->GetName());
}

void ANPC_DinosaurPawn::UpdateStats(float DeltaTime)
{
    // Hunger increases over time
    Stats.Hunger = FMath::Clamp(Stats.Hunger + (DeltaTime * 0.5f), 0.0f, 100.0f);
    
    // Stamina regeneration when not in combat
    if (CurrentState != ENPC_DinosaurState::Attacking && CurrentState != ENPC_DinosaurState::Fleeing)
    {
        Stats.Stamina = FMath::Clamp(Stats.Stamina + (DeltaTime * 10.0f), 0.0f, 100.0f);
    }
    
    // Health regeneration when well-fed and not in combat
    if (Stats.Hunger < 30.0f && CurrentState == ENPC_DinosaurState::Idle)
    {
        Stats.Health = FMath::Clamp(Stats.Health + (DeltaTime * 2.0f), 0.0f, Stats.MaxHealth);
    }
    
    // Reduce aggression and fear over time
    AggressionLevel = FMath::Max(AggressionLevel - (DeltaTime * 0.1f), 0.0f);
    FearLevel = FMath::Max(FearLevel - (DeltaTime * 0.1f), 0.0f);
}

void ANPC_DinosaurPawn::CheckForThreats()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Check if current target is still valid and visible
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        if (!CanSeeTarget(CurrentTarget))
        {
            // Lost sight of target
            if (StateTimer > 5.0f) // Give up after 5 seconds
            {
                ClearTarget();
                SetState(ENPC_DinosaurState::Patrolling);
            }
        }
    }
    
    // Look for new threats among detected actors
    for (AActor* DetectedActor : DetectedActors)
    {
        if (!DetectedActor || !IsValid(DetectedActor))
        {
            continue;
        }
        
        // Check if it's a player character
        if (DetectedActor->IsA<APawn>() && CanSeeTarget(DetectedActor))
        {
            float Distance = FVector::Dist(GetActorLocation(), DetectedActor->GetActorLocation());
            
            // React based on species and current state
            if (bIsCarnivore && Distance < Stats.SightRange * 0.8f)
            {
                SetTarget(DetectedActor);
                SetState(ENPC_DinosaurState::Hunting);
                
                // Call pack if this is a pack hunter
                if (bIsPackHunter && bIsPackLeader)
                {
                    CallPackToTarget(DetectedActor);
                }
            }
            else if (!bIsCarnivore && Distance < Stats.SightRange * 0.5f)
            {
                // Herbivores flee from threats
                SetState(ENPC_DinosaurState::Fleeing);
                FearLevel = FMath::Min(FearLevel + 0.2f, 1.0f);
            }
        }
    }
}

void ANPC_DinosaurPawn::UpdateMovement(float DeltaTime)
{
    FVector TargetLocation = GetActorLocation();
    bool bShouldMove = false;
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Patrolling:
        {
            if (PatrolPoints.Num() > 0)
            {
                TargetLocation = PatrolPoints[CurrentPatrolIndex];
                float Distance = FVector::Dist(GetActorLocation(), TargetLocation);
                
                if (Distance < 200.0f)
                {
                    TargetLocation = GetNextPatrolPoint();
                }
                bShouldMove = true;
            }
            break;
        }
        
        case ENPC_DinosaurState::Hunting:
        case ENPC_DinosaurState::Attacking:
        {
            if (CurrentTarget && IsValid(CurrentTarget))
            {
                TargetLocation = CurrentTarget->GetActorLocation();
                bShouldMove = true;
            }
            else
            {
                SetState(ENPC_DinosaurState::Patrolling);
            }
            break;
        }
        
        case ENPC_DinosaurState::Fleeing:
        {
            if (CurrentTarget && IsValid(CurrentTarget))
            {
                // Move away from threat
                FVector FleeDirection = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
                TargetLocation = GetActorLocation() + (FleeDirection * 1000.0f);
                bShouldMove = true;
            }
            else
            {
                SetState(ENPC_DinosaurState::Idle);
            }
            break;
        }
        
        default:
            break;
    }
    
    if (bShouldMove)
    {
        FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
        FVector NewLocation = GetActorLocation() + (Direction * Stats.MovementSpeed * DeltaTime);
        SetActorLocation(NewLocation);
        
        // Face movement direction
        if (!Direction.IsZero())
        {
            FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 2.0f));
        }
    }
}

void ANPC_DinosaurPawn::ApplySpeciesModifiers()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
        {
            Stats.MaxHealth = 200.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 50.0f;
            Stats.MovementSpeed = 400.0f;
            Stats.SightRange = 3000.0f;
            bIsCarnivore = true;
            bIsPackHunter = false;
            TerritoryRadius = 5000.0f;
            DetectionSphere->SetSphereRadius(Stats.SightRange);
            CollisionComponent->SetSphereRadius(150.0f);
            break;
        }
        
        case ENPC_DinosaurSpecies::Raptor:
        {
            Stats.MaxHealth = 80.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 30.0f;
            Stats.MovementSpeed = 600.0f;
            Stats.SightRange = 2500.0f;
            bIsCarnivore = true;
            bIsPackHunter = true;
            TerritoryRadius = 3000.0f;
            DetectionSphere->SetSphereRadius(Stats.SightRange);
            CollisionComponent->SetSphereRadius(80.0f);
            break;
        }
        
        case ENPC_DinosaurSpecies::Triceratops:
        {
            Stats.MaxHealth = 250.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 40.0f;
            Stats.MovementSpeed = 250.0f;
            Stats.SightRange = 1500.0f;
            bIsCarnivore = false;
            bIsPackHunter = false;
            TerritoryRadius = 2000.0f;
            DetectionSphere->SetSphereRadius(Stats.SightRange);
            CollisionComponent->SetSphereRadius(120.0f);
            break;
        }
        
        case ENPC_DinosaurSpecies::Brachiosaurus:
        {
            Stats.MaxHealth = 400.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 60.0f;
            Stats.MovementSpeed = 150.0f;
            Stats.SightRange = 2000.0f;
            bIsCarnivore = false;
            bIsPackHunter = false;
            TerritoryRadius = 4000.0f;
            DetectionSphere->SetSphereRadius(Stats.SightRange);
            CollisionComponent->SetSphereRadius(200.0f);
            break;
        }
        
        case ENPC_DinosaurSpecies::Stegosaurus:
        {
            Stats.MaxHealth = 180.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 35.0f;
            Stats.MovementSpeed = 200.0f;
            Stats.SightRange = 1800.0f;
            bIsCarnivore = false;
            bIsPackHunter = false;
            TerritoryRadius = 2500.0f;
            DetectionSphere->SetSphereRadius(Stats.SightRange);
            CollisionComponent->SetSphereRadius(100.0f);
            break;
        }
    }
}

void ANPC_DinosaurPawn::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        DetectedActors.AddUnique(OtherActor);
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s detected actor: %s"), 
            *GetName(), *OtherActor->GetName());
    }
}

void ANPC_DinosaurPawn::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        DetectedActors.Remove(OtherActor);
        
        if (CurrentTarget == OtherActor)
        {
            ClearTarget();
            SetState(ENPC_DinosaurState::Patrolling);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s lost detection of actor: %s"), 
            *GetName(), *OtherActor->GetName());
    }
}