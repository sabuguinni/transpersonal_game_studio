#include "DinosaurMassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurMassProcessor::UDinosaurMassProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UDinosaurMassProcessor::ConfigureQueries()
{
    // Query principal para dinossauros ativos
    ActiveDinosaurQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    ActiveDinosaurQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    ActiveDinosaurQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    ActiveDinosaurQuery.AddRequirement<FDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    ActiveDinosaurQuery.AddRequirement<FDinosaurNeedsFragment>(EMassFragmentAccess::ReadWrite);
    ActiveDinosaurQuery.AddRequirement<FDinosaurVariationFragment>(EMassFragmentAccess::ReadOnly);
    ActiveDinosaurQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
    
    // Filtros para LOD
    ActiveDinosaurQuery.AddTagRequirement<FMassVisibilityCanBeSeenTag>(EMassFragmentPresence::All);
    
    // Query para detecção de proximidade (otimizada)
    ProximityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    ProximityQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    ProximityQuery.AddRequirement<FDinosaurBehaviorFragment>(EMassFragmentAccess::ReadOnly);
}

void UDinosaurMassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Processa necessidades básicas primeiro
    ProcessNeeds(Context);
    
    // Depois comportamento baseado nas necessidades
    ProcessBehavior(Context);
    
    // Finalmente interações entre dinossauros
    ProcessInteractions(Context);
}

void UDinosaurMassProcessor::ProcessNeeds(FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    ActiveDinosaurQuery.ForEachEntityChunk(Context.GetEntityManager(), Context,
        [this, DeltaTime](FMassExecutionContext& Context)
        {
            const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
            auto& NeedsList = Context.GetMutableFragmentView<FDinosaurNeedsFragment>();
            auto& BehaviorList = Context.GetMutableFragmentView<FDinosaurBehaviorFragment>();
            const auto& SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
            
            const int32 NumEntities = Context.GetNumEntities();
            
            for (int32 i = 0; i < NumEntities; ++i)
            {
                auto& Needs = NeedsList[i];
                auto& Behavior = BehaviorList[i];
                const auto& Species = SpeciesList[i];
                
                // Decaimento das necessidades baseado na espécie
                float HungerRate = Needs.HungerDecayRate;
                float ThirstRate = Needs.ThirstDecayRate;
                float EnergyRate = Needs.EnergyDecayRate;
                
                // Carnívoros têm fome mais rápida, herbívoros sede mais rápida
                if (Species.SpeciesType == 1) // Carnívoro
                {
                    HungerRate *= 1.5f;
                }
                else if (Species.SpeciesType == 0) // Herbívoro
                {
                    ThirstRate *= 1.3f;
                }
                
                // Aplica decaimento
                Needs.Hunger = FMath::Max(0, (int32)Needs.Hunger - (int32)(HungerRate * DeltaTime * 255.0f));
                Needs.Thirst = FMath::Max(0, (int32)Needs.Thirst - (int32)(ThirstRate * DeltaTime * 255.0f));
                
                // Energia decai mais rápido durante atividade
                float EnergyMultiplier = 1.0f;
                if (Behavior.CurrentState == FDinosaurBehaviorFragment::EBehaviorState::Hunting ||
                    Behavior.CurrentState == FDinosaurBehaviorFragment::EBehaviorState::Fleeing)
                {
                    EnergyMultiplier = 3.0f;
                }
                
                Needs.Energy = FMath::Max(0, (int32)Needs.Energy - (int32)(EnergyRate * EnergyMultiplier * DeltaTime * 255.0f));
            }
        });
}

