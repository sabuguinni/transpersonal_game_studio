#include "Quest_NarrativeController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

AQuest_NarrativeController::AQuest_NarrativeController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveQuests = 5;
    QuestGenerationInterval = 120.0f; // 2 minutes
    bAutoGenerateQuests = true;
    PlayerProximityThreshold = 500.0f;
}

void AQuest_NarrativeController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeNarrativeSystem();
    
    if (bAutoGenerateQuests)
    {
        GetWorldTimerManager().SetTimer(
            QuestGenerationTimer,
            this,
            &AQuest_NarrativeController::GenerateRandomQuest,
            QuestGenerationInterval,
            true
        );
    }
}

void AQuest_NarrativeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CheckPlayerProximityToSurvivors();
}

void AQuest_NarrativeController::InitializeNarrativeSystem()
{
    // Criar perfis de sobreviventes predefinidos
    FQuest_SurvivorProfile Elena;
    Elena.SurvivorName = TEXT("Elena");
    Elena.BackgroundStory = TEXT("Former park ranger with knowledge of local wildlife");
    Elena.TrustLevel = 0.6f;
    Elena.FearLevel = 0.8f;
    Elena.KnownInformation.Add(TEXT("Safe water sources"));
    Elena.KnownInformation.Add(TEXT("Raptor hunting patterns"));
    Elena.LastKnownLocation = FVector(500, 500, 100);
    SurvivorProfiles.Add(Elena);
    
    FQuest_SurvivorProfile Marcus;
    Marcus.SurvivorName = TEXT("Marcus");
    Marcus.BackgroundStory = TEXT("Military survival expert with tactical knowledge");
    Marcus.TrustLevel = 0.7f;
    Marcus.FearLevel = 0.5f;
    Marcus.KnownInformation.Add(TEXT("T-Rex movement patterns"));
    Marcus.KnownInformation.Add(TEXT("Defensive positions"));
    Marcus.LastKnownLocation = FVector(-300, 800, 120);
    SurvivorProfiles.Add(Marcus);
    
    FQuest_SurvivorProfile Sarah;
    Sarah.SurvivorName = TEXT("Sarah");
    Sarah.BackgroundStory = TEXT("Botanist with knowledge of edible plants and medicine");
    Sarah.TrustLevel = 0.5f;
    Sarah.FearLevel = 0.9f;
    Sarah.KnownInformation.Add(TEXT("Medicinal plants"));
    Sarah.KnownInformation.Add(TEXT("Poisonous vegetation"));
    Sarah.LastKnownLocation = FVector(1000, -200, 80);
    SurvivorProfiles.Add(Sarah);
    
    // Gerar quests iniciais
    GenerateRandomQuest();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Narrative System initialized with %d survivors"), SurvivorProfiles.Num());
}

FQuest_NarrativeData AQuest_NarrativeController::CreateSurvivalQuest(EQuest_NarrativeType QuestType, const FVector& Location)
{
    FQuest_NarrativeData NewQuest;
    NewQuest.TargetLocation = Location;
    NewQuest.NarrativeType = QuestType;
    
    switch (QuestType)
    {
        case EQuest_NarrativeType::RescueMission:
            return CreateRescueQuest(Location);
            
        case EQuest_NarrativeType::ExplorationQuest:
            return CreateExplorationQuest(Location);
            
        case EQuest_NarrativeType::SurvivalTask:
            return CreateResourceQuest(Location);
            
        default:
            NewQuest.QuestTitle = TEXT("Unknown Quest");
            NewQuest.QuestDescription = TEXT("A mysterious task awaits");
            break;
    }
    
    return NewQuest;
}

FQuest_NarrativeData AQuest_NarrativeController::CreateRescueQuest(const FVector& Location)
{
    FQuest_NarrativeData RescueQuest;
    RescueQuest.QuestTitle = TEXT("Survivor in Distress");
    RescueQuest.QuestDescription = TEXT("A fellow survivor is trapped and needs immediate help. Time is running out.");
    RescueQuest.NarrativeType = EQuest_NarrativeType::RescueMission;
    RescueQuest.EmotionalTone = EQuest_EmotionalTone::Desperate;
    RescueQuest.UrgencyLevel = 0.9f;
    RescueQuest.TargetLocation = Location;
    RescueQuest.RewardValue = 100.0f;
    RescueQuest.DialogueText = TEXT("Help! I'm trapped near the water source. There are raptors circling the area.");
    
    return RescueQuest;
}

