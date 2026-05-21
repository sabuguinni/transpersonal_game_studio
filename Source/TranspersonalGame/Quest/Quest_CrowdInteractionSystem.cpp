#include "Quest_CrowdInteractionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UQuest_CrowdInteractionSystem::UQuest_CrowdInteractionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    DetectionRadius = 2000.0f;
    CompletionThreshold = 100.0f;
    CurrentProgress = 0.0f;
    CurrentObjective = TEXT("Find and observe dinosaur herds");
    bQuestActive = false;
    LastKnownHerdLocation = FVector::ZeroVector;
    InteractionTimer = 0.0f;
}

void UQuest_CrowdInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Component initialized"));
    
    // Initialize quest data
    InteractionData.InteractionType = TEXT("Herd Observation");
    InteractionData.RequiredCrowdSize = 10;
    InteractionData.InteractionRadius = 1500.0f;
    InteractionData.bIsCompleted = false;
}

void UQuest_CrowdInteractionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bQuestActive)
    {
        UpdateQuestProgress(DeltaTime);
        CheckProximityToHerd();
        AnalyzeCrowdBehavior();
        
        InteractionTimer += DeltaTime;
        
        if (ValidateQuestCompletion())
        {
            CompleteCrowdInteraction();
        }
    }
}

void UQuest_CrowdInteractionSystem::InitializeCrowdQuest(const FString& QuestType, int32 CrowdSize, const FVector& Location)
{
    InteractionData.InteractionType = QuestType;
    InteractionData.RequiredCrowdSize = CrowdSize;
    InteractionData.TargetLocation = Location;
    InteractionData.bIsCompleted = false;
    
    bQuestActive = true;
    CurrentProgress = 0.0f;
    InteractionTimer = 0.0f;
    
    if (QuestType == TEXT("Migration"))
    {
        CurrentObjective = FString::Printf(TEXT("Follow %d dinosaurs during migration"), CrowdSize);
    }
    else if (QuestType == TEXT("Feeding"))
    {
        CurrentObjective = FString::Printf(TEXT("Observe %d dinosaurs feeding behavior"), CrowdSize);
    }
    else if (QuestType == TEXT("Territorial"))
    {
        CurrentObjective = FString::Printf(TEXT("Study %d dinosaurs territorial display"), CrowdSize);
    }
    else
    {
        CurrentObjective = FString::Printf(TEXT("Observe %d dinosaurs in their natural behavior"), CrowdSize);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Quest initialized - %s"), *CurrentObjective);
}

bool UQuest_CrowdInteractionSystem::CheckCrowdInteraction(const FVector& PlayerLocation)
{
    int32 NearbyDinosaurs = CountNearbyDinosaurs(PlayerLocation, DetectionRadius);
    
    if (NearbyDinosaurs >= InteractionData.RequiredCrowdSize)
    {
        float DistanceToTarget = FVector::Dist(PlayerLocation, InteractionData.TargetLocation);
        
        if (DistanceToTarget <= InteractionData.InteractionRadius)
        {
            CurrentProgress = FMath::Min(CurrentProgress + 5.0f, CompletionThreshold);
            LastKnownHerdLocation = PlayerLocation;
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Crowd interaction progress: %.1f%%"), 
                   (CurrentProgress / CompletionThreshold) * 100.0f);
            
            return true;
        }
    }
    
    return false;
}

void UQuest_CrowdInteractionSystem::CompleteCrowdInteraction()
{
    if (!InteractionData.bIsCompleted)
    {
        InteractionData.bIsCompleted = true;
        bQuestActive = false;
        CurrentProgress = CompletionThreshold;
        
        CurrentObjective = TEXT("Quest Completed: Return to camp");
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Crowd interaction quest completed!"));
        
        // Award experience and resources
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    // Quest completion rewards would be handled here
                    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Awarding quest rewards"));
                }
            }
        }
    }
}

void UQuest_CrowdInteractionSystem::UpdateCrowdObjective(const FString& NewObjective)
{
    CurrentObjective = NewObjective;
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Objective updated - %s"), *NewObjective);
}

int32 UQuest_CrowdInteractionSystem::CountNearbyDinosaurs(const FVector& Location, float Radius)
{
    int32 Count = 0;
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor && Actor->GetActorLabel().Contains(TEXT("Dino")) || 
                Actor->GetActorLabel().Contains(TEXT("Triceratops")) ||
                Actor->GetActorLabel().Contains(TEXT("Brachiosaurus")) ||
                Actor->GetActorLabel().Contains(TEXT("Parasaurolophus")) ||
                Actor->GetActorLabel().Contains(TEXT("Ankylosaurus")))
            {
                float Distance = FVector::Dist(Location, Actor->GetActorLocation());
                if (Distance <= Radius)
                {
                    Count++;
                    
                    // Track this actor for quest purposes
                    if (!TrackedCrowdActors.Contains(Actor))
                    {
                        TrackedCrowdActors.Add(Actor);
                    }
                }
            }
        }
    }
    
    return Count;
}

