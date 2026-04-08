#include "JurassicCrowdManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UJurassicCrowdManager::UJurassicCrowdManager()
{
    MassEntitySubsystem = nullptr;
}

void UJurassicCrowdManager::InitializeCrowdSystem(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("UJurassicCrowdManager: World is null"));
        return;
    }

    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UJurassicCrowdManager: Failed to get MassEntitySubsystem"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Crowd system initialized successfully"));

    // Limpar dados anteriores
    ActiveHerds.Empty();
    ActivePredators.Empty();
}

void UJurassicCrowdManager::SpawnHerd(FName Species, int32 HerdSize, FVector SpawnLocation, float SpawnRadius)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UJurassicCrowdManager: MassEntitySubsystem not initialized"));
        return;
    }

    // Validação de parâmetros
    HerdSize = FMath::Clamp(HerdSize, 1, MaxDinosaursPerSpecies);
    
    // Gerar ID único para a manada
    FGuid HerdID = FGuid::NewGuid();
    TArray<FMassEntityHandle> HerdMembers;

    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Spawning herd of %d %s at location %s"), 
           HerdSize, *Species.ToString(), *SpawnLocation.ToString());

    // Spawn individual de cada membro da manada
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Posição aleatória dentro do raio de spawn
        FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(0.0f, SpawnRadius);
        FVector MemberSpawnLocation = SpawnLocation + RandomOffset;

        // Criar entidade Mass
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        
        if (!EntityHandle.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("UJurassicCrowdManager: Failed to create entity for herd member %d"), i);
            continue;
        }

        // Adicionar fragmentos básicos
        FTransformFragment TransformFragment;
        TransformFragment.GetMutableTransform().SetLocation(MemberSpawnLocation);
        TransformFragment.GetMutableTransform().SetRotation(FQuat::MakeFromEuler(FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f))));
        MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, TransformFragment);

        FMassVelocityFragment VelocityFragment;
        VelocityFragment.Value = FVector::ZeroVector;
        MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, VelocityFragment);

        // Gerar características únicas do dinossauro
        FJurassicDinosaurFragment DinosaurFragment = GenerateUniqueTraits(Species);
        MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, DinosaurFragment);

        // Adicionar fragmento de manada
        FJurassicHerdFragment HerdFragment;
        HerdFragment.HerdID = HerdID;
        HerdFragment.bIsHerdLeader = (i == 0); // Primeiro membro é o líder
        HerdFragment.bIsScout = (i == 1 && HerdSize > 1); // Segundo membro é scout
        MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, HerdFragment);

        // Adicionar rotina diária
        FJurassicDailyRoutineFragment RoutineFragment;
        RoutineFragment.FeedingLocation = SpawnLocation + UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(500.0f, 2000.0f);
        RoutineFragment.RestingLocation = SpawnLocation + UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(200.0f, 800.0f);
        RoutineFragment.WaterSource = SpawnLocation + UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(1000.0f, 3000.0f);
        MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, RoutineFragment);

        HerdMembers.Add(EntityHandle);
    }

    // Estabelecer hierarquia social na manada
    CreateHerdHierarchy(HerdMembers, HerdID);

    // Registrar manada ativa
    ActiveHerds.Add(HerdID, HerdMembers);

    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Successfully spawned herd %s with %d members"), 
           *HerdID.ToString(), HerdMembers.Num());
}

