#include "DinosaurCombatAI.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UDinosaurCombatComponent
// ============================================================

UDinosaurCombatComponent::UDinosaurCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UDinosaurCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ECombat_DinoState::Patrolling;
    AttackCooldownTimer = 0.f;
    StateTimer = 0.f;
}

void UDinosaurCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AttackCooldownTimer = FMath::Max(0.f, AttackCooldownTimer - DeltaTime);
    StateTimer += DeltaTime;

    switch (CurrentState)
    {
    case ECombat_DinoState::Hunting:
        UpdateHuntingBehavior(DeltaTime);
        break;
    case ECombat_DinoState::Patrolling:
        UpdatePatrolBehavior(DeltaTime);
        break;
    case ECombat_DinoState::Fleeing:
        UpdateFleeingBehavior(DeltaTime);
        break;
    case ECombat_DinoState::Attacking:
        if (AttackCooldownTimer <= 0.f && CurrentTarget)
        {
            ExecuteAttack(CurrentTarget);
        }
        break;
    default:
        break;
    }
}

void UDinosaurCombatComponent::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.f;
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombat: State -> %d"), (int32)NewState);
}

void UDinosaurCombatComponent::DetectPlayer(AActor* PlayerActor)
{
    if (!PlayerActor || !IsAlive()) return;

    CurrentTarget = PlayerActor;

    if (DinoStats.bIsPackHunter)
    {
        NotifyPackMembersOfTarget(PlayerActor);
    }

    SetCombatState(ECombat_DinoState::Hunting);
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombat: Player detected! Hunting initiated."));
}

void UDinosaurCombatComponent::ExecuteAttack(AActor* Target)
{
    if (!Target || AttackCooldownTimer > 0.f || !IsAlive()) return;

    if (!IsTargetInAttackRange()) return;

    AttackCooldownTimer = DinoStats.AttackCooldown;
    SetCombatState(ECombat_DinoState::Attacking);

    // Apply damage to target
    FCombat_AttackData AttackData;
    AttackData.Damage = DinoStats.AttackDamage;
    AttackData.AttackType = FName("Bite");

    UGameplayStatics::ApplyDamage(Target, AttackData.Damage, nullptr, GetOwner(), nullptr);
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombat: Attack executed! Damage=%.1f"), AttackData.Damage);
}

float UDinosaurCombatComponent::TakeDamage_Combat(float DamageAmount, AActor* DamageCauser)
{
    if (!IsAlive()) return 0.f;

    DinoStats.CurrentHealth = FMath::Max(0.f, DinoStats.CurrentHealth - DamageAmount);
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombat: Took %.1f damage. HP=%.1f/%.1f"),
        DamageAmount, DinoStats.CurrentHealth, DinoStats.MaxHealth);

    // Flee if critically wounded (below 25% health)
    if (DinoStats.CurrentHealth < DinoStats.MaxHealth * 0.25f)
    {
        SetCombatState(ECombat_DinoState::Fleeing);
    }
    else if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
    {
        // Counter-attack if hit while passive
        CurrentTarget = DamageCauser;
        SetCombatState(ECombat_DinoState::Hunting);
    }

    return DamageAmount;
}

void UDinosaurCombatComponent::NotifyPackMembersOfTarget(AActor* Target)
{
    for (AActor* Member : PackMembers)
    {
        if (!Member || Member == GetOwner()) continue;
        UDinosaurCombatComponent* MemberCombat = Member->FindComponentByClass<UDinosaurCombatComponent>();
        if (MemberCombat && MemberCombat->CurrentState == ECombat_DinoState::Idle ||
            (MemberCombat && MemberCombat->CurrentState == ECombat_DinoState::Patrolling))
        {
            MemberCombat->CurrentTarget = Target;
            MemberCombat->SetCombatState(ECombat_DinoState::Hunting);
        }
    }
}

void UDinosaurCombatComponent::UpdateHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !IsAlive()) return;

    float DistToTarget = GetDistanceToTarget();

    if (DistToTarget <= DinoStats.AttackRange)
    {
        SetCombatState(ECombat_DinoState::Attacking);
    }
    else if (DistToTarget > DinoStats.DetectionRadius * 1.5f)
    {
        // Lost the target
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Patrolling);
    }
}

