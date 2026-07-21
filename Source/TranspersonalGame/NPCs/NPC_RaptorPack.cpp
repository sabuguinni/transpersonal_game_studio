#include "NPC_RaptorPack.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ANPC_RaptorPack::ANPC_RaptorPack()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create capsule component
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->SetCapsuleHalfHeight(88.0f);
    CapsuleComponent->SetCapsuleRadius(34.0f);

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(3000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create pawn sensing component
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SetPeripheralVisionAngle(90.0f);
    PawnSensing->SightRadius = 4000.0f;

    // Initialize pack data
    PackData.PackSize = 3;
    PackData.PackRadius = 1500.0f;
    PackData.HuntingRange = 5000.0f;
    PackData.bIsHunting = false;

    // Initialize stats
    MovementSpeed = 600.0f;
    AttackDamage = 45.0f;
    MaxHealth = 120.0f;
    Health = MaxHealth;

    // Initialize state
    CurrentTarget = nullptr;
    PatrolCenter = FVector::ZeroVector;
    bIsPackLeader = false;

    // Bind events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANPC_RaptorPack::OnDetectionOverlapBegin);
    PawnSensing->OnSeePawn.AddDynamic(this, &ANPC_RaptorPack::OnSeePlayer);
}

void ANPC_RaptorPack::BeginPlay()
{
    Super::BeginPlay();

    // Set initial patrol center
    PatrolCenter = GetActorLocation();

    // Load basic mesh if available
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (MeshAsset.Succeeded() && MeshComponent)
    {
        MeshComponent->SetStaticMesh(MeshAsset.Object);
        MeshComponent->SetWorldScale3D(FVector(2.0f, 1.0f, 1.5f));
    }

    UE_LOG(LogTemp, Warning, TEXT("RaptorPack spawned at %s"), *GetActorLocation().ToString());
}

void ANPC_RaptorPack::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Health <= 0.0f)
    {
        return;
    }

    UpdatePackCoordination();

    if (PackData.bIsHunting && CurrentTarget)
    {
        if (IsTargetInRange())
        {
            MoveTowardsTarget(DeltaTime);
            
            // Attack if close enough
            float DistanceToTarget = GetDistanceToTarget();
            if (DistanceToTarget < 200.0f)
            {
                AttackTarget();
            }
        }
        else
        {
            // Target too far, stop hunting
            StopHunting();
        }
    }
    else
    {
        // Patrol behavior
        PatrolArea();
    }
}

void ANPC_RaptorPack::InitializePackMember(bool bAsLeader)
{
    bIsPackLeader = bAsLeader;
    
    if (bIsPackLeader)
    {
        PackMembers.Add(this);
        UE_LOG(LogTemp, Warning, TEXT("RaptorPack initialized as pack leader"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RaptorPack initialized as pack member"));
    }
}

void ANPC_RaptorPack::AddToPackMember(ANPC_RaptorPack* NewMember)
{
    if (NewMember && !PackMembers.Contains(NewMember))
    {
        PackMembers.Add(NewMember);
        NewMember->PackMembers = PackMembers;
        UE_LOG(LogTemp, Warning, TEXT("Added raptor to pack, total size: %d"), PackMembers.Num());
    }
}

void ANPC_RaptorPack::SetPackTarget(APawn* Target)
{
    CurrentTarget = Target;
    
    // Share target with pack members
    for (ANPC_RaptorPack* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->CurrentTarget = Target;
        }
    }

    if (Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("RaptorPack acquired target: %s"), *Target->GetName());
    }
}

void ANPC_RaptorPack::StartHunting()
{
    PackData.bIsHunting = true;
    
    // Coordinate with pack
    for (ANPC_RaptorPack* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->PackData.bIsHunting = true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("RaptorPack started hunting"));
}

void ANPC_RaptorPack::StopHunting()
{
    PackData.bIsHunting = false;
    CurrentTarget = nullptr;
    
    // Coordinate with pack
    for (ANPC_RaptorPack* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->PackData.bIsHunting = false;
            Member->CurrentTarget = nullptr;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("RaptorPack stopped hunting"));
}

