#include "Combat_DinosaurBehaviorTree.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCombat_DinosaurBehaviorTree::UCombat_DinosaurBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Configuração padrão
    AggressionLevel = 0.5f;
    FearLevel = 0.2f;
    HuntingRange = 2000.0f;
    FleeDistance = 500.0f;
    PackRadius = 1500.0f;
    
    CurrentBehaviorState = ECombat_DinosaurBehaviorState::Idle;
    LastPlayerSighting = FVector::ZeroVector;
    TimeSinceLastSighting = 0.0f;
    
    bIsInPack = false;
    PackLeader = nullptr;
}

void UCombat_DinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();
    
    // Configurar AI Controller
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            SetupAIPerception(AIController);
            SetupBehaviorTree(AIController);
        }
    }
    
    // Detectar outros dinossauros próximos para formação de pack
    DetectNearbyDinosaurs();
}

void UCombat_DinosaurBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastSighting += DeltaTime;
    
    // Atualizar comportamento baseado no estado atual
    UpdateBehaviorState(DeltaTime);
    
    // Atualizar coordenação de pack
    if (bIsInPack)
    {
        UpdatePackBehavior(DeltaTime);
    }
    
    // Verificar ameaças próximas
    CheckForThreats();
}

void UCombat_DinosaurBehaviorTree::SetupAIPerception(AAIController* AIController)
{
    if (!AIController) return;
    
    // Configurar componente de percepção
    UAIPerceptionComponent* PerceptionComponent = AIController->GetAIPerceptionComponent();
    if (!PerceptionComponent)
    {
        PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
        AIController->SetPerceptionComponent(*PerceptionComponent);
    }
    
    // Configurar visão
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = HuntingRange;
        SightConfig->LoseSightRadius = HuntingRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        PerceptionComponent->ConfigureSense(*SightConfig);
        PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }
    
    // Bind eventos de percepção
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UCombat_DinosaurBehaviorTree::OnPerceptionUpdated);
}

void UCombat_DinosaurBehaviorTree::SetupBehaviorTree(AAIController* AIController)
{
    if (!AIController) return;
    
    // Configurar Blackboard
    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
        BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), FearLevel);
        BlackboardComp->SetValueAsFloat(TEXT("HuntingRange"), HuntingRange);
        BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    }
}

void UCombat_DinosaurBehaviorTree::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;
    
    // Verificar se é o jogador
    if (Actor->IsA<ACharacter>() && Actor->ActorHasTag(TEXT("Player")))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            LastPlayerSighting = Actor->GetActorLocation();
            TimeSinceLastSighting = 0.0f;
            
            // Calcular distância para determinar comportamento
            float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastPlayerSighting);
            
            if (DistanceToPlayer < FleeDistance && FearLevel > 0.6f)
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Fleeing);
            }
            else if (DistanceToPlayer < HuntingRange && AggressionLevel > 0.4f)
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Hunting);
            }
            else
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Investigating);
            }
        }
        else
        {
            // Perdeu de vista o jogador
            if (CurrentBehaviorState == ECombat_DinosaurBehaviorState::Hunting)
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Searching);
            }
        }
    }
}

void UCombat_DinosaurBehaviorTree::SetBehaviorState(ECombat_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;
    
    ECombat_DinosaurBehaviorState PreviousState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    
    // Atualizar Blackboard
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
            }
        }
    }
    
    // Log mudança de estado
    UE_LOG(LogTemp, Warning, TEXT("Dinossauro %s mudou estado de %d para %d"), 
           *GetOwner()->GetName(), 
           static_cast<int32>(PreviousState), 
           static_cast<int32>(CurrentBehaviorState));
    
    // Notificar pack se necessário
    if (bIsInPack && PackLeader)
    {
        NotifyPackOfStateChange(NewState);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateBehaviorState(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
        case ECombat_DinosaurBehaviorState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Investigating:
            UpdateInvestigatingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Hunting:
            UpdateHuntingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Attacking:
            UpdateAttackingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Searching:
            UpdateSearchingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehaviorState::Coordinating:
            UpdateCoordinatingBehavior(DeltaTime);
            break;
    }
}

void UCombat_DinosaurBehaviorTree::UpdateIdleBehavior(float DeltaTime)
{
    // Comportamento idle - ocasionalmente mudar para patrol
    static float IdleTimer = 0.0f;
    IdleTimer += DeltaTime;
    
    if (IdleTimer > 10.0f) // 10 segundos de idle
    {
        IdleTimer = 0.0f;
        if (FMath::RandRange(0.0f, 1.0f) > 0.7f)
        {
            SetBehaviorState(ECombat_DinosaurBehaviorState::Patrolling);
        }
    }
}

