#include "Quest_MigrationSystem.h"
#include "../Core/TranspersonalCharacter.h"
#include "../Crowd/Crowd_MigrationManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AQuest_MigrationSystem::AQuest_MigrationSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create quest marker mesh
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    QuestMarkerMesh->SetupAttachment(RootComponent);
    QuestMarkerMesh->SetRelativeScale3D(FVector(2.0f, 0.5f, 3.0f)); // Stone tablet shape

    // Initialize properties
    CurrentMigrationPhase = EQuest_MigrationPhase::PreMigration;
    MigrationPhaseTimer = 0.0f;
    PhaseDuration = 120.0f; // 2 minutes per phase
    ActiveQuestCount = 0;
    MaxConcurrentQuests = 3;

    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_MigrationSystem::OnPlayerEnterTrigger);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_MigrationSystem::OnPlayerExitTrigger);
}

void AQuest_MigrationSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find migration manager
    MigrationManager = GetWorld()->GetSubsystem<UCrowd_MigrationManager>();
    if (!MigrationManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MigrationSystem: Could not find MigrationManager"));
    }

    // Initialize quest system
    InitializeMigrationQuests();
    SpawnQuestNPCs();
}

void AQuest_MigrationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMigrationPhase(DeltaTime);
    UpdateQuestMarkers();

    // Check for nearby players
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATranspersonalCharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(Actor);
        if (Player)
        {
            CheckQuestTriggers(Player);
        }
    }
}

void AQuest_MigrationSystem::InitializeMigrationQuests()
{
    MigrationTriggers.Empty();

    // Create quest triggers at key migration points
    TArray<FVector> MigrationPoints = {
        FVector(2000, 0, 100),     // Herbivore Route Start
        FVector(4000, 2000, 150),  // Herbivore Grazing Area
        FVector(6000, 4000, 200),  // Herbivore Route End
        FVector(-2000, 1000, 120), // Carnivore Hunt Start
        FVector(-4000, 3000, 180), // Carnivore Ambush Point
        FVector(-6000, 5000, 250)  // Carnivore Territory End
    };

    TArray<EQuest_MigrationQuestType> QuestTypes = {
        EQuest_MigrationQuestType::FollowHerd,
        EQuest_MigrationQuestType::ProtectFromPredators,
        EQuest_MigrationQuestType::HuntDuringMigration,
        EQuest_MigrationQuestType::AmbushPredators,
        EQuest_MigrationQuestType::ScavengeAfterHunt,
        EQuest_MigrationQuestType::GuideHerdToSafety
    };

    for (int32 i = 0; i < MigrationPoints.Num(); i++)
    {
        FQuest_MigrationTrigger NewTrigger;
        NewTrigger.TriggerLocation = MigrationPoints[i];
        NewTrigger.TriggerRadius = 1500.0f;
        NewTrigger.RequiredPhase = EQuest_MigrationPhase::MigrationActive;
        NewTrigger.bIsActive = true;

        // Create quest data
        FQuest_MigrationQuestData QuestData;
        QuestData.QuestType = QuestTypes[i % QuestTypes.Num()];
        
        switch (QuestData.QuestType)
        {
        case EQuest_MigrationQuestType::FollowHerd:
            QuestData.QuestName = TEXT("Follow the Great Migration");
            QuestData.QuestDescription = TEXT("Track the herbivore herds as they move to new grazing grounds. Learn their patterns and find safe passage.");
            QuestData.RequiredHerdSize = 8;
            QuestData.RewardExperience = 150;
            QuestData.RewardItems = {TEXT("Migration Map"), TEXT("Tracking Knowledge")};
            break;
            
        case EQuest_MigrationQuestType::ProtectFromPredators:
            QuestData.QuestName = TEXT("Guardian of the Herd");
            QuestData.QuestDescription = TEXT("Protect the migrating herbivores from predator attacks. Drive off the hunters that stalk the edges.");
            QuestData.RequiredHerdSize = 5;
            QuestData.RewardExperience = 200;
            QuestData.RewardItems = {TEXT("Protector's Spear"), TEXT("Herd Trust")};
            break;
            
        case EQuest_MigrationQuestType::HuntDuringMigration:
            QuestData.QuestName = TEXT("Hunter's Opportunity");
            QuestData.QuestDescription = TEXT("Use the chaos of migration to hunt. Target the weak and separated, but beware of protective herds.");
            QuestData.RequiredHerdSize = 3;
            QuestData.RewardExperience = 180;
            QuestData.RewardItems = {TEXT("Fresh Meat"), TEXT("Hunting Experience")};
            break;
            
        case EQuest_MigrationQuestType::AmbushPredators:
            QuestData.QuestName = TEXT("Turn the Tables");
            QuestData.QuestDescription = TEXT("While predators focus on the herds, ambush them from behind. Strike the hunters while they hunt.");
            QuestData.RequiredHerdSize = 2;
            QuestData.RewardExperience = 220;
            QuestData.RewardItems = {TEXT("Predator Fang"), TEXT("Ambush Tactics")};
            break;
            
        case EQuest_MigrationQuestType::ScavengeAfterHunt:
            QuestData.QuestName = TEXT("Scavenger's Feast");
            QuestData.QuestDescription = TEXT("Follow in the wake of the great hunt. Claim what the predators leave behind and gather valuable resources.");
            QuestData.RequiredHerdSize = 1;
            QuestData.RewardExperience = 120;
            QuestData.RewardItems = {TEXT("Bone Tools"), TEXT("Scavenged Materials")};
            break;
            
        case EQuest_MigrationQuestType::GuideHerdToSafety:
            QuestData.QuestName = TEXT("Shepherd of Giants");
            QuestData.QuestDescription = TEXT("Guide lost or separated animals back to their herds. Navigate them through dangerous territory to safety.");
            QuestData.RequiredHerdSize = 4;
            QuestData.RewardExperience = 170;
            QuestData.RewardItems = {TEXT("Animal Bond"), TEXT("Navigation Skills")};
            break;
        }

        NewTrigger.QuestData = QuestData;
        MigrationTriggers.Add(NewTrigger);
    }

    UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Initialized %d migration quest triggers"), MigrationTriggers.Num());
}

