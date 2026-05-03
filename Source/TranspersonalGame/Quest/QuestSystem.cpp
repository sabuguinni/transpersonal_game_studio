#include "QuestSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// UQuestSystem Implementation
UQuestSystem::UQuestSystem()
{
    bIsInitialized = false;
    LastUpdateTime = 0.0f;
}

void UQuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Initializing Quest System"));
    
    bIsInitialized = true;
    LastUpdateTime = 0.0f;
    
    // Initialize default quests after a short delay to ensure world is ready
    FTimerHandle InitTimer;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(InitTimer, this, &UQuestSystem::InitializeDefaultQuests, 2.0f, false);
    }
}

void UQuestSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Deinitializing Quest System"));
    
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UQuestSystem::StartQuest(const FString& QuestID)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestSystem: Cannot start quest - system not initialized"));
        return;
    }

    if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        if (QuestData->Status == EQuest_Status::NotStarted)
        {
            QuestData->Status = EQuest_Status::Active;
            QuestData->StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            
            // Activate all objectives
            for (FQuest_Objective& Objective : QuestData->Objectives)
            {
                if (Objective.Status == EQuest_Status::NotStarted)
                {
                    Objective.Status = EQuest_Status::Active;
                }
            }
            
            ActiveQuestIDs.AddUnique(QuestID);
            
            UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Started quest '%s'"), *QuestData->Title);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QuestSystem: Quest '%s' not found"), *QuestID);
    }
}

void UQuestSystem::CompleteQuest(const FString& QuestID)
{
    if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        QuestData->Status = EQuest_Status::Completed;
        ActiveQuestIDs.Remove(QuestID);
        
        UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Completed quest '%s'"), *QuestData->Title);
    }
}

void UQuestSystem::FailQuest(const FString& QuestID)
{
    if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        QuestData->Status = EQuest_Status::Failed;
        ActiveQuestIDs.Remove(QuestID);
        
        UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Failed quest '%s'"), *QuestData->Title);
    }
}

void UQuestSystem::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        for (FQuest_Objective& Objective : QuestData->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.CurrentProgress = FMath::Min(Progress, Objective.RequiredCount);
                
                if (Objective.CurrentProgress >= Objective.RequiredCount && Objective.Status == EQuest_Status::Active)
                {
                    OnObjectiveCompleted(QuestID, ObjectiveID);
                }
                break;
            }
        }
    }
}

bool UQuestSystem::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

TArray<FQuest_Data> UQuestSystem::GetActiveQuests() const
{
    TArray<FQuest_Data> ActiveQuests;
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (const FQuest_Data* QuestData = AllQuests.Find(QuestID))
        {
            ActiveQuests.Add(*QuestData);
        }
    }
    
    return ActiveQuests;
}

FQuest_Data UQuestSystem::GetQuestData(const FString& QuestID) const
{
    if (const FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        return *QuestData;
    }
    
    return FQuest_Data();
}

void UQuestSystem::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
        {
            for (FQuest_Objective& Objective : QuestData->Objectives)
            {
                if (Objective.Status == EQuest_Status::Active)
                {
                    if (CheckObjectiveCompletion(Objective, PlayerLocation))
                    {
                        OnObjectiveCompleted(QuestID, Objective.ObjectiveID);
                    }
                }
            }
        }
    }
}

void UQuestSystem::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
        {
            for (FQuest_Objective& Objective : QuestData->Objectives)
            {
                if (Objective.Status == EQuest_Status::Active && 
                    Objective.Type == EQuest_ObjectiveType::Gather_Resources &&
                    Objective.TargetActorClass == ResourceType)
                {
                    Objective.CurrentProgress += Amount;
                    if (Objective.CurrentProgress >= Objective.RequiredCount)
                    {
                        OnObjectiveCompleted(QuestID, Objective.ObjectiveID);
                    }
                }
            }
        }
    }
}

