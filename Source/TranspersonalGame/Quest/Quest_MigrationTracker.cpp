#include "Quest_MigrationTracker.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "../Core/TranspersonalCharacter.h"
#include "Kismet/GameplayStatics.h"

AQuest_MigrationTracker::AQuest_MigrationTracker()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Detection sphere for player proximity
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(500.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Quest marker mesh
    QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
    QuestMarker->SetupAttachment(RootComponent);
    QuestMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize quest parameters
    DetectionRadius = 2000.0f;
    UpdateInterval = 5.0f;
    RequiredMigrationsToTrack = 3;
    QuestDuration = 300.0f; // 5 minutes
    
    // Initialize quest state
    bQuestActive = false;
    MigrationsTracked = 0;
    QuestTimeRemaining = 0.0f;
    LastUpdateTime = 0.0f;
    bPlayerInRange = false;
    
    CrowdManager = nullptr;
}

void AQuest_MigrationTracker::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_MigrationTracker::OnPlayerEnterDetection);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_MigrationTracker::OnPlayerExitDetection);
    
    // Find crowd simulation manager
    if (UWorld* World = GetWorld())
    {
        if (ATranspersonalGameState* GameState = Cast<ATranspersonalGameState>(World->GetGameState()))
        {
            // CrowdManager will be set via Blueprint or direct reference
        }
    }
    
    UpdateQuestMarkerVisibility();
}

void AQuest_MigrationTracker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bQuestActive)
    {
        QuestTimeRemaining -= DeltaTime;
        
        // Update migration tracking periodically
        if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= UpdateInterval)
        {
            UpdateMigrationTracking();
            LastUpdateTime = GetWorld()->GetTimeSeconds();
        }
        
        // Check quest completion or timeout
        if (MigrationsTracked >= RequiredMigrationsToTrack)
        {
            CompleteMigrationQuest();
        }
        else if (QuestTimeRemaining <= 0.0f)
        {
            // Quest failed - timeout
            bQuestActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Migration tracking quest timed out"));
        }
    }
}

void AQuest_MigrationTracker::StartMigrationQuest()
{
    if (!bQuestActive)
    {
        bQuestActive = true;
        MigrationsTracked = 0;
        QuestTimeRemaining = QuestDuration;
        TrackedMigrations.Empty();
        
        UE_LOG(LogTemp, Log, TEXT("Migration tracking quest started - Track %d migrations in %.1f seconds"), 
               RequiredMigrationsToTrack, QuestDuration);
        
        UpdateQuestMarkerVisibility();
    }
}

void AQuest_MigrationTracker::CompleteMigrationQuest()
{
    if (bQuestActive)
    {
        bQuestActive = false;
        
        UE_LOG(LogTemp, Log, TEXT("Migration tracking quest completed! Tracked %d migrations"), MigrationsTracked);
        
        // Process rewards
        ProcessMigrationRewards();
        
        UpdateQuestMarkerVisibility();
    }
}

void AQuest_MigrationTracker::UpdateMigrationTracking()
{
    if (!bQuestActive || !bPlayerInRange)
        return;
    
    // Detect nearby herds
    if (DetectNearbyHerds())
    {
        // Update progress of existing migrations
        for (FQuest_MigrationData& Migration : TrackedMigrations)
        {
            if (Migration.bIsActive)
            {
                float NewProgress = CalculateMigrationProgress(Migration);
                if (NewProgress > Migration.MigrationProgress + 0.1f) // Significant progress
                {
                    Migration.MigrationProgress = NewProgress;
                    LogMigrationProgress();
                }
            }
        }
    }
}

