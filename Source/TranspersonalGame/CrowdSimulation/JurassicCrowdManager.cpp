#include "JurassicCrowdManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AJurassicCrowdManager::AJurassicCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Configuração padrão dos perfis de comportamento
    SetupDefaultHerdProfiles();
}

void AJurassicCrowdManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    SpawnInitialHerds();
}

void AJurassicCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateDayNightCycle(DeltaTime);
    UpdateHerdBehaviors(DeltaTime);
    ProcessEcosystemInteractions();
    UpdateLODSystem();
}

void AJurassicCrowdManager::InitializeMassSystem()
{
    UWorld* World = GetWorld();
    if (!World) return;

    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();

    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicCrowdManager: Failed to initialize Mass subsystems"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicCrowdManager: Mass subsystems initialized successfully"));
}

void AJurassicCrowdManager::SetupDefaultHerdProfiles()
{
    HerdProfiles.Empty();

    // Pequenos Herbívoros (Compsognathus)
    FHerdBehaviorProfile SmallHerb;
    SmallHerb.HerdType = EDinosaurHerdType::SmallHerbivore;
    SmallHerb.MinHerdSize = 8;
    SmallHerb.MaxHerdSize = 25;
    SmallHerb.CohesionRadius = 500.0f;
    SmallHerb.SeparationRadius = 100.0f;
    SmallHerb.AlertRadius = 1500.0f;
    SmallHerb.MovementSpeed = 400.0f;
    SmallHerb.PanicSpeedMultiplier = 4.0f;
    SmallHerb.PredatorTypes = {EDinosaurHerdType::SmallCarnivore, EDinosaurHerdType::LargeCarnivore};
    HerdProfiles.Add(SmallHerb);

    // Médios Herbívoros (Parasaurolophus)
    FHerdBehaviorProfile MediumHerb;
    MediumHerb.HerdType = EDinosaurHerdType::MediumHerbivore;
    MediumHerb.MinHerdSize = 5;
    MediumHerb.MaxHerdSize = 20;
    MediumHerb.CohesionRadius = 800.0f;
    MediumHerb.SeparationRadius = 200.0f;
    MediumHerb.AlertRadius = 2000.0f;
    MediumHerb.MovementSpeed = 300.0f;
    MediumHerb.PanicSpeedMultiplier = 2.5f;
    MediumHerb.PredatorTypes = {EDinosaurHerdType::LargeCarnivore};
    HerdProfiles.Add(MediumHerb);

    // Grandes Herbívoros (Brontosaurus)
    FHerdBehaviorProfile LargeHerb;
    LargeHerb.HerdType = EDinosaurHerdType::LargeHerbivore;
    LargeHerb.MinHerdSize = 3;
    LargeHerb.MaxHerdSize = 12;
    LargeHerb.CohesionRadius = 1200.0f;
    LargeHerb.SeparationRadius = 400.0f;
    LargeHerb.AlertRadius = 1000.0f; // Menos alerta devido ao tamanho
    LargeHerb.MovementSpeed = 200.0f;
    LargeHerb.PanicSpeedMultiplier = 1.8f;
    LargeHerb.PredatorTypes = {EDinosaurHerdType::LargeCarnivore}; // Apenas grandes predadores
    HerdProfiles.Add(LargeHerb);

    // Pequenos Carnívoros (Velociraptor)
    FHerdBehaviorProfile SmallCarn;
    SmallCarn.HerdType = EDinosaurHerdType::SmallCarnivore;
    SmallCarn.MinHerdSize = 2;
    SmallCarn.MaxHerdSize = 6;
    SmallCarn.CohesionRadius = 300.0f;
    SmallCarn.SeparationRadius = 150.0f;
    SmallCarn.AlertRadius = 2500.0f; // Predadores têm maior alcance de detecção
    SmallCarn.MovementSpeed = 500.0f;
    SmallCarn.PanicSpeedMultiplier = 2.0f;
    SmallCarn.PreyTypes = {EDinosaurHerdType::SmallHerbivore, EDinosaurHerdType::MediumHerbivore};
    SmallCarn.PredatorTypes = {EDinosaurHerdType::LargeCarnivore};
    HerdProfiles.Add(SmallCarn);

    // Grandes Carnívoros (T-Rex)
    FHerdBehaviorProfile LargeCarn;
    LargeCarn.HerdType = EDinosaurHerdType::LargeCarnivore;
    LargeCarn.MinHerdSize = 1;
    LargeCarn.MaxHerdSize = 3;
    LargeCarn.CohesionRadius = 2000.0f;
    LargeCarn.SeparationRadius = 800.0f;
    LargeCarn.AlertRadius = 3000.0f;
    LargeCarn.MovementSpeed = 350.0f;
    LargeCarn.PanicSpeedMultiplier = 1.5f;
    LargeCarn.PreyTypes = {EDinosaurHerdType::SmallHerbivore, EDinosaurHerdType::MediumHerbivore, 
                          EDinosaurHerdType::LargeHerbivore, EDinosaurHerdType::SmallCarnivore};
    HerdProfiles.Add(LargeCarn);

    // Bandos Voadores (Pteranodon)
    FHerdBehaviorProfile Flying;
    Flying.HerdType = EDinosaurHerdType::FlyingHerd;
    Flying.MinHerdSize = 15;
    Flying.MaxHerdSize = 80;
    Flying.CohesionRadius = 1500.0f;
    Flying.SeparationRadius = 300.0f;
    Flying.AlertRadius = 4000.0f; // Vista aérea privilegiada
    Flying.MovementSpeed = 800.0f;
    Flying.PanicSpeedMultiplier = 2.0f;
    Flying.bIsNocturnal = false;
    HerdProfiles.Add(Flying);
}

