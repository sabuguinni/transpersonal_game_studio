#include "DinosaurBehaviorProcessor.h"
#include "MassEntityTypes.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DinosaurMassFragments.h"
#include "Crowd_SharedTypes.h"

UDinosaurBehaviorProcessor::UDinosaurBehaviorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UDinosaurBehaviorProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDinosaurStatsFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddOptionalRequirement<FMassNavigationEdgesFragment>(EMassFragmentAccess::ReadOnly);
}

void UDinosaurBehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const int32 NumEntities = EntityCollection.GetNumEntities();
        const auto TransformList = EntityCollection.GetFragmentView<FTransformFragment>();
        const auto VelocityList = EntityCollection.GetFragmentView<FMassVelocityFragment>();
        const auto SpeciesList = EntityCollection.GetFragmentView<FDinosaurSpeciesFragment>();
        const auto BehaviorList = EntityCollection.GetFragmentView<FDinosaurBehaviorFragment>();
        const auto StatsList = EntityCollection.GetFragmentView<FDinosaurStatsFragment>();
        
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FDinosaurStatsFragment& Stats = StatsList[EntityIndex];
            
            ProcessDinosaurBehavior(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
        }
    });
}

void UDinosaurBehaviorProcessor::ProcessDinosaurBehavior(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    // Atualizar timers de comportamento
    Behavior.StateTimer += DeltaTime;
    Behavior.DecisionCooldown -= DeltaTime;
    
    // Reduzir fome e sede ao longo do tempo
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - DeltaTime * 0.1f);
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - DeltaTime * 0.15f);
    
    // Recuperar stamina quando não está em combate
    if (Behavior.CurrentState != ECrowd_DinosaurBehaviorState::Fleeing && 
        Behavior.CurrentState != ECrowd_DinosaurBehaviorState::Hunting)
    {
        Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + DeltaTime * 20.0f);
    }
    
    // Decidir novo comportamento se o cooldown expirou
    if (Behavior.DecisionCooldown <= 0.0f)
    {
        DecideNewBehavior(Transform, Velocity, Species, Behavior, Stats);
        Behavior.DecisionCooldown = FMath::RandRange(2.0f, 5.0f); // Decisão a cada 2-5 segundos
    }
    
    // Executar comportamento atual
    ExecuteCurrentBehavior(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
}

void UDinosaurBehaviorProcessor::DecideNewBehavior(
    const FTransformFragment& Transform,
    const FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    const FDinosaurStatsFragment& Stats)
{
    // Prioridades baseadas nas necessidades
    float HungerPriority = (100.0f - Stats.Hunger) * 0.01f;
    float ThirstPriority = (100.0f - Stats.Thirst) * 0.01f;
    float RestPriority = (100.0f - Stats.Stamina) * 0.005f;
    
    // Carnívoros têm comportamento de caça
    if (Species.Diet == ECrowd_DinosaurDiet::Carnivore)
    {
        if (HungerPriority > 0.7f && Stats.Stamina > 30.0f)
        {
            Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Hunting;
            Behavior.StateTimer = 0.0f;
            return;
        }
    }
    
    // Herbívoros procuram comida quando com fome
    if (Species.Diet == ECrowd_DinosaurDiet::Herbivore && HungerPriority > 0.6f)
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Foraging;
        Behavior.StateTimer = 0.0f;
        return;
    }
    
    // Procurar água quando com sede
    if (ThirstPriority > 0.8f)
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::SeekingWater;
        Behavior.StateTimer = 0.0f;
        return;
    }
    
    // Descansar quando com pouca stamina
    if (RestPriority > 0.8f)
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Resting;
        Behavior.StateTimer = 0.0f;
        return;
    }
    
    // Comportamento social para dinossauros de grupo
    if (Species.SocialBehavior == ECrowd_SocialBehavior::Herd)
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Socializing;
        Behavior.StateTimer = 0.0f;
        return;
    }
    
    // Comportamento padrão: vagar
    Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Wandering;
    Behavior.StateTimer = 0.0f;
}