void UQuestSystem::OnDinosaurKilled(const FString& DinosaurType)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
        {
            for (FQuest_Objective& Objective : QuestData->Objectives)
            {
                if (Objective.Status == EQuest_Status::Active && 
                    Objective.Type == EQuest_ObjectiveType::Hunt_Dinosaur &&
                    (Objective.TargetActorClass.IsEmpty() || Objective.TargetActorClass == DinosaurType))
                {
                    Objective.CurrentProgress++;
                    if (Objective.CurrentProgress >= Objective.RequiredCount)
                    {
                        OnObjectiveCompleted(QuestID, Objective.ObjectiveID);
                    }
                }
            }
        }
    }
}

void UQuestSystem::OnItemCrafted(const FString& ItemType)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
        {
            for (FQuest_Objective& Objective : QuestData->Objectives)
            {
                if (Objective.Status == EQuest_Status::Active && 
                    Objective.Type == EQuest_ObjectiveType::Craft_Item &&
                    Objective.TargetActorClass == ItemType)
                {
                    Objective.CurrentProgress++;
                    if (Objective.CurrentProgress >= Objective.RequiredCount)
                    {
                        OnObjectiveCompleted(QuestID, Objective.ObjectiveID);
                    }
                }
            }
        }
    }
}

void UQuestSystem::InitializeDefaultQuests()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Creating default survival quests"));
    
    CreateExplorationQuests();
    CreateSurvivalQuests();
    CreateHuntingQuests();
    CreateCraftingQuests();
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Created %d default quests"), AllQuests.Num());
}

void UQuestSystem::CreateExplorationQuests()
{
    // Main exploration quest - discover all biomes
    FQuest_Data ExploreAllBiomes;
    ExploreAllBiomes.QuestID = "EXPLORE_ALL_BIOMES";
    ExploreAllBiomes.Title = "Explorer of the Prehistoric World";
    ExploreAllBiomes.Description = "Discover all five biomes of this prehistoric world to understand the diverse ecosystems.";
    ExploreAllBiomes.Status = EQuest_Status::NotStarted;
    ExploreAllBiomes.bIsMainQuest = true;
    
    // Biome exploration objectives
    TArray<FString> BiomeNames = {"Swamp", "Forest", "Savanna", "Desert", "Snow Mountain"};
    TArray<FVector> BiomeCenters = {
        FVector(-50000, -45000, 0),    // Swamp
        FVector(-45000, 40000, 0),     // Forest
        FVector(0, 0, 0),              // Savanna
        FVector(55000, 0, 0),          // Desert
        FVector(40000, 50000, 500)     // Snow Mountain
    };
    
    for (int32 i = 0; i < BiomeNames.Num(); i++)
    {
        FQuest_Objective BiomeObjective;
        BiomeObjective.ObjectiveID = FString::Printf(TEXT("EXPLORE_%s"), *BiomeNames[i].ToUpper());
        BiomeObjective.Description = FString::Printf(TEXT("Explore the %s biome"), *BiomeNames[i]);
        BiomeObjective.Type = EQuest_ObjectiveType::Explore_Biome;
        BiomeObjective.Status = EQuest_Status::NotStarted;
        BiomeObjective.TargetLocation = BiomeCenters[i];
        BiomeObjective.RequiredDistance = 5000.0f;
        BiomeObjective.RequiredCount = 1;
        BiomeObjective.CurrentProgress = 0;
        
        ExploreAllBiomes.Objectives.Add(BiomeObjective);
    }
    
    AllQuests.Add(ExploreAllBiomes.QuestID, ExploreAllBiomes);
}

