#include "Quest_DinosaurHuntSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UQuest_DinosaurHuntSystem::UQuest_DinosaurHuntSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    HuntDetectionRadius = 5000.0f;
    bAutoTrackNearbyDinosaurs = true;
}

void UQuest_DinosaurHuntSystem::BeginPlay()
{
    Super::BeginPlay();
    CreateBasicHuntMissions();
}

void UQuest_DinosaurHuntSystem::StartDinosaurHunt(const FQuest_DinosaurTarget& Target)
{
    if (CurrentHunt.bHuntActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hunt already active! End current hunt first."));
        return;
    }

    CurrentHunt.bHuntActive = true;
    CurrentHunt.CurrentKills = 0;
    CurrentHunt.TimeRemaining = Target.TimeLimit;
    CurrentHunt.TrackedDinosaurs.Empty();

    // Start hunt timer
    if (Target.TimeLimit > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            HuntTimerHandle,
            this,
            &UQuest_DinosaurHuntSystem::OnHuntTimerExpired,
            Target.TimeLimit,
            false
        );
    }

    // Scan for nearby dinosaurs if auto-tracking enabled
    if (bAutoTrackNearbyDinosaurs)
    {
        ScanForNearbyDinosaurs();
    }

    OnHuntStarted(Target);
    UE_LOG(LogTemp, Log, TEXT("Dinosaur hunt started: %s"), 
           *UEnum::GetValueAsString(Target.Species));
}

void UQuest_DinosaurHuntSystem::EndCurrentHunt(bool bSuccess)
{
    if (!CurrentHunt.bHuntActive)
    {
        return;
    }

    // Clear timer
    if (HuntTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(HuntTimerHandle);
    }

    int32 FinalKillCount = CurrentHunt.CurrentKills;
    CurrentHunt.bHuntActive = false;
    CurrentHunt.CurrentKills = 0;
    CurrentHunt.TimeRemaining = 0.0f;
    CurrentHunt.TrackedDinosaurs.Empty();

    OnHuntCompleted(bSuccess, FinalKillCount);
    UE_LOG(LogTemp, Log, TEXT("Hunt ended. Success: %s, Kills: %d"), 
           bSuccess ? TEXT("true") : TEXT("false"), FinalKillCount);
}

void UQuest_DinosaurHuntSystem::RegisterDinosaurKill(AActor* DinosaurActor)
{
    if (!CurrentHunt.bHuntActive || !DinosaurActor)
    {
        return;
    }

    CurrentHunt.CurrentKills++;
    OnDinosaurKilled(DinosaurActor);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur killed! Total kills: %d"), CurrentHunt.CurrentKills);
    
    CheckHuntCompletion();
}

bool UQuest_DinosaurHuntSystem::IsHuntActive() const
{
    return CurrentHunt.bHuntActive;
}

FQuest_HuntProgress UQuest_DinosaurHuntSystem::GetCurrentHuntProgress() const
{
    return CurrentHunt;
}

TArray<FQuest_DinosaurTarget> UQuest_DinosaurHuntSystem::GetAvailableHunts() const
{
    return AvailableHunts;
}

void UQuest_DinosaurHuntSystem::CreateBasicHuntMissions()
{
    AvailableHunts.Empty();

    // Easy Raptor Hunt
    FQuest_DinosaurTarget RaptorHunt;
    RaptorHunt.Species = EQuest_DinosaurSpecies::Raptor;
    RaptorHunt.RequiredKills = 2;
    RaptorHunt.Difficulty = EQuest_HuntDifficulty::Easy;
    RaptorHunt.HuntLocation = FVector(1000, 1000, 100); // Savana
    RaptorHunt.TimeLimit = 600.0f; // 10 minutes
    AvailableHunts.Add(RaptorHunt);

    // Medium Triceratops Hunt
    FQuest_DinosaurTarget TriceratopsHunt;
    TriceratopsHunt.Species = EQuest_DinosaurSpecies::Triceratops;
    TriceratopsHunt.RequiredKills = 1;
    TriceratopsHunt.Difficulty = EQuest_HuntDifficulty::Medium;
    TriceratopsHunt.HuntLocation = FVector(5000, 5000, 100); // Savana
    TriceratopsHunt.TimeLimit = 900.0f; // 15 minutes
    AvailableHunts.Add(TriceratopsHunt);

    // Hard Ankylosaurus Hunt
    FQuest_DinosaurTarget AnkyloHunt;
    AnkyloHunt.Species = EQuest_DinosaurSpecies::Ankylo;
    AnkyloHunt.RequiredKills = 1;
    AnkyloHunt.Difficulty = EQuest_HuntDifficulty::Hard;
    AnkyloHunt.HuntLocation = FVector(-45000, 40000, 100); // Forest
    AnkyloHunt.TimeLimit = 1200.0f; // 20 minutes
    AvailableHunts.Add(AnkyloHunt);

    // Legendary T-Rex Hunt
    FQuest_DinosaurTarget TRexHunt;
    TRexHunt.Species = EQuest_DinosaurSpecies::TRex;
    TRexHunt.RequiredKills = 1;
    TRexHunt.Difficulty = EQuest_HuntDifficulty::Legendary;
    TRexHunt.HuntLocation = FVector(10000, 10000, 100); // Savana
    TRexHunt.TimeLimit = 1800.0f; // 30 minutes
    AvailableHunts.Add(TRexHunt);

    UE_LOG(LogTemp, Log, TEXT("Created %d basic hunt missions"), AvailableHunts.Num());
}

void UQuest_DinosaurHuntSystem::ScanForNearbyDinosaurs()
{
    if (!GetWorld())
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    TArray<AActor*> FoundActors;
    
    // Get all actors in the world
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    CurrentHunt.TrackedDinosaurs.Empty();
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName();
        
        // Check if actor is a dinosaur (simple name-based check)
        if (ActorName.Contains(TEXT("Rex")) || 
            ActorName.Contains(TEXT("Raptor")) || 
            ActorName.Contains(TEXT("Triceratops")) ||
            ActorName.Contains(TEXT("Ankylo")) ||
            ActorName.Contains(TEXT("Brachio")))
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= HuntDetectionRadius)
            {
                CurrentHunt.TrackedDinosaurs.Add(Actor);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Scanned and found %d nearby dinosaurs"), 
           CurrentHunt.TrackedDinosaurs.Num());
}

void UQuest_DinosaurHuntSystem::OnHuntTimerExpired()
{
    UE_LOG(LogTemp, Warning, TEXT("Hunt timer expired!"));
    EndCurrentHunt(false); // Failed due to timeout
}

void UQuest_DinosaurHuntSystem::UpdateHuntProgress()
{
    if (!CurrentHunt.bHuntActive)
    {
        return;
    }

    // Update remaining time
    if (HuntTimerHandle.IsValid())
    {
        CurrentHunt.TimeRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(HuntTimerHandle);
    }
}

void UQuest_DinosaurHuntSystem::CheckHuntCompletion()
{
    if (!CurrentHunt.bHuntActive)
    {
        return;
    }

    // For now, simple completion check - this would be expanded to check specific targets
    if (CurrentHunt.CurrentKills >= 1) // Basic completion threshold
    {
        EndCurrentHunt(true);
    }
}