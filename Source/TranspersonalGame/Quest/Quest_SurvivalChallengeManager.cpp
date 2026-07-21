#include "Quest_SurvivalChallengeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UQuest_SurvivalChallengeManager::UQuest_SurvivalChallengeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bAutoStartNextChallenge = false;
    ChallengeCheckInterval = 2.0f;
}

void UQuest_SurvivalChallengeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: BeginPlay - Challenge system initialized"));
}

void UQuest_SurvivalChallengeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentChallenge.bIsActive && !CurrentChallenge.bIsCompleted)
    {
        CheckChallengeCompletion();
        CheckChallengeTimeout();
    }
}

void UQuest_SurvivalChallengeManager::StartChallenge(EQuest_ChallengeType ChallengeType, EQuest_Difficulty Difficulty)
{
    if (CurrentChallenge.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalChallengeManager: Cannot start new challenge - one already active"));
        return;
    }

    switch (ChallengeType)
    {
        case EQuest_ChallengeType::HungerSurvival:
            CurrentChallenge = CreateHungerChallenge(Difficulty);
            break;
        case EQuest_ChallengeType::ThirstSurvival:
            CurrentChallenge = CreateThirstChallenge(Difficulty);
            break;
        case EQuest_ChallengeType::ResourceGather:
            CurrentChallenge = CreateResourceGatherChallenge(Difficulty);
            break;
        case EQuest_ChallengeType::ShelterBuild:
            CurrentChallenge = CreateShelterBuildChallenge(Difficulty);
            break;
        case EQuest_ChallengeType::PredatorEscape:
            CurrentChallenge = CreatePredatorEscapeChallenge(Difficulty);
            break;
        default:
            CurrentChallenge = CreateResourceGatherChallenge(Difficulty);
            break;
    }

    CurrentChallenge.bIsActive = true;
    CurrentChallenge.StartTime = GetWorld()->GetTimeSeconds();
    
    InitializeChallengeTimer();
    
    OnChallengeStarted.Broadcast(CurrentChallenge);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Started challenge - %s"), *CurrentChallenge.ChallengeName);
}

void UQuest_SurvivalChallengeManager::CompleteCurrentChallenge()
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    CurrentChallenge.bIsCompleted = true;
    CurrentChallenge.bIsActive = false;
    CompletedChallenges.Add(CurrentChallenge);
    
    CleanupChallengeTimer();
    
    OnChallengeCompleted.Broadcast(CurrentChallenge);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Completed challenge - %s"), *CurrentChallenge.ChallengeName);

    if (bAutoStartNextChallenge)
    {
        // Start a random new challenge after a delay
        FTimerHandle DelayHandle;
        GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this]()
        {
            EQuest_ChallengeType RandomType = static_cast<EQuest_ChallengeType>(FMath::RandRange(1, 10));
            StartChallenge(RandomType, EQuest_Difficulty::Beginner);
        }, 5.0f, false);
    }
}

void UQuest_SurvivalChallengeManager::FailCurrentChallenge()
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    CurrentChallenge.bIsActive = false;
    
    CleanupChallengeTimer();
    
    OnChallengeFailed.Broadcast(CurrentChallenge);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Failed challenge - %s"), *CurrentChallenge.ChallengeName);
}

void UQuest_SurvivalChallengeManager::CancelCurrentChallenge()
{
    if (CurrentChallenge.bIsActive)
    {
        CurrentChallenge.bIsActive = false;
        CleanupChallengeTimer();
        UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Cancelled challenge - %s"), *CurrentChallenge.ChallengeName);
    }
}

bool UQuest_SurvivalChallengeManager::HasActiveChallenge() const
{
    return CurrentChallenge.bIsActive;
}

FQuest_SurvivalChallenge UQuest_SurvivalChallengeManager::GetCurrentChallenge() const
{
    return CurrentChallenge;
}

