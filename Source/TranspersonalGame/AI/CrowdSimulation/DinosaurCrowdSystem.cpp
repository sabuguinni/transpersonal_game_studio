#include "DinosaurCrowdSystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UDinosaurCrowdSystem::UDinosaurCrowdSystem()
{
    // Configurar pesos de spawn por tipo de dinossauro
    DinosaurSpawnWeights.Add(0, 0.6f); // Herbívoros - 60%
    DinosaurSpawnWeights.Add(1, 0.25f); // Carnívoros - 25%
    DinosaurSpawnWeights.Add(2, 0.15f); // Omnívoros - 15%
    
    // Configurar tamanhos típicos de manada
    TypicalHerdSizes.Add(0, 12); // Herbívoros em grupos grandes
    TypicalHerdSizes.Add(1, 3);  // Carnívoros em grupos pequenos
    TypicalHerdSizes.Add(2, 6);  // Omnívoros em grupos médios
}

void UDinosaurCrowdSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Obter referência ao subsistema Mass Entity
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurCrowdSystem: Failed to get MassEntitySubsystem"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSystem: Initialized successfully"));
}

void UDinosaurCrowdSystem::Deinitialize()
{
    MassEntitySubsystem = nullptr;
    Super::Deinitialize();
}

void UDinosaurCrowdSystem::SpawnDinosaurHerd(FVector Location, int32 HerdSize, uint8 DinosaurType, uint32 HerdID)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurCrowdSystem: MassEntitySubsystem not available"));
        return;
    }
    
    // Usar HerdID fornecido ou gerar novo
    uint32 ActualHerdID = (HerdID == 0) ? NextHerdID++ : HerdID;
    
    // Spawnar líder da manada primeiro
    FMassEntityHandle LeaderEntity = SpawnSolitaryDinosaur(Location, DinosaurType);
    
    if (LeaderEntity.IsValid())
    {
        // Configurar como líder
        FHerdBehaviorFragment LeaderHerdBehavior = GenerateHerdBehavior(DinosaurType, ActualHerdID, true);
        MassEntitySubsystem->GetFragmentDataPtr<FHerdBehaviorFragment>(LeaderEntity)->HerdRank = 0; // Líder
        MassEntitySubsystem->GetFragmentDataPtr<FHerdBehaviorFragment>(LeaderEntity)->HerdID = ActualHerdID;
    }
    
    // Spawnar seguidores da manada
    for (int32 i = 1; i < HerdSize; i++)
    {
        // Posição aleatória ao redor do líder
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(200.0f, 800.0f);
        FVector FollowerLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FMassEntityHandle FollowerEntity = SpawnSolitaryDinosaur(FollowerLocation, DinosaurType);
        
        if (FollowerEntity.IsValid())
        {
            // Configurar como seguidor
            FHerdBehaviorFragment* HerdBehavior = MassEntitySubsystem->GetFragmentDataPtr<FHerdBehaviorFragment>(FollowerEntity);
            if (HerdBehavior)
            {
                HerdBehavior->HerdID = ActualHerdID;
                HerdBehavior->HerdRank = (i < HerdSize * 0.2f) ? 2 : 1; // 20% jovens, 80% adultos
                HerdBehavior->PreferredDistanceFromLeader = FMath::RandRange(300.0f, 600.0f);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSystem: Spawned herd %d with %d members of type %d at %s"), 
           ActualHerdID, HerdSize, DinosaurType, *Location.ToString());
}

FMassEntityHandle UDinosaurCrowdSystem::SpawnSolitaryDinosaur(FVector Location, uint8 DinosaurType)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }
    
    // Criar nova entidade Mass
    FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
    
    if (!NewEntity.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurCrowdSystem: Failed to create Mass entity"));
        return FMassEntityHandle();
    }
    
    // Adicionar fragment de transformação
    FTransformFragment TransformFragment;
    TransformFragment.GetMutableTransform().SetLocation(Location);
    TransformFragment.GetMutableTransform().SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, TransformFragment);
    
    // Adicionar fragment de tipo de dinossauro
    FDinosaurTypeFragment DinosaurFragment;
    DinosaurFragment.DinosaurType = DinosaurType;
    DinosaurFragment.IndividualID = NextIndividualID++;
    
    // Determinar tamanho baseado no tipo
    switch (DinosaurType)
    {
        case 0: // Herbívoro
            DinosaurFragment.SizeCategory = FMath::RandRange(0, 2); // Pequeno a Grande
            DinosaurFragment.AggressionLevel = FMath::RandRange(10, 40);
            break;
        case 1: // Carnívoro
            DinosaurFragment.SizeCategory = FMath::RandRange(1, 3); // Médio a Gigante
            DinosaurFragment.AggressionLevel = FMath::RandRange(60, 90);
            break;
        case 2: // Omnívoro
            DinosaurFragment.SizeCategory = FMath::RandRange(0, 2); // Pequeno a Grande
            DinosaurFragment.AggressionLevel = FMath::RandRange(30, 60);
            break;
    }
    
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, DinosaurFragment);
    
    // Adicionar fragment de comportamento de grupo
    FHerdBehaviorFragment HerdBehavior = GenerateHerdBehavior(DinosaurType, 0, false); // Solitário por padrão
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, HerdBehavior);
    
    // Adicionar fragment de rotina diária
    FDailyRoutineFragment DailyRoutine;
    DailyRoutine.CurrentActivity = (DinosaurType == 1) ? 4 : 0; // Carnívoros caçam, outros pastam
    DailyRoutine.ActivityDuration = FMath::RandRange(180.0f, 600.0f);
    DailyRoutine.PreferredActivityLocation = Location;
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, DailyRoutine);
    
    // Adicionar fragment de variação física
    uint32 VariationSeed = FMath::Rand();
    FPhysicalVariationFragment PhysicalVariation = GeneratePhysicalVariation(VariationSeed);
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, PhysicalVariation);
    
    // Adicionar fragments de movimento Mass
    FMassVelocityFragment VelocityFragment;
    VelocityFragment.Value = FVector::ZeroVector;
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, VelocityFragment);
    
    FMassMoveTargetFragment MoveTargetFragment;
    MoveTargetFragment.Center = Location;
    MoveTargetFragment.Forward = FVector::ForwardVector;
    MassEntitySubsystem->AddFragmentToEntity(NewEntity, MoveTargetFragment);
    
    return NewEntity;
}

