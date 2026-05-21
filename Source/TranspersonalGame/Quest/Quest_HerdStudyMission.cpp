#include "Quest_HerdStudyMission.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "../Character/TranspersonalCharacter.h"
#include "../Crowd/Crowd_MassSimulationManager.h"

AQuest_HerdStudyMission::AQuest_HerdStudyMission()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize quest state
    bIsQuestActive = false;
    bIsQuestComplete = false;
    QuestTimeLimit = 600.0f; // 10 minutes
    QuestTimeRemaining = QuestTimeLimit;
    
    // Initialize observation requirements
    RequiredObservations = 5;
    MinObservationDistance = 500.0f;
    MaxObservationDistance = 2000.0f;
    MinObservationDuration = 15.0f;
    
    // Initialize target species
    TargetSpecies.Add(TEXT("Triceratops"));
    TargetSpecies.Add(TEXT("Brachiosaurus"));
    TargetSpecies.Add(TEXT("Parasaurolophus"));
    TargetSpecies.Add(TEXT("Ankylosaurus"));
    TargetSpecies.Add(TEXT("Protoceratops"));
    
    // Initialize references
    PlayerCharacter = nullptr;
    CrowdManager = nullptr;
}

void AQuest_HerdStudyMission::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
        
        // Find crowd simulation manager
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, UCrowd_MassSimulationManager::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            CrowdManager = Cast<UCrowd_MassSimulationManager>(FoundActors[0]);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: BeginPlay - Quest system initialized"));
}

void AQuest_HerdStudyMission::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsQuestActive && !bIsQuestComplete)
    {
        // Update quest timer
        QuestTimeRemaining -= DeltaTime;
        
        // Check for quest timeout
        if (QuestTimeRemaining <= 0.0f)
        {
            OnQuestTimeExpired();
        }
        
        // Periodically detect nearby herds
        static float HerdDetectionTimer = 0.0f;
        HerdDetectionTimer += DeltaTime;
        if (HerdDetectionTimer >= 2.0f) // Check every 2 seconds
        {
            DetectNearbyHerds();
            HerdDetectionTimer = 0.0f;
        }
    }
}

void AQuest_HerdStudyMission::StartHerdStudyMission()
{
    if (bIsQuestActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Quest already active"));
        return;
    }
    
    bIsQuestActive = true;
    bIsQuestComplete = false;
    QuestTimeRemaining = QuestTimeLimit;
    CompletedObservations.Empty();
    
    // Set up quest timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(QuestTimerHandle, this, 
            &AQuest_HerdStudyMission::UpdateQuestTimer, 1.0f, true);
        
        World->GetTimerManager().SetTimer(HerdDetectionTimerHandle, this,
            &AQuest_HerdStudyMission::DetectNearbyHerds, 3.0f, true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Herd study mission started - %d observations required"), RequiredObservations);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("QUEST STARTED: Study %d different herd behaviors within %d minutes"), 
            RequiredObservations, FMath::RoundToInt(QuestTimeLimit / 60.0f)));
    }
}

void AQuest_HerdStudyMission::CompleteHerdStudyMission()
{
    if (!bIsQuestActive || bIsQuestComplete)
    {
        return;
    }
    
    bIsQuestComplete = true;
    bIsQuestActive = false;
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestTimerHandle);
        World->GetTimerManager().ClearTimer(HerdDetectionTimerHandle);
        World->GetTimerManager().ClearTimer(ObservationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Mission completed successfully! %d observations recorded"), CompletedObservations.Num());
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Gold, 
            FString::Printf(TEXT("QUEST COMPLETE: Herd Study Mission - %d behaviors documented!"), CompletedObservations.Num()));
    }
}

void AQuest_HerdStudyMission::FailHerdStudyMission()
{
    if (!bIsQuestActive)
    {
        return;
    }
    
    bIsQuestActive = false;
    bIsQuestComplete = false;
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestTimerHandle);
        World->GetTimerManager().ClearTimer(HerdDetectionTimerHandle);
        World->GetTimerManager().ClearTimer(ObservationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Mission failed - time expired"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, 
            TEXT("QUEST FAILED: Herd Study Mission - Time expired!"));
    }
}

