#include "Quest_TerritorialDefense.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AQuest_TerritorialDefense::AQuest_TerritorialDefense()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create quest trigger
    QuestTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("QuestTrigger"));
    QuestTrigger->SetupAttachment(RootComponent);
    QuestTrigger->SetSphereRadius(1000.0f);
    QuestTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    QuestTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    QuestTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // Create quest marker
    QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
    QuestMarker->SetupAttachment(RootComponent);
    QuestMarker->SetRelativeLocation(FVector(0, 0, 200));
    QuestMarker->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

    // Initialize quest parameters
    QuestActivationRadius = 1000.0f;
    bQuestActive = false;
    bQuestCompleted = false;
    bPlayerInRange = false;
    QuestTimer = 0.0f;
    ConflictDetectionTimer = 0.0f;

    // Initialize territory data
    TerritoryData.TerritoryCenter = FVector::ZeroVector;
    TerritoryData.TerritoryRadius = 5000.0f;
    TerritoryData.bIsActive = false;

    // Initialize defense objective
    DefenseObjective.ObjectiveText = TEXT("Observe 3 territorial defense behaviors");
    DefenseObjective.RequiredDefenses = 3;
    DefenseObjective.CurrentDefenses = 0;
    DefenseObjective.TimeLimit = 300.0f; // 5 minutes
    DefenseObjective.bCompleted = false;

    // Setup patrol points in a circle around territory
    PatrolPoints.Empty();
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (i * 2.0f * PI) / 8.0f;
        FVector PatrolPoint = FVector(
            FMath::Cos(Angle) * TerritoryData.TerritoryRadius * 0.8f,
            FMath::Sin(Angle) * TerritoryData.TerritoryRadius * 0.8f,
            0.0f
        );
        PatrolPoints.Add(PatrolPoint);
    }

    // Bind trigger events
    QuestTrigger->OnComponentBeginOverlap.AddDynamic(this, &AQuest_TerritorialDefense::OnTriggerBeginOverlap);
    QuestTrigger->OnComponentEndOverlap.AddDynamic(this, &AQuest_TerritorialDefense::OnTriggerEndOverlap);
}

void AQuest_TerritorialDefense::BeginPlay()
{
    Super::BeginPlay();

    InitializeQuestMarker();
    
    // Set territory center to current location
    TerritoryData.TerritoryCenter = GetActorLocation();
    
    // Find initial dinosaurs in territory
    TerritoryData.DefendingDinosaurs = FindDinosaursInTerritory(TerritoryData.TerritoryCenter, TerritoryData.TerritoryRadius);
    
    UE_LOG(LogTemp, Warning, TEXT("TerritorialDefense Quest initialized at %s with %d defending dinosaurs"), 
           *TerritoryData.TerritoryCenter.ToString(), TerritoryData.DefendingDinosaurs.Num());
}

void AQuest_TerritorialDefense::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bQuestActive && !bQuestCompleted)
    {
        QuestTimer += DeltaTime;
        ConflictDetectionTimer += DeltaTime;

        // Check for territorial conflicts every 2 seconds
        if (ConflictDetectionTimer >= 2.0f)
        {
            DetectTerritorialConflict();
            ConflictDetectionTimer = 0.0f;
        }

        // Update quest progress
        UpdateQuestProgress();

        // Check for quest timeout
        if (QuestTimer >= DefenseObjective.TimeLimit)
        {
            HandleQuestTimeout();
        }

        // Update quest marker visibility
        UpdateQuestMarkerVisibility();

        // Draw debug territory boundary
        if (GetWorld())
        {
            DrawDebugCircle(GetWorld(), TerritoryData.TerritoryCenter, TerritoryData.TerritoryRadius, 32, FColor::Yellow, false, -1.0f, 0, 50.0f, FVector(1, 0, 0), FVector(0, 1, 0));
        }
    }
}