void ANPC_RaptorPack::PatrolArea()
{
    static float PatrolTimer = 0.0f;
    PatrolTimer += GetWorld()->GetDeltaSeconds();

    if (PatrolTimer > 5.0f) // Change direction every 5 seconds
    {
        FVector RandomPoint = GetRandomPatrolPoint();
        MoveTowardsLocation(RandomPoint, GetWorld()->GetDeltaSeconds());
        PatrolTimer = 0.0f;
    }
}

void ANPC_RaptorPack::AttackTarget()
{
    if (!CurrentTarget)
    {
        return;
    }

    // Simple attack implementation
    UE_LOG(LogTemp, Warning, TEXT("RaptorPack attacking target!"));

    // Apply damage if target has health component
    if (CurrentTarget->IsA<APawn>())
    {
        // For now, just log the attack
        UE_LOG(LogTemp, Warning, TEXT("RaptorPack deals %f damage to %s"), AttackDamage, *CurrentTarget->GetName());
    }
}

float ANPC_RaptorPack::GetDistanceToTarget() const
{
    if (!CurrentTarget)
    {
        return 99999.0f;
    }

    return FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ANPC_RaptorPack::IsTargetInRange() const
{
    return GetDistanceToTarget() <= PackData.HuntingRange;
}

void ANPC_RaptorPack::TakeDamage(float DamageAmount)
{
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    
    UE_LOG(LogTemp, Warning, TEXT("RaptorPack took %f damage, health: %f/%f"), DamageAmount, Health, MaxHealth);

    if (Health <= 0.0f)
    {
        Die();
    }
}

void ANPC_RaptorPack::Die()
{
    UE_LOG(LogTemp, Warning, TEXT("RaptorPack died"));
    
    // Remove from pack
    for (ANPC_RaptorPack* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->PackMembers.Remove(this);
        }
    }

    // Disable collision and movement
    SetActorEnableCollision(false);
    PrimaryActorTick.bCanEverTick = false;

    // Schedule destruction
    SetLifeSpan(5.0f);
}

void ANPC_RaptorPack::OnSeePlayer(APawn* SeenPawn)
{
    if (!SeenPawn || PackData.bIsHunting)
    {
        return;
    }

    // Check if it's the player
    if (SeenPawn->IsA<APawn>() && SeenPawn->IsPlayerControlled())
    {
        SetPackTarget(SeenPawn);
        StartHunting();
        UE_LOG(LogTemp, Warning, TEXT("RaptorPack spotted player, starting hunt!"));
    }
}

void ANPC_RaptorPack::OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || PackData.bIsHunting)
    {
        return;
    }

    APawn* OtherPawn = Cast<APawn>(OtherActor);
    if (OtherPawn && OtherPawn->IsPlayerControlled())
    {
        SetPackTarget(OtherPawn);
        StartHunting();
        UE_LOG(LogTemp, Warning, TEXT("RaptorPack detected player in range!"));
    }
}

FVector ANPC_RaptorPack::GetRandomPatrolPoint() const
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();

    float RandomDistance = FMath::RandRange(500.0f, PackData.PackRadius);
    return PatrolCenter + (RandomDirection * RandomDistance);
}

void ANPC_RaptorPack::UpdatePackCoordination()
{
    // Clean up invalid pack members
    PackMembers.RemoveAll([](ANPC_RaptorPack* Member) {
        return !IsValid(Member) || Member->Health <= 0.0f;
    });

    // Update pack leader status
    if (PackMembers.Num() > 0 && !bIsPackLeader)
    {
        // Check if we should become the leader
        bool bHasLeader = false;
        for (ANPC_RaptorPack* Member : PackMembers)
        {
            if (Member && Member->bIsPackLeader)
            {
                bHasLeader = true;
                break;
            }
        }

        if (!bHasLeader)
        {
            bIsPackLeader = true;
            UE_LOG(LogTemp, Warning, TEXT("RaptorPack became new pack leader"));
        }
    }
}

void ANPC_RaptorPack::MoveTowardsTarget(float DeltaTime)
{
    if (!CurrentTarget)
    {
        return;
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    MoveTowardsLocation(TargetLocation, DeltaTime);
}

void ANPC_RaptorPack::MoveTowardsLocation(const FVector& TargetLocation, float DeltaTime)
{
    FVector CurrentLocation = GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);
    SetActorLocation(NewLocation);

    // Rotate to face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = Direction.Rotation();
        SetActorRotation(NewRotation);
    }
}