void UJurassicCrowdManager::SpawnPredator(FName Species, FVector SpawnLocation, FVector TerritoryCenter, float TerritoryRadius)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UJurassicCrowdManager: MassEntitySubsystem not initialized"));
        return;
    }

    // Criar entidade para predador
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    
    if (!EntityHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UJurassicCrowdManager: Failed to create predator entity"));
        return;
    }

    // Fragmentos básicos
    FTransformFragment TransformFragment;
    TransformFragment.GetMutableTransform().SetLocation(SpawnLocation);
    TransformFragment.GetMutableTransform().SetRotation(FQuat::MakeFromEuler(FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f))));
    MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, TransformFragment);

    FMassVelocityFragment VelocityFragment;
    VelocityFragment.Value = FVector::ZeroVector;
    MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, VelocityFragment);

    // Características do predador
    FJurassicDinosaurFragment DinosaurFragment = GenerateUniqueTraits(Species);
    DinosaurFragment.bIsHerbivore = false;
    DinosaurFragment.AggressionLevel = UKismetMathLibrary::RandomFloatInRange(0.7f, 1.0f);
    DinosaurFragment.FearLevel = UKismetMathLibrary::RandomFloatInRange(0.1f, 0.3f);
    MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, DinosaurFragment);

    // Rotina de predador
    FJurassicDailyRoutineFragment RoutineFragment;
    RoutineFragment.bIsHunting = true;
    RoutineFragment.FeedingLocation = TerritoryCenter;
    RoutineFragment.RestingLocation = TerritoryCenter + UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(200.0f, 500.0f);
    MassEntitySubsystem->GetEntityManager().AddFragmentToEntity(EntityHandle, RoutineFragment);

    // Registrar predador ativo
    FGuid PredatorID = DinosaurFragment.DinosaurID;
    ActivePredators.Add(PredatorID, EntityHandle);

    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Spawned predator %s at territory %s"), 
           *Species.ToString(), *TerritoryCenter.ToString());
}

bool UJurassicCrowdManager::AttemptDomestication(FGuid DinosaurID, float PlayerTrustGain)
{
    // Encontrar a entidade correspondente
    for (auto& HerdPair : ActiveHerds)
    {
        for (FMassEntityHandle& EntityHandle : HerdPair.Value)
        {
            if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicDinosaurFragment>(EntityHandle))
            {
                FJurassicDinosaurFragment* DinosaurFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicDinosaurFragment>(EntityHandle);
                
                if (DinosaurFragment && DinosaurFragment->DinosaurID == DinosaurID)
                {
                    // Apenas herbívoros podem ser domesticados
                    if (!DinosaurFragment->bIsHerbivore)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("UJurassicCrowdManager: Cannot domesticate carnivore %s"), *DinosaurID.ToString());
                        return false;
                    }

                    // Aumentar confiança no jogador
                    DinosaurFragment->PlayerTrustLevel = FMath::Clamp(DinosaurFragment->PlayerTrustLevel + PlayerTrustGain, 0.0f, 1.0f);

                    // Domesticação bem-sucedida se confiança > 0.8
                    if (DinosaurFragment->PlayerTrustLevel > 0.8f && !DinosaurFragment->bIsDomesticated)
                    {
                        DinosaurFragment->bIsDomesticated = true;
                        DinosaurFragment->AggressionLevel *= 0.3f; // Reduzir agressividade
                        DinosaurFragment->FearLevel *= 0.5f; // Reduzir medo
                        
                        UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Successfully domesticated dinosaur %s"), *DinosaurID.ToString());
                        return true;
                    }

                    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Trust level increased to %f for dinosaur %s"), 
                           DinosaurFragment->PlayerTrustLevel, *DinosaurID.ToString());
                    return false;
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UJurassicCrowdManager: Dinosaur %s not found"), *DinosaurID.ToString());
    return false;
}

TArray<FGuid> UJurassicCrowdManager::GetNearbyDinosaurs(FVector Location, float Radius)
{
    TArray<FGuid> NearbyDinosaurs;

    if (!MassEntitySubsystem)
    {
        return NearbyDinosaurs;
    }

    float RadiusSquared = Radius * Radius;

    // Verificar todas as manadas ativas
    for (auto& HerdPair : ActiveHerds)
    {
        for (FMassEntityHandle& EntityHandle : HerdPair.Value)
        {
            if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FTransformFragment>(EntityHandle) &&
                MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicDinosaurFragment>(EntityHandle))
            {
                const FTransformFragment* TransformFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FTransformFragment>(EntityHandle);
                const FJurassicDinosaurFragment* DinosaurFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicDinosaurFragment>(EntityHandle);

                if (TransformFragment && DinosaurFragment)
                {
                    float DistanceSquared = FVector::DistSquared(Location, TransformFragment->GetTransform().GetLocation());
                    if (DistanceSquared <= RadiusSquared)
                    {
                        NearbyDinosaurs.Add(DinosaurFragment->DinosaurID);
                    }
                }
            }
        }
    }

    // Verificar predadores ativos
    for (auto& PredatorPair : ActivePredators)
    {
        FMassEntityHandle EntityHandle = PredatorPair.Value;
        
        if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FTransformFragment>(EntityHandle) &&
            MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicDinosaurFragment>(EntityHandle))
        {
            const FTransformFragment* TransformFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FTransformFragment>(EntityHandle);
            const FJurassicDinosaurFragment* DinosaurFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicDinosaurFragment>(EntityHandle);

            if (TransformFragment && DinosaurFragment)
            {
                float DistanceSquared = FVector::DistSquared(Location, TransformFragment->GetTransform().GetLocation());
                if (DistanceSquared <= RadiusSquared)
                {
                    NearbyDinosaurs.Add(DinosaurFragment->DinosaurID);
                }
            }
        }
    }

    return NearbyDinosaurs;
}

