#include "Quest_SurvivalQuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UQuest_SurvivalQuestManager::UQuest_SurvivalQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    HealthThreshold = 0.3f;
    HungerThreshold = 0.2f;
    ThirstThreshold = 0.2f;
    TemperatureRange = 10.0f;
    bAutoGenerateChallenges = true;
    ChallengeGenerationInterval = 180.0f; // 3 minutes
    LastChallengeTime = 0.0f;
}

void UQuest_SurvivalQuestManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default survival objectives
    AddSurvivalObjective(TEXT("Maintain health above 30%"), 1, 50.0f);
    AddSurvivalObjective(TEXT("Keep hunger below critical level"), 1, 75.0f);
    AddSurvivalObjective(TEXT("Maintain hydration"), 1, 75.0f);
    AddSurvivalObjective(TEXT("Survive for 10 minutes"), 600, 200.0f);

    UE_LOG(LogTemp, Log, TEXT("SurvivalQuestManager: Initialized with %d global objectives"), GlobalObjectives.Num());
}

void UQuest_SurvivalQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ProcessActiveChallenges(DeltaTime);
    
    if (bAutoGenerateChallenges)
    {
        CheckAutoGeneration(DeltaTime);
    }

    // Update survival time objective
    for (auto& Objective : GlobalObjectives)
    {
        if (Objective.ObjectiveDescription.Contains(TEXT("Survive for")) && !Objective.bIsCompleted)
        {
            UpdateObjectiveProgress(Objective, static_cast<int32>(DeltaTime));
        }
    }
}

void UQuest_SurvivalQuestManager::StartSurvivalChallenge(const FString& ChallengeName, float TimeLimit, const FVector& Location)
{
    FQuest_SurvivalChallenge NewChallenge;
    NewChallenge.ChallengeName = ChallengeName;
    NewChallenge.TimeLimit = TimeLimit;
    NewChallenge.CurrentTime = 0.0f;
    NewChallenge.bIsActive = true;
    NewChallenge.ChallengeLocation = Location;

    // Add challenge-specific objectives
    if (ChallengeName.Contains(TEXT("Extreme Weather")))
    {
        FQuest_SurvivalObjective TempObjective;
        TempObjective.ObjectiveDescription = TEXT("Survive extreme temperature");
        TempObjective.TargetValue = static_cast<int32>(TimeLimit);
        TempObjective.RewardExperience = 150.0f;
        NewChallenge.Objectives.Add(TempObjective);
    }
    else if (ChallengeName.Contains(TEXT("Predator Territory")))
    {
        FQuest_SurvivalObjective PredatorObjective;
        PredatorObjective.ObjectiveDescription = TEXT("Avoid or defeat predators");
        PredatorObjective.TargetValue = 3;
        PredatorObjective.RewardExperience = 200.0f;
        NewChallenge.Objectives.Add(PredatorObjective);
    }
    else if (ChallengeName.Contains(TEXT("Resource Scarcity")))
    {
        FQuest_SurvivalObjective ResourceObjective;
        ResourceObjective.ObjectiveDescription = TEXT("Find water and food sources");
        ResourceObjective.TargetValue = 2;
        ResourceObjective.RewardExperience = 125.0f;
        NewChallenge.Objectives.Add(ResourceObjective);
    }

    ActiveChallenges.Add(NewChallenge);
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalQuestManager: Started challenge '%s' for %.1f seconds at location %s"), 
           *ChallengeName, TimeLimit, *Location.ToString());

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("SURVIVAL CHALLENGE: %s"), *ChallengeName));
    }
}

void UQuest_SurvivalQuestManager::EndSurvivalChallenge(bool bSuccess)
{
    for (int32 i = ActiveChallenges.Num() - 1; i >= 0; --i)
    {
        if (ActiveChallenges[i].bIsActive)
        {
            FQuest_SurvivalChallenge& Challenge = ActiveChallenges[i];
            Challenge.bIsActive = false;

            if (bSuccess)
            {
                float TotalReward = 0.0f;
                for (const auto& Objective : Challenge.Objectives)
                {
                    if (Objective.bIsCompleted)
                    {
                        TotalReward += Objective.RewardExperience;
                    }
                }
                GrantSurvivalRewards(TotalReward);

                UE_LOG(LogTemp, Log, TEXT("SurvivalQuestManager: Challenge '%s' completed successfully! Reward: %.1f XP"), 
                       *Challenge.ChallengeName, TotalReward);

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                        FString::Printf(TEXT("CHALLENGE COMPLETED: %s (+%.0f XP)"), *Challenge.ChallengeName, TotalReward));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SurvivalQuestManager: Challenge '%s' failed"), *Challenge.ChallengeName);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                        FString::Printf(TEXT("CHALLENGE FAILED: %s"), *Challenge.ChallengeName));
                }
            }

            ActiveChallenges.RemoveAt(i);
            break;
        }
    }
}