bool UQuest_CrowdInteractionSystem::ValidateHerdBehavior(const FVector& HerdCenter)
{
    int32 HerdSize = CountNearbyDinosaurs(HerdCenter, 3000.0f);
    
    if (HerdSize >= 5)
    {
        // Check if dinosaurs are moving in similar direction (basic flocking)
        bool bValidFlocking = true;
        
        // Simple validation - in a real implementation, this would check velocity vectors
        if (TrackedCrowdActors.Num() >= 3)
        {
            FVector AveragePosition = FVector::ZeroVector;
            for (AActor* Actor : TrackedCrowdActors)
            {
                if (Actor)
                {
                    AveragePosition += Actor->GetActorLocation();
                }
            }
            AveragePosition /= TrackedCrowdActors.Num();
            
            // Check if herd is reasonably clustered
            float AverageDistance = 0.0f;
            for (AActor* Actor : TrackedCrowdActors)
            {
                if (Actor)
                {
                    AverageDistance += FVector::Dist(Actor->GetActorLocation(), AveragePosition);
                }
            }
            AverageDistance /= TrackedCrowdActors.Num();
            
            bValidFlocking = (AverageDistance < 2000.0f); // Herd is clustered within 20m average
        }
        
        return bValidFlocking;
    }
    
    return false;
}

void UQuest_CrowdInteractionSystem::TrackMigrationPattern(const FVector& StartPoint, const FVector& EndPoint)
{
    float MigrationDistance = FVector::Dist(StartPoint, EndPoint);
    
    if (MigrationDistance > 5000.0f) // Significant migration distance
    {
        CurrentProgress += 15.0f; // Bonus progress for tracking migration
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionSystem: Migration pattern tracked - Distance: %.1f"), 
               MigrationDistance);
        
        // Update objective to reflect migration tracking
        CurrentObjective = FString::Printf(TEXT("Migration tracked: %.0fm distance"), MigrationDistance);
    }
}

float UQuest_CrowdInteractionSystem::GetInteractionProgress() const
{
    return (CurrentProgress / CompletionThreshold) * 100.0f;
}

FString UQuest_CrowdInteractionSystem::GetCurrentObjective() const
{
    return CurrentObjective;
}

void UQuest_CrowdInteractionSystem::UpdateQuestProgress(float DeltaTime)
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                CheckCrowdInteraction(PlayerLocation);
            }
        }
    }
}

void UQuest_CrowdInteractionSystem::CheckProximityToHerd()
{
    if (TrackedCrowdActors.Num() > 0)
    {
        // Calculate herd center
        FVector HerdCenter = FVector::ZeroVector;
        int32 ValidActors = 0;
        
        for (AActor* Actor : TrackedCrowdActors)
        {
            if (Actor && IsValid(Actor))
            {
                HerdCenter += Actor->GetActorLocation();
                ValidActors++;
            }
        }
        
        if (ValidActors > 0)
        {
            HerdCenter /= ValidActors;
            LastKnownHerdLocation = HerdCenter;
            
            // Validate herd behavior for quest progression
            if (ValidateHerdBehavior(HerdCenter))
            {
                CurrentProgress += 2.0f; // Gradual progress for sustained observation
            }
        }
    }
}

void UQuest_CrowdInteractionSystem::AnalyzeCrowdBehavior()
{
    if (TrackedCrowdActors.Num() >= InteractionData.RequiredCrowdSize)
    {
        // Analyze different behavior patterns
        if (InteractionData.InteractionType == TEXT("Feeding"))
        {
            // Check if dinosaurs are stationary (feeding behavior)
            bool bFeedingBehavior = true;
            // In real implementation, check velocity of tracked actors
            
            if (bFeedingBehavior)
            {
                CurrentProgress += 1.0f;
                CurrentObjective = TEXT("Observing feeding behavior - Stay quiet");
            }
        }
        else if (InteractionData.InteractionType == TEXT("Migration"))
        {
            // Check if herd is moving in coordinated direction
            if (LastKnownHerdLocation != FVector::ZeroVector)
            {
                // Track migration progress
                CurrentProgress += 1.5f;
                CurrentObjective = TEXT("Following migration - Maintain distance");
            }
        }
        else if (InteractionData.InteractionType == TEXT("Territorial"))
        {
            // Check for aggressive posturing or territorial displays
            CurrentProgress += 1.2f;
            CurrentObjective = TEXT("Observing territorial behavior - Keep safe distance");
        }
    }
}

bool UQuest_CrowdInteractionSystem::ValidateQuestCompletion()
{
    return (CurrentProgress >= CompletionThreshold && InteractionTimer > 30.0f); // Minimum 30 seconds observation
}

// Quest Marker Implementation
AQuest_CrowdInteractionMarker::AQuest_CrowdInteractionMarker()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create marker mesh component
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    RootComponent = MarkerMesh;
    
    // Set default properties
    QuestTitle = TEXT("Herd Observation");
    QuestDescription = TEXT("Study dinosaur crowd behavior in their natural habitat");
    TargetBiome = EBiomeType::Savana;
    bIsActive = true;
    
    // Try to load a basic mesh for the marker
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        MarkerMesh->SetStaticMesh(SphereMesh.Object);
        MarkerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }
}

void AQuest_CrowdInteractionMarker::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionMarker: Marker spawned - %s"), *QuestTitle);
    
    if (bIsActive)
    {
        ActivateQuestMarker();
    }
}

void AQuest_CrowdInteractionMarker::ActivateQuestMarker()
{
    bIsActive = true;
    
    if (MarkerMesh)
    {
        MarkerMesh->SetVisibility(true);
        MarkerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionMarker: Quest marker activated"));
    OnQuestActivated();
}

void AQuest_CrowdInteractionMarker::DeactivateQuestMarker()
{
    bIsActive = false;
    
    if (MarkerMesh)
    {
        MarkerMesh->SetVisibility(false);
        MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionMarker: Quest marker deactivated"));
    OnQuestCompleted();
}