void UQuest_SurvivalChallengeManager::UpdateChallengeProgress(int32 ProgressAmount)
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    CurrentChallenge.CurrentProgress += ProgressAmount;
    
    float ProgressPercent = GetChallengeProgressPercent();
    OnChallengeProgress.Broadcast(CurrentChallenge, ProgressPercent);
    
    if (CurrentChallenge.CurrentProgress >= CurrentChallenge.RequiredAmount)
    {
        CompleteCurrentChallenge();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Progress updated - %d/%d (%.1f%%)"), 
           CurrentChallenge.CurrentProgress, CurrentChallenge.RequiredAmount, ProgressPercent);
}

float UQuest_SurvivalChallengeManager::GetChallengeProgressPercent() const
{
    if (CurrentChallenge.RequiredAmount <= 0)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(static_cast<float>(CurrentChallenge.CurrentProgress) / static_cast<float>(CurrentChallenge.RequiredAmount) * 100.0f, 0.0f, 100.0f);
}

float UQuest_SurvivalChallengeManager::GetRemainingTime() const
{
    if (!CurrentChallenge.bIsActive)
    {
        return 0.0f;
    }
    
    float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentChallenge.StartTime;
    return FMath::Max(0.0f, CurrentChallenge.TimeLimit - ElapsedTime);
}

FQuest_SurvivalChallenge UQuest_SurvivalChallengeManager::CreateHungerChallenge(EQuest_Difficulty Difficulty)
{
    FQuest_SurvivalChallenge Challenge;
    Challenge.ChallengeName = TEXT("Hunger Survival");
    Challenge.Description = TEXT("Find and consume food before starvation");
    Challenge.ChallengeType = EQuest_ChallengeType::HungerSurvival;
    Challenge.Difficulty = Difficulty;
    Challenge.TimeLimit = 600.0f / GetDifficultyMultiplier(Difficulty);
    Challenge.RequiredAmount = FMath::RoundToInt(3 * GetDifficultyMultiplier(Difficulty));
    Challenge.TargetLocation = GetRandomLocationInBiome(EBiomeType::Savana);
    Challenge.TargetRadius = 2000.0f;
    
    return Challenge;
}

FQuest_SurvivalChallenge UQuest_SurvivalChallengeManager::CreateThirstChallenge(EQuest_Difficulty Difficulty)
{
    FQuest_SurvivalChallenge Challenge;
    Challenge.ChallengeName = TEXT("Thirst Survival");
    Challenge.Description = TEXT("Find clean water source before dehydration");
    Challenge.ChallengeType = EQuest_ChallengeType::ThirstSurvival;
    Challenge.Difficulty = Difficulty;
    Challenge.TimeLimit = 400.0f / GetDifficultyMultiplier(Difficulty);
    Challenge.RequiredAmount = FMath::RoundToInt(2 * GetDifficultyMultiplier(Difficulty));
    Challenge.TargetLocation = GetRandomLocationInBiome(EBiomeType::Forest);
    Challenge.TargetRadius = 1500.0f;
    
    return Challenge;
}

FQuest_SurvivalChallenge UQuest_SurvivalChallengeManager::CreateResourceGatherChallenge(EQuest_Difficulty Difficulty)
{
    FQuest_SurvivalChallenge Challenge;
    Challenge.ChallengeName = TEXT("Resource Gathering");
    Challenge.Description = TEXT("Collect essential survival materials");
    Challenge.ChallengeType = EQuest_ChallengeType::ResourceGather;
    Challenge.Difficulty = Difficulty;
    Challenge.TimeLimit = 900.0f / GetDifficultyMultiplier(Difficulty);
    Challenge.RequiredAmount = FMath::RoundToInt(10 * GetDifficultyMultiplier(Difficulty));
    Challenge.TargetLocation = GetRandomLocationInBiome(EBiomeType::Savana);
    Challenge.TargetRadius = 3000.0f;
    
    return Challenge;
}

