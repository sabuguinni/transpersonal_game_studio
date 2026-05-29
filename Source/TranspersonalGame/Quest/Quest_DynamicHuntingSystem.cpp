#include "Quest_DynamicHuntingSystem.h"
#include "Engine/World.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TranspersonalGame/TranspersonalGameState.h"

AQuest_DynamicHuntingSystem::AQuest_DynamicHuntingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Configurar componente raiz
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Configurar componente de detecção
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(5000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    
    // Bind eventos
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_DynamicHuntingSystem::OnPlayerEnterHuntingZone);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_DynamicHuntingSystem::OnPlayerExitHuntingZone);
    
    // Configurações padrão
    HuntingZoneRadius = 5000.0f;
    MaxActiveHunts = 3;
    HuntDifficultyMultiplier = 1.0f;
    bIsHuntingZoneActive = false;
    
    // Definir tipos de caça por bioma
    InitializeHuntingTargets();
}

void AQuest_DynamicHuntingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Determinar bioma baseado na localização
    CurrentBiome = DetermineBiomeFromLocation(GetActorLocation());
    
    // Configurar zona de caça
    SetupHuntingZone();
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Initialized in biome %s"), *UEnum::GetValueAsString(CurrentBiome));
}

void AQuest_DynamicHuntingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsHuntingZoneActive)
    {
        UpdateActiveHunts(DeltaTime);
        CheckHuntCompletion();
    }
}

void AQuest_DynamicHuntingSystem::InitializeHuntingTargets()
{
    // Pantano - Predadores aquáticos e anfíbios
    BiomeHuntingTargets.Add(EQuest_BiomeType::Swamp, {
        FQuest_HuntingTarget{TEXT("Spinosaurus"), 1, EQuest_HuntDifficulty::Extreme, 1000},
        FQuest_HuntingTarget{TEXT("Sarcosuchus"), 2, EQuest_HuntDifficulty::Hard, 500},
        FQuest_HuntingTarget{TEXT("Deinosuchus"), 3, EQuest_HuntDifficulty::Medium, 300}
    });
    
    // Floresta - Herbívoros e pequenos carnívoros
    BiomeHuntingTargets.Add(EQuest_BiomeType::Forest, {
        FQuest_HuntingTarget{TEXT("Parasaurolophus"), 2, EQuest_HuntDifficulty::Easy, 200},
        FQuest_HuntingTarget{TEXT("Compsognathus"), 5, EQuest_HuntDifficulty::Easy, 50},
        FQuest_HuntingTarget{TEXT("Carnotaurus"), 1, EQuest_HuntDifficulty::Hard, 600}
    });
    
    // Savana - Grandes herbívoros e pack hunters
    BiomeHuntingTargets.Add(EQuest_BiomeType::Savanna, {
        FQuest_HuntingTarget{TEXT("Triceratops"), 1, EQuest_HuntDifficulty::Medium, 400},
        FQuest_HuntingTarget{TEXT("Velociraptor"), 3, EQuest_HuntDifficulty::Medium, 250},
        FQuest_HuntingTarget{TEXT("Ankylosaurus"), 1, EQuest_HuntDifficulty::Hard, 500}
    });
    
    // Deserto - Adaptados ao calor
    BiomeHuntingTargets.Add(EQuest_BiomeType::Desert, {
        FQuest_HuntingTarget{TEXT("Carcharodontosaurus"), 1, EQuest_HuntDifficulty::Extreme, 800},
        FQuest_HuntingTarget{TEXT("Ouranosaurus"), 2, EQuest_HuntDifficulty::Medium, 300},
        FQuest_HuntingTarget{TEXT("Dilophosaurus"), 2, EQuest_HuntDifficulty::Medium, 200}
    });
    
    // Montanha - Dinossauros de altitude
    BiomeHuntingTargets.Add(EQuest_BiomeType::Mountain, {
        FQuest_HuntingTarget{TEXT("Therizinosaurus"), 1, EQuest_HuntDifficulty::Hard, 600},
        FQuest_HuntingTarget{TEXT("Utahraptor"), 2, EQuest_HuntDifficulty::Hard, 400},
        FQuest_HuntingTarget{TEXT("Amargasaurus"), 1, EQuest_HuntDifficulty::Medium, 350}
    });
}

EQuest_BiomeType AQuest_DynamicHuntingSystem::DetermineBiomeFromLocation(const FVector& Location)
{
    float X = Location.X;
    float Y = Location.Y;
    float Z = Location.Z;
    
    // Pantano (sudoeste)
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
        return EQuest_BiomeType::Swamp;
    
    // Floresta (noroeste)
    if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
        return EQuest_BiomeType::Forest;
    
    // Montanha (nordeste)
    if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500 && Z > 300)
        return EQuest_BiomeType::Mountain;
    
    // Deserto (leste)
    if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
        return EQuest_BiomeType::Desert;
    
    // Savana (centro) - default
    return EQuest_BiomeType::Savanna;
}

void AQuest_DynamicHuntingSystem::SetupHuntingZone()
{
    DetectionSphere->SetSphereRadius(HuntingZoneRadius);
    
    // Configurar visual da zona (opcional)
    if (UStaticMeshComponent* VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneVisual")))
    {
        VisualComponent->SetupAttachment(RootComponent);
        VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualComponent->SetVisibility(false); // Invisível por padrão
    }
}

void AQuest_DynamicHuntingSystem::OnPlayerEnterHuntingZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        if (PlayerPawn->IsPlayerControlled())
        {
            bIsHuntingZoneActive = true;
            CurrentPlayerPawn = PlayerPawn;
            
            // Gerar missão de caça
            GenerateHuntingMission();
            
            UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Player entered hunting zone in %s"), *UEnum::GetValueAsString(CurrentBiome));
        }
    }
}

