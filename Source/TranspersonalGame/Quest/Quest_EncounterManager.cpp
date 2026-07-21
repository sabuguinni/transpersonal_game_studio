#include "Quest_EncounterManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AQuest_EncounterManager::AQuest_EncounterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create encounter marker component
    EncounterMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EncounterMarker"));
    RootComponent = EncounterMarker;

    // Create trigger zone
    TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerZone"));
    TriggerZone->SetupAttachment(RootComponent);
    TriggerZone->SetSphereRadius(1000.0f);
    TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    bEncounterActive = false;
    EncounterRadius = 1000.0f;
    QuestDialogue = TEXT("A dangerous encounter awaits ahead. Are you prepared?");
    CompletionDialogue = TEXT("Well done, survivor! You have proven your courage.");

    // Set default encounter data
    EncounterData.EncounterName = TEXT("Default Encounter");
    EncounterData.DinosaurType = EQuest_DinosaurType::Velociraptor;
    EncounterData.SpawnLocation = FVector::ZeroVector;
    EncounterData.DinosaurCount = 1;
    EncounterData.bIsCompleted = false;
    EncounterData.DifficultyRating = 1.0f;

    // Set default rewards
    EncounterReward.ExperiencePoints = 100;
    EncounterReward.SurvivalBonus = 0.1f;
    EncounterReward.ItemRewards.Add(TEXT("Stone Fragment"));
}

void AQuest_EncounterManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEncounter();
    
    // Bind trigger events
    if (TriggerZone)
    {
        TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AQuest_EncounterManager::OnTriggerEnter);
    }
}

void AQuest_EncounterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEncounterActive)
    {
        UpdateEncounterState();
    }
}

void AQuest_EncounterManager::StartEncounter()
{
    if (bEncounterActive || EncounterData.bIsCompleted)
    {
        return;
    }

    if (!ValidateEncounterSetup())
    {
        UE_LOG(LogTemp, Warning, TEXT("Encounter setup validation failed"));
        return;
    }

    bEncounterActive = true;
    SpawnEncounterDinosaurs();
    
    // Broadcast encounter started event
    OnEncounterStarted();
    
    UE_LOG(LogTemp, Log, TEXT("Encounter started: %s"), *EncounterData.EncounterName);
}

void AQuest_EncounterManager::CompleteEncounter()
{
    if (!bEncounterActive)
    {
        return;
    }

    bEncounterActive = false;
    EncounterData.bIsCompleted = true;
    
    GiveRewards();
    CleanupEncounter();
    
    // Broadcast encounter completed event
    OnEncounterCompleted();
    
    UE_LOG(LogTemp, Log, TEXT("Encounter completed: %s"), *EncounterData.EncounterName);
}

void AQuest_EncounterManager::SpawnEncounterDinosaurs()
{
    if (!GetWorld())
    {
        return;
    }

    // Clear existing spawned dinosaurs
    for (AActor* Dinosaur : SpawnedDinosaurs)
    {
        if (IsValid(Dinosaur))
        {
            Dinosaur->Destroy();
        }
    }
    SpawnedDinosaurs.Empty();

    // Spawn new dinosaurs based on encounter data
    FVector BaseLocation = EncounterData.SpawnLocation.IsZero() ? GetActorLocation() : EncounterData.SpawnLocation;
    
    for (int32 i = 0; i < EncounterData.DinosaurCount; i++)
    {
        // Calculate spawn position with some randomization
        FVector SpawnOffset = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        FVector SpawnLocation = BaseLocation + SpawnOffset;
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);

        // Try to spawn dinosaur (placeholder for now - would use actual dinosaur classes)
        AActor* SpawnedDinosaur = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
        
        if (SpawnedDinosaur)
        {
            SpawnedDinosaur->SetActorLabel(FString::Printf(TEXT("EncounterDinosaur_%s_%d"), *EncounterData.EncounterName, i));
            SpawnedDinosaurs.Add(SpawnedDinosaur);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d dinosaurs for encounter: %s"), SpawnedDinosaurs.Num(), *EncounterData.EncounterName);
}

bool AQuest_EncounterManager::CheckEncounterCompletion()
{
    if (!bEncounterActive)
    {
        return false;
    }

    // Check if all spawned dinosaurs are defeated (simplified check)
    int32 AliveDinosaurs = 0;
    for (AActor* Dinosaur : SpawnedDinosaurs)
    {
        if (IsValid(Dinosaur))
        {
            AliveDinosaurs++;
        }
    }

    bool bCompleted = (AliveDinosaurs == 0);
    
    if (bCompleted && !EncounterData.bIsCompleted)
    {
        CompleteEncounter();
    }

    return bCompleted;
}

void AQuest_EncounterManager::GiveRewards()
{
    // Find player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    // Log reward giving (actual implementation would integrate with player stats system)
    UE_LOG(LogTemp, Log, TEXT("Giving rewards: %d XP, %.2f survival bonus"), 
           EncounterReward.ExperiencePoints, EncounterReward.SurvivalBonus);

    for (const FString& Item : EncounterReward.ItemRewards)
    {
        UE_LOG(LogTemp, Log, TEXT("Reward item: %s"), *Item);
    }

    // Display completion message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Quest Complete: %s - Gained %d XP!"), 
                          *EncounterData.EncounterName, EncounterReward.ExperiencePoints));
    }
}

void AQuest_EncounterManager::SetEncounterData(const FQuest_EncounterData& NewEncounterData)
{
    EncounterData = NewEncounterData;
    InitializeEncounter();
}

FQuest_EncounterData AQuest_EncounterManager::GetEncounterData() const
{
    return EncounterData;
}

void AQuest_EncounterManager::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if player entered trigger zone
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        if (!bEncounterActive && !EncounterData.bIsCompleted)
        {
            // Display quest dialogue
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, QuestDialogue);
            }

            // Auto-start encounter (could be changed to require player confirmation)
            StartEncounter();
        }
    }
}

void AQuest_EncounterManager::InitializeEncounter()
{
    if (TriggerZone)
    {
        TriggerZone->SetSphereRadius(EncounterRadius);
    }

    // Reset encounter state
    bEncounterActive = false;
    SpawnedDinosaurs.Empty();
}

void AQuest_EncounterManager::CleanupEncounter()
{
    // Clean up any remaining spawned actors
    for (AActor* Dinosaur : SpawnedDinosaurs)
    {
        if (IsValid(Dinosaur))
        {
            Dinosaur->Destroy();
        }
    }
    SpawnedDinosaurs.Empty();
}

bool AQuest_EncounterManager::ValidateEncounterSetup() const
{
    // Basic validation checks
    if (EncounterData.EncounterName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Encounter name is empty"));
        return false;
    }

    if (EncounterData.DinosaurCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid dinosaur count: %d"), EncounterData.DinosaurCount);
        return false;
    }

    if (EncounterData.DifficultyRating <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid difficulty rating: %.2f"), EncounterData.DifficultyRating);
        return false;
    }

    return true;
}

void AQuest_EncounterManager::UpdateEncounterState()
{
    // Continuously check for encounter completion
    CheckEncounterCompletion();
    
    // Additional state updates could be added here
    // (e.g., updating UI, checking player health, etc.)
}