void UQuest_SurvivalQuestManager::UpdateSurvivalProgress(const FString& ObjectiveType, int32 ProgressAmount)
{
    // Update global objectives
    for (auto& Objective : GlobalObjectives)
    {
        if (Objective.ObjectiveDescription.Contains(ObjectiveType) && !Objective.bIsCompleted)
        {
            UpdateObjectiveProgress(Objective, ProgressAmount);
        }
    }

    // Update active challenge objectives
    for (auto& Challenge : ActiveChallenges)
    {
        if (Challenge.bIsActive)
        {
            for (auto& Objective : Challenge.Objectives)
            {
                if (Objective.ObjectiveDescription.Contains(ObjectiveType) && !Objective.bIsCompleted)
                {
                    UpdateObjectiveProgress(Objective, ProgressAmount);
                }
            }
        }
    }
}

void UQuest_SurvivalQuestManager::AddSurvivalObjective(const FString& Description, int32 TargetValue, float RewardXP)
{
    FQuest_SurvivalObjective NewObjective;
    NewObjective.ObjectiveDescription = Description;
    NewObjective.TargetValue = TargetValue;
    NewObjective.RewardExperience = RewardXP;
    NewObjective.QuestType = EQuestType::Survival;

    GlobalObjectives.Add(NewObjective);
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalQuestManager: Added objective '%s' (Target: %d, Reward: %.1f XP)"), 
           *Description, TargetValue, RewardXP);
}

bool UQuest_SurvivalQuestManager::CheckSurvivalCompletion()
{
    int32 CompletedCount = 0;
    int32 TotalCount = 0;

    for (const auto& Objective : GlobalObjectives)
    {
        TotalCount++;
        if (Objective.bIsCompleted)
        {
            CompletedCount++;
        }
    }

    for (const auto& Challenge : ActiveChallenges)
    {
        for (const auto& Objective : Challenge.Objectives)
        {
            TotalCount++;
            if (Objective.bIsCompleted)
            {
                CompletedCount++;
            }
        }
    }

    float CompletionRate = TotalCount > 0 ? static_cast<float>(CompletedCount) / static_cast<float>(TotalCount) : 0.0f;
    return CompletionRate >= 0.8f; // 80% completion threshold
}

void UQuest_SurvivalQuestManager::MonitorPlayerHealth(float CurrentHealth, float MaxHealth)
{
    float HealthPercentage = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
    
    if (HealthPercentage <= HealthThreshold)
    {
        UpdateSurvivalProgress(TEXT("health"), 0); // Trigger health warning
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                TEXT("WARNING: Health critically low!"));
        }
    }
    else
    {
        UpdateSurvivalProgress(TEXT("health above"), 1);
    }
}

void UQuest_SurvivalQuestManager::MonitorPlayerHunger(float CurrentHunger, float MaxHunger)
{
    float HungerPercentage = MaxHunger > 0.0f ? CurrentHunger / MaxHunger : 0.0f;
    
    if (HungerPercentage >= (1.0f - HungerThreshold))
    {
        UpdateSurvivalProgress(TEXT("hunger"), 0); // Trigger hunger warning
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
                TEXT("WARNING: Hunger level critical!"));
        }
    }
    else
    {
        UpdateSurvivalProgress(TEXT("hunger below"), 1);
    }
}

void UQuest_SurvivalQuestManager::MonitorPlayerThirst(float CurrentThirst, float MaxThirst)
{
    float ThirstPercentage = MaxThirst > 0.0f ? CurrentThirst / MaxThirst : 0.0f;
    
    if (ThirstPercentage >= (1.0f - ThirstThreshold))
    {
        UpdateSurvivalProgress(TEXT("thirst"), 0); // Trigger thirst warning
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
                TEXT("WARNING: Dehydration critical!"));
        }
    }
    else
    {
        UpdateSurvivalProgress(TEXT("hydration"), 1);
    }
}

void UQuest_SurvivalQuestManager::MonitorPlayerTemperature(float CurrentTemp, float OptimalTemp)
{
    float TempDifference = FMath::Abs(CurrentTemp - OptimalTemp);
    
    if (TempDifference > TemperatureRange)
    {
        UpdateSurvivalProgress(TEXT("temperature"), 1);
        
        if (GEngine)
        {
            FColor TempColor = CurrentTemp > OptimalTemp ? FColor::Red : FColor::Cyan;
            FString TempWarning = CurrentTemp > OptimalTemp ? TEXT("Overheating!") : TEXT("Hypothermia risk!");
            
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, TempColor, 
                FString::Printf(TEXT("WARNING: %s (%.1f°C)"), *TempWarning, CurrentTemp));
        }
    }
}