void AQuest_TerritorialDefense::StartTerritorialDefenseQuest()
{
    if (bQuestActive || bQuestCompleted)
    {
        return;
    }

    bQuestActive = true;
    TerritoryData.bIsActive = true;
    TerritoryData.DefenseStartTime = GetWorld()->GetTimeSeconds();
    QuestTimer = 0.0f;

    // Reset objective progress
    DefenseObjective.CurrentDefenses = 0;
    DefenseObjective.bCompleted = false;

    // Find current dinosaurs in territory
    TerritoryData.DefendingDinosaurs = FindDinosaursInTerritory(TerritoryData.TerritoryCenter, TerritoryData.TerritoryRadius);

    UE_LOG(LogTemp, Warning, TEXT("Territorial Defense Quest Started! Observe %d defense behaviors within %.0f seconds"), 
           DefenseObjective.RequiredDefenses, DefenseObjective.TimeLimit);

    OnQuestStarted();
}

void AQuest_TerritorialDefense::EndTerritorialDefenseQuest(bool bSuccess)
{
    if (!bQuestActive)
    {
        return;
    }

    bQuestActive = false;
    bQuestCompleted = true;
    TerritoryData.bIsActive = false;

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Territorial Defense Quest Completed Successfully! Observed %d defense behaviors"), 
               DefenseObjective.CurrentDefenses);
        OnQuestCompleted(bSuccess);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Territorial Defense Quest Failed - Time expired"));
        OnQuestFailed();
    }

    // Hide quest marker
    if (QuestMarker)
    {
        QuestMarker->SetVisibility(false);
    }
}

void AQuest_TerritorialDefense::DetectTerritorialConflict()
{
    if (!bQuestActive || !GetWorld())
    {
        return;
    }

    // Find all dinosaurs in expanded area to detect intruders
    TArray<AActor*> AllNearbyDinosaurs = FindDinosaursInTerritory(TerritoryData.TerritoryCenter, TerritoryData.TerritoryRadius * 1.5f);
    
    // Clear previous intruders
    TerritoryData.IntruderDinosaurs.Empty();

    // Identify intruders (dinosaurs not in original defending group)
    for (AActor* DinosaurActor : AllNearbyDinosaurs)
    {
        if (DinosaurActor && !TerritoryData.DefendingDinosaurs.Contains(DinosaurActor))
        {
            float DistanceToCenter = FVector::Dist(DinosaurActor->GetActorLocation(), TerritoryData.TerritoryCenter);
            if (DistanceToCenter <= TerritoryData.TerritoryRadius)
            {
                TerritoryData.IntruderDinosaurs.Add(DinosaurActor);
            }
        }
    }

    // Check for defensive behaviors
    if (TerritoryData.IntruderDinosaurs.Num() > 0)
    {
        for (AActor* Defender : TerritoryData.DefendingDinosaurs)
        {
            if (Defender)
            {
                for (AActor* Intruder : TerritoryData.IntruderDinosaurs)
                {
                    if (Intruder)
                    {
                        float Distance = FVector::Dist(Defender->GetActorLocation(), Intruder->GetActorLocation());
                        if (Distance <= 1500.0f) // Close enough for territorial behavior
                        {
                            RegisterDefenseAction(Defender, Intruder);
                        }
                    }
                }
            }
        }
    }
}

void AQuest_TerritorialDefense::RegisterDefenseAction(AActor* Defender, AActor* Intruder)
{
    if (!Defender || !Intruder || DefenseObjective.bCompleted)
    {
        return;
    }

    DefenseObjective.CurrentDefenses++;
    
    UE_LOG(LogTemp, Warning, TEXT("Territorial Defense Observed! Progress: %d/%d"), 
           DefenseObjective.CurrentDefenses, DefenseObjective.RequiredDefenses);

    OnDefenseRegistered();

    // Check if objective is complete
    if (DefenseObjective.CurrentDefenses >= DefenseObjective.RequiredDefenses)
    {
        DefenseObjective.bCompleted = true;
        EndTerritorialDefenseQuest(true);
    }
}

bool AQuest_TerritorialDefense::ValidateDefenseSuccess()
{
    return DefenseObjective.bCompleted && DefenseObjective.CurrentDefenses >= DefenseObjective.RequiredDefenses;
}

