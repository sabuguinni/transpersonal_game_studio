#include "DinosaurBase.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configurar mesh principal
    DinosaurMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DinosaurMesh"));
    RootComponent = DinosaurMesh;

    // Configurar esfera de detecção
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Configurar esfera de ataque
    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetupAttachment(RootComponent);
    AttackSphere->SetSphereRadius(500.0f);
    AttackSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AttackSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AttackSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Configurações iniciais
    Species = ENPC_DinosaurSpecies::TRex;
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    CurrentPatrolIndex = 0;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;

    // Configurar stats padrão
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.AttackDamage = 25.0f;
    Stats.MovementSpeed = 300.0f;
    Stats.DetectionRange = 2000.0f;
    Stats.AttackRange = 500.0f;
    Stats.Hunger = 50.0f;
    Stats.Aggression = 0.5f;

    // Configurar AI Controller
    AIControllerClass = AAIController::StaticClass();
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Configurar eventos de overlap
    if (DetectionSphere)
    {
        DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionSphereBeginOverlap);
        DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionSphereEndOverlap);
        DetectionSphere->SetSphereRadius(Stats.DetectionRange);
    }

    if (AttackSphere)
    {
        AttackSphere->SetSphereRadius(Stats.AttackRange);
    }

    // Configurar para a espécie
    ConfigureForSpecies(Species);

    // Iniciar Behavior Tree se disponível
    if (BehaviorTreeAsset)
    {
        if (AAIController* AIController = Cast<AAIController>(GetController()))
        {
            AIController->RunBehaviorTree(BehaviorTreeAsset);
        }
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateHunger(DeltaTime);
    UpdateBehavior(DeltaTime);
}

void ADinosaurBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ADinosaurBase::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;

        // Atualizar Blackboard se disponível
        if (AAIController* AIController = Cast<AAIController>(GetController()))
        {
            if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
            }
        }

        // Log da mudança de estado
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s changed behavior to %d"), 
               *GetName(), (int32)NewState);
    }
}

void ADinosaurBase::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Clamp(Stats.Health - DamageAmount, 0.0f, Stats.MaxHealth);

    if (Stats.Health <= 0.0f)
    {
        // Dinossauro morreu
        SetBehaviorState(ENPC_DinosaurBehaviorState::Flee);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s died!"), *GetName());
        return;
    }

    // Ficar agressivo se atacado
    if (DamageSource && Stats.Aggression > 0.3f)
    {
        CurrentTarget = DamageSource;
        SetBehaviorState(ENPC_DinosaurBehaviorState::Aggressive);
    }
}

void ADinosaurBase::AttackTarget(AActor* Target)
{
    if (!Target || !CanAttack())
    {
        return;
    }

    float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistanceToTarget <= Stats.AttackRange)
    {
        // Aplicar dano se o target for um Character
        if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
        {
            // Aqui seria aplicado o dano ao jogador
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacked %s for %f damage!"), 
                   *GetName(), *Target->GetName(), Stats.AttackDamage);
        }

        LastAttackTime = GetWorld()->GetTimeSeconds();
    }
}

bool ADinosaurBase::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

FVector ADinosaurBase::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetActorLocation();
    }

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex];
}

void ADinosaurBase::SetPatrolPoints(const TArray<FVector>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
}

void ADinosaurBase::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Detectar jogador
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        CurrentTarget = PlayerCharacter;

        // Comportamento baseado na espécie e agressividade
        if (Stats.Aggression > 0.7f)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Aggressive);
        }
        else if (Stats.Aggression > 0.3f)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Hunt);
        }

        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s detected player %s"), 
               *GetName(), *OtherActor->GetName());
    }
}

void ADinosaurBase::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor == CurrentTarget)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(ENPC_DinosaurBehaviorState::Patrol);
    }
}

void ADinosaurBase::ConfigureForSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;

    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.MaxHealth = 500.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 100.0f;
            Stats.MovementSpeed = 400.0f;
            Stats.DetectionRange = 3000.0f;
            Stats.AttackRange = 800.0f;
            Stats.Aggression = 0.9f;
            break;

        case ENPC_DinosaurSpecies::Raptor:
            Stats.MaxHealth = 150.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 40.0f;
            Stats.MovementSpeed = 600.0f;
            Stats.DetectionRange = 2500.0f;
            Stats.AttackRange = 400.0f;
            Stats.Aggression = 0.8f;
            break;

        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.MaxHealth = 800.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 60.0f;
            Stats.MovementSpeed = 200.0f;
            Stats.DetectionRange = 1500.0f;
            Stats.AttackRange = 600.0f;
            Stats.Aggression = 0.2f;
            break;

        case ENPC_DinosaurSpecies::Triceratops:
            Stats.MaxHealth = 400.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 80.0f;
            Stats.MovementSpeed = 300.0f;
            Stats.DetectionRange = 2000.0f;
            Stats.AttackRange = 500.0f;
            Stats.Aggression = 0.6f;
            break;

        case ENPC_DinosaurSpecies::Stegosaurus:
            Stats.MaxHealth = 350.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.AttackDamage = 70.0f;
            Stats.MovementSpeed = 250.0f;
            Stats.DetectionRange = 1800.0f;
            Stats.AttackRange = 450.0f;
            Stats.Aggression = 0.4f;
            break;
    }

    // Atualizar raios de detecção
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(Stats.DetectionRange);
    }
    if (AttackSphere)
    {
        AttackSphere->SetSphereRadius(Stats.AttackRange);
    }
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    // Aumentar fome ao longo do tempo
    Stats.Hunger = FMath::Clamp(Stats.Hunger + (DeltaTime * 0.1f), 0.0f, 100.0f);

    // Se muito faminto, procurar comida
    if (Stats.Hunger > 80.0f && CurrentBehaviorState != ENPC_DinosaurBehaviorState::Feed)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Feed);
    }
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    // Lógica básica de comportamento
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            // Depois de um tempo, começar a patrulhar
            if (PatrolPoints.Num() > 0)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrol);
            }
            break;

        case ENPC_DinosaurBehaviorState::Patrol:
            // Continuar patrulha se não há target
            if (!CurrentTarget)
            {
                // Lógica de patrulha seria implementada aqui
            }
            break;

        case ENPC_DinosaurBehaviorState::Hunt:
            // Perseguir target se ainda está na área de detecção
            if (CurrentTarget)
            {
                float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
                if (DistanceToTarget <= Stats.AttackRange)
                {
                    AttackTarget(CurrentTarget);
                }
            }
            break;

        case ENPC_DinosaurBehaviorState::Aggressive:
            // Atacar agressivamente
            if (CurrentTarget)
            {
                AttackTarget(CurrentTarget);
            }
            break;

        case ENPC_DinosaurBehaviorState::Feed:
            // Reduzir fome quando alimentando
            Stats.Hunger = FMath::Clamp(Stats.Hunger - (DeltaTime * 2.0f), 0.0f, 100.0f);
            if (Stats.Hunger < 30.0f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
            }
            break;
    }
}