void UDinosaurMassProcessor::ProcessBehavior(FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    ActiveDinosaurQuery.ForEachEntityChunk(Context.GetEntityManager(), Context,
        [this, DeltaTime](FMassExecutionContext& Context)
        {
            auto& TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            auto& VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
            auto& BehaviorList = Context.GetMutableFragmentView<FDinosaurBehaviorFragment>();
            const auto& NeedsList = Context.GetFragmentView<FDinosaurNeedsFragment>();
            const auto& SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
            const auto& VariationList = Context.GetFragmentView<FDinosaurVariationFragment>();
            
            const int32 NumEntities = Context.GetNumEntities();
            
            for (int32 i = 0; i < NumEntities; ++i)
            {
                auto& Transform = TransformList[i];
                auto& Velocity = VelocityList[i];
                auto& Behavior = BehaviorList[i];
                const auto& Needs = NeedsList[i];
                const auto& Species = SpeciesList[i];
                const auto& Variation = VariationList[i];
                
                // Atualiza timer do estado
                Behavior.StateTimer += DeltaTime;
                
                // Determina se deve mudar de estado
                UpdateBehaviorState(Behavior, Needs, Species, DeltaTime);
                
                // Executa comportamento baseado no estado atual
                FVector DesiredVelocity = FVector::ZeroVector;
                float MaxSpeed = 300.0f * Variation.MovementSpeedMultiplier;
                
                switch (Behavior.CurrentState)
                {
                    case FDinosaurBehaviorFragment::EBehaviorState::Wandering:
                    {
                        // Movimento aleatório suave
                        if (Behavior.TargetLocation.IsZero() || 
                            FVector::Dist(Transform.GetLocation(), Behavior.TargetLocation) < 200.0f)
                        {
                            // Novo ponto aleatório num raio de 2km
                            FVector RandomDirection = FMath::VRand();
                            RandomDirection.Z = 0; // Manter no plano horizontal
                            Behavior.TargetLocation = Transform.GetLocation() + RandomDirection * FMath::RandRange(500.0f, 2000.0f);
                        }
                        
                        FVector Direction = (Behavior.TargetLocation - Transform.GetLocation()).GetSafeNormal();
                        DesiredVelocity = Direction * MaxSpeed * 0.3f; // Velocidade de caminhada
                        break;
                    }
                    
                    case FDinosaurBehaviorFragment::EBehaviorState::Feeding:
                    {
                        // Procura comida ou move-se para área de alimentação
                        if (Behavior.TargetLocation.IsZero())
                        {
                            Behavior.TargetLocation = FindNearestResource(Transform.GetLocation(), Species.SpeciesType);
                        }
                        
                        float DistanceToFood = FVector::Dist(Transform.GetLocation(), Behavior.TargetLocation);
                        if (DistanceToFood > 100.0f)
                        {
                            FVector Direction = (Behavior.TargetLocation - Transform.GetLocation()).GetSafeNormal();
                            DesiredVelocity = Direction * MaxSpeed * 0.5f;
                        }
                        else
                        {
                            // Chegou à comida, fica parado
                            DesiredVelocity = FVector::ZeroVector;
                        }
                        break;
                    }
                    
                    case FDinosaurBehaviorFragment::EBehaviorState::Drinking:
                    {
                        // Similar ao feeding, mas procura água
                        if (Behavior.TargetLocation.IsZero())
                        {
                            Behavior.TargetLocation = FindNearestResource(Transform.GetLocation(), 255); // Água
                        }
                        
                        float DistanceToWater = FVector::Dist(Transform.GetLocation(), Behavior.TargetLocation);
                        if (DistanceToWater > 150.0f)
                        {
                            FVector Direction = (Behavior.TargetLocation - Transform.GetLocation()).GetSafeNormal();
                            DesiredVelocity = Direction * MaxSpeed * 0.6f;
                        }
                        break;
                    }
                    
                    case FDinosaurBehaviorFragment::EBehaviorState::Hunting:
                    {
                        // Apenas carnívoros caçam
                        if (Species.SpeciesType == 1 && !Behavior.TargetEntity.IsSet())
                        {
                            // Procura presa próxima (implementar busca por herbívoros)
                            MaxSpeed *= 1.5f; // Velocidade de caça
                        }
                        break;
                    }
                    
                    case FDinosaurBehaviorFragment::EBehaviorState::Fleeing:
                    {
                        // Foge de predadores
                        if (!Behavior.TargetLocation.IsZero())
                        {
                            FVector FleeDirection = (Transform.GetLocation() - Behavior.TargetLocation).GetSafeNormal();
                            DesiredVelocity = FleeDirection * MaxSpeed * 2.0f; // Velocidade máxima
                        }
                        break;
                    }
                    
                    case FDinosaurBehaviorFragment::EBehaviorState::Resting:
                    {
                        // Parado, recuperando energia
                        DesiredVelocity = FVector::ZeroVector;
                        break;
                    }
                }
                
                // Aplica velocidade suavizada
                Velocity.Value = FMath::VInterpTo(Velocity.Value, DesiredVelocity, DeltaTime, 2.0f);
                
                // Atualiza rotação baseada na velocidade
                if (!Velocity.Value.IsNearlyZero())
                {
                    FRotator NewRotation = Velocity.Value.Rotation();
                    NewRotation.Pitch = 0; // Manter no plano horizontal
                    Transform.SetRotation(FMath::RInterpTo(Transform.GetRotation().Rotator(), NewRotation, DeltaTime, 3.0f).Quaternion());
                }
            }
        });
}