FJurassicDinosaurFragment UJurassicCrowdManager::GetDinosaurInfo(FGuid DinosaurID)
{
    FJurassicDinosaurFragment EmptyFragment;

    // Procurar nas manadas
    for (auto& HerdPair : ActiveHerds)
    {
        for (FMassEntityHandle& EntityHandle : HerdPair.Value)
        {
            if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicDinosaurFragment>(EntityHandle))
            {
                const FJurassicDinosaurFragment* DinosaurFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicDinosaurFragment>(EntityHandle);
                
                if (DinosaurFragment && DinosaurFragment->DinosaurID == DinosaurID)
                {
                    return *DinosaurFragment;
                }
            }
        }
    }

    // Procurar nos predadores
    for (auto& PredatorPair : ActivePredators)
    {
        FMassEntityHandle EntityHandle = PredatorPair.Value;
        
        if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicDinosaurFragment>(EntityHandle))
        {
            const FJurassicDinosaurFragment* DinosaurFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicDinosaurFragment>(EntityHandle);
            
            if (DinosaurFragment && DinosaurFragment->DinosaurID == DinosaurID)
            {
                return *DinosaurFragment;
            }
        }
    }

    return EmptyFragment;
}

void UJurassicCrowdManager::UpdatePlayerMemory(FGuid DinosaurID, FVector PlayerLocation, bool bPositiveInteraction)
{
    // Implementação similar ao GetDinosaurInfo, mas modificando o fragmento
    for (auto& HerdPair : ActiveHerds)
    {
        for (FMassEntityHandle& EntityHandle : HerdPair.Value)
        {
            if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicDinosaurFragment>(EntityHandle))
            {
                FJurassicDinosaurFragment* DinosaurFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicDinosaurFragment>(EntityHandle);
                
                if (DinosaurFragment && DinosaurFragment->DinosaurID == DinosaurID)
                {
                    // Atualizar memória do jogador
                    DinosaurFragment->PlayerMemory = FGuid::NewGuid(); // Simular memória
                    
                    if (bPositiveInteraction)
                    {
                        DinosaurFragment->PlayerTrustLevel = FMath::Clamp(DinosaurFragment->PlayerTrustLevel + 0.1f, 0.0f, 1.0f);
                        DinosaurFragment->FearLevel = FMath::Clamp(DinosaurFragment->FearLevel - 0.05f, 0.0f, 1.0f);
                    }
                    else
                    {
                        DinosaurFragment->PlayerTrustLevel = FMath::Clamp(DinosaurFragment->PlayerTrustLevel - 0.2f, 0.0f, 1.0f);
                        DinosaurFragment->FearLevel = FMath::Clamp(DinosaurFragment->FearLevel + 0.1f, 0.0f, 1.0f);
                    }
                    
                    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Updated memory for dinosaur %s. Trust: %f, Fear: %f"), 
                           *DinosaurID.ToString(), DinosaurFragment->PlayerTrustLevel, DinosaurFragment->FearLevel);
                    return;
                }
            }
        }
    }
}