void UDinosaurCombatComponent::UpdatePatrolBehavior(float DeltaTime)
{
    // Patrol logic — scan for player every 2 seconds
    if (StateTimer > 2.f)
    {
        StateTimer = 0.f;
        UWorld* World = GetWorld();
        if (!World) return;

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            AActor* Owner = GetOwner();
            if (Owner)
            {
                float Dist = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
                if (Dist <= DinoStats.DetectionRadius)
                {
                    DetectPlayer(PlayerPawn);
                }
            }
        }
    }
}

void UDinosaurCombatComponent::UpdateFleeingBehavior(float DeltaTime)
{
    // After 10 seconds of fleeing, return to patrol
    if (StateTimer > 10.f)
    {
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Patrolling);
    }
}

bool UDinosaurCombatComponent::IsTargetInAttackRange() const
{
    if (!CurrentTarget || !GetOwner()) return false;
    return GetDistanceToTarget() <= DinoStats.AttackRange;
}

float UDinosaurCombatComponent::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

// ============================================================
// ADinosaurCombatAI
// ============================================================

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetSphereRadius(1500.f);
    DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = DetectionSphere;

    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetSphereRadius(200.f);
    AttackSphere->SetCollisionProfileName(TEXT("Trigger"));
    AttackSphere->SetupAttachment(RootComponent);

    CombatComponent = CreateDefaultSubobject<UDinosaurCombatComponent>(TEXT("CombatComponent"));
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurCombatAI::OnDetectionSphereBeginOverlap);
    AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurCombatAI::OnAttackSphereBeginOverlap);
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADinosaurCombatAI::InitializeDinosaur(ECombat_DinoSpecies InSpecies)
{
    if (!CombatComponent) return;
    CombatComponent->Species = InSpecies;

    switch (InSpecies)
    {
    case ECombat_DinoSpecies::Velociraptor:
        CombatComponent->DinoStats.MaxHealth = 80.f;
        CombatComponent->DinoStats.CurrentHealth = 80.f;
        CombatComponent->DinoStats.AttackDamage = 20.f;
        CombatComponent->DinoStats.AttackRange = 150.f;
        CombatComponent->DinoStats.MoveSpeed = 800.f;
        CombatComponent->DinoStats.bIsPackHunter = true;
        CombatComponent->DinoStats.PackSize = 3;
        DetectionSphere->SetSphereRadius(1200.f);
        break;

    case ECombat_DinoSpecies::TyrannosaurusRex:
        CombatComponent->DinoStats.MaxHealth = 500.f;
        CombatComponent->DinoStats.CurrentHealth = 500.f;
        CombatComponent->DinoStats.AttackDamage = 80.f;
        CombatComponent->DinoStats.AttackRange = 350.f;
        CombatComponent->DinoStats.MoveSpeed = 500.f;
        CombatComponent->DinoStats.bIsPackHunter = false;
        CombatComponent->DinoStats.PackSize = 1;
        DetectionSphere->SetSphereRadius(2500.f);
        break;

    case ECombat_DinoSpecies::Triceratops:
        CombatComponent->DinoStats.MaxHealth = 300.f;
        CombatComponent->DinoStats.CurrentHealth = 300.f;
        CombatComponent->DinoStats.AttackDamage = 40.f;
        CombatComponent->DinoStats.AttackRange = 250.f;
        CombatComponent->DinoStats.MoveSpeed = 450.f;
        CombatComponent->DinoStats.bIsPackHunter = false;
        DetectionSphere->SetSphereRadius(800.f);
        break;

    default:
        break;
    }

    AttackSphere->SetSphereRadius(CombatComponent->DinoStats.AttackRange);
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Initialized as species %d"), (int32)InSpecies);
}

void ADinosaurCombatAI::OnPlayerDetected(AActor* Player)
{
    if (CombatComponent)
    {
        CombatComponent->DetectPlayer(Player);
    }
}

float ADinosaurCombatAI::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (CombatComponent)
    {
        CombatComponent->TakeDamage_Combat(DamageAmount, DamageCauser);
    }
    return ActualDamage;
}

void ADinosaurCombatAI::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    // Check if it's the player pawn
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && PlayerPawn->IsPlayerControlled())
    {
        OnPlayerDetected(OtherActor);
    }
}

void ADinosaurCombatAI::OnAttackSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && PlayerPawn->IsPlayerControlled() && CombatComponent)
    {
        CombatComponent->ExecuteAttack(OtherActor);
    }
}