FQuest_SurvivalChallenge UQuest_SurvivalChallengeManager::CreateShelterBuildChallenge(EQuest_Difficulty Difficulty)
{
    FQuest_SurvivalChallenge Challenge;
    Challenge.ChallengeName = TEXT("Shelter Construction");
    Challenge.Description = TEXT("Build adequate shelter for protection");
    Challenge.ChallengeType = EQuest_ChallengeType::ShelterBuild;
    Challenge.Difficulty = Difficulty;
    Challenge.TimeLimit = 1200.0f / GetDifficultyMultiplier(Difficulty);
    Challenge.RequiredAmount = 1;
    Challenge.TargetLocation = GetRandomLocationInBiome(EBiomeType::Forest);
    Challenge.TargetRadius = 1000.0f;
    
    return Challenge;
}

FQuest_SurvivalChallenge UQuest_SurvivalChallengeManager::CreatePredatorEscapeChallenge(EQuest_Difficulty Difficulty)
{
    FQuest_SurvivalChallenge Challenge;
    Challenge.ChallengeName = TEXT("Predator Escape");
    Challenge.Description = TEXT("Survive predator encounter and reach safety");
    Challenge.ChallengeType = EQuest_ChallengeType::PredatorEscape;
    Challenge.Difficulty = Difficulty;
    Challenge.TimeLimit = 300.0f / GetDifficultyMultiplier(Difficulty);
    Challenge.RequiredAmount = 1;
    Challenge.TargetLocation = GetRandomLocationInBiome(EBiomeType::Desert);
    Challenge.TargetRadius = 500.0f;
    
    return Challenge;
}

void UQuest_SurvivalChallengeManager::CheckLocationObjective(FVector PlayerLocation)
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    float Distance = FVector::Dist(PlayerLocation, CurrentChallenge.TargetLocation);
    if (Distance <= CurrentChallenge.TargetRadius)
    {
        if (CurrentChallenge.ChallengeType == EQuest_ChallengeType::PredatorEscape)
        {
            UpdateChallengeProgress(1);
        }
    }
}

void UQuest_SurvivalChallengeManager::RegisterResourceCollection(const FString& ResourceType)
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    if (CurrentChallenge.ChallengeType == EQuest_ChallengeType::ResourceGather)
    {
        UpdateChallengeProgress(1);
        UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Resource collected - %s"), *ResourceType);
    }
}

void UQuest_SurvivalChallengeManager::RegisterCraftingActivity(const FString& ItemCrafted)
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    if (CurrentChallenge.ChallengeType == EQuest_ChallengeType::ToolCraft)
    {
        UpdateChallengeProgress(1);
        UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Item crafted - %s"), *ItemCrafted);
    }
}

void UQuest_SurvivalChallengeManager::RegisterShelterBuilding()
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    if (CurrentChallenge.ChallengeType == EQuest_ChallengeType::ShelterBuild)
    {
        UpdateChallengeProgress(1);
        UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Shelter built"));
    }
}

void UQuest_SurvivalChallengeManager::RegisterPredatorEncounter()
{
    if (!CurrentChallenge.bIsActive)
    {
        return;
    }

    if (CurrentChallenge.ChallengeType == EQuest_ChallengeType::PredatorEscape)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalChallengeManager: Predator encountered - escape challenge activated"));
    }
}

void UQuest_SurvivalChallengeManager::StartBiomeChallenge(EBiomeType BiomeType)
{
    TArray<EQuest_ChallengeType> BiomeChallenges;
    
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BiomeChallenges = {EQuest_ChallengeType::HungerSurvival, EQuest_ChallengeType::ResourceGather, EQuest_ChallengeType::PredatorEscape};
            break;
        case EBiomeType::Forest:
            BiomeChallenges = {EQuest_ChallengeType::ShelterBuild, EQuest_ChallengeType::ThirstSurvival, EQuest_ChallengeType::ToolCraft};
            break;
        case EBiomeType::Desert:
            BiomeChallenges = {EQuest_ChallengeType::ThirstSurvival, EQuest_ChallengeType::ColdSurvival, EQuest_ChallengeType::PredatorEscape};
            break;
        case EBiomeType::Swamp:
            BiomeChallenges = {EQuest_ChallengeType::WaterFind, EQuest_ChallengeType::ResourceGather, EQuest_ChallengeType::ShelterBuild};
            break;
        case EBiomeType::Mountain:
            BiomeChallenges = {EQuest_ChallengeType::ColdSurvival, EQuest_ChallengeType::ShelterBuild, EQuest_ChallengeType::ToolCraft};
            break;
        default:
            BiomeChallenges = {EQuest_ChallengeType::ResourceGather};
            break;
    }

    if (BiomeChallenges.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, BiomeChallenges.Num() - 1);
        StartChallenge(BiomeChallenges[RandomIndex], EQuest_Difficulty::Beginner);
    }
}

