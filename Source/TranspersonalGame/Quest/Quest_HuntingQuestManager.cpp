#include "Quest_HuntingQuestManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AQuest_HuntingQuestManager::AQuest_HuntingQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create quest marker mesh component
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    RootComponent = QuestMarkerMesh;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize quest settings
    QuestUpdateInterval = 5.0f;
    bAutoGenerateQuests = true;
    MaxActiveQuests = 3;
    LastQuestUpdateTime = 0.0f;
    QuestIDCounter = 1;

    // Initialize with a default hunting quest
    FQuest_HuntingQuest DefaultQuest;
    DefaultQuest.QuestName = TEXT("First Hunt");
    DefaultQuest.QuestDescription = TEXT("Hunt a small dinosaur to prove your skills as a hunter");
    
    FQuest_HuntTarget RaptorTarget = CreateHuntTarget(EDinosaurSpecies::Raptor, 1, FVector::ZeroVector);
    DefaultQuest.HuntTargets.Add(RaptorTarget);
    DefaultQuest.QuestStatus = EQuestStatus::Available;
    
    ActiveHuntingQuests.Add(DefaultQuest);
}

void AQuest_HuntingQuestManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_HuntingQuestManager::OnInteractionSphereBeginOverlap);
    }

    // Generate initial quests if auto-generation is enabled
    if (bAutoGenerateQuests)
    {
        GenerateNewQuests();
    }

    UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: BeginPlay completed with %d active quests"), ActiveHuntingQuests.Num());
}

