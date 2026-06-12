#include "Quest_ObjectiveManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AQuest_ObjectiveManager::AQuest_ObjectiveManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(200.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create objective marker mesh
    ObjectiveMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveMarker"));
    ObjectiveMarker->SetupAttachment(RootComponent);
    ObjectiveMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize settings
    ObjectiveCheckRadius = 200.0f;
    bAutoActivateObjectives = true;

    // Bind overlap event
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ObjectiveManager::OnTriggerBeginOverlap);
}

void AQuest_ObjectiveManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultObjectives();
    UpdateObjectiveMarkers();
}

void AQuest_ObjectiveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check player location for location-based objectives
    if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        CheckLocationObjectives(PlayerCharacter->GetActorLocation());
    }
}

void AQuest_ObjectiveManager::AddObjective(const FQuest_ObjectiveData& NewObjective)
{
    ActiveObjectives.Add(NewObjective);
    UpdateObjectiveMarkers();
    
    if (bAutoActivateObjectives)
    {
        ActivateObjective(NewObjective.ObjectiveID);
    }
}

void AQuest_ObjectiveManager::ActivateObjective(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID))
    {
        Objective->Status = EQuest_ObjectiveStatus::Active;
        OnObjectiveActivated(*Objective);
        
        UE_LOG(LogTemp, Log, TEXT("Quest Objective Activated: %s"), *Objective->ObjectiveTitle);
    }
}

void AQuest_ObjectiveManager::CompleteObjective(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID))
    {
        Objective->Status = EQuest_ObjectiveStatus::Completed;
        CompletedObjectives.Add(*Objective);
        ActiveObjectives.RemoveAll([&ObjectiveID](const FQuest_ObjectiveData& Obj) {
            return Obj.ObjectiveID == ObjectiveID;
        });
        
        OnObjectiveCompleted(*Objective);
        UpdateObjectiveMarkers();
        
        UE_LOG(LogTemp, Log, TEXT("Quest Objective Completed: %s"), *Objective->ObjectiveTitle);
    }
}

void AQuest_ObjectiveManager::FailObjective(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID))
    {
        Objective->Status = EQuest_ObjectiveStatus::Failed;
        OnObjectiveFailed(*Objective);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest Objective Failed: %s"), *Objective->ObjectiveTitle);
    }
}

bool AQuest_ObjectiveManager::IsObjectiveActive(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && Objective.Status == EQuest_ObjectiveStatus::Active)
        {
            return true;
        }
    }
    return false;
}

bool AQuest_ObjectiveManager::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : CompletedObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return true;
        }
    }
    return false;
}

FQuest_ObjectiveData AQuest_ObjectiveManager::GetObjectiveData(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    
    for (const FQuest_ObjectiveData& Objective : CompletedObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    
    return FQuest_ObjectiveData();
}

TArray<FQuest_ObjectiveData> AQuest_ObjectiveManager::GetActiveObjectives() const
{
    TArray<FQuest_ObjectiveData> ActiveList;
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active)
        {
            ActiveList.Add(Objective);
        }
    }
    return ActiveList;
}

void AQuest_ObjectiveManager::UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount)
{
    if (FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID))
    {
        Objective->CurrentCount += ProgressAmount;
        
        if (Objective->CurrentCount >= Objective->RequiredCount)
        {
            CompleteObjective(ObjectiveID);
        }
        else
        {
            OnObjectiveProgressUpdated(*Objective);
        }
    }
}

void AQuest_ObjectiveManager::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.ObjectiveType == EQuest_ObjectiveType::Reach_Location)
        {
            float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
            if (Distance <= Objective.CompletionRadius)
            {
                CompleteObjective(Objective.ObjectiveID);
            }
        }
    }
}

void AQuest_ObjectiveManager::CheckHuntingObjectives(const FString& DinosaurType)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.ObjectiveType == EQuest_ObjectiveType::Hunt_Dinosaur)
        {
            if (Objective.RequiredItems.Contains(DinosaurType))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, 1);
            }
        }
    }
}

void AQuest_ObjectiveManager::CheckGatheringObjectives(const FString& ItemType, int32 Amount)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.ObjectiveType == EQuest_ObjectiveType::Gather_Resources)
        {
            if (Objective.RequiredItems.Contains(ItemType))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, Amount);
            }
        }
    }
}

void AQuest_ObjectiveManager::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                   bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        CheckLocationObjectives(PlayerCharacter->GetActorLocation());
    }
}

void AQuest_ObjectiveManager::InitializeDefaultObjectives()
{
    // Initialize hunting objectives
    FQuest_ObjectiveData HuntRaptorObjective;
    HuntRaptorObjective.ObjectiveID = "HUNT_RAPTOR_001";
    HuntRaptorObjective.ObjectiveTitle = "Hunt Velociraptors";
    HuntRaptorObjective.ObjectiveDescription = "Hunt 3 Velociraptors for meat and materials";
    HuntRaptorObjective.ObjectiveType = EQuest_ObjectiveType::Hunt_Dinosaur;
    HuntRaptorObjective.Status = EQuest_ObjectiveStatus::Inactive;
    HuntRaptorObjective.RequiredCount = 3;
    HuntRaptorObjective.CurrentCount = 0;
    HuntRaptorObjective.RequiredItems.Add("Velociraptor");
    ActiveObjectives.Add(HuntRaptorObjective);

    // Initialize exploration objectives
    FQuest_ObjectiveData ExploreForestObjective;
    ExploreForestObjective.ObjectiveID = "EXPLORE_FOREST_001";
    ExploreForestObjective.ObjectiveTitle = "Explore the Ancient Forest";
    ExploreForestObjective.ObjectiveDescription = "Discover the secrets of the ancient forest";
    ExploreForestObjective.ObjectiveType = EQuest_ObjectiveType::Explore_Location;
    ExploreForestObjective.Status = EQuest_ObjectiveStatus::Inactive;
    ExploreForestObjective.TargetLocation = FVector(1000, 1000, 100);
    ExploreForestObjective.CompletionRadius = 300.0f;
    ExploreForestObjective.RequiredCount = 1;
    ExploreForestObjective.CurrentCount = 0;
    ActiveObjectives.Add(ExploreForestObjective);

    // Initialize gathering objectives
    FQuest_ObjectiveData GatherStonesObjective;
    GatherStonesObjective.ObjectiveID = "GATHER_STONES_001";
    GatherStonesObjective.ObjectiveTitle = "Gather Stones";
    GatherStonesObjective.ObjectiveDescription = "Collect 10 stones for crafting tools";
    GatherStonesObjective.ObjectiveType = EQuest_ObjectiveType::Gather_Resources;
    GatherStonesObjective.Status = EQuest_ObjectiveStatus::Inactive;
    GatherStonesObjective.RequiredCount = 10;
    GatherStonesObjective.CurrentCount = 0;
    GatherStonesObjective.RequiredItems.Add("Stone");
    ActiveObjectives.Add(GatherStonesObjective);
}

void AQuest_ObjectiveManager::UpdateObjectiveMarkers()
{
    // Update visual markers based on active objectives
    if (ObjectiveMarker && ActiveObjectives.Num() > 0)
    {
        ObjectiveMarker->SetVisibility(true);
    }
    else if (ObjectiveMarker)
    {
        ObjectiveMarker->SetVisibility(false);
    }
}

FQuest_ObjectiveData* AQuest_ObjectiveManager::FindObjectiveByID(const FString& ObjectiveID)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return &Objective;
        }
    }
    return nullptr;
}