void AQuest_MigrationSystem::UpdateMigrationPhase(float DeltaTime)
{
    MigrationPhaseTimer += DeltaTime;

    if (MigrationPhaseTimer >= PhaseDuration)
    {
        MigrationPhaseTimer = 0.0f;
        
        // Advance to next phase
        int32 CurrentPhaseInt = static_cast<int32>(CurrentMigrationPhase);
        CurrentPhaseInt = (CurrentPhaseInt + 1) % static_cast<int32>(EQuest_MigrationPhase::Dormant);
        CurrentMigrationPhase = static_cast<EQuest_MigrationPhase>(CurrentPhaseInt);
        
        ProcessMigrationPhaseChange();
        
        UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Migration phase changed to %d"), CurrentPhaseInt);
    }
}

void AQuest_MigrationSystem::CheckQuestTriggers(ATranspersonalCharacter* Player)
{
    if (!Player || ActiveQuestCount >= MaxConcurrentQuests)
    {
        return;
    }

    FVector PlayerLocation = Player->GetActorLocation();

    for (FQuest_MigrationTrigger& Trigger : MigrationTriggers)
    {
        if (!Trigger.bIsActive || Trigger.RequiredPhase != CurrentMigrationPhase)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Trigger.TriggerLocation);
        if (Distance <= Trigger.TriggerRadius)
        {
            // Check if player already has this quest
            if (!ActiveQuests.Contains(Trigger.QuestData.QuestName))
            {
                if (StartMigrationQuest(Trigger.QuestData, Player))
                {
                    Trigger.bIsActive = false; // Prevent duplicate activation
                }
            }
        }
    }
}

bool AQuest_MigrationSystem::StartMigrationQuest(const FQuest_MigrationQuestData& QuestData, ATranspersonalCharacter* Player)
{
    if (ActiveQuestCount >= MaxConcurrentQuests)
    {
        return false;
    }

    // Add to active quests
    ActiveQuests.Add(QuestData.QuestName, QuestData);
    ActiveQuestCount++;

    // Log quest start
    UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Started quest '%s' for player"), *QuestData.QuestName);

    // Display quest notification (in a real implementation, this would trigger UI)
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("New Quest: %s - %s"), *QuestData.QuestName, *QuestData.QuestDescription);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }

    return true;
}

void AQuest_MigrationSystem::CompleteMigrationQuest(const FString& QuestName, ATranspersonalCharacter* Player)
{
    if (!ActiveQuests.Contains(QuestName))
    {
        return;
    }

    FQuest_MigrationQuestData CompletedQuest = ActiveQuests[QuestName];
    ActiveQuests.Remove(QuestName);
    ActiveQuestCount--;

    // Award rewards
    UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Completed quest '%s' - awarded %d XP"), *QuestName, CompletedQuest.RewardExperience);

    // Display completion notification
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Quest Complete: %s (+%d XP)"), *QuestName, CompletedQuest.RewardExperience);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
    }
}

TArray<FQuest_MigrationQuestData> AQuest_MigrationSystem::GetAvailableQuests() const
{
    TArray<FQuest_MigrationQuestData> AvailableQuests;

    for (const FQuest_MigrationTrigger& Trigger : MigrationTriggers)
    {
        if (Trigger.bIsActive && Trigger.RequiredPhase == CurrentMigrationPhase)
        {
            AvailableQuests.Add(Trigger.QuestData);
        }
    }

    return AvailableQuests;
}

void AQuest_MigrationSystem::OnMigrationEvent(const FString& EventType, const FVector& Location)
{
    UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Migration event '%s' at location %s"), *EventType, *Location.ToString());

    // Generate contextual quest based on migration event
    if (ActiveQuestCount < MaxConcurrentQuests)
    {
        FQuest_MigrationQuestData ContextualQuest = GenerateContextualQuest(Location, CurrentMigrationPhase);
        
        // Find nearby players and offer quest
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATranspersonalCharacter::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(Actor);
            if (Player)
            {
                float Distance = FVector::Dist(Player->GetActorLocation(), Location);
                if (Distance <= 3000.0f) // Within quest offer range
                {
                    StartMigrationQuest(ContextualQuest, Player);
                    break; // Only offer to first nearby player
                }
            }
        }
    }
}