bool AQuest_MigrationTracker::DetectNearbyHerds()
{
    if (!GetWorld())
        return false;
    
    // Find dinosaur actors in detection range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    bool bFoundNewMigration = false;
    TMap<FString, int32> SpeciesCount;
    TMap<FString, FVector> SpeciesCenter;
    
    // Count species and calculate centers
    for (AActor* Actor : FoundActors)
    {
        if (!Actor)
            continue;
            
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Triceratops")) || 
            ActorLabel.Contains(TEXT("Parasaurolophus")) ||
            ActorLabel.Contains(TEXT("Brachiosaurus")))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= DetectionRadius)
            {
                FString Species = TEXT("Unknown");
                if (ActorLabel.Contains(TEXT("Triceratops"))) Species = TEXT("Triceratops");
                else if (ActorLabel.Contains(TEXT("Parasaurolophus"))) Species = TEXT("Parasaurolophus");
                else if (ActorLabel.Contains(TEXT("Brachiosaurus"))) Species = TEXT("Brachiosaurus");
                
                SpeciesCount.FindOrAdd(Species)++;
                
                if (!SpeciesCenter.Contains(Species))
                {
                    SpeciesCenter.Add(Species, Actor->GetActorLocation());
                }
                else
                {
                    SpeciesCenter[Species] = (SpeciesCenter[Species] + Actor->GetActorLocation()) * 0.5f;
                }
            }
        }
    }
    
    // Check for new migrations (herds of 10+ animals)
    for (auto& SpeciesPair : SpeciesCount)
    {
        if (SpeciesPair.Value >= 10)
        {
            // Check if this migration is already tracked
            bool bAlreadyTracked = false;
            for (const FQuest_MigrationData& Migration : TrackedMigrations)
            {
                if (Migration.HerdSpecies == SpeciesPair.Key && Migration.bIsActive)
                {
                    bAlreadyTracked = true;
                    break;
                }
            }
            
            if (!bAlreadyTracked)
            {
                // Add new migration
                FVector StartPos = SpeciesCenter[SpeciesPair.Key];
                FVector EndPos = StartPos + FVector(FMath::RandRange(-5000, 5000), FMath::RandRange(-5000, 5000), 0);
                
                AddMigrationToTracker(SpeciesPair.Key, SpeciesPair.Value, StartPos, EndPos);
                bFoundNewMigration = true;
                
                UE_LOG(LogTemp, Log, TEXT("Detected new %s migration with %d animals"), 
                       *SpeciesPair.Key, SpeciesPair.Value);
            }
        }
    }
    
    return bFoundNewMigration;
}

void AQuest_MigrationTracker::AddMigrationToTracker(const FString& Species, int32 HerdSize, FVector StartPos, FVector EndPos)
{
    FQuest_MigrationData NewMigration;
    NewMigration.HerdSpecies = Species;
    NewMigration.HerdSize = HerdSize;
    NewMigration.StartLocation = StartPos;
    NewMigration.EndLocation = EndPos;
    NewMigration.MigrationProgress = 0.0f;
    NewMigration.bIsActive = true;
    
    TrackedMigrations.Add(NewMigration);
    MigrationsTracked++;
}

float AQuest_MigrationTracker::CalculateMigrationProgress(const FQuest_MigrationData& Migration)
{
    if (!GetWorld())
        return 0.0f;
    
    // Find current center of this species herd
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    FVector CurrentCenter = FVector::ZeroVector;
    int32 Count = 0;
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(Migration.HerdSpecies))
        {
            CurrentCenter += Actor->GetActorLocation();
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CurrentCenter /= Count;
        
        float TotalDistance = FVector::Dist(Migration.StartLocation, Migration.EndLocation);
        float TraveledDistance = FVector::Dist(Migration.StartLocation, CurrentCenter);
        
        return FMath::Clamp(TraveledDistance / TotalDistance, 0.0f, 1.0f);
    }
    
    return Migration.MigrationProgress;
}

void AQuest_MigrationTracker::OnPlayerEnterDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor))
    {
        bPlayerInRange = true;
        
        if (!bQuestActive)
        {
            // Auto-start quest when player approaches
            StartMigrationQuest();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Player entered migration tracking area"));
    }
}

void AQuest_MigrationTracker::OnPlayerExitDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor))
    {
        bPlayerInRange = false;
        UE_LOG(LogTemp, Log, TEXT("Player left migration tracking area"));
    }
}

void AQuest_MigrationTracker::UpdateQuestMarkerVisibility()
{
    if (QuestMarker)
    {
        QuestMarker->SetVisibility(bQuestActive);
    }
}

void AQuest_MigrationTracker::ProcessMigrationRewards()
{
    // Find player and give rewards
    if (UWorld* World = GetWorld())
    {
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)))
        {
            // Award experience or resources for successful migration tracking
            UE_LOG(LogTemp, Log, TEXT("Migration tracking rewards processed for player"));
        }
    }
}

void AQuest_MigrationTracker::LogMigrationProgress()
{
    for (const FQuest_MigrationData& Migration : TrackedMigrations)
    {
        if (Migration.bIsActive)
        {
            UE_LOG(LogTemp, Log, TEXT("%s migration progress: %.1f%% (%d animals)"), 
                   *Migration.HerdSpecies, Migration.MigrationProgress * 100.0f, Migration.HerdSize);
        }
    }
}