void UDinosaurBehaviorProcessor::ExecuteCurrentBehavior(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    const FVector CurrentLocation = Transform.GetTransform().GetLocation();
    
    switch (Behavior.CurrentState)
    {
        case ECrowd_DinosaurBehaviorState::Wandering:
            ExecuteWandering(Transform, Velocity, Species, Behavior, DeltaTime);
            break;
            
        case ECrowd_DinosaurBehaviorState::Foraging:
            ExecuteForaging(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
            break;
            
        case ECrowd_DinosaurBehaviorState::Hunting:
            ExecuteHunting(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
            break;
            
        case ECrowd_DinosaurBehaviorState::SeekingWater:
            ExecuteSeekingWater(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
            break;
            
        case ECrowd_DinosaurBehaviorState::Resting:
            ExecuteResting(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
            break;
            
        case ECrowd_DinosaurBehaviorState::Socializing:
            ExecuteSocializing(Transform, Velocity, Species, Behavior, DeltaTime);
            break;
            
        case ECrowd_DinosaurBehaviorState::Fleeing:
            ExecuteFleeing(Transform, Velocity, Species, Behavior, Stats, DeltaTime);
            break;
            
        default:
            // Estado desconhecido, voltar para wandering
            Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Wandering;
            break;
    }
}

void UDinosaurBehaviorProcessor::ExecuteWandering(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    float DeltaTime)
{
    // Movimento aleatório com mudança de direção ocasional
    if (Behavior.StateTimer > 5.0f || Velocity.Value.Size() < 10.0f)
    {
        // Escolher nova direção aleatória
        const float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        const FVector NewDirection = FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);
        
        Velocity.Value = NewDirection * Species.MaxSpeed * 0.3f; // Velocidade casual
        Behavior.StateTimer = 0.0f;
    }
}

void UDinosaurBehaviorProcessor::ExecuteForaging(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    // Simular procura por comida
    if (Behavior.StateTimer > 3.0f)
    {
        // "Encontrou" comida - aumentar fome
        Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + 30.0f);
        
        // Parar para "comer"
        Velocity.Value = FVector::ZeroVector;
        
        if (Behavior.StateTimer > 8.0f)
        {
            // Terminou de comer
            Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Wandering;
            Behavior.StateTimer = 0.0f;
        }
    }
    else
    {
        // Movimento lento procurando comida
        const FVector CurrentLocation = Transform.GetTransform().GetLocation();
        const FVector SearchDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        Velocity.Value = SearchDirection * Species.MaxSpeed * 0.2f;
    }
}

void UDinosaurBehaviorProcessor::ExecuteHunting(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    // Comportamento de caça - movimento mais agressivo
    Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - DeltaTime * 10.0f);
    
    if (Stats.Stamina < 10.0f)
    {
        // Muito cansado para caçar
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Resting;
        Behavior.StateTimer = 0.0f;
        return;
    }
    
    // Movimento rápido e errático
    if (Behavior.StateTimer > 2.0f)
    {
        const float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        const FVector HuntDirection = FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);
        
        Velocity.Value = HuntDirection * Species.MaxSpeed * 0.8f;
        Behavior.StateTimer = 0.0f;
    }
    
    // Simular sucesso na caça ocasionalmente
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
    {
        Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + 50.0f);
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Resting;
        Behavior.StateTimer = 0.0f;
    }
}

void UDinosaurBehaviorProcessor::ExecuteSeekingWater(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    // Simular movimento em direção à água (pântano)
    const FVector SwampCenter(-50000.0f, -45000.0f, 0.0f);
    const FVector CurrentLocation = Transform.GetTransform().GetLocation();
    const FVector ToWater = (SwampCenter - CurrentLocation).GetSafeNormal();
    
    Velocity.Value = ToWater * Species.MaxSpeed * 0.5f;
    
    // Se chegou perto da água
    const float DistanceToWater = FVector::Dist(CurrentLocation, SwampCenter);
    if (DistanceToWater < 5000.0f || Behavior.StateTimer > 10.0f)
    {
        // "Bebeu" água
        Stats.Thirst = FMath::Min(100.0f, Stats.Thirst + 40.0f);
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Wandering;
        Behavior.StateTimer = 0.0f;
    }
}

void UDinosaurBehaviorProcessor::ExecuteResting(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    // Parar movimento
    Velocity.Value = FVector::ZeroVector;
    
    // Recuperar stamina rapidamente
    Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + DeltaTime * 40.0f);
    
    // Descansar por pelo menos 5 segundos
    if (Behavior.StateTimer > 5.0f && Stats.Stamina > 80.0f)
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Wandering;
        Behavior.StateTimer = 0.0f;
    }
}

void UDinosaurBehaviorProcessor::ExecuteSocializing(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    float DeltaTime)
{
    // Movimento lento e circular (simulando interação social)
    const float CircleRadius = 500.0f;
    const float AngularSpeed = 0.5f;
    const float Angle = Behavior.StateTimer * AngularSpeed;
    
    const FVector CircleOffset = FVector(
        FMath::Cos(Angle) * CircleRadius,
        FMath::Sin(Angle) * CircleRadius,
        0.0f
    );
    
    Velocity.Value = FVector(-FMath::Sin(Angle), FMath::Cos(Angle), 0.0f) * Species.MaxSpeed * 0.2f;
    
    // Socializar por 10-15 segundos
    if (Behavior.StateTimer > FMath::RandRange(10.0f, 15.0f))
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Wandering;
        Behavior.StateTimer = 0.0f;
    }
}

void UDinosaurBehaviorProcessor::ExecuteFleeing(
    FTransformFragment& Transform,
    FMassVelocityFragment& Velocity,
    const FDinosaurSpeciesFragment& Species,
    FDinosaurBehaviorFragment& Behavior,
    FDinosaurStatsFragment& Stats,
    float DeltaTime)
{
    // Movimento rápido para longe do perigo
    Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - DeltaTime * 15.0f);
    
    // Direção de fuga (aleatória por agora)
    if (Behavior.StateTimer < 0.1f)
    {
        const float FleeAngle = FMath::RandRange(0.0f, 2.0f * PI);
        const FVector FleeDirection = FVector(FMath::Cos(FleeAngle), FMath::Sin(FleeAngle), 0.0f);
        Velocity.Value = FleeDirection * Species.MaxSpeed;
    }
    
    // Parar de fugir após 5 segundos ou quando sem stamina
    if (Behavior.StateTimer > 5.0f || Stats.Stamina < 5.0f)
    {
        Behavior.CurrentState = ECrowd_DinosaurBehaviorState::Resting;
        Behavior.StateTimer = 0.0f;
    }
}