TArray<FQuest_SurvivalChallenge> UQuest_SurvivalChallengeManager::GetAvailableChallengesForBiome(EBiomeType BiomeType)
{
    TArray<FQuest_SurvivalChallenge> AvailableChallenges;
    
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            AvailableChallenges.Add(CreateHungerChallenge(EQuest_Difficulty::Beginner));
            AvailableChallenges.Add(CreateResourceGatherChallenge(EQuest_Difficulty::Beginner));
            AvailableChallenges.Add(CreatePredatorEscapeChallenge(EQuest_Difficulty::Novice));
            break;
        case EBiomeType::Forest:
            AvailableChallenges.Add(CreateShelterBuildChallenge(EQuest_Difficulty::Beginner));
            AvailableChallenges.Add(CreateThirstChallenge(EQuest_Difficulty::Beginner));
            break;
        case EBiomeType::Desert:
            AvailableChallenges.Add(CreateThirstChallenge(EQuest_Difficulty::Experienced));
            AvailableChallenges.Add(CreatePredatorEscapeChallenge(EQuest_Difficulty::Expert));
            break;
        default:
            AvailableChallenges.Add(CreateResourceGatherChallenge(EQuest_Difficulty::Beginner));
            break;
    }
    
    return AvailableChallenges;
}

void UQuest_SurvivalChallengeManager::CheckChallengeCompletion()
{
    if (CurrentChallenge.CurrentProgress >= CurrentChallenge.RequiredAmount)
    {
        CompleteCurrentChallenge();
    }
}

void UQuest_SurvivalChallengeManager::CheckChallengeTimeout()
{
    if (GetRemainingTime() <= 0.0f)
    {
        FailCurrentChallenge();
    }
}

void UQuest_SurvivalChallengeManager::InitializeChallengeTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(ProgressCheckHandle, this, &UQuest_SurvivalChallengeManager::CheckChallengeCompletion, ChallengeCheckInterval, true);
    }
}

void UQuest_SurvivalChallengeManager::CleanupChallengeTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ProgressCheckHandle);
        GetWorld()->GetTimerManager().ClearTimer(ChallengeTimerHandle);
    }
}

FVector UQuest_SurvivalChallengeManager::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector BaseLocation;
    float RandomRadius = 5000.0f;
    
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BaseLocation = FVector(0.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Forest:
            BaseLocation = FVector(-45000.0f, 40000.0f, 100.0f);
            break;
        case EBiomeType::Desert:
            BaseLocation = FVector(55000.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Swamp:
            BaseLocation = FVector(-50000.0f, -45000.0f, 100.0f);
            break;
        case EBiomeType::Mountain:
            BaseLocation = FVector(40000.0f, 50000.0f, 500.0f);
            break;
        default:
            BaseLocation = FVector::ZeroVector;
            break;
    }
    
    FVector RandomOffset = FVector(
        FMath::RandRange(-RandomRadius, RandomRadius),
        FMath::RandRange(-RandomRadius, RandomRadius),
        0.0f
    );
    
    return BaseLocation + RandomOffset;
}

float UQuest_SurvivalChallengeManager::GetDifficultyMultiplier(EQuest_Difficulty Difficulty)
{
    switch (Difficulty)
    {
        case EQuest_Difficulty::Beginner:
            return 0.5f;
        case EQuest_Difficulty::Novice:
            return 0.75f;
        case EQuest_Difficulty::Experienced:
            return 1.0f;
        case EQuest_Difficulty::Expert:
            return 1.5f;
        case EQuest_Difficulty::Master:
            return 2.0f;
        case EQuest_Difficulty::Legendary:
            return 3.0f;
        default:
            return 1.0f;
    }
}