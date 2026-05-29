#include "Quest_HuntingSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AQuest_HuntingSystem::AQuest_HuntingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger zone component
    TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerZone"));
    RootComponent = TriggerZone;
    TriggerZone->SetSphereRadius(500.0f);
    TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerZone->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    TriggerZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerZone->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create quest marker mesh
    QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
    QuestMarker->SetupAttachment(RootComponent);
    QuestMarker->SetRelativeLocation(FVector(0, 0, 200));
    QuestMarker->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.1f));

    // Initialize quest parameters
    bQuestActive = false;
    QuestRadius = 2000.0f;
    HuntingGrounds = FVector(0, 0, 100);
    TimeLimit = 600.0f; // 10 minutes
    RemainingTime = TimeLimit;

    // Setup default hunt targets
    FQuest_HuntTarget VelociraptorHunt;
    VelociraptorHunt.DinosaurSpecies = TEXT("Velociraptor");
    VelociraptorHunt.RequiredKills = 3;
    VelociraptorHunt.CurrentKills = 0;
    VelociraptorHunt.RewardMeat = 15.0f;
    VelociraptorHunt.RewardHide = 5.0f;
    ActiveHuntTargets.Add(VelociraptorHunt);

    FQuest_HuntTarget TricerHunt;
    TricerHunt.DinosaurSpecies = TEXT("Triceratops");
    TricerHunt.RequiredKills = 1;
    TricerHunt.CurrentKills = 0;
    TricerHunt.RewardMeat = 50.0f;
    TricerHunt.RewardHide = 20.0f;
    ActiveHuntTargets.Add(TricerHunt);
}

void AQuest_HuntingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind trigger events
    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AQuest_HuntingSystem::OnTriggerEnter);
    TriggerZone->OnComponentEndOverlap.AddDynamic(this, &AQuest_HuntingSystem::OnTriggerExit);

    UE_LOG(LogTemp, Warning, TEXT("Quest_HuntingSystem initialized at %s"), *GetActorLocation().ToString());
}

void AQuest_HuntingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bQuestActive && RemainingTime > 0)
    {
        RemainingTime -= DeltaTime;
        
        if (RemainingTime <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Hunt quest time expired!"));
            CompleteHuntQuest();
        }
    }
}

void AQuest_HuntingSystem::StartHuntQuest()
{
    if (bQuestActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hunt quest already active"));
        return;
    }

    bQuestActive = true;
    RemainingTime = TimeLimit;

    // Reset all hunt targets
    for (FQuest_HuntTarget& Target : ActiveHuntTargets)
    {
        Target.CurrentKills = 0;
    }

    SpawnHuntTargets();

    // Start quest timer
    GetWorldTimerManager().SetTimer(QuestTimerHandle, this, &AQuest_HuntingSystem::UpdateQuestTimer, 1.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("Hunt quest started! Time limit: %.0f seconds"), TimeLimit);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("HUNT QUEST STARTED! Eliminate targets within %.0f seconds"), TimeLimit));
    }
}

void AQuest_HuntingSystem::RegisterDinosaurKill(const FString& Species)
{
    if (!bQuestActive)
    {
        return;
    }

    for (FQuest_HuntTarget& Target : ActiveHuntTargets)
    {
        if (Target.DinosaurSpecies == Species && Target.CurrentKills < Target.RequiredKills)
        {
            Target.CurrentKills++;
            UE_LOG(LogTemp, Warning, TEXT("Killed %s: %d/%d"), *Species, Target.CurrentKills, Target.RequiredKills);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                    FString::Printf(TEXT("%s killed: %d/%d"), *Species, Target.CurrentKills, Target.RequiredKills));
            }

            if (CheckQuestCompletion())
            {
                CompleteHuntQuest();
            }
            break;
        }
    }
}

bool AQuest_HuntingSystem::CheckQuestCompletion()
{
    for (const FQuest_HuntTarget& Target : ActiveHuntTargets)
    {
        if (Target.CurrentKills < Target.RequiredKills)
        {
            return false;
        }
    }
    return true;
}

void AQuest_HuntingSystem::CompleteHuntQuest()
{
    if (!bQuestActive)
    {
        return;
    }

    bQuestActive = false;
    GetWorldTimerManager().ClearTimer(QuestTimerHandle);

    // Calculate total rewards
    float TotalMeat = 0.0f;
    float TotalHide = 0.0f;
    bool QuestSuccess = CheckQuestCompletion();

    if (QuestSuccess)
    {
        for (const FQuest_HuntTarget& Target : ActiveHuntTargets)
        {
            if (Target.CurrentKills >= Target.RequiredKills)
            {
                TotalMeat += Target.RewardMeat;
                TotalHide += Target.RewardHide;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Hunt quest completed successfully! Rewards: %.1f meat, %.1f hide"), TotalMeat, TotalHide);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                FString::Printf(TEXT("QUEST COMPLETE! Rewards: %.1f Meat, %.1f Hide"), TotalMeat, TotalHide));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Hunt quest failed - time expired or targets not eliminated"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("QUEST FAILED - Time expired!"));
        }
    }

    // Clean up spawned targets
    for (AActor* Target : SpawnedTargets)
    {
        if (IsValid(Target))
        {
            Target->Destroy();
        }
    }
    SpawnedTargets.Empty();
}

void AQuest_HuntingSystem::SpawnHuntTargets()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Clear existing spawned targets
    for (AActor* Target : SpawnedTargets)
    {
        if (IsValid(Target))
        {
            Target->Destroy();
        }
    }
    SpawnedTargets.Empty();

    // Spawn hunt targets around the hunting grounds
    for (const FQuest_HuntTarget& Target : ActiveHuntTargets)
    {
        for (int32 i = 0; i < Target.RequiredKills; i++)
        {
            // Calculate spawn position around hunting grounds
            float Angle = FMath::RandRange(0.0f, 360.0f);
            float Distance = FMath::RandRange(500.0f, QuestRadius);
            FVector SpawnLocation = HuntingGrounds + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
                0
            );

            // Try to spawn target (placeholder for now - would spawn actual dinosaur actors)
            UE_LOG(LogTemp, Warning, TEXT("Spawning %s at %s"), *Target.DinosaurSpecies, *SpawnLocation.ToString());
        }
    }
}

void AQuest_HuntingSystem::UpdateQuestTimer()
{
    if (bQuestActive && GEngine)
    {
        int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
        int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;
        
        GEngine->AddOnScreenDebugMessage(100, 1.0f, FColor::White, 
            FString::Printf(TEXT("Hunt Quest Time: %02d:%02d"), Minutes, Seconds));
    }
}

void AQuest_HuntingSystem::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Player = Cast<ACharacter>(OtherActor))
    {
        if (!bQuestActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player entered hunt quest zone"));
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
                    TEXT("Press [H] to start Hunt Quest"));
            }
        }
    }
}

void AQuest_HuntingSystem::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (ACharacter* Player = Cast<ACharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player left hunt quest zone"));
    }
}