FJurassicDinosaurFragment UJurassicCrowdManager::GenerateUniqueTraits(FName Species)
{
    FJurassicDinosaurFragment DinosaurTraits;
    
    // ID único
    DinosaurTraits.DinosaurID = FGuid::NewGuid();
    DinosaurTraits.Species = Species;

    // Aplicar variação genética baseada na espécie
    ApplyGeneticVariation(DinosaurTraits, Species);

    // Configurações padrão baseadas na espécie
    if (Species == TEXT("Triceratops") || Species == TEXT("Stegosaurus") || Species == TEXT("Brontosaurus"))
    {
        DinosaurTraits.bIsHerbivore = true;
        DinosaurTraits.AggressionLevel = UKismetMathLibrary::RandomFloatInRange(0.2f, 0.5f);
        DinosaurTraits.FearLevel = UKismetMathLibrary::RandomFloatInRange(0.3f, 0.7f);
    }
    else if (Species == TEXT("TRex") || Species == TEXT("Velociraptor") || Species == TEXT("Allosaurus"))
    {
        DinosaurTraits.bIsHerbivore = false;
        DinosaurTraits.AggressionLevel = UKismetMathLibrary::RandomFloatInRange(0.7f, 1.0f);
        DinosaurTraits.FearLevel = UKismetMathLibrary::RandomFloatInRange(0.1f, 0.3f);
    }
    else
    {
        // Espécie desconhecida - configuração genérica
        DinosaurTraits.bIsHerbivore = UKismetMathLibrary::RandomBool();
        DinosaurTraits.AggressionLevel = UKismetMathLibrary::RandomFloatInRange(0.3f, 0.8f);
        DinosaurTraits.FearLevel = UKismetMathLibrary::RandomFloatInRange(0.2f, 0.6f);
    }

    return DinosaurTraits;
}

void UJurassicCrowdManager::CreateHerdHierarchy(TArray<FMassEntityHandle>& HerdMembers, FGuid HerdID)
{
    if (HerdMembers.Num() == 0)
    {
        return;
    }

    // O primeiro membro já foi definido como líder
    // Definir papéis adicionais baseados no tamanho da manada
    int32 NumScouts = FMath::Max(1, HerdMembers.Num() / 10); // 1 scout por 10 membros
    int32 ScoutCount = 0;

    for (int32 i = 1; i < HerdMembers.Num() && ScoutCount < NumScouts; i++)
    {
        if (MassEntitySubsystem->GetEntityManager().HasFragmentType<FJurassicHerdFragment>(HerdMembers[i]))
        {
            FJurassicHerdFragment* HerdFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FJurassicHerdFragment>(HerdMembers[i]);
            if (HerdFragment)
            {
                HerdFragment->bIsScout = true;
                ScoutCount++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UJurassicCrowdManager: Created herd hierarchy for %s with %d scouts"), 
           *HerdID.ToString(), ScoutCount);
}

void UJurassicCrowdManager::ApplyGeneticVariation(FJurassicDinosaurFragment& DinosaurTraits, FName Species)
{
    // Variação de tamanho corporal (±20%)
    DinosaurTraits.BodyScale = UKismetMathLibrary::RandomFloatInRange(0.8f, 1.2f);

    // Variação de cor da pele
    float Hue = UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f);
    float Saturation = UKismetMathLibrary::RandomFloatInRange(0.3f, 0.8f);
    float Value = UKismetMathLibrary::RandomFloatInRange(0.4f, 0.9f);
    DinosaurTraits.SkinTone = FLinearColor::MakeFromHSV8(Hue, Saturation * 255, Value * 255);

    // Variações específicas por espécie
    if (Species == TEXT("Triceratops"))
    {
        // Variação no tamanho dos chifres
        DinosaurTraits.HornSize = UKismetMathLibrary::RandomFloatInRange(0.7f, 1.3f);
    }
    else if (Species == TEXT("TRex"))
    {
        // Variação no tamanho dos olhos (predadores têm olhos maiores)
        DinosaurTraits.EyeSize = UKismetMathLibrary::RandomFloatInRange(1.0f, 1.4f);
    }
    else
    {
        // Variações genéricas
        DinosaurTraits.HornSize = UKismetMathLibrary::RandomFloatInRange(0.8f, 1.2f);
        DinosaurTraits.EyeSize = UKismetMathLibrary::RandomFloatInRange(0.8f, 1.2f);
    }
}