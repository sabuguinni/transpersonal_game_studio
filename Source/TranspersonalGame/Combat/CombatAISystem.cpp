#include "CombatAISystem.h"
#include "DinosaurAI.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "DrawDebugHelpers.h"

ACombatAISystem::ACombatAISystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x por segundo para performance
}

void ACombatAISystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontra todos os dinossauros no nível e regista-os
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurAI::StaticClass(), FoundDinosaurs);
    
    for (AActor* Actor : FoundDinosaurs)
    {
        if (ADinosaurAI* Dinosaur = Cast<ADinosaurAI>(Actor))
        {
            RegisterCombatant(Dinosaur);
        }
    }
}

void ACombatAISystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdatePackCoordination(DeltaTime);
}

void ACombatAISystem::RegisterCombatant(ADinosaurAI* Dinosaur)
{
    if (Dinosaur && !RegisteredCombatants.Contains(Dinosaur))
    {
        RegisteredCombatants.Add(Dinosaur);
        
        // Log para debugging
        UE_LOG(LogTemp, Log, TEXT("Combat AI: Registered dinosaur %s"), *Dinosaur->GetName());
    }
}

void ACombatAISystem::UnregisterCombatant(ADinosaurAI* Dinosaur)
{
    if (Dinosaur)
    {
        RegisteredCombatants.Remove(Dinosaur);
        
        // Remove de todos os grupos
        for (auto& Pack : ActiveHuntingPacks)
        {
            Pack.Value.Remove(Dinosaur);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI: Unregistered dinosaur %s"), *Dinosaur->GetName());
    }
}

TArray<AActor*> ACombatAISystem::FindNearbyEnemies(ADinosaurAI* Dinosaur, float SearchRadius)
{
    TArray<AActor*> NearbyEnemies;
    
    if (!Dinosaur)
        return NearbyEnemies;
    
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    // Procura por outros dinossauros
    for (ADinosaurAI* OtherDinosaur : RegisteredCombatants)
    {
        if (OtherDinosaur == Dinosaur || !IsValid(OtherDinosaur))
            continue;
        
        float Distance = FVector::Dist(DinosaurLocation, OtherDinosaur->GetActorLocation());
        if (Distance <= SearchRadius)
        {
            // Verifica se são inimigos (lógica simplificada)
            if (ShouldBeEnemies(Dinosaur, OtherDinosaur))
            {
                NearbyEnemies.Add(OtherDinosaur);
            }
        }
    }
    
    // Procura pelo jogador
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(DinosaurLocation, PlayerPawn->GetActorLocation());
        if (Distance <= SearchRadius)
        {
            NearbyEnemies.Add(PlayerPawn);
        }
    }
    
    return NearbyEnemies;
}

void ACombatAISystem::CoordinatePackAttack(const TArray<ADinosaurAI*>& PackMembers, AActor* Target)
{
    if (PackMembers.Num() < 2 || !Target)
        return;
    
    // Estratégia de coordenação baseada no número de membros
    if (PackMembers.Num() == 2)
    {
        // Ataque coordenado simples - um distrai, outro flanqueia
        CoordinatePincer(PackMembers, Target);
    }
    else if (PackMembers.Num() >= 3)
    {
        // Ataque em círculo - cercam a presa
        CoordinateCircle(PackMembers, Target);
    }
}

bool ACombatAISystem::ShouldInitiateAttack(ADinosaurAI* Attacker, AActor* Target)
{
    if (!Attacker || !Target)
        return false;
    
    float TacticalAdvantage = CalculateTacticalAdvantage(Attacker, Target);
    
    // Obtém personalidade do atacante
    ADinosaurCombatController* Controller = Cast<ADinosaurCombatController>(Attacker->GetController());
    if (!Controller)
        return false;
    
    float AgressionThreshold = 0.3f + (Controller->CombatPersonality.Aggression * 0.4f);
    float CautionPenalty = Controller->CombatPersonality.Caution * 0.2f;
    
    float AttackThreshold = AgressionThreshold - CautionPenalty;
    
    return TacticalAdvantage >= AttackThreshold;
}

FVector ACombatAISystem::FindAmbushPosition(ADinosaurAI* Dinosaur, AActor* Target)
{
    if (!Dinosaur || !Target)
        return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    // Procura posições num raio de 500-1500 unidades do alvo
    float MinDistance = 500.0f;
    float MaxDistance = 1500.0f;
    
    TArray<FVector> CandidatePositions;
    
    // Gera posições candidatas em círculo
    for (int32 i = 0; i < 16; i++)
    {
        float Angle = (i * 22.5f) * PI / 180.0f; // 16 posições em círculo
        float Distance = FMath::RandRange(MinDistance, MaxDistance);
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector CandidatePos = TargetLocation + Offset;
        
        // Verifica se a posição é navegável
        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLocation;
            if (NavSys->ProjectPointToNavigation(CandidatePos, NavLocation, FVector(100.0f)))
            {
                CandidatePositions.Add(NavLocation.Location);
            }
        }
    }
    
    // Escolhe a melhor posição (mais escondida e com boa linha de ataque)
    FVector BestPosition = DinosaurLocation;
    float BestScore = 0.0f;
    
    for (const FVector& Position : CandidatePositions)
    {
        float Score = EvaluateAmbushPosition(Position, TargetLocation, DinosaurLocation);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPosition = Position;
        }
    }
    
    return BestPosition;
}