void UCombat_DinosaurBehaviorTree::UpdatePatrolBehavior(float DeltaTime)
{
    // Implementar patrulhamento básico
    // Por agora, voltar ao idle após um tempo
    static float PatrolTimer = 0.0f;
    PatrolTimer += DeltaTime;
    
    if (PatrolTimer > 15.0f)
    {
        PatrolTimer = 0.0f;
        SetBehaviorState(ECombat_DinosaurBehaviorState::Idle);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateInvestigatingBehavior(float DeltaTime)
{
    // Se perdeu o jogador de vista por muito tempo, voltar ao patrulhamento
    if (TimeSinceLastSighting > 5.0f)
    {
        SetBehaviorState(ECombat_DinosaurBehaviorState::Patrolling);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateHuntingBehavior(float DeltaTime)
{
    // Verificar se ainda pode ver o jogador
    if (TimeSinceLastSighting > 3.0f)
    {
        SetBehaviorState(ECombat_DinosaurBehaviorState::Searching);
        return;
    }
    
    // Verificar distância para ataque
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastPlayerSighting);
    if (DistanceToPlayer < 300.0f) // Distância de ataque
    {
        SetBehaviorState(ECombat_DinosaurBehaviorState::Attacking);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateAttackingBehavior(float DeltaTime)
{
    // Implementar lógica de ataque
    static float AttackCooldown = 0.0f;
    AttackCooldown -= DeltaTime;
    
    if (AttackCooldown <= 0.0f)
    {
        PerformAttack();
        AttackCooldown = 2.0f; // 2 segundos entre ataques
    }
    
    // Verificar se o jogador fugiu
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastPlayerSighting);
    if (DistanceToPlayer > 500.0f)
    {
        SetBehaviorState(ECombat_DinosaurBehaviorState::Hunting);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateFleeingBehavior(float DeltaTime)
{
    // Se está longe o suficiente, voltar ao idle
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastPlayerSighting);
    if (DistanceToPlayer > FleeDistance * 2.0f)
    {
        SetBehaviorState(ECombat_DinosaurBehaviorState::Idle);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateSearchingBehavior(float DeltaTime)
{
    // Procurar por um tempo limitado
    static float SearchTimer = 0.0f;
    SearchTimer += DeltaTime;
    
    if (SearchTimer > 8.0f)
    {
        SearchTimer = 0.0f;
        SetBehaviorState(ECombat_DinosaurBehaviorState::Patrolling);
    }
}

void UCombat_DinosaurBehaviorTree::UpdateCoordinatingBehavior(float DeltaTime)
{
    // Comportamento de coordenação com pack
    if (!bIsInPack || !PackLeader)
    {
        SetBehaviorState(ECombat_DinosaurBehaviorState::Idle);
        return;
    }
    
    // Seguir comandos do líder do pack
    if (PackLeader != GetOwner())
    {
        // Implementar seguimento do líder
    }
}

void UCombat_DinosaurBehaviorTree::PerformAttack()
{
    // Implementar ataque básico
    UE_LOG(LogTemp, Warning, TEXT("Dinossauro %s está atacando!"), *GetOwner()->GetName());
    
    // Aqui seria implementado o sistema de dano
    // Por agora, apenas log
}

void UCombat_DinosaurBehaviorTree::DetectNearbyDinosaurs()
{
    if (!GetWorld()) return;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue;
        
        // Verificar se é um dinossauro
        if (Actor->GetName().Contains(TEXT("Raptor")) || 
            Actor->GetName().Contains(TEXT("TRex")) ||
            Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < PackRadius)
            {
                PackMembers.AddUnique(Actor);
                bIsInPack = true;
                
                // Se não há líder, tornar-se o líder
                if (!PackLeader)
                {
                    PackLeader = GetOwner();
                }
            }
        }
    }
}

void UCombat_DinosaurBehaviorTree::UpdatePackBehavior(float DeltaTime)
{
    // Limpar membros mortos ou muito distantes
    PackMembers.RemoveAll([this](AActor* Member) {
        if (!Member || !IsValid(Member)) return true;
        
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Member->GetActorLocation());
        return Distance > PackRadius * 2.0f;
    });
    
    // Se não há mais membros, sair do pack
    if (PackMembers.Num() == 0)
    {
        bIsInPack = false;
        PackLeader = nullptr;
    }
}

void UCombat_DinosaurBehaviorTree::NotifyPackOfStateChange(ECombat_DinosaurBehaviorState NewState)
{
    // Notificar outros membros do pack sobre mudança de estado
    for (AActor* Member : PackMembers)
    {
        if (Member && Member != GetOwner())
        {
            if (UCombat_DinosaurBehaviorTree* MemberBehavior = Member->FindComponentByClass<UCombat_DinosaurBehaviorTree>())
            {
                // Sincronizar comportamento do pack
                if (NewState == ECombat_DinosaurBehaviorState::Hunting)
                {
                    MemberBehavior->SetBehaviorState(ECombat_DinosaurBehaviorState::Coordinating);
                }
            }
        }
    }
}

void UCombat_DinosaurBehaviorTree::CheckForThreats()
{
    // Verificar ameaças próximas
    if (TimeSinceLastSighting < 1.0f) // Jogador visto recentemente
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastPlayerSighting);
        
        // Ajustar níveis de agressão e medo baseado na distância
        if (DistanceToPlayer < FleeDistance)
        {
            FearLevel = FMath::Min(1.0f, FearLevel + 0.1f);
        }
        else if (DistanceToPlayer < HuntingRange)
        {
            AggressionLevel = FMath::Min(1.0f, AggressionLevel + 0.05f);
        }
    }
    else
    {
        // Reduzir gradualmente os níveis quando não há ameaças
        FearLevel = FMath::Max(0.0f, FearLevel - 0.02f);
        AggressionLevel = FMath::Max(0.0f, AggressionLevel - 0.01f);
    }
}