void AQuest_DynamicHuntingSystem::OnPlayerExitHuntingZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        if (PlayerPawn->IsPlayerControlled())
        {
            bIsHuntingZoneActive = false;
            CurrentPlayerPawn = nullptr;
            
            // Limpar missões activas
            ActiveHunts.Empty();
            
            UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Player exited hunting zone"));
        }
    }
}

void AQuest_DynamicHuntingSystem::GenerateHuntingMission()
{
    if (!BiomeHuntingTargets.Contains(CurrentBiome))
        return;
    
    const TArray<FQuest_HuntingTarget>& AvailableTargets = BiomeHuntingTargets[CurrentBiome];
    
    if (AvailableTargets.Num() == 0)
        return;
    
    // Seleccionar alvo aleatório
    int32 RandomIndex = FMath::RandRange(0, AvailableTargets.Num() - 1);
    const FQuest_HuntingTarget& SelectedTarget = AvailableTargets[RandomIndex];
    
    // Criar missão de caça
    FQuest_ActiveHunt NewHunt;
    NewHunt.TargetSpecies = SelectedTarget.SpeciesName;
    NewHunt.RequiredKills = SelectedTarget.RequiredCount;
    NewHunt.CurrentKills = 0;
    NewHunt.Difficulty = SelectedTarget.Difficulty;
    NewHunt.ExperienceReward = SelectedTarget.ExperienceReward;
    NewHunt.bIsCompleted = false;
    NewHunt.TimeRemaining = CalculateHuntTimeLimit(SelectedTarget.Difficulty);
    
    ActiveHunts.Add(NewHunt);
    
    // Notificar jogador
    OnHuntingMissionGenerated.Broadcast(NewHunt);
    
    UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Generated hunt mission for %s (x%d)"), 
           *SelectedTarget.SpeciesName, SelectedTarget.RequiredCount);
}

void AQuest_DynamicHuntingSystem::UpdateActiveHunts(float DeltaTime)
{
    for (int32 i = ActiveHunts.Num() - 1; i >= 0; i--)
    {
        FQuest_ActiveHunt& Hunt = ActiveHunts[i];
        
        if (!Hunt.bIsCompleted)
        {
            Hunt.TimeRemaining -= DeltaTime;
            
            // Verificar se o tempo expirou
            if (Hunt.TimeRemaining <= 0.0f)
            {
                OnHuntingMissionFailed.Broadcast(Hunt);
                ActiveHunts.RemoveAt(i);
                UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Hunt mission failed - time expired"));
            }
        }
    }
}

void AQuest_DynamicHuntingSystem::CheckHuntCompletion()
{
    for (int32 i = 0; i < ActiveHunts.Num(); i++)
    {
        FQuest_ActiveHunt& Hunt = ActiveHunts[i];
        
        if (!Hunt.bIsCompleted && Hunt.CurrentKills >= Hunt.RequiredKills)
        {
            Hunt.bIsCompleted = true;
            
            // Dar recompensas
            AwardHuntingRewards(Hunt);
            
            OnHuntingMissionCompleted.Broadcast(Hunt);
            
            UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Hunt mission completed for %s"), *Hunt.TargetSpecies);
        }
    }
}

void AQuest_DynamicHuntingSystem::RegisterDinosaurKill(const FString& SpeciesName, const FVector& KillLocation)
{
    for (FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (!Hunt.bIsCompleted && Hunt.TargetSpecies == SpeciesName)
        {
            Hunt.CurrentKills++;
            
            // Verificar se está dentro da zona
            float DistanceToZone = FVector::Dist(KillLocation, GetActorLocation());
            if (DistanceToZone <= HuntingZoneRadius)
            {
                UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Registered kill for %s (%d/%d)"), 
                       *SpeciesName, Hunt.CurrentKills, Hunt.RequiredKills);
            }
            else
            {
                Hunt.CurrentKills--; // Não conta se foi fora da zona
                UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Kill outside hunting zone - not counted"));
            }
            
            break;
        }
    }
}

float AQuest_DynamicHuntingSystem::CalculateHuntTimeLimit(EQuest_HuntDifficulty Difficulty)
{
    switch (Difficulty)
    {
        case EQuest_HuntDifficulty::Easy:
            return 300.0f; // 5 minutos
        case EQuest_HuntDifficulty::Medium:
            return 600.0f; // 10 minutos
        case EQuest_HuntDifficulty::Hard:
            return 900.0f; // 15 minutos
        case EQuest_HuntDifficulty::Extreme:
            return 1200.0f; // 20 minutos
        default:
            return 600.0f;
    }
}

void AQuest_DynamicHuntingSystem::AwardHuntingRewards(const FQuest_ActiveHunt& CompletedHunt)
{
    if (CurrentPlayerPawn)
    {
        // Tentar obter GameState para dar experiência
        if (ATranspersonalGameState* GameState = GetWorld()->GetGameState<ATranspersonalGameState>())
        {
            // Dar experiência baseada na dificuldade
            int32 ExperienceGain = CompletedHunt.ExperienceReward * HuntDifficultyMultiplier;
            
            UE_LOG(LogTemp, Warning, TEXT("DynamicHuntingSystem: Awarded %d experience for completing hunt"), ExperienceGain);
        }
    }
}

TArray<FQuest_ActiveHunt> AQuest_DynamicHuntingSystem::GetActiveHunts() const
{
    return ActiveHunts;
}

bool AQuest_DynamicHuntingSystem::IsPlayerInHuntingZone() const
{
    return bIsHuntingZoneActive;
}

EQuest_BiomeType AQuest_DynamicHuntingSystem::GetCurrentBiome() const
{
    return CurrentBiome;
}