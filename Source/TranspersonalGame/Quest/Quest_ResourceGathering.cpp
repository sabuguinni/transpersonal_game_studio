#include "Quest_ResourceGathering.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AQuest_ResourceGathering::AQuest_ResourceGathering()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bIsQuestActive = false;
    bIsQuestCompleted = false;
    QuestTimeLimit = 300.0f; // 5 minutes
    QuestStartTime = 0.0f;
    QuestDescription = TEXT("Gather essential survival resources to establish your first camp");
}

void AQuest_ResourceGathering::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeResourceRequirements();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Resource Gathering Quest initialized"));
    }
}

void AQuest_ResourceGathering::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsQuestActive && !bIsQuestCompleted)
    {
        UpdateQuestProgress();
        
        // Check for time limit
        float ElapsedTime = GetWorld()->GetTimeSeconds() - QuestStartTime;
        if (ElapsedTime >= QuestTimeLimit)
        {
            FailQuest();
        }
    }
}

void AQuest_ResourceGathering::InitializeResourceRequirements()
{
    RequiredResources.Empty();
    
    // Stone for tools
    FQuest_ResourceItem Stone;
    Stone.ResourceName = TEXT("Stone");
    Stone.RequiredAmount = 5;
    Stone.CurrentAmount = 0;
    Stone.SpawnLocation = FVector(2000.0f, 1000.0f, 100.0f);
    RequiredResources.Add(Stone);
    
    // Wood for fire and shelter
    FQuest_ResourceItem Wood;
    Wood.ResourceName = TEXT("Wood");
    Wood.RequiredAmount = 8;
    Wood.CurrentAmount = 0;
    Wood.SpawnLocation = FVector(1500.0f, 2000.0f, 100.0f);
    RequiredResources.Add(Wood);
    
    // Plant fiber for rope and bedding
    FQuest_ResourceItem Fiber;
    Fiber.ResourceName = TEXT("Plant_Fiber");
    Fiber.RequiredAmount = 10;
    Fiber.CurrentAmount = 0;
    Fiber.SpawnLocation = FVector(3000.0f, 1500.0f, 100.0f);
    RequiredResources.Add(Fiber);
    
    // Fresh water container materials
    FQuest_ResourceItem Clay;
    Clay.ResourceName = TEXT("Clay");
    Clay.RequiredAmount = 3;
    Clay.CurrentAmount = 0;
    Clay.SpawnLocation = FVector(2500.0f, 3000.0f, 100.0f);
    RequiredResources.Add(Clay);
}

void AQuest_ResourceGathering::StartQuest()
{
    if (!bIsQuestActive)
    {
        bIsQuestActive = true;
        bIsQuestCompleted = false;
        QuestStartTime = GetWorld()->GetTimeSeconds();
        
        SpawnResourceNodes();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow,
                FString::Printf(TEXT("QUEST STARTED: %s"), *QuestDescription));
            GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan,
                FString::Printf(TEXT("Time Limit: %.0f seconds"), QuestTimeLimit));
        }
    }
}

void AQuest_ResourceGathering::AddResourceToInventory(const FString& ResourceName, int32 Amount)
{
    if (!bIsQuestActive || bIsQuestCompleted)
    {
        return;
    }
    
    for (FQuest_ResourceItem& Resource : RequiredResources)
    {
        if (Resource.ResourceName == ResourceName)
        {
            Resource.CurrentAmount = FMath::Min(Resource.CurrentAmount + Amount, Resource.RequiredAmount);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                    FString::Printf(TEXT("Collected %s: %d/%d"), 
                        *ResourceName, Resource.CurrentAmount, Resource.RequiredAmount));
            }
            
            CheckQuestCompletion();
            break;
        }
    }
}

bool AQuest_ResourceGathering::CheckQuestCompletion()
{
    if (!bIsQuestActive || bIsQuestCompleted)
    {
        return false;
    }
    
    bool bAllResourcesGathered = true;
    for (const FQuest_ResourceItem& Resource : RequiredResources)
    {
        if (Resource.CurrentAmount < Resource.RequiredAmount)
        {
            bAllResourcesGathered = false;
            break;
        }
    }
    
    if (bAllResourcesGathered)
    {
        CompleteQuest();
        return true;
    }
    
    return false;
}

void AQuest_ResourceGathering::SpawnResourceNodes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn resource collection points for each required resource
    for (const FQuest_ResourceItem& Resource : RequiredResources)
    {
        // Create a simple marker actor at the resource location
        AActor* ResourceNode = World->SpawnActor<AActor>(AActor::StaticClass(), 
            Resource.SpawnLocation, FRotator::ZeroRotator);
        
        if (ResourceNode)
        {
            ResourceNode->SetActorLabel(FString::Printf(TEXT("%s_Node"), *Resource.ResourceName));
            
            // Add a static mesh component for visibility
            UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
            if (MeshComp)
            {
                ResourceNode->SetRootComponent(MeshComp);
                MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
            }
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue,
            FString::Printf(TEXT("Spawned %d resource nodes"), RequiredResources.Num()));
    }
}

float AQuest_ResourceGathering::GetRemainingTime() const
{
    if (!bIsQuestActive)
    {
        return 0.0f;
    }
    
    float ElapsedTime = GetWorld()->GetTimeSeconds() - QuestStartTime;
    return FMath::Max(0.0f, QuestTimeLimit - ElapsedTime);
}

FString AQuest_ResourceGathering::GetQuestStatus() const
{
    if (!bIsQuestActive)
    {
        return TEXT("Quest not started");
    }
    
    if (bIsQuestCompleted)
    {
        return TEXT("Quest completed successfully!");
    }
    
    FString Status = TEXT("Resources needed:\n");
    for (const FQuest_ResourceItem& Resource : RequiredResources)
    {
        Status += FString::Printf(TEXT("- %s: %d/%d\n"), 
            *Resource.ResourceName, Resource.CurrentAmount, Resource.RequiredAmount);
    }
    
    Status += FString::Printf(TEXT("Time remaining: %.0f seconds"), GetRemainingTime());
    return Status;
}

void AQuest_ResourceGathering::UpdateQuestProgress()
{
    // Display current progress every 10 seconds
    float ElapsedTime = GetWorld()->GetTimeSeconds() - QuestStartTime;
    if (FMath::Fmod(ElapsedTime, 10.0f) < 0.1f && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, GetQuestStatus());
    }
}

void AQuest_ResourceGathering::CompleteQuest()
{
    bIsQuestCompleted = true;
    bIsQuestActive = false;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
            TEXT("QUEST COMPLETED: You have successfully gathered all survival resources!"));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
            TEXT("You can now craft basic tools and establish your first camp."));
    }
}

void AQuest_ResourceGathering::FailQuest()
{
    bIsQuestActive = false;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
            TEXT("QUEST FAILED: Time limit exceeded. Resources scattered by weather."));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow,
            TEXT("You must find shelter and try again when conditions improve."));
    }
}