void AQuest_TerritorialDefense::UpdateQuestProgress()
{
    if (!bQuestActive)
    {
        return;
    }

    // Update territory data
    float ElapsedTime = QuestTimer;
    float RemainingTime = DefenseObjective.TimeLimit - ElapsedTime;

    if (RemainingTime <= 0.0f)
    {
        HandleQuestTimeout();
    }
}

TArray<AActor*> AQuest_TerritorialDefense::FindDinosaursInTerritory(FVector Center, float Radius)
{
    TArray<AActor*> DinosaurActors;
    
    if (!GetWorld())
    {
        return DinosaurActors;
    }

    // Find all actors in range
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != this)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                // Check if it's a dinosaur (has "Dinosaur" or species name in label)
                FString ActorLabel = Actor->GetActorLabel();
                if (ActorLabel.Contains(TEXT("Trex")) || 
                    ActorLabel.Contains(TEXT("Raptor")) || 
                    ActorLabel.Contains(TEXT("Triceratops")) ||
                    ActorLabel.Contains(TEXT("Dinosaur")) ||
                    IsTerritorialSpecies(Actor))
                {
                    DinosaurActors.Add(Actor);
                }
            }
        }
    }

    return DinosaurActors;
}

bool AQuest_TerritorialDefense::IsTerritorialSpecies(AActor* DinosaurActor)
{
    if (!DinosaurActor)
    {
        return false;
    }

    FString ActorLabel = DinosaurActor->GetActorLabel();
    
    // Territorial species are typically carnivores or large herbivores
    return ActorLabel.Contains(TEXT("Trex")) || 
           ActorLabel.Contains(TEXT("Raptor")) || 
           ActorLabel.Contains(TEXT("Triceratops")) ||
           ActorLabel.Contains(TEXT("Ankylosaurus"));
}

void AQuest_TerritorialDefense::SetupPatrolRoute()
{
    // Patrol points are already set up in constructor
    // This function can be used to dynamically adjust patrol routes
    
    if (PatrolPoints.Num() == 0)
    {
        // Create default patrol route
        for (int32 i = 0; i < 6; i++)
        {
            float Angle = (i * 2.0f * PI) / 6.0f;
            FVector PatrolPoint = TerritoryData.TerritoryCenter + FVector(
                FMath::Cos(Angle) * TerritoryData.TerritoryRadius * 0.7f,
                FMath::Sin(Angle) * TerritoryData.TerritoryRadius * 0.7f,
                0.0f
            );
            PatrolPoints.Add(PatrolPoint);
        }
    }
}

void AQuest_TerritorialDefense::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInRange = true;
        
        if (!bQuestActive && !bQuestCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player entered territorial defense quest area"));
            // Auto-start quest when player approaches
            StartTerritorialDefenseQuest();
        }
    }
}

void AQuest_TerritorialDefense::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInRange = false;
    }
}

void AQuest_TerritorialDefense::InitializeQuestMarker()
{
    if (QuestMarker)
    {
        QuestMarker->SetVisibility(true);
        // Quest marker will be a simple colored sphere for now
        // In a full implementation, this would be a custom quest icon mesh
    }
}

void AQuest_TerritorialDefense::UpdateQuestMarkerVisibility()
{
    if (QuestMarker)
    {
        bool bShouldBeVisible = !bQuestCompleted && (bPlayerInRange || bQuestActive);
        QuestMarker->SetVisibility(bShouldBeVisible);
    }
}

void AQuest_TerritorialDefense::CheckQuestCompletion()
{
    if (DefenseObjective.CurrentDefenses >= DefenseObjective.RequiredDefenses)
    {
        DefenseObjective.bCompleted = true;
        EndTerritorialDefenseQuest(true);
    }
}

void AQuest_TerritorialDefense::HandleQuestTimeout()
{
    if (bQuestActive && !bQuestCompleted)
    {
        EndTerritorialDefenseQuest(false);
    }
}