#include "Quest_PracticalSurvivalMissions.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalCharacter.h"

AQuest_PracticalSurvivalMissions::AQuest_PracticalSurvivalMissions()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Criar esfera de detecção
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(500.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Criar texto de missão
    MissionText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MissionText"));
    MissionText->SetupAttachment(RootComponent);
    MissionText->SetText(FText::FromString(TEXT("Survival Mission System")));
    MissionText->SetTextRenderColor(FColor::Yellow);
    MissionText->SetWorldSize(100.0f);
    MissionText->SetHorizontalAlignment(EHTA_Center);
    MissionText->SetVerticalAlignment(EVTA_Center);

    // Configurações iniciais
    bAutoStartNextMission = true;
    MissionCheckInterval = 2.0f;
    LastMissionCheck = 0.0f;
    bPlayerInRange = false;
    PlayerCharacter = nullptr;

    // Inicializar missão atual
    CurrentMission = FQuest_SurvivalMissionData();
}

void AQuest_PracticalSurvivalMissions::BeginPlay()
{
    Super::BeginPlay();

    // Configurar eventos de overlap
    if (DetectionSphere)
    {
        DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_PracticalSurvivalMissions::OnPlayerEnterDetection);
        DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_PracticalSurvivalMissions::OnPlayerExitDetection);
    }

    // Inicializar sistema de missões
    InitializeSurvivalMissions();

    // Encontrar referência do jogador
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PlayerPawn);
        }
    }
}

void AQuest_PracticalSurvivalMissions::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Verificar progresso das missões periodicamente
    LastMissionCheck += DeltaTime;
    if (LastMissionCheck >= MissionCheckInterval)
    {
        LastMissionCheck = 0.0f;
        
        if (CurrentMission.bIsActive && !CurrentMission.bIsCompleted)
        {
            CheckMissionProgress();
            CheckPlayerProximityToTarget();
        }

        UpdateMissionDisplay();
    }
}

