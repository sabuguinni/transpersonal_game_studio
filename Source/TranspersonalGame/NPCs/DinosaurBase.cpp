#include "DinosaurBase.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(200.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);

    // Create mesh component
    DinosaurMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DinosaurMesh"));
    DinosaurMesh->SetupAttachment(RootComponent);
    DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create pawn sensing component
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SetSensingUpdatesEnabled(true);
    PawnSensing->SightRadius = 3000.0f;
    PawnSensing->HearingThreshold = 1000.0f;
    PawnSensing->LOSHearingThreshold = 1500.0f;

    // Initialize default stats
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    MovementSpeed = 300.0f;
    AttackDamage = 25.0f;
    DetectionRange = 2000.0f;
    AttackRange = 150.0f;

    // Initialize territory
    TerritoryCenter = FVector::ZeroVector;
    TerritoryRadius = 5000.0f;
    PatrolTarget = FVector::ZeroVector;

    // Initialize pack behavior
    bIsPackLeader = false;
    PackLeader = nullptr;

    // Set default species and behavior
    Species = EDinosaurSpecies::Velociraptor;
    BehaviorType = EDinosaurBehavior::Territorial;

    // Set AI controller class
    AIControllerClass = AAIController::StaticClass();
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize territory center to spawn location
    TerritoryCenter = GetActorLocation();
    PatrolTarget = GetRandomPatrolPoint();

    // Bind pawn sensing events
    if (PawnSensing)
    {
        PawnSensing->OnSeePawn.AddDynamic(this, &ADinosaurBase::OnPawnSeen);
        PawnSensing->OnHearNoise.AddDynamic(this, &ADinosaurBase::OnPawnLost);
    }

    // Initialize AI
    InitializeAI();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update patrol behavior
    UpdatePatrol();

    // Update pack behavior if in a pack
    if (PackLeader || bIsPackLeader)
    {
        UpdatePackBehavior();
    }
}

void ADinosaurBase::InitializeAI()
{
    DinosaurAI = Cast<AAIController>(GetController());
    if (DinosaurAI && BehaviorTree)
    {
        DinosaurAI->RunBehaviorTree(BehaviorTree);
        
        // Set blackboard values
        if (UBlackboardComponent* BlackboardComp = DinosaurAI->GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
            BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), TerritoryRadius);
            BlackboardComp->SetValueAsVector(TEXT("PatrolTarget"), PatrolTarget);
            BlackboardComp->SetValueAsFloat(TEXT("DetectionRange"), DetectionRange);
            BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
        }
    }
}

void ADinosaurBase::SetTarget(APawn* NewTarget)
{
    TargetPawn = NewTarget;
    
    if (DinosaurAI && DinosaurAI->GetBlackboardComponent())
    {
        DinosaurAI->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetPawn"), NewTarget);
    }

    // Alert pack if this is a pack leader
    if (bIsPackLeader && NewTarget)
    {
        AlertPack(NewTarget);
    }
}

void ADinosaurBase::ClearTarget()
{
    TargetPawn = nullptr;
    
    if (DinosaurAI && DinosaurAI->GetBlackboardComponent())
    {
        DinosaurAI->GetBlackboardComponent()->ClearValue(TEXT("TargetPawn"));
    }
}

bool ADinosaurBase::IsInAttackRange()
{
    if (!TargetPawn)
        return false;

    float Distance = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
    return Distance <= AttackRange;
}

bool ADinosaurBase::IsInDetectionRange(APawn* TestPawn)
{
    if (!TestPawn)
        return false;

    float Distance = FVector::Dist(GetActorLocation(), TestPawn->GetActorLocation());
    return Distance <= DetectionRange;
}

void ADinosaurBase::AttackTarget()
{
    if (!TargetPawn || !IsInAttackRange())
        return;

    // Simple damage application
    if (IsPlayerPawn(TargetPawn))
    {
        // Apply damage to player character
        UE_LOG(LogTemp, Warning, TEXT("%s attacking player for %f damage"), *GetName(), AttackDamage);
    }
}

FVector ADinosaurBase::GetRandomPatrolPoint()
{
    // Generate random point within territory
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
    FVector NewPatrolPoint = TerritoryCenter + (RandomDirection * RandomDistance);
    
    return NewPatrolPoint;
}

void ADinosaurBase::UpdatePatrol()
{
    if (TargetPawn)
        return; // Don't patrol when we have a target

    // Check if we've reached our patrol target
    float DistanceToPatrol = FVector::Dist(GetActorLocation(), PatrolTarget);
    if (DistanceToPatrol < 200.0f)
    {
        // Generate new patrol point
        PatrolTarget = GetRandomPatrolPoint();
        
        if (DinosaurAI && DinosaurAI->GetBlackboardComponent())
        {
            DinosaurAI->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolTarget"), PatrolTarget);
        }
    }
}

void ADinosaurBase::JoinPack(ADinosaurBase* Leader)
{
    if (!Leader || Leader == this)
        return;

    PackLeader = Leader;
    bIsPackLeader = false;

    // Add to leader's pack
    Leader->PackMembers.AddUnique(this);
}

void ADinosaurBase::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }

    // If this was a leader, disband the pack
    if (bIsPackLeader)
    {
        for (ADinosaurBase* Member : PackMembers)
        {
            if (Member)
            {
                Member->PackLeader = nullptr;
            }
        }
        PackMembers.Empty();
        bIsPackLeader = false;
    }
}

void ADinosaurBase::AlertPack(APawn* Threat)
{
    if (!bIsPackLeader || !Threat)
        return;

    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member && Member->IsAlive())
        {
            Member->SetTarget(Threat);
        }
    }
}

void ADinosaurBase::UpdatePackBehavior()
{
    if (bIsPackLeader)
    {
        // Leader behavior - coordinate pack
        if (TargetPawn)
        {
            // Ensure all pack members are targeting the same threat
            AlertPack(TargetPawn);
        }
    }
    else if (PackLeader)
    {
        // Member behavior - follow leader's commands
        if (PackLeader->TargetPawn && !TargetPawn)
        {
            SetTarget(PackLeader->TargetPawn);
        }
    }
}

void ADinosaurBase::TakeDamage(float Damage)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
    
    if (CurrentHealth <= 0.0f)
    {
        // Handle death
        LeavePack();
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
    }
}

bool ADinosaurBase::IsAlive()
{
    return CurrentHealth > 0.0f;
}

bool ADinosaurBase::IsPlayerPawn(APawn* TestPawn)
{
    if (!TestPawn)
        return false;

    return TestPawn->IsA<APawn>() && TestPawn->GetController() && TestPawn->GetController()->IsA<APlayerController>();
}

void ADinosaurBase::OnPawnSeen(APawn* SeenPawn)
{
    if (!SeenPawn || !IsAlive())
        return;

    // Only react to player pawns
    if (IsPlayerPawn(SeenPawn))
    {
        SetTarget(SeenPawn);
        UE_LOG(LogTemp, Warning, TEXT("%s detected player at distance %f"), *GetName(), 
               FVector::Dist(GetActorLocation(), SeenPawn->GetActorLocation()));
    }
}

void ADinosaurBase::OnPawnLost(APawn* LostPawn)
{
    if (LostPawn == TargetPawn)
    {
        // Lost sight of target, clear it after a delay
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { ClearTarget(); },
            5.0f, // 5 second delay before giving up
            false
        );
    }
}