void AQuest_MigrationSystem::SpawnQuestNPCs()
{
    // Clear existing NPCs
    for (AActor* NPC : QuestNPCs)
    {
        if (IsValid(NPC))
        {
            NPC->Destroy();
        }
    }
    QuestNPCs.Empty();

    // Spawn quest giver NPCs at key locations
    TArray<FVector> NPCLocations = {
        FVector(0, 0, 200),        // Central quest hub
        FVector(3000, 1000, 150),  // Migration route observer
        FVector(-3000, 2000, 180)  // Predator territory scout
    };

    for (const FVector& Location : NPCLocations)
    {
        // In a full implementation, this would spawn actual NPC actors
        // For now, we create placeholder static mesh actors
        UClass* StaticMeshClass = UGameplayStatics::GetActorOfClass(GetWorld(), AStaticMeshActor::StaticClass())->GetClass();
        if (StaticMeshClass)
        {
            AActor* QuestNPC = GetWorld()->SpawnActor<AActor>(StaticMeshClass, Location, FRotator::ZeroRotator);
            if (QuestNPC)
            {
                QuestNPC->SetActorLabel(TEXT("QuestGiver_MigrationElder"));
                QuestNPCs.Add(QuestNPC);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Spawned %d quest NPCs"), QuestNPCs.Num());
}

void AQuest_MigrationSystem::UpdateQuestMarkers()
{
    // Update quest marker visibility based on active quests and migration phase
    bool bShouldShowMarker = (CurrentMigrationPhase == EQuest_MigrationPhase::MigrationActive || 
                             CurrentMigrationPhase == EQuest_MigrationPhase::MigrationPeak) &&
                             ActiveQuestCount < MaxConcurrentQuests;

    if (QuestMarkerMesh)
    {
        QuestMarkerMesh->SetVisibility(bShouldShowMarker);
    }
}

void AQuest_MigrationSystem::OnPlayerEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor);
    if (Player)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Player entered quest trigger area"));
        CheckQuestTriggers(Player);
    }
}

void AQuest_MigrationSystem::OnPlayerExitTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor);
    if (Player)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_MigrationSystem: Player exited quest trigger area"));
    }
}

void AQuest_MigrationSystem::ProcessMigrationPhaseChange()
{
    // Reactivate appropriate quest triggers for new phase
    for (FQuest_MigrationTrigger& Trigger : MigrationTriggers)
    {
        if (Trigger.RequiredPhase == CurrentMigrationPhase)
        {
            Trigger.bIsActive = true;
        }
    }

    // Notify migration manager of phase change
    if (MigrationManager)
    {
        OnMigrationEvent(TEXT("PhaseChange"), GetActorLocation());
    }
}

FQuest_MigrationQuestData AQuest_MigrationSystem::GenerateContextualQuest(const FVector& Location, EQuest_MigrationPhase Phase)
{
    FQuest_MigrationQuestData ContextualQuest;
    
    // Generate quest based on current phase and location
    switch (Phase)
    {
    case EQuest_MigrationPhase::PreMigration:
        ContextualQuest.QuestName = TEXT("Prepare for Migration");
        ContextualQuest.QuestDescription = TEXT("Gather supplies and scout routes before the great migration begins.");
        ContextualQuest.QuestType = EQuest_MigrationQuestType::FollowHerd;
        break;
        
    case EQuest_MigrationPhase::MigrationActive:
        ContextualQuest.QuestName = TEXT("Migration Crisis");
        ContextualQuest.QuestDescription = TEXT("A herd has been separated by predators. Help reunite them or exploit the chaos.");
        ContextualQuest.QuestType = EQuest_MigrationQuestType::ProtectFromPredators;
        break;
        
    case EQuest_MigrationPhase::MigrationPeak:
        ContextualQuest.QuestName = TEXT("Peak Migration Hunt");
        ContextualQuest.QuestDescription = TEXT("The migration is at its peak. Maximum opportunity for hunting and gathering.");
        ContextualQuest.QuestType = EQuest_MigrationQuestType::HuntDuringMigration;
        break;
        
    case EQuest_MigrationPhase::PostMigration:
        ContextualQuest.QuestName = TEXT("After the Storm");
        ContextualQuest.QuestDescription = TEXT("The migration has passed. Scavenge what remains and help stragglers.");
        ContextualQuest.QuestType = EQuest_MigrationQuestType::ScavengeAfterHunt;
        break;
        
    default:
        ContextualQuest.QuestName = TEXT("Dormant Period");
        ContextualQuest.QuestDescription = TEXT("Prepare for the next migration cycle.");
        ContextualQuest.QuestType = EQuest_MigrationQuestType::FollowHerd;
        break;
    }
    
    ContextualQuest.RequiredHerdSize = 3;
    ContextualQuest.RewardExperience = 100;
    ContextualQuest.QuestRadius = 2000.0f;
    
    return ContextualQuest;
}