void AQuest_PracticalSurvivalMissions::InitializeSurvivalMissions()
{
    CreateBasicSurvivalMissions();

    // Iniciar primeira missão se configurado
    if (bAutoStartNextMission && AvailableMissions.Num() > 0)
    {
        FQuest_SurvivalMissionData FirstMission = GetNextPriorityMission();
        if (FirstMission.Priority > 0)
        {
            StartMission(FirstMission.MissionType);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest System: %d survival missions initialized"), AvailableMissions.Num());
}

void AQuest_PracticalSurvivalMissions::CreateBasicSurvivalMissions()
{
    AvailableMissions.Empty();

    // Missão 1: Encontrar água
    FQuest_SurvivalMissionData WaterMission;
    WaterMission.MissionType = EQuest_SurvivalMissionType::FindWater;
    WaterMission.MissionTitle = TEXT("Find Clean Water");
    WaterMission.MissionDescription = TEXT("Locate a safe water source to maintain hydration. Look for flowing streams or natural springs.");
    WaterMission.TargetLocation = FVector(-2000, -2000, 200);
    WaterMission.CompletionRadius = 300.0f;
    WaterMission.Priority = 10;
    AvailableMissions.Add(WaterMission);

    // Missão 2: Construir abrigo
    FQuest_SurvivalMissionData ShelterMission;
    ShelterMission.MissionType = EQuest_SurvivalMissionType::BuildShelter;
    ShelterMission.MissionTitle = TEXT("Build Basic Shelter");
    ShelterMission.MissionDescription = TEXT("Construct a simple shelter to protect yourself from the elements and predators.");
    ShelterMission.TargetLocation = FVector(1500, 1500, 250);
    ShelterMission.CompletionRadius = 400.0f;
    ShelterMission.Priority = 8;
    AvailableMissions.Add(ShelterMission);

    // Missão 3: Criar ferramenta
    FQuest_SurvivalMissionData ToolMission;
    ToolMission.MissionType = EQuest_SurvivalMissionType::CraftTool;
    ToolMission.MissionTitle = TEXT("Craft Stone Tool");
    ToolMission.MissionDescription = TEXT("Create a basic stone tool for hunting and gathering. Find suitable rocks and learn primitive crafting.");
    ToolMission.TargetLocation = FVector(3000, -1000, 300);
    ToolMission.CompletionRadius = 250.0f;
    ToolMission.Priority = 7;
    AvailableMissions.Add(ToolMission);

    // Missão 4: Explorar caverna
    FQuest_SurvivalMissionData CaveMission;
    CaveMission.MissionType = EQuest_SurvivalMissionType::ExploreCave;
    CaveMission.MissionTitle = TEXT("Explore Ancient Cave");
    CaveMission.MissionDescription = TEXT("Investigate the cave system for shelter opportunities and ancient markings left by previous inhabitants.");
    CaveMission.TargetLocation = FVector(5000, 3000, 300);
    CaveMission.CompletionRadius = 500.0f;
    CaveMission.Priority = 6;
    AvailableMissions.Add(CaveMission);

    // Missão 5: Evitar predador
    FQuest_SurvivalMissionData PredatorMission;
    PredatorMission.MissionType = EQuest_SurvivalMissionType::AvoidPredator;
    PredatorMission.MissionTitle = TEXT("Avoid Large Predator");
    PredatorMission.MissionDescription = TEXT("Navigate safely through predator territory without being detected. Use stealth and environmental cover.");
    PredatorMission.TargetLocation = FVector(1000, -5000, 400);
    PredatorMission.CompletionRadius = 800.0f;
    PredatorMission.Priority = 9;
    AvailableMissions.Add(PredatorMission);

    // Missão 6: Reconhecer área
    FQuest_SurvivalMissionData ScoutMission;
    ScoutMission.MissionType = EQuest_SurvivalMissionType::ScoutArea;
    ScoutMission.MissionTitle = TEXT("Scout Safe Territory");
    ScoutMission.MissionDescription = TEXT("Survey the surrounding area to identify safe zones, resources, and potential dangers.");
    ScoutMission.TargetLocation = FVector(-1000, 4000, 350);
    ScoutMission.CompletionRadius = 600.0f;
    ScoutMission.Priority = 5;
    AvailableMissions.Add(ScoutMission);
}

void AQuest_PracticalSurvivalMissions::StartMission(EQuest_SurvivalMissionType MissionType)
{
    // Encontrar missão do tipo especificado
    for (FQuest_SurvivalMissionData& Mission : AvailableMissions)
    {
        if (Mission.MissionType == MissionType && !Mission.bIsCompleted)
        {
            // Desativar missão atual se existir
            if (CurrentMission.bIsActive)
            {
                CurrentMission.bIsActive = false;
            }

            // Ativar nova missão
            Mission.bIsActive = true;
            CurrentMission = Mission;

            UE_LOG(LogTemp, Warning, TEXT("Quest Started: %s"), *Mission.MissionTitle);
            
            // Atualizar display
            UpdateMissionDisplay();
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest System: Mission type not found or already completed"));
}

void AQuest_PracticalSurvivalMissions::CompleteMission()
{
    if (CurrentMission.bIsActive && !CurrentMission.bIsCompleted)
    {
        CurrentMission.bIsCompleted = true;
        CurrentMission.bIsActive = false;

        // Atualizar missão na lista
        for (FQuest_SurvivalMissionData& Mission : AvailableMissions)
        {
            if (Mission.MissionType == CurrentMission.MissionType)
            {
                Mission.bIsCompleted = true;
                Mission.bIsActive = false;
                break;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Quest Completed: %s"), *CurrentMission.MissionTitle);

        // Iniciar próxima missão automaticamente se configurado
        if (bAutoStartNextMission)
        {
            FQuest_SurvivalMissionData NextMission = GetNextPriorityMission();
            if (NextMission.Priority > 0)
            {
                StartMission(NextMission.MissionType);
            }
        }
    }
}

bool AQuest_PracticalSurvivalMissions::CheckMissionProgress()
{
    if (!CurrentMission.bIsActive || CurrentMission.bIsCompleted || !PlayerCharacter)
    {
        return false;
    }

    float DistanceToTarget = GetDistanceToMissionTarget();
    
    // Verificar se jogador está próximo o suficiente do objetivo
    if (DistanceToTarget <= CurrentMission.CompletionRadius)
    {
        CompleteMission();
        return true;
    }

    return false;
}

FQuest_SurvivalMissionData AQuest_PracticalSurvivalMissions::GetNextPriorityMission()
{
    FQuest_SurvivalMissionData NextMission;
    int32 HighestPriority = 0;

    for (const FQuest_SurvivalMissionData& Mission : AvailableMissions)
    {
        if (!Mission.bIsCompleted && !Mission.bIsActive && Mission.Priority > HighestPriority)
        {
            NextMission = Mission;
            HighestPriority = Mission.Priority;
        }
    }

    return NextMission;
}

void AQuest_PracticalSurvivalMissions::UpdateMissionDisplay()
{
    if (!MissionText)
    {
        return;
    }

    FString DisplayText;
    
    if (CurrentMission.bIsActive)
    {
        DisplayText = FString::Printf(TEXT("ACTIVE MISSION:\n%s\n\nDistance: %.0fm"), 
            *CurrentMission.MissionTitle, 
            GetDistanceToMissionTarget());
    }
    else
    {
        DisplayText = TEXT("No Active Mission\nApproach to start survival training");
    }

    MissionText->SetText(FText::FromString(DisplayText));
}

TArray<FQuest_SurvivalMissionData> AQuest_PracticalSurvivalMissions::GetActiveMissions()
{
    TArray<FQuest_SurvivalMissionData> ActiveMissions;
    
    for (const FQuest_SurvivalMissionData& Mission : AvailableMissions)
    {
        if (Mission.bIsActive)
        {
            ActiveMissions.Add(Mission);
        }
    }

    return ActiveMissions;
}

float AQuest_PracticalSurvivalMissions::GetDistanceToMissionTarget()
{
    if (!PlayerCharacter)
    {
        return -1.0f;
    }

    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    return FVector::Dist(PlayerLocation, CurrentMission.TargetLocation);
}

void AQuest_PracticalSurvivalMissions::OnPlayerEnterDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor))
    {
        bPlayerInRange = true;
        PlayerCharacter = Player;
        
        UE_LOG(LogTemp, Warning, TEXT("Player entered quest area"));
        
        // Iniciar missão se não houver nenhuma ativa
        if (!CurrentMission.bIsActive && bAutoStartNextMission)
        {
            FQuest_SurvivalMissionData NextMission = GetNextPriorityMission();
            if (NextMission.Priority > 0)
            {
                StartMission(NextMission.MissionType);
            }
        }
    }
}

void AQuest_PracticalSurvivalMissions::OnPlayerExitDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Cast<ATranspersonalCharacter>(OtherActor))
    {
        bPlayerInRange = false;
        UE_LOG(LogTemp, Warning, TEXT("Player left quest area"));
    }
}

void AQuest_PracticalSurvivalMissions::CheckPlayerProximityToTarget()
{
    if (!PlayerCharacter || !CurrentMission.bIsActive)
    {
        return;
    }

    float Distance = GetDistanceToMissionTarget();
    
    // Feedback baseado na distância
    if (Distance <= CurrentMission.CompletionRadius * 2.0f)
    {
        // Jogador está próximo - dar feedback visual/sonoro
        if (MissionText)
        {
            MissionText->SetTextRenderColor(FColor::Green);
        }
    }
    else if (Distance <= CurrentMission.CompletionRadius * 5.0f)
    {
        // Jogador está na direção certa
        if (MissionText)
        {
            MissionText->SetTextRenderColor(FColor::Yellow);
        }
    }
    else
    {
        // Jogador está longe
        if (MissionText)
        {
            MissionText->SetTextRenderColor(FColor::Red);
        }
    }
}