void UQuestSystem::CreateSurvivalQuests()
{
    // Basic survival quest
    FQuest_Data SurvivalBasics;
    SurvivalBasics.QuestID = "SURVIVAL_BASICS";
    SurvivalBasics.Title = "Survival Basics";
    SurvivalBasics.Description = "Learn the fundamentals of survival in this dangerous prehistoric world.";
    SurvivalBasics.Status = EQuest_Status::NotStarted;
    SurvivalBasics.bIsMainQuest = false;
    
    // Survive for 10 minutes objective
    FQuest_Objective SurviveTime;
    SurviveTime.ObjectiveID = "SURVIVE_10_MINUTES";
    SurviveTime.Description = "Survive for 10 minutes without dying";
    SurviveTime.Type = EQuest_ObjectiveType::Survive_Duration;
    SurviveTime.Status = EQuest_Status::NotStarted;
    SurviveTime.RequiredCount = 600; // 10 minutes in seconds
    SurviveTime.CurrentProgress = 0;
    
    SurvivalBasics.Objectives.Add(SurviveTime);
    AllQuests.Add(SurvivalBasics.QuestID, SurvivalBasics);
}

void UQuestSystem::CreateHuntingQuests()
{
    // First hunt quest
    FQuest_Data FirstHunt;
    FirstHunt.QuestID = "FIRST_HUNT";
    FirstHunt.Title = "First Hunt";
    FirstHunt.Description = "Prove your hunting skills by taking down your first dinosaur.";
    FirstHunt.Status = EQuest_Status::NotStarted;
    FirstHunt.bIsMainQuest = false;
    
    FQuest_Objective HuntObjective;
    HuntObjective.ObjectiveID = "KILL_ANY_DINOSAUR";
    HuntObjective.Description = "Kill any dinosaur";
    HuntObjective.Type = EQuest_ObjectiveType::Hunt_Dinosaur;
    HuntObjective.Status = EQuest_Status::NotStarted;
    HuntObjective.TargetActorClass = ""; // Any dinosaur
    HuntObjective.RequiredCount = 1;
    HuntObjective.CurrentProgress = 0;
    
    FirstHunt.Objectives.Add(HuntObjective);
    AllQuests.Add(FirstHunt.QuestID, FirstHunt);
    
    // Raptor pack quest
    FQuest_Data RaptorHunt;
    RaptorHunt.QuestID = "RAPTOR_PACK_HUNT";
    RaptorHunt.Title = "Raptor Pack Hunter";
    RaptorHunt.Description = "Eliminate a pack of raptors to secure a safe area.";
    RaptorHunt.Status = EQuest_Status::NotStarted;
    RaptorHunt.bIsMainQuest = false;
    
    FQuest_Objective RaptorObjective;
    RaptorObjective.ObjectiveID = "KILL_3_RAPTORS";
    RaptorObjective.Description = "Kill 3 raptors";
    RaptorObjective.Type = EQuest_ObjectiveType::Hunt_Dinosaur;
    RaptorObjective.Status = EQuest_Status::NotStarted;
    RaptorObjective.TargetActorClass = "Raptor";
    RaptorObjective.RequiredCount = 3;
    RaptorObjective.CurrentProgress = 0;
    
    RaptorHunt.Objectives.Add(RaptorObjective);
    AllQuests.Add(RaptorHunt.QuestID, RaptorHunt);
}

void UQuestSystem::CreateCraftingQuests()
{
    // Basic crafting quest
    FQuest_Data BasicCrafting;
    BasicCrafting.QuestID = "BASIC_CRAFTING";
    BasicCrafting.Title = "Tool Maker";
    BasicCrafting.Description = "Craft your first tools to improve your chances of survival.";
    BasicCrafting.Status = EQuest_Status::NotStarted;
    BasicCrafting.bIsMainQuest = false;
    
    FQuest_Objective CraftAxe;
    CraftAxe.ObjectiveID = "CRAFT_STONE_AXE";
    CraftAxe.Description = "Craft a stone axe";
    CraftAxe.Type = EQuest_ObjectiveType::Craft_Item;
    CraftAxe.Status = EQuest_Status::NotStarted;
    CraftAxe.TargetActorClass = "StoneAxe";
    CraftAxe.RequiredCount = 1;
    CraftAxe.CurrentProgress = 0;
    
    BasicCrafting.Objectives.Add(CraftAxe);
    AllQuests.Add(BasicCrafting.QuestID, BasicCrafting);
}