void AQuest_HuntingQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateActiveQuests(DeltaTime);

    // Update quests periodically
    if (GetWorld()->GetTimeSeconds() - LastQuestUpdateTime >= QuestUpdateInterval)
    {
        CheckQuestCompletion();
        
        if (bAutoGenerateQuests && ActiveHuntingQuests.Num() < MaxActiveQuests)
        {
            GenerateNewQuests();
        }
        
        LastQuestUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void AQuest_HuntingQuestManager::StartHuntingQuest(const FString& QuestName)
{
    for (FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
    {
        if (Quest.QuestName == QuestName && Quest.QuestStatus == EQuestStatus::Available)
        {
            Quest.QuestStatus = EQuestStatus::InProgress;
            Quest.ElapsedTime = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Started quest '%s'"), *QuestName);
            
            // Reset all hunt targets
            for (FQuest_HuntTarget& Target : Quest.HuntTargets)
            {
                Target.CurrentKills = 0;
            }
            
            break;
        }
    }
}

void AQuest_HuntingQuestManager::CompleteHuntingQuest(const FString& QuestName)
{
    for (int32 i = 0; i < ActiveHuntingQuests.Num(); i++)
    {
        if (ActiveHuntingQuests[i].QuestName == QuestName)
        {
            ActiveHuntingQuests[i].QuestStatus = EQuestStatus::Completed;
            CompletedHuntingQuests.Add(ActiveHuntingQuests[i]);
            ActiveHuntingQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Completed quest '%s'"), *QuestName);
            break;
        }
    }
}

void AQuest_HuntingQuestManager::RegisterDinosaurKill(EDinosaurSpecies Species, const FVector& KillLocation)
{
    bool bQuestUpdated = false;

    for (FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
    {
        if (Quest.QuestStatus == EQuestStatus::InProgress)
        {
            for (FQuest_HuntTarget& Target : Quest.HuntTargets)
            {
                if (Target.TargetSpecies == Species)
                {
                    // Check if kill is within hunting area (if specified)
                    if (Target.HuntingArea != FVector::ZeroVector)
                    {
                        float Distance = FVector::Dist(KillLocation, Target.HuntingArea);
                        if (Distance > Target.HuntingRadius)
                        {
                            continue; // Kill outside hunting area
                        }
                    }

                    Target.CurrentKills = FMath::Min(Target.CurrentKills + 1, Target.RequiredKills);
                    bQuestUpdated = true;
                    
                    UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Registered kill for species %d. Progress: %d/%d"), 
                           (int32)Species, Target.CurrentKills, Target.RequiredKills);
                }
            }
        }
    }

    if (bQuestUpdated)
    {
        CheckQuestCompletion();
    }
}

void AQuest_HuntingQuestManager::UpdateQuestProgress(const FString& QuestName)
{
    for (FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
    {
        if (Quest.QuestName == QuestName && Quest.QuestStatus == EQuestStatus::InProgress)
        {
            bool bAllTargetsComplete = true;
            
            for (const FQuest_HuntTarget& Target : Quest.HuntTargets)
            {
                if (Target.CurrentKills < Target.RequiredKills)
                {
                    bAllTargetsComplete = false;
                    break;
                }
            }
            
            if (bAllTargetsComplete)
            {
                CompleteHuntingQuest(QuestName);
            }
            
            break;
        }
    }
}

FQuest_HuntingQuest AQuest_HuntingQuestManager::CreateRandomHuntingQuest()
{
    FQuest_HuntingQuest NewQuest;
    
    // Generate random quest name and description
    TArray<FString> QuestNames = {
        TEXT("The Great Hunt"),
        TEXT("Predator's Challenge"),
        TEXT("Beast Slayer"),
        TEXT("Survival Hunt"),
        TEXT("Apex Predator")
    };
    
    TArray<FString> QuestDescriptions = {
        TEXT("Hunt dangerous predators threatening the area"),
        TEXT("Prove your hunting skills by taking down formidable prey"),
        TEXT("Eliminate territorial beasts to secure safe passage"),
        TEXT("Hunt for food and resources to survive another day"),
        TEXT("Face the apex predators of this prehistoric world")
    };
    
    int32 NameIndex = FMath::RandRange(0, QuestNames.Num() - 1);
    int32 DescIndex = FMath::RandRange(0, QuestDescriptions.Num() - 1);
    
    NewQuest.QuestName = FString::Printf(TEXT("%s %d"), *QuestNames[NameIndex], QuestIDCounter++);
    NewQuest.QuestDescription = QuestDescriptions[DescIndex];
    NewQuest.QuestStatus = EQuestStatus::Available;
    
    // Generate random hunt targets
    TArray<EDinosaurSpecies> PossibleSpecies = {
        EDinosaurSpecies::Raptor,
        EDinosaurSpecies::TRex,
        EDinosaurSpecies::Triceratops,
        EDinosaurSpecies::Brachiosaurus,
        EDinosaurSpecies::Stegosaurus
    };
    
    int32 NumTargets = FMath::RandRange(1, 3);
    for (int32 i = 0; i < NumTargets; i++)
    {
        EDinosaurSpecies RandomSpecies = PossibleSpecies[FMath::RandRange(0, PossibleSpecies.Num() - 1)];
        int32 KillCount = FMath::RandRange(1, 3);
        
        FQuest_HuntTarget Target = CreateHuntTarget(RandomSpecies, KillCount, FVector::ZeroVector);
        NewQuest.HuntTargets.Add(Target);
    }
    
    // Set random time limit
    NewQuest.TimeLimit = FMath::RandRange(300.0f, 900.0f); // 5-15 minutes
    
    return NewQuest;
}

TArray<FQuest_HuntingQuest> AQuest_HuntingQuestManager::GetActiveHuntingQuests() const
{
    return ActiveHuntingQuests;
}

bool AQuest_HuntingQuestManager::IsQuestActive(const FString& QuestName) const
{
    for (const FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            return Quest.QuestStatus == EQuestStatus::InProgress;
        }
    }
    return false;
}

void AQuest_HuntingQuestManager::GenerateHuntingQuestsForArea(const FVector& AreaCenter, float AreaRadius)
{
    if (ActiveHuntingQuests.Num() >= MaxActiveQuests)
    {
        return;
    }
    
    FQuest_HuntingQuest AreaQuest = CreateRandomHuntingQuest();
    AreaQuest.QuestName = FString::Printf(TEXT("Area Hunt %d"), QuestIDCounter++);
    AreaQuest.QuestDescription = TEXT("Hunt creatures in the specified area");
    
    // Set hunting area for all targets
    for (FQuest_HuntTarget& Target : AreaQuest.HuntTargets)
    {
        Target.HuntingArea = AreaCenter;
        Target.HuntingRadius = AreaRadius;
    }
    
    ActiveHuntingQuests.Add(AreaQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Generated area quest at %s with radius %f"), 
           *AreaCenter.ToString(), AreaRadius);
}

void AQuest_HuntingQuestManager::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Player entered interaction range"));
        
        // Show available quests to player
        for (const FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
        {
            if (Quest.QuestStatus == EQuestStatus::Available)
            {
                UE_LOG(LogTemp, Warning, TEXT("Available Quest: %s - %s"), *Quest.QuestName, *Quest.QuestDescription);
            }
        }
    }
}

void AQuest_HuntingQuestManager::UpdateActiveQuests(float DeltaTime)
{
    for (FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
    {
        if (Quest.QuestStatus == EQuestStatus::InProgress)
        {
            Quest.ElapsedTime += DeltaTime;
            
            // Check for time limit expiration
            if (Quest.TimeLimit > 0.0f && Quest.ElapsedTime >= Quest.TimeLimit)
            {
                Quest.QuestStatus = EQuestStatus::Failed;
                UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Quest '%s' failed due to time limit"), *Quest.QuestName);
            }
        }
    }
}

void AQuest_HuntingQuestManager::CheckQuestCompletion()
{
    TArray<FString> QuestsToComplete;
    
    for (const FQuest_HuntingQuest& Quest : ActiveHuntingQuests)
    {
        if (Quest.QuestStatus == EQuestStatus::InProgress)
        {
            bool bAllTargetsComplete = true;
            
            for (const FQuest_HuntTarget& Target : Quest.HuntTargets)
            {
                if (Target.CurrentKills < Target.RequiredKills)
                {
                    bAllTargetsComplete = false;
                    break;
                }
            }
            
            if (bAllTargetsComplete)
            {
                QuestsToComplete.Add(Quest.QuestName);
            }
        }
    }
    
    // Complete all finished quests
    for (const FString& QuestName : QuestsToComplete)
    {
        CompleteHuntingQuest(QuestName);
    }
}

void AQuest_HuntingQuestManager::GenerateNewQuests()
{
    while (ActiveHuntingQuests.Num() < MaxActiveQuests)
    {
        FQuest_HuntingQuest NewQuest = CreateRandomHuntingQuest();
        ActiveHuntingQuests.Add(NewQuest);
        
        UE_LOG(LogTemp, Warning, TEXT("HuntingQuestManager: Generated new quest '%s'"), *NewQuest.QuestName);
    }
}

FQuest_HuntTarget AQuest_HuntingQuestManager::CreateHuntTarget(EDinosaurSpecies Species, int32 KillCount, const FVector& Area)
{
    FQuest_HuntTarget Target;
    Target.TargetSpecies = Species;
    Target.RequiredKills = KillCount;
    Target.CurrentKills = 0;
    Target.HuntingArea = Area;
    Target.HuntingRadius = (Area == FVector::ZeroVector) ? 0.0f : 1000.0f;
    
    return Target;
}