void AJurassicCrowdManager::SpawnInitialHerds()
{
    if (EcosystemZones.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicCrowdManager: No ecosystem zones defined"));
        return;
    }

    int32 TotalSpawned = 0;

    for (const FEcosystemZone& Zone : EcosystemZones)
    {
        int32 ZonePopulation = 0;
        
        for (const FHerdBehaviorProfile& AllowedHerd : Zone.AllowedHerdTypes)
        {
            // Calcula quantos grupos spawnar baseado na capacidade da zona
            int32 MaxGroups = FMath::Max(1, Zone.MaxTotalPopulation / (AllowedHerd.MaxHerdSize * Zone.AllowedHerdTypes.Num()));
            int32 GroupsToSpawn = FMath::RandRange(1, MaxGroups);

            for (int32 i = 0; i < GroupsToSpawn; i++)
            {
                // Posição aleatória dentro da zona
                FVector SpawnLocation = Zone.ZoneCenter + FMath::VRand() * FMath::RandRange(0.0f, Zone.ZoneRadius);
                SpawnLocation.Z = 0.0f; // Ajustar para o terreno depois

                int32 HerdSize = FMath::RandRange(AllowedHerd.MinHerdSize, AllowedHerd.MaxHerdSize);
                
                SpawnHerdAtLocation(AllowedHerd.HerdType, SpawnLocation, HerdSize);
                
                ZonePopulation += HerdSize;
                TotalSpawned += HerdSize;

                if (ZonePopulation >= Zone.MaxTotalPopulation) break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicCrowdManager: Spawned %d total entities across %d zones"), 
           TotalSpawned, EcosystemZones.Num());
}

void AJurassicCrowdManager::SpawnHerdAtLocation(EDinosaurHerdType HerdType, FVector Location, int32 HerdSize)
{
    if (!MassEntitySubsystem || !MassSpawnerSubsystem) return;

    // Encontra o perfil correspondente
    const FHerdBehaviorProfile* Profile = HerdProfiles.FindByPredicate(
        [HerdType](const FHerdBehaviorProfile& P) { return P.HerdType == HerdType; });

    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicCrowdManager: No profile found for herd type %d"), (int32)HerdType);
        return;
    }

    int32 ActualHerdSize = (HerdSize > 0) ? HerdSize : FMath::RandRange(Profile->MinHerdSize, Profile->MaxHerdSize);

    // TODO: Implementar spawn usando Mass Entity
    // Por agora, registra o spawn para debug
    UE_LOG(LogTemp, Log, TEXT("JurassicCrowdManager: Spawning %d %s at %s"), 
           ActualHerdSize, 
           *UEnum::GetValueAsString(HerdType),
           *Location.ToString());

    // Debug visual no editor
    if (GetWorld())
    {
        DrawDebugSphere(GetWorld(), Location, Profile->CohesionRadius, 12, FColor::Green, false, 5.0f);
    }
}

void AJurassicCrowdManager::UpdateDayNightCycle(float DeltaTime)
{
    if (!bEnableDayNightBehavior) return;

    // Simula passagem do tempo (24 horas = 24 minutos de jogo)
    CurrentTimeOfDay += DeltaTime * (24.0f / (24.0f * 60.0f)); // 1 hora de jogo = 1 minuto real
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    bool WasNight = bIsNightTime;
    bIsNightTime = (CurrentTimeOfDay < DawnHour || CurrentTimeOfDay > DuskHour);

    // Transição dia/noite
    if (WasNight != bIsNightTime)
    {
        UE_LOG(LogTemp, Log, TEXT("JurassicCrowdManager: Day/Night transition - Now %s (Time: %.1f)"), 
               bIsNightTime ? TEXT("Night") : TEXT("Day"), CurrentTimeOfDay);
        
        // Trigger comportamentos noturnos/diurnos
        OnDayNightTransition();
    }
}

void AJurassicCrowdManager::OnDayNightTransition()
{
    // Ativar/desativar espécies noturnas
    // Alterar padrões de movimento
    // Modificar comportamentos de caça
    
    for (const FHerdBehaviorProfile& Profile : HerdProfiles)
    {
        if (Profile.bIsNocturnal)
        {
            // Ativar grupos noturnos
            if (bIsNightTime)
            {
                UE_LOG(LogTemp, Log, TEXT("Activating nocturnal behavior for %s"), 
                       *UEnum::GetValueAsString(Profile.HerdType));
            }
        }
    }
}

void AJurassicCrowdManager::UpdateHerdBehaviors(float DeltaTime)
{
    // Atualiza comportamentos dos grupos ativos
    // Implementar usando Mass Processors
}

void AJurassicCrowdManager::ProcessEcosystemInteractions()
{
    // Processa interações predador-presa
    // Competição por recursos
    // Migração sazonal
}

void AJurassicCrowdManager::UpdateLODSystem()
{
    // Sistema de LOD baseado na distância do jogador
    // Reduz detalhes de simulação para entidades distantes
}

void AJurassicCrowdManager::TriggerPanicInRadius(FVector Location, float Radius, EDinosaurHerdType ThreatType)
{
    UE_LOG(LogTemp, Log, TEXT("JurassicCrowdManager: Panic triggered at %s (Radius: %.0f, Threat: %s)"), 
           *Location.ToString(), Radius, *UEnum::GetValueAsString(ThreatType));

    // TODO: Implementar usando Mass Entity queries
    // Encontrar todas as entidades no raio
    // Aplicar estado de pânico baseado no tipo de ameaça
    
    if (GetWorld())
    {
        DrawDebugSphere(GetWorld(), Location, Radius, 16, FColor::Red, false, 3.0f);
    }
}

void AJurassicCrowdManager::SetTimeOfDay(float Hour)
{
    CurrentTimeOfDay = FMath::Clamp(Hour, 0.0f, 24.0f);
    
    bool WasNight = bIsNightTime;
    bIsNightTime = (CurrentTimeOfDay < DawnHour || CurrentTimeOfDay > DuskHour);
    
    if (WasNight != bIsNightTime)
    {
        OnDayNightTransition();
    }
}

int32 AJurassicCrowdManager::GetActiveEntityCount() const
{
    if (!MassEntitySubsystem) return 0;
    
    // TODO: Implementar contagem real usando Mass Entity
    return 0; // Placeholder
}