void AQuest_HerdStudyMission::RegisterHerdObservation(const FString& SpeciesName, 
    EQuest_HerdStudyObjective ObjectiveType, FVector Location, float Duration, int32 HerdSize)
{
    if (!bIsQuestActive || bIsQuestComplete)
    {
        return;
    }
    
    // Validate observation requirements
    if (!PlayerCharacter || !IsPlayerInObservationRange(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Player not in observation range"));
        return;
    }
    
    if (Duration < MinObservationDuration)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Observation duration too short: %f"), Duration);
        return;
    }
    
    // Check if this species is already observed for this objective type
    bool bAlreadyObserved = false;
    for (const FQuest_HerdObservation& Obs : CompletedObservations)
    {
        if (Obs.SpeciesName == SpeciesName && Obs.ObjectiveType == ObjectiveType)
        {
            bAlreadyObserved = true;
            break;
        }
    }
    
    if (bAlreadyObserved)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Species %s already observed for this objective"), *SpeciesName);
        return;
    }
    
    // Create new observation record
    FQuest_HerdObservation NewObservation;
    NewObservation.SpeciesName = SpeciesName;
    NewObservation.ObjectiveType = ObjectiveType;
    NewObservation.ObservationLocation = Location;
    NewObservation.ObservationDuration = Duration;
    NewObservation.HerdSize = HerdSize;
    
    CompletedObservations.Add(NewObservation);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HerdStudyMission: Observation registered - %s (%d/%d)"), 
        *SpeciesName, CompletedObservations.Num(), RequiredObservations);
    
    if (GEngine)
    {
        FString ObjectiveName;
        switch (ObjectiveType)
        {
            case EQuest_HerdStudyObjective::ObserveFeeding:
                ObjectiveName = TEXT("Feeding");
                break;
            case EQuest_HerdStudyObjective::ObserveMigration:
                ObjectiveName = TEXT("Migration");
                break;
            case EQuest_HerdStudyObjective::ObserveDefense:
                ObjectiveName = TEXT("Defense");
                break;
            case EQuest_HerdStudyObjective::ObserveSocialBehavior:
                ObjectiveName = TEXT("Social Behavior");
                break;
            case EQuest_HerdStudyObjective::DocumentSpecies:
                ObjectiveName = TEXT("Species Documentation");
                break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("OBSERVATION RECORDED: %s - %s (%d/%d)"), 
            *SpeciesName, *ObjectiveName, CompletedObservations.Num(), RequiredObservations));
    }
    
    // Check if quest is complete
    CheckObservationProgress();
}

bool AQuest_HerdStudyMission::CheckObservationProgress()
{
    if (CompletedObservations.Num() >= RequiredObservations)
    {
        CompleteHerdStudyMission();
        return true;
    }
    
    return false;
}

void AQuest_HerdStudyMission::DetectNearbyHerds()
{
    if (!PlayerCharacter || !bIsQuestActive)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Find all actors in the world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (!Actor || Actor == this || Actor == PlayerCharacter)
            {
                continue;
            }
            
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            
            // Check if this is a dinosaur actor
            bool bIsDinosaur = false;
            FString SpeciesName;
            
            for (const FString& Species : TargetSpecies)
            {
                if (ActorLabel.Contains(Species.ToLower()))
                {
                    bIsDinosaur = true;
                    SpeciesName = Species;
                    break;
                }
            }
            
            if (bIsDinosaur)
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                
                if (Distance >= MinObservationDistance && Distance <= MaxObservationDistance)
                {
                    // Analyze this herd actor
                    AnalyzeHerdBehavior(Actor);
                }
            }
        }
    }
}