bool UQuestSystem::CheckObjectiveCompletion(FQuest_Objective& Objective, const FVector& PlayerLocation)
{
    switch (Objective.Type)
    {
        case EQuest_ObjectiveType::Explore_Biome:
        case EQuest_ObjectiveType::Reach_Location:
        {
            float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
            if (Distance <= Objective.RequiredDistance)
            {
                Objective.CurrentProgress = Objective.RequiredCount;
                return true;
            }
            break;
        }
        case EQuest_ObjectiveType::Survive_Duration:
        {
            if (UWorld* World = GetWorld())
            {
                float CurrentTime = World->GetTimeSeconds();
                Objective.CurrentProgress = FMath::FloorToInt(CurrentTime - LastUpdateTime);
                if (Objective.CurrentProgress >= Objective.RequiredCount)
                {
                    return true;
                }
            }
            break;
        }
        default:
            // Other objective types are handled by specific event functions
            break;
    }
    
    return false;
}

void UQuestSystem::OnObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID)
{
    if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        for (FQuest_Objective& Objective : QuestData->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.Status = EQuest_Status::Completed;
                UE_LOG(LogTemp, Warning, TEXT("QuestSystem: Completed objective '%s' in quest '%s'"), *ObjectiveID, *QuestData->Title);
                break;
            }
        }
        
        CheckQuestCompletion(QuestID);
    }
}

void UQuestSystem::CheckQuestCompletion(const FString& QuestID)
{
    if (FQuest_Data* QuestData = AllQuests.Find(QuestID))
    {
        bool bAllObjectivesComplete = true;
        
        for (const FQuest_Objective& Objective : QuestData->Objectives)
        {
            if (Objective.Status != EQuest_Status::Completed)
            {
                bAllObjectivesComplete = false;
                break;
            }
        }
        
        if (bAllObjectivesComplete && QuestData->Status == EQuest_Status::Active)
        {
            CompleteQuest(QuestID);
        }
    }
}

// AQuest_Marker Implementation
AQuest_Marker::AQuest_Marker()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create components
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    RootComponent = MarkerMesh;
    
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    
    // Initialize properties
    AssociatedQuestID = "";
    AssociatedObjectiveID = "";
    MarkerType = EQuest_ObjectiveType::Explore_Biome;
    bIsActive = true;
    
    // Set up trigger events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_Marker::OnTriggerEnter);
    
    // Load default mesh (cube for now)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MarkerMesh->SetStaticMesh(CubeMeshAsset.Object);
    }
    
    // Set scale and material
    SetActorScale3D(FVector(2.0f, 2.0f, 2.0f));
}

void AQuest_Marker::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Marker spawned: %s"), *GetName());
}

void AQuest_Marker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Simple floating animation
    if (bIsActive)
    {
        FVector CurrentLocation = GetActorLocation();
        float FloatOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 50.0f;
        SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z + FloatOffset * DeltaTime));
    }
}

void AQuest_Marker::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || AssociatedQuestID.IsEmpty() || AssociatedObjectiveID.IsEmpty())
    {
        return;
    }
    
    // Check if it's the player character
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        if (UQuestSystem* QuestSystem = GetGameInstance()->GetSubsystem<UQuestSystem>())
        {
            if (QuestSystem->IsQuestActive(AssociatedQuestID))
            {
                // Update objective progress
                QuestSystem->UpdateObjectiveProgress(AssociatedQuestID, AssociatedObjectiveID, 1);
                
                // Deactivate marker
                SetMarkerActive(false);
                
                UE_LOG(LogTemp, Warning, TEXT("Quest Marker triggered by player: %s"), *AssociatedObjectiveID);
            }
        }
    }
}

void AQuest_Marker::SetMarkerActive(bool bActive)
{
    bIsActive = bActive;
    SetActorHiddenInGame(!bActive);
    SetActorEnableCollision(bActive);
}

void AQuest_Marker::SetMarkerData(const FString& QuestID, const FString& ObjectiveID, EQuest_ObjectiveType Type)
{
    AssociatedQuestID = QuestID;
    AssociatedObjectiveID = ObjectiveID;
    MarkerType = Type;
}