FQuest_NarrativeData AQuest_NarrativeController::CreateExplorationQuest(const FVector& Location)
{
    FQuest_NarrativeData ExplorationQuest;
    ExplorationQuest.QuestTitle = TEXT("Uncharted Territory");
    ExplorationQuest.QuestDescription = TEXT("Explore the unknown area and report back with valuable information.");
    ExplorationQuest.NarrativeType = EQuest_NarrativeType::ExplorationQuest;
    ExplorationQuest.EmotionalTone = EQuest_EmotionalTone::Cautious;
    ExplorationQuest.UrgencyLevel = 0.4f;
    ExplorationQuest.TargetLocation = Location;
    ExplorationQuest.RewardValue = 75.0f;
    ExplorationQuest.DialogueText = TEXT("I've seen movement in that direction. Could be resources... or danger.");
    
    return ExplorationQuest;
}

FQuest_NarrativeData AQuest_NarrativeController::CreateResourceQuest(const FVector& Location)
{
    FQuest_NarrativeData ResourceQuest;
    ResourceQuest.QuestTitle = TEXT("Essential Supplies");
    ResourceQuest.QuestDescription = TEXT("Gather critical resources needed for survival.");
    ResourceQuest.NarrativeType = EQuest_NarrativeType::SurvivalTask;
    ResourceQuest.EmotionalTone = EQuest_EmotionalTone::Determined;
    ResourceQuest.UrgencyLevel = 0.6f;
    ResourceQuest.TargetLocation = Location;
    ResourceQuest.RewardValue = 50.0f;
    ResourceQuest.RequiredItems.Add(TEXT("Clean Water"));
    ResourceQuest.RequiredItems.Add(TEXT("Medical Supplies"));
    ResourceQuest.DialogueText = TEXT("We're running low on supplies. Can you help us gather what we need?");
    
    return ResourceQuest;
}

void AQuest_NarrativeController::AddSurvivorProfile(const FQuest_SurvivorProfile& NewSurvivor)
{
    SurvivorProfiles.Add(NewSurvivor);
    UE_LOG(LogTemp, Log, TEXT("Added new survivor profile: %s"), *NewSurvivor.SurvivorName);
}

bool AQuest_NarrativeController::StartQuest(const FQuest_NarrativeData& QuestData)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start quest: Maximum active quests reached"));
        return false;
    }
    
    ActiveQuests.Add(QuestData);
    UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *QuestData.QuestTitle);
    return true;
}

void AQuest_NarrativeController::CompleteQuest(const FString& QuestTitle)
{
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        if (ActiveQuests[i].QuestTitle == QuestTitle)
        {
            UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *QuestTitle);
            ActiveQuests.RemoveAt(i);
            break;
        }
    }
}

TArray<FQuest_NarrativeData> AQuest_NarrativeController::GetAvailableQuests() const
{
    return ActiveQuests;
}

FQuest_SurvivorProfile AQuest_NarrativeController::GetSurvivorByName(const FString& SurvivorName) const
{
    for (const FQuest_SurvivorProfile& Profile : SurvivorProfiles)
    {
        if (Profile.SurvivorName == SurvivorName)
        {
            return Profile;
        }
    }
    
    return FQuest_SurvivorProfile(); // Return default if not found
}

void AQuest_NarrativeController::UpdateSurvivorTrust(const FString& SurvivorName, float TrustChange)
{
    for (FQuest_SurvivorProfile& Profile : SurvivorProfiles)
    {
        if (Profile.SurvivorName == SurvivorName)
        {
            Profile.TrustLevel = FMath::Clamp(Profile.TrustLevel + TrustChange, 0.0f, 1.0f);
            UE_LOG(LogTemp, Log, TEXT("Updated trust for %s: %f"), *SurvivorName, Profile.TrustLevel);
            break;
        }
    }
}