void AQuest_HerdStudyMission::AnalyzeHerdBehavior(AActor* HerdActor)
{
    if (!HerdActor || !PlayerCharacter)
    {
        return;
    }
    
    FString ActorLabel = HerdActor->GetActorLabel();
    FString SpeciesName;
    
    // Determine species
    for (const FString& Species : TargetSpecies)
    {
        if (ActorLabel.ToLower().Contains(Species.ToLower()))
        {
            SpeciesName = Species;
            break;
        }
    }
    
    if (SpeciesName.IsEmpty())
    {
        return;
    }
    
    // Determine behavior type based on context
    EQuest_HerdStudyObjective BehaviorType;
    FVector ActorLocation = HerdActor->GetActorLocation();
    
    // Simple behavior detection logic
    static TMap<AActor*, float> ObservationStartTimes;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (!ObservationStartTimes.Contains(HerdActor))
    {
        ObservationStartTimes.Add(HerdActor, CurrentTime);
        return; // Need to observe for minimum duration
    }
    
    float ObservationDuration = CurrentTime - ObservationStartTimes[HerdActor];
    
    if (ObservationDuration >= MinObservationDuration)
    {
        // Determine behavior based on location and context
        if (ActorLocation.Z < 50.0f) // Near water/ground level
        {
            BehaviorType = EQuest_HerdStudyObjective::ObserveFeeding;
        }
        else if (FMath::Abs(ActorLocation.X) > 30000.0f || FMath::Abs(ActorLocation.Y) > 30000.0f)
        {
            BehaviorType = EQuest_HerdStudyObjective::ObserveMigration;
        }
        else
        {
            // Randomize between social behavior and defense
            BehaviorType = FMath::RandBool() ? EQuest_HerdStudyObjective::ObserveSocialBehavior : EQuest_HerdStudyObjective::ObserveDefense;
        }
        
        // Count nearby actors of same species for herd size
        int32 HerdSize = 1;
        if (UWorld* World = GetWorld())
        {
            TArray<AActor*> NearbyActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);
            
            for (AActor* NearbyActor : NearbyActors)
            {
                if (NearbyActor && NearbyActor != HerdActor)
                {
                    if (NearbyActor->GetActorLabel().ToLower().Contains(SpeciesName.ToLower()))
                    {
                        float Distance = FVector::Dist(ActorLocation, NearbyActor->GetActorLocation());
                        if (Distance <= 1500.0f) // Within herd range
                        {
                            HerdSize++;
                        }
                    }
                }
            }
        }
        
        // Register the observation
        RegisterHerdObservation(SpeciesName, BehaviorType, ActorLocation, ObservationDuration, HerdSize);
        
        // Remove from observation tracking
        ObservationStartTimes.Remove(HerdActor);
    }
}

void AQuest_HerdStudyMission::UpdateQuestTimer()
{
    if (bIsQuestActive && !bIsQuestComplete)
    {
        int32 MinutesRemaining = FMath::CeilToInt(QuestTimeRemaining / 60.0f);
        
        if (GEngine && MinutesRemaining <= 2) // Show warning in last 2 minutes
        {
            GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Yellow, 
                FString::Printf(TEXT("Quest Time Remaining: %d minutes"), MinutesRemaining));
        }
    }
}

void AQuest_HerdStudyMission::OnQuestTimeExpired()
{
    FailHerdStudyMission();
}

bool AQuest_HerdStudyMission::IsPlayerInObservationRange(const FVector& HerdLocation) const
{
    if (!PlayerCharacter)
    {
        return false;
    }
    
    float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), HerdLocation);
    return Distance >= MinObservationDistance && Distance <= MaxObservationDistance;
}

void AQuest_HerdStudyMission::ProcessHerdBehaviorData(AActor* HerdActor, EQuest_HerdStudyObjective ObjectiveType)
{
    // Additional processing for specific behavior types
    // This can be extended with more sophisticated analysis
}

void AQuest_HerdStudyMission::ValidateObservationRequirements()
{
    // Validate that observation requirements are reasonable
    if (RequiredObservations <= 0)
    {
        RequiredObservations = 3;
    }
    
    if (MinObservationDistance >= MaxObservationDistance)
    {
        MaxObservationDistance = MinObservationDistance + 1000.0f;
    }
}

void AQuest_HerdStudyMission::UpdateQuestObjectives()
{
    // Update quest objectives based on current progress
    // This can be extended with more dynamic objective updates
}

void AQuest_HerdStudyMission::NotifyQuestProgress()
{
    // Notify other systems about quest progress
    // This can be extended with event dispatchers or delegates
}