void ACombatAISystem::UpdatePackCoordination(float DeltaTime)
{
    // Remove grupos vazios
    TArray<FString> EmptyPacks;
    for (auto& Pack : ActiveHuntingPacks)
    {
        Pack.Value.RemoveAll([](ADinosaurAI* Dinosaur) { return !IsValid(Dinosaur); });
        
        if (Pack.Value.Num() == 0)
        {
            EmptyPacks.Add(Pack.Key);
        }
    }
    
    for (const FString& PackName : EmptyPacks)
    {
        ActiveHuntingPacks.Remove(PackName);
    }
    
    // Actualiza coordenação de cada grupo
    for (auto& Pack : ActiveHuntingPacks)
    {
        UpdatePackBehavior(Pack.Value, DeltaTime);
    }
}

float ACombatAISystem::CalculateTacticalAdvantage(ADinosaurAI* Attacker, AActor* Target)
{
    if (!Attacker || !Target)
        return 0.0f;
    
    float Advantage = 0.5f; // Base neutra
    
    ADinosaurCombatController* Controller = Cast<ADinosaurCombatController>(Attacker->GetController());
    if (!Controller)
        return Advantage;
    
    // Factores que aumentam vantagem
    
    // 1. Vantagem numérica (se tem aliados próximos)
    int32 AlliesNearby = CountAlliesNearby(Attacker, 1000.0f);
    if (AlliesNearby > 0)
    {
        Advantage += AlliesNearby * 0.15f;
    }
    
    // 2. Elemento surpresa (se alvo não viu atacante)
    if (!TargetCanSeeAttacker(Target, Attacker))
    {
        Advantage += 0.3f;
    }
    
    // 3. Vantagem de terreno (atacante em posição mais alta)
    float HeightDifference = Attacker->GetActorLocation().Z - Target->GetActorLocation().Z;
    if (HeightDifference > 100.0f)
    {
        Advantage += 0.2f;
    }
    
    // 4. Distância ideal para ataque
    float Distance = FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation());
    if (Distance <= Controller->CombatCapabilities.AttackRange * 1.5f)
    {
        Advantage += 0.2f;
    }
    
    // 5. Estado de stamina
    if (Controller->CombatCapabilities.Stamina > 70.0f)
    {
        Advantage += 0.1f;
    }
    
    // Factores que diminuem vantagem
    
    // 1. Alvo é muito maior (se for dinossauro)
    if (ADinosaurAI* TargetDinosaur = Cast<ADinosaurAI>(Target))
    {
        ADinosaurCombatController* TargetController = Cast<ADinosaurCombatController>(TargetDinosaur->GetController());
        if (TargetController && TargetController->ThreatLevel > Controller->ThreatLevel)
        {
            Advantage -= 0.3f;
        }
    }
    
    // 2. Alvo tem aliados próximos
    int32 TargetAllies = CountAlliesNearby(Cast<ADinosaurAI>(Target), 1000.0f);
    if (TargetAllies > 0)
    {
        Advantage -= TargetAllies * 0.1f;
    }
    
    return FMath::Clamp(Advantage, 0.0f, 1.0f);
}

// Implementação do Controlador de Combate

ADinosaurCombatController::ADinosaurCombatController()
{
    // Configura componente de percepção
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configura visão
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    
    // Configura audição
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    
    // Configura detecção de dano
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    
    // Define visão como sentido dominante
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind eventos de percepção
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatController::OnPerceptionUpdated);
}

void ADinosaurCombatController::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicia Behavior Tree se definido
    if (CombatBehaviorTree)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
    
    // Gera personalidade aleatória se não definida
    if (CombatPersonality.Aggression == 0.5f && CombatPersonality.Intelligence == 0.5f)
    {
        GenerateRandomPersonality();
    }
}

void ADinosaurCombatController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        // Actualiza Blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), (uint8)NewState);
        }
        
        // Log para debugging
        UE_LOG(LogTemp, Log, TEXT("Combat AI: %s changed state from %d to %d"), 
               *GetPawn()->GetName(), (int32)PreviousState, (int32)NewState);
        
        OnCombatStateChanged(PreviousState, NewState);
    }
}

void ADinosaurCombatController::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        // Actualiza Blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), NewTarget);
        }
        
        // Muda estado baseado no alvo
        if (NewTarget)
        {
            if (CurrentCombatState == ECombatState::Idle)
            {
                SetCombatState(ECombatState::Stalking);
            }
        }
        else
        {
            if (CurrentCombatState != ECombatState::Idle && CurrentCombatState != ECombatState::Resting)
            {
                SetCombatState(ECombatState::Idle);
            }
        }
    }
}

bool ADinosaurCombatController::CanSeeTarget(AActor* Target)
{
    if (!Target || !AIPerceptionComponent)
        return false;
    
    FActorPerceptionBlueprintInfo PerceptionInfo;
    AIPerceptionComponent->GetActorsPerception(Target, PerceptionInfo);
    
    for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
        {
            return true;
        }
    }
    
    return false;
}

float ADinosaurCombatController::GetDistanceToTarget(AActor* Target)
{
    if (!Target || !GetPawn())
        return -1.0f;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

void ADinosaurCombatController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;
        
        FActorPerceptionBlueprintInfo PerceptionInfo;
        AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);
        
        // Verifica se viu algo novo
        for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
        {
            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
            {
                OnActorSeen(Actor);
            }
            else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>() && Stimulus.WasSuccessfullySensed())
            {
                OnSoundHeard(Actor, Stimulus.StimulusLocation);
            }
        }
    }
    
    EvaluateThreatLevel();
}