void UDinosaurMassProcessor::ProcessInteractions(FMassExecutionContext& Context)
{
    // Implementação simplificada - versão completa usaria spatial hash
    // Por agora, processa apenas interações de proximidade imediata
    
    ActiveDinosaurQuery.ForEachEntityChunk(Context.GetEntityManager(), Context,
        [this](FMassExecutionContext& Context)
        {
            const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
            const auto& SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
            auto& BehaviorList = Context.GetMutableFragmentView<FDinosaurBehaviorFragment>();
            
            const int32 NumEntities = Context.GetNumEntities();
            
            // Verifica interações entre entidades próximas
            for (int32 i = 0; i < NumEntities; ++i)
            {
                const auto& Transform1 = TransformList[i];
                const auto& Species1 = SpeciesList[i];
                auto& Behavior1 = BehaviorList[i];
                
                for (int32 j = i + 1; j < NumEntities; ++j)
                {
                    const auto& Transform2 = TransformList[j];
                    const auto& Species2 = SpeciesList[j];
                    auto& Behavior2 = BehaviorList[j];
                    
                    float Distance = FVector::Dist(Transform1.GetLocation(), Transform2.GetLocation());
                    float InteractionRange = FMath::Max(Species1.DetectionRadius, Species2.DetectionRadius);
                    
                    if (Distance < InteractionRange)
                    {
                        // Determina tipo de interação baseado nas espécies
                        bool IsPredatorPrey = (Species1.SpeciesType == 1 && Species2.SpeciesType == 0) ||
                                            (Species1.SpeciesType == 0 && Species2.SpeciesType == 1);
                        
                        if (IsPredatorPrey)
                        {
                            // Predador caça, presa foge
                            if (Species1.SpeciesType == 1) // Species1 é carnívoro
                            {
                                if (Behavior1.CurrentState != FDinosaurBehaviorFragment::EBehaviorState::Hunting)
                                {
                                    Behavior1.CurrentState = FDinosaurBehaviorFragment::EBehaviorState::Hunting;
                                    Behavior1.TargetLocation = Transform2.GetLocation();
                                    Behavior1.StateTimer = 0.0f;
                                }
                                
                                if (Behavior2.CurrentState != FDinosaurBehaviorFragment::EBehaviorState::Fleeing)
                                {
                                    Behavior2.CurrentState = FDinosaurBehaviorFragment::EBehaviorState::Fleeing;
                                    Behavior2.TargetLocation = Transform1.GetLocation();
                                    Behavior2.StateTimer = 0.0f;
                                }
                            }
                            else // Species2 é carnívoro
                            {
                                if (Behavior2.CurrentState != FDinosaurBehaviorFragment::EBehaviorState::Hunting)
                                {
                                    Behavior2.CurrentState = FDinosaurBehaviorFragment::EBehaviorState::Hunting;
                                    Behavior2.TargetLocation = Transform1.GetLocation();
                                    Behavior2.StateTimer = 0.0f;
                                }
                                
                                if (Behavior1.CurrentState != FDinosaurBehaviorFragment::EBehaviorState::Fleeing)
                                {
                                    Behavior1.CurrentState = FDinosaurBehaviorFragment::EBehaviorState::Fleeing;
                                    Behavior1.TargetLocation = Transform2.GetLocation();
                                    Behavior1.StateTimer = 0.0f;
                                }
                            }
                        }
                    }
                }
            }
        });
}

void UDinosaurMassProcessor::UpdateBehaviorState(FDinosaurBehaviorFragment& Behavior, 
                                               const FDinosaurNeedsFragment& Needs,
                                               const FDinosaurSpeciesFragment& Species,
                                               float DeltaTime)
{
    // Verifica se deve mudar de estado baseado nas necessidades
    bool ShouldChangeState = false;
    FDinosaurBehaviorFragment::EBehaviorState NewState = Behavior.CurrentState;
    
    // Prioridades por necessidade crítica
    if (Needs.Thirst < 50) // Sede crítica
    {
        NewState = FDinosaurBehaviorFragment::EBehaviorState::Drinking;
        ShouldChangeState = true;
    }
    else if (Needs.Hunger < 30) // Fome crítica
    {
        NewState = FDinosaurBehaviorFragment::EBehaviorState::Feeding;
        ShouldChangeState = true;
    }
    else if (Needs.Energy < 40) // Energia baixa
    {
        NewState = FDinosaurBehaviorFragment::EBehaviorState::Resting;
        ShouldChangeState = true;
    }
    else if (Behavior.StateTimer > Behavior.MaxStateDuration)
    {
        // Tempo no estado atual expirou, escolhe novo estado aleatório
        int32 RandomState = FMath::RandRange(0, 2);
        switch (RandomState)
        {
            case 0: NewState = FDinosaurBehaviorFragment::EBehaviorState::Wandering; break;
            case 1: NewState = FDinosaurBehaviorFragment::EBehaviorState::Socializing; break;
            case 2: NewState = FDinosaurBehaviorFragment::EBehaviorState::Feeding; break;
        }
        ShouldChangeState = true;
    }
    
    // Aplica mudança de estado se necessário
    if (ShouldChangeState && NewState != Behavior.CurrentState)
    {
        Behavior.CurrentState = NewState;
        Behavior.StateTimer = 0.0f;
        Behavior.TargetLocation = FVector::ZeroVector; // Reset target
        
        // Define duração aleatória para o novo estado
        Behavior.MaxStateDuration = FMath::RandRange(Behavior.MinStateDuration, 
                                                   Behavior.MinStateDuration * 3.0f);
    }
}

FVector UDinosaurMassProcessor::FindNearestResource(const FVector& Location, uint8 ResourceType)
{
    // Implementação simplificada - na versão final, consultaria sistema de recursos do mundo
    // Por agora, retorna posição aleatória próxima
    
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0;
    
    float SearchRadius = 1000.0f;
    if (ResourceType == 255) // Água
    {
        SearchRadius = 2000.0f; // Água é mais rara
    }
    
    return Location + RandomDirection * FMath::RandRange(200.0f, SearchRadius);
}