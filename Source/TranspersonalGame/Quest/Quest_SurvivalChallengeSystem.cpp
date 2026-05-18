#include "Quest_SurvivalChallengeSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UQuest_SurvivalChallengeSystem::UQuest_SurvivalChallengeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    ChallengeCheckInterval = 5.0f;
    MaxActiveChallenges = 3;
}

void UQuest_SurvivalChallengeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Start challenge monitoring timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ChallengeTimerHandle,
            this,
            &UQuest_SurvivalChallengeSystem::OnChallengeTimer,
            ChallengeCheckInterval,
            true
        );
    }

    // Create initial survival challenges
    CreateNightSurvivalChallenge();
    CreateHungerChallenge();
}

void UQuest_SurvivalChallengeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update challenge timers
    for (FQuest_SurvivalChallenge& Challenge : ActiveChallenges)
    {
        if (!Challenge.bIsCompleted)
        {
            Challenge.TimeLimit -= DeltaTime;
            if (Challenge.TimeLimit <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Challenge '%s' expired!"), *Challenge.ChallengeName);
            }
        }
    }
    
    RemoveExpiredChallenges();
}

void UQuest_SurvivalChallengeSystem::StartSurvivalChallenge(const FString& ChallengeName, ESurvivalStat StatType, float Target, float Duration, FVector Location)
{
    if (ActiveChallenges.Num() >= MaxActiveChallenges)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start challenge '%s' - too many active challenges"), *ChallengeName);
        return;
    }

    FQuest_SurvivalChallenge NewChallenge;
    NewChallenge.ChallengeName = ChallengeName;
    NewChallenge.RequiredStat = StatType;
    NewChallenge.TargetValue = Target;
    NewChallenge.TimeLimit = Duration;
    NewChallenge.bIsCompleted = false;
    NewChallenge.ChallengeLocation = Location;

    ActiveChallenges.Add(NewChallenge);
    SpawnChallengeMarker(Location, ChallengeName);

    UE_LOG(LogTemp, Log, TEXT("Started survival challenge: %s"), *ChallengeName);
}

void UQuest_SurvivalChallengeSystem::CheckChallengeProgress()
{
    // This would typically check player stats against challenge requirements
    // For now, we'll simulate progress checking
    for (FQuest_SurvivalChallenge& Challenge : ActiveChallenges)
    {
        if (!Challenge.bIsCompleted)
        {
            // Simulate checking player stats (would integrate with character system)
            float CurrentStatValue = 75.0f; // Placeholder - would get from player character
            
            if (CurrentStatValue >= Challenge.TargetValue)
            {
                Challenge.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("Challenge completed: %s"), *Challenge.ChallengeName);
            }
        }
    }
}

bool UQuest_SurvivalChallengeSystem::CompleteSurvivalChallenge(const FString& ChallengeName)
{
    for (FQuest_SurvivalChallenge& Challenge : ActiveChallenges)
    {
        if (Challenge.ChallengeName == ChallengeName && !Challenge.bIsCompleted)
        {
            Challenge.bIsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("Manually completed challenge: %s"), *ChallengeName);
            return true;
        }
    }
    return false;
}

TArray<FQuest_SurvivalChallenge> UQuest_SurvivalChallengeSystem::GetActiveChallenges() const
{
    return ActiveChallenges;
}

void UQuest_SurvivalChallengeSystem::CreateNightSurvivalChallenge()
{
    FVector NightLocation = FVector(2000.0f, 2000.0f, 100.0f); // Savana biome
    StartSurvivalChallenge(
        TEXT("Survive the Night"),
        ESurvivalStat::Health,
        30.0f,
        600.0f, // 10 minutes
        NightLocation
    );
}

void UQuest_SurvivalChallengeSystem::CreateHungerChallenge()
{
    FVector HuntLocation = FVector(5000.0f, 3000.0f, 100.0f); // Savana hunting grounds
    StartSurvivalChallenge(
        TEXT("Find Food Before Starvation"),
        ESurvivalStat::Hunger,
        60.0f,
        480.0f, // 8 minutes
        HuntLocation
    );
}

void UQuest_SurvivalChallengeSystem::CreateThirstChallenge()
{
    FVector WaterLocation = FVector(-1000.0f, 4000.0f, 50.0f); // Near water source
    StartSurvivalChallenge(
        TEXT("Find Water Source"),
        ESurvivalStat::Thirst,
        70.0f,
        360.0f, // 6 minutes
        WaterLocation
    );
}

void UQuest_SurvivalChallengeSystem::CreateTemperatureChallenge()
{
    FVector ShelterLocation = FVector(3000.0f, -2000.0f, 150.0f); // Higher ground
    StartSurvivalChallenge(
        TEXT("Build Shelter from Cold"),
        ESurvivalStat::Temperature,
        40.0f,
        720.0f, // 12 minutes
        ShelterLocation
    );
}

void UQuest_SurvivalChallengeSystem::OnChallengeTimer()
{
    CheckChallengeProgress();
    
    // Randomly create new challenges if we have room
    if (ActiveChallenges.Num() < MaxActiveChallenges && FMath::RandRange(0, 100) < 20)
    {
        int32 ChallengeType = FMath::RandRange(0, 3);
        switch (ChallengeType)
        {
        case 0:
            CreateThirstChallenge();
            break;
        case 1:
            CreateTemperatureChallenge();
            break;
        case 2:
            CreateHungerChallenge();
            break;
        case 3:
            CreateNightSurvivalChallenge();
            break;
        }
    }
}

void UQuest_SurvivalChallengeSystem::SpawnChallengeMarker(FVector Location, const FString& ChallengeName)
{
    if (UWorld* World = GetWorld())
    {
        // Spawn a visual marker for the challenge location
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("ChallengeMarker_%s"), *ChallengeName));
        
        if (AStaticMeshActor* Marker = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams))
        {
            Marker->SetActorLabel(FString::Printf(TEXT("Challenge: %s"), *ChallengeName));
            UE_LOG(LogTemp, Log, TEXT("Spawned challenge marker for: %s at %s"), *ChallengeName, *Location.ToString());
        }
    }
}

void UQuest_SurvivalChallengeSystem::RemoveExpiredChallenges()
{
    ActiveChallenges.RemoveAll([](const FQuest_SurvivalChallenge& Challenge)
    {
        return Challenge.TimeLimit <= 0.0f || Challenge.bIsCompleted;
    });
}