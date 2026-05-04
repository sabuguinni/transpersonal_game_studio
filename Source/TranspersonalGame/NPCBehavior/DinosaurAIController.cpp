#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configurações padrão
    SightRadius = 3000.0f;
    LoseSightRadius = 3500.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRange = 2000.0f;

    // Personalidade padrão (neutro)
    Aggressiveness = 0.5f;
    Curiosity = 0.3f;
    Sociability = 0.4f;

    // Estado inicial
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;

    SetupPerception();
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    if (BlackboardAsset && BlackboardComponent)
    {
        BlackboardComponent->InitializeBlackboard(*BlackboardAsset);
    }

    // Configurar baseado na espécie
    ConfigureForSpecies(DinosaurSpecies);
    
    // Iniciar behavior tree
    StartBehaviorTree();
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        // Detectar espécie baseado no nome do pawn
        FString PawnName = InPawn->GetName().ToLower();
        if (PawnName.Contains(TEXT("trex")))
        {
            DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
        }
        else if (PawnName.Contains(TEXT("raptor")))
        {
            DinosaurSpecies = ENPC_DinosaurSpecies::Raptor;
        }
        else if (PawnName.Contains(TEXT("brachio")))
        {
            DinosaurSpecies = ENPC_DinosaurSpecies::Brachiosaurus;
        }
        
        ConfigureForSpecies(DinosaurSpecies);
        UpdateBlackboard();
    }
}

void ADinosaurAIController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ADinosaurAIController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configurar visão
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configurar audição
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Configurar sentido dominante
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind do evento de percepção
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
}

void ADinosaurAIController::StartBehaviorTree()
{
    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }
}

void ADinosaurAIController::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;

    // Atualizar valores do blackboard
    BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    BlackboardComponent->SetValueAsEnum(TEXT("Species"), static_cast<uint8>(DinosaurSpecies));
    BlackboardComponent->SetValueAsFloat(TEXT("Aggressiveness"), Aggressiveness);
    BlackboardComponent->SetValueAsFloat(TEXT("Curiosity"), Curiosity);
    BlackboardComponent->SetValueAsFloat(TEXT("Sociability"), Sociability);

    if (GetPawn())
    {
        BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), GetPawn()->GetActorLocation());
    }
}

void ADinosaurAIController::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        UpdateBlackboard();
        
        // Log da mudança de estado
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to %d"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
               static_cast<int32>(NewState));
    }
}

void ADinosaurAIController::StartHunting(AActor* Target)
{
    if (!Target || !BlackboardComponent)
        return;

    SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s started hunting %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
           *Target->GetName());
}

void ADinosaurAIController::StartFleeing(AActor* Threat)
{
    if (!Threat || !BlackboardComponent)
        return;

    SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);
    BlackboardComponent->SetValueAsObject(TEXT("ThreatActor"), Threat);
    
    // Calcular direção de fuga (oposta ao threat)
    if (GetPawn())
    {
        FVector FleeDirection = GetPawn()->GetActorLocation() - Threat->GetActorLocation();
        FleeDirection.Normalize();
        FVector FleeLocation = GetPawn()->GetActorLocation() + FleeDirection * 5000.0f;
        BlackboardComponent->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s started fleeing from %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
           *Threat->GetName());
}

void ADinosaurAIController::ReturnToPatrol()
{
    SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(TEXT("TargetActor"));
        BlackboardComponent->ClearValue(TEXT("ThreatActor"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s returned to patrol"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
}

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !GetPawn())
        return;

    bool bIsPlayer = Actor->IsA<APawn>() && Actor != GetPawn();
    
    if (Stimulus.WasSuccessfullySensed())
    {
        // Detectou algo
        float Distance = GetDistanceToActor(Actor);
        
        if (bIsPlayer)
        {
            // Reação baseada na espécie e personalidade
            switch (DinosaurSpecies)
            {
                case ENPC_DinosaurSpecies::TRex:
                    if (Aggressiveness > 0.6f && Distance < 2000.0f)
                    {
                        StartHunting(Actor);
                    }
                    break;
                    
                case ENPC_DinosaurSpecies::Raptor:
                    if (Aggressiveness > 0.4f && Distance < 1500.0f)
                    {
                        StartHunting(Actor);
                    }
                    break;
                    
                case ENPC_DinosaurSpecies::Brachiosaurus:
                    if (Distance < 1000.0f)
                    {
                        StartFleeing(Actor);
                    }
                    break;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s detected %s at distance %.1f"), 
               *GetPawn()->GetName(), *Actor->GetName(), Distance);
    }
    else
    {
        // Perdeu de vista
        if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Hunting && BlackboardComponent)
        {
            AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(TEXT("TargetActor")));
            if (CurrentTarget == Actor)
            {
                ReturnToPatrol();
            }
        }
    }
}

bool ADinosaurAIController::CanSeeActor(AActor* Actor) const
{
    if (!AIPerceptionComponent || !Actor)
        return false;

    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(Actor, Info);
    
    return Info.LastSensedStimuli.Num() > 0 && Info.LastSensedStimuli[0].WasSuccessfullySensed();
}

float ADinosaurAIController::GetDistanceToActor(AActor* Actor) const
{
    if (!Actor || !GetPawn())
        return -1.0f;

    return FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
}

void ADinosaurAIController::ConfigureForSpecies(ENPC_DinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Aggressiveness = 0.8f;
            Curiosity = 0.3f;
            Sociability = 0.1f;
            SightRadius = 4000.0f;
            HearingRange = 3000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            Aggressiveness = 0.7f;
            Curiosity = 0.6f;
            Sociability = 0.8f;
            SightRadius = 3000.0f;
            HearingRange = 2500.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Aggressiveness = 0.1f;
            Curiosity = 0.4f;
            Sociability = 0.6f;
            SightRadius = 2500.0f;
            HearingRange = 2000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            Aggressiveness = 0.5f;
            Curiosity = 0.3f;
            Sociability = 0.7f;
            SightRadius = 2800.0f;
            HearingRange = 2200.0f;
            break;
            
        case ENPC_DinosaurSpecies::Pteranodon:
            Aggressiveness = 0.3f;
            Curiosity = 0.8f;
            Sociability = 0.5f;
            SightRadius = 5000.0f;
            HearingRange = 1500.0f;
            break;
    }
    
    UpdateBlackboard();
}

AActor* ADinosaurAIController::FindNearestThreat() const
{
    // Implementação básica - procurar por players próximos
    if (!GetPawn())
        return nullptr;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = FLT_MAX;
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetPawn() && CanSeeActor(Actor))
        {
            float Distance = GetDistanceToActor(Actor);
            if (Distance < NearestDistance && Distance < SightRadius)
            {
                NearestDistance = Distance;
                NearestThreat = Actor;
            }
        }
    }
    
    return NearestThreat;
}

AActor* ADinosaurAIController::FindNearestPrey() const
{
    // Implementação similar ao FindNearestThreat
    return FindNearestThreat();
}

TArray<AActor*> ADinosaurAIController::FindNearbyAllies() const
{
    TArray<AActor*> Allies;
    
    if (!GetPawn())
        return Allies;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetPawn()->GetClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetPawn())
        {
            float Distance = GetDistanceToActor(Actor);
            if (Distance < SightRadius * 0.5f) // Aliados dentro de metade do range de visão
            {
                Allies.Add(Actor);
            }
        }
    }
    
    return Allies;
}