void AQuest_NarrativeController::GenerateEmergencyQuest(const FVector& DangerLocation)
{
    FQuest_NarrativeData EmergencyQuest;
    EmergencyQuest.QuestTitle = TEXT("Emergency Situation");
    EmergencyQuest.QuestDescription = TEXT("Immediate danger detected! Urgent action required.");
    EmergencyQuest.NarrativeType = EQuest_NarrativeType::RescueMission;
    EmergencyQuest.EmotionalTone = EQuest_EmotionalTone::Desperate;
    EmergencyQuest.UrgencyLevel = 1.0f;
    EmergencyQuest.TargetLocation = DangerLocation;
    EmergencyQuest.RewardValue = 150.0f;
    EmergencyQuest.DialogueText = TEXT("HELP! Something terrible is happening! We need help NOW!");
    
    StartQuest(EmergencyQuest);
}

FString AQuest_NarrativeController::GetContextualDialogue(const FString& SurvivorName, EQuest_EmotionalTone Tone) const
{
    FQuest_SurvivorProfile Survivor = GetSurvivorByName(SurvivorName);
    
    if (Survivor.SurvivorName.IsEmpty())
    {
        return TEXT("Unknown survivor");
    }
    
    switch (Tone)
    {
        case EQuest_EmotionalTone::Desperate:
            return FString::Printf(TEXT("Please, you have to help! I don't know how much longer I can hold on."));
            
        case EQuest_EmotionalTone::Hopeful:
            return FString::Printf(TEXT("I believe we can make it through this if we work together."));
            
        case EQuest_EmotionalTone::Fearful:
            return FString::Printf(TEXT("I'm scared... but I trust you to help us survive."));
            
        case EQuest_EmotionalTone::Determined:
            return FString::Printf(TEXT("We can't give up now. Let's do what needs to be done."));
            
        case EQuest_EmotionalTone::Cautious:
            return FString::Printf(TEXT("Be careful out there. This place is more dangerous than it looks."));
            
        default:
            return TEXT("Stay safe out there.");
    }
}

bool AQuest_NarrativeController::IsQuestAvailable(const FString& QuestTitle) const
{
    for (const FQuest_NarrativeData& Quest : ActiveQuests)
    {
        if (Quest.QuestTitle == QuestTitle)
        {
            return true;
        }
    }
    return false;
}

void AQuest_NarrativeController::UpdateQuestProgress(const FString& QuestTitle, float ProgressAmount)
{
    // Implementation for quest progress tracking
    UE_LOG(LogTemp, Log, TEXT("Updated progress for quest %s: %f"), *QuestTitle, ProgressAmount);
}

void AQuest_NarrativeController::GenerateRandomQuest()
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        return;
    }
    
    // Gerar localização aleatória
    FVector RandomLocation = FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(50.0f, 200.0f)
    );
    
    // Escolher tipo de quest aleatório
    TArray<EQuest_NarrativeType> QuestTypes = {
        EQuest_NarrativeType::RescueMission,
        EQuest_NarrativeType::ExplorationQuest,
        EQuest_NarrativeType::SurvivalTask
    };
    
    EQuest_NarrativeType RandomType = QuestTypes[FMath::RandRange(0, QuestTypes.Num() - 1)];
    
    FQuest_NarrativeData NewQuest = CreateSurvivalQuest(RandomType, RandomLocation);
    StartQuest(NewQuest);
}

void AQuest_NarrativeController::CheckPlayerProximityToSurvivors()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (const FQuest_SurvivorProfile& Survivor : SurvivorProfiles)
    {
        float Distance = FVector::Dist(PlayerLocation, Survivor.LastKnownLocation);
        
        if (Distance <= PlayerProximityThreshold)
        {
            // Trigger proximity-based quest generation or dialogue
            UE_LOG(LogTemp, Log, TEXT("Player near survivor %s"), *Survivor.SurvivorName);
        }
    }
}