void UQuest_SurvivalQuestManager::GrantSurvivalRewards(float ExperiencePoints)
{
    UE_LOG(LogTemp, Log, TEXT("SurvivalQuestManager: Granted %.1f survival experience points"), ExperiencePoints);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("SURVIVAL XP: +%.0f"), ExperiencePoints));
    }

    // TODO: Integrate with player progression system
}

void UQuest_SurvivalQuestManager::UnlockSurvivalSkill(const FString& SkillName)
{
    UE_LOG(LogTemp, Log, TEXT("SurvivalQuestManager: Unlocked survival skill '%s'"), *SkillName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, 
            FString::Printf(TEXT("SKILL UNLOCKED: %s"), *SkillName));
    }

    // TODO: Integrate with skill tree system
}

bool UQuest_SurvivalQuestManager::IsAnySurvivalChallengeActive() const
{
    for (const auto& Challenge : ActiveChallenges)
    {
        if (Challenge.bIsActive)
        {
            return true;
        }
    }
    return false;
}

float UQuest_SurvivalQuestManager::GetCurrentChallengeTimeRemaining() const
{
    for (const auto& Challenge : ActiveChallenges)
    {
        if (Challenge.bIsActive)
        {
            return FMath::Max(0.0f, Challenge.TimeLimit - Challenge.CurrentTime);
        }
    }
    return 0.0f;
}

int32 UQuest_SurvivalQuestManager::GetCompletedObjectivesCount() const
{
    int32 CompletedCount = 0;
    
    for (const auto& Objective : GlobalObjectives)
    {
        if (Objective.bIsCompleted)
        {
            CompletedCount++;
        }
    }
    
    for (const auto& Challenge : ActiveChallenges)
    {
        for (const auto& Objective : Challenge.Objectives)
        {
            if (Objective.bIsCompleted)
            {
                CompletedCount++;
            }
        }
    }
    
    return CompletedCount;
}

void UQuest_SurvivalQuestManager::ProcessActiveChallenges(float DeltaTime)
{
    for (int32 i = ActiveChallenges.Num() - 1; i >= 0; --i)
    {
        FQuest_SurvivalChallenge& Challenge = ActiveChallenges[i];
        
        if (Challenge.bIsActive)
        {
            Challenge.CurrentTime += DeltaTime;
            
            // Check if challenge time expired
            if (Challenge.CurrentTime >= Challenge.TimeLimit)
            {
                bool bAllObjectivesCompleted = true;
                for (const auto& Objective : Challenge.Objectives)
                {
                    if (!Objective.bIsCompleted)
                    {
                        bAllObjectivesCompleted = false;
                        break;
                    }
                }
                
                EndSurvivalChallenge(bAllObjectivesCompleted);
            }
        }
    }
}

void UQuest_SurvivalQuestManager::CheckAutoGeneration(float DeltaTime)
{
    LastChallengeTime += DeltaTime;
    
    if (LastChallengeTime >= ChallengeGenerationInterval && !IsAnySurvivalChallengeActive())
    {
        GenerateRandomChallenge();
        LastChallengeTime = 0.0f;
    }
}

void UQuest_SurvivalQuestManager::GenerateRandomChallenge()
{
    TArray<FString> ChallengeTypes = {
        TEXT("Extreme Weather Survival"),
        TEXT("Predator Territory Navigation"),
        TEXT("Resource Scarcity Challenge"),
        TEXT("Night Survival Trial"),
        TEXT("Wilderness Endurance Test")
    };
    
    TArray<float> ChallengeDurations = { 120.0f, 180.0f, 240.0f, 300.0f };
    
    int32 RandomType = FMath::RandRange(0, ChallengeTypes.Num() - 1);
    int32 RandomDuration = FMath::RandRange(0, ChallengeDurations.Num() - 1);
    
    FVector RandomLocation = FVector(
        FMath::RandRange(-2000.0f, 2000.0f),
        FMath::RandRange(-2000.0f, 2000.0f),
        100.0f
    );
    
    StartSurvivalChallenge(ChallengeTypes[RandomType], ChallengeDurations[RandomDuration], RandomLocation);
}

bool UQuest_SurvivalQuestManager::ValidateSurvivalConditions()
{
    // TODO: Implement validation logic for survival conditions
    return true;
}

void UQuest_SurvivalQuestManager::UpdateObjectiveProgress(FQuest_SurvivalObjective& Objective, int32 Progress)
{
    if (!Objective.bIsCompleted)
    {
        Objective.CurrentProgress += Progress;
        
        if (Objective.CurrentProgress >= Objective.TargetValue)
        {
            Objective.bIsCompleted = true;
            GrantSurvivalRewards(Objective.RewardExperience);
            
            UE_LOG(LogTemp, Log, TEXT("SurvivalQuestManager: Objective completed '%s' (+%.1f XP)"), 
                   *Objective.ObjectiveDescription, Objective.RewardExperience);
        }
    }
}