bool UDinosaurCrowdSystem::GetDinosaurInfo(FMassEntityHandle Entity, FDinosaurTypeFragment& OutDinosaurInfo)
{
    if (!MassEntitySubsystem || !Entity.IsValid())
    {
        return false;
    }
    
    const FDinosaurTypeFragment* DinosaurInfo = MassEntitySubsystem->GetFragmentDataPtr<FDinosaurTypeFragment>(Entity);
    if (DinosaurInfo)
    {
        OutDinosaurInfo = *DinosaurInfo;
        return true;
    }
    
    return false;
}

void UDinosaurCrowdSystem::SetHerdDestination(uint32 HerdID, FVector Destination)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Implementação simplificada - em produção usaria query Mass para encontrar entidades da manada
    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSystem: Setting destination %s for herd %d"), 
           *Destination.ToString(), HerdID);
    
    // TODO: Implementar query Mass para encontrar todas as entidades com HerdID específico
    // e atualizar seus MoveTargetFragment
}

FPhysicalVariationFragment UDinosaurCrowdSystem::GeneratePhysicalVariation(uint32 Seed)
{
    FMath::RandInit(Seed);
    
    FPhysicalVariationFragment Variation;
    Variation.OverallScale = FMath::RandRange(0.85f, 1.15f);
    Variation.ColorVariation = FMath::RandRange(0, 7); // 8 variações de cor
    Variation.DistinctiveFeatures = FMath::Rand(); // Características distintivas aleatórias
    Variation.VariationSeed = Seed;
    
    return Variation;
}

FHerdBehaviorFragment UDinosaurCrowdSystem::GenerateHerdBehavior(uint8 DinosaurType, uint32 HerdID, bool IsLeader)
{
    FHerdBehaviorFragment HerdBehavior;
    HerdBehavior.HerdID = HerdID;
    HerdBehavior.HerdRank = IsLeader ? 0 : 1;
    
    // Configurar comportamento baseado no tipo
    switch (DinosaurType)
    {
        case 0: // Herbívoro - grupos coesos
            HerdBehavior.PreferredDistanceFromLeader = FMath::RandRange(200.0f, 500.0f);
            HerdBehavior.CohesionRadius = 800.0f;
            break;
        case 1: // Carnívoro - grupos dispersos
            HerdBehavior.PreferredDistanceFromLeader = FMath::RandRange(500.0f, 1000.0f);
            HerdBehavior.CohesionRadius = 1500.0f;
            break;
        case 2: // Omnívoro - grupos médios
            HerdBehavior.PreferredDistanceFromLeader = FMath::RandRange(300.0f, 700.0f);
            HerdBehavior.CohesionRadius = 1000.0f;
            break;
    }
    
    return HerdBehavior;
}