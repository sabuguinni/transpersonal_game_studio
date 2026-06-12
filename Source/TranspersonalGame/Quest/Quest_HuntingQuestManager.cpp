#include "Quest_HuntingQuestManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UQuest_HuntingQuestManager::UQuest_HuntingQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    QuestTimeLimit = 1800.0f; // 30 minutes default
    ElapsedTime = 0.0f;
    bIsQuestActive = false;
    QuestDescription = TEXT("Hunt the specified targets to complete this quest.");
    QuestGiver = TEXT("Tribal Elder");
}

void UQuest_HuntingQuestManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HuntingQuestManager initialized"));
}

void UQuest_HuntingQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsQuestActive)
    {
        UpdateQuestTimer(DeltaTime);
        CheckTimeLimit();
    }
}

void UQuest_HuntingQuestManager::StartHuntingQuest(const TArray<FQuest_HuntingTarget>& Targets, const FQuest_HuntingReward& Reward, float TimeLimit)
{
    if (bIsQuestActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new hunting quest - quest already active"));
        return;
    }

    ActiveHuntingTargets = Targets;
    CurrentReward = Reward;
    QuestTimeLimit = TimeLimit;
    ElapsedTime = 0.0f;
    bIsQuestActive = true;

    // Reset all target counts
    for (FQuest_HuntingTarget& Target : ActiveHuntingTargets)
    {
        Target.CurrentCount = 0;
    }

    UE_LOG(LogTemp, Warning, TEXT("Hunting quest started with %d targets"), ActiveHuntingTargets.Num());
    
    // Notify player
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("New Hunting Quest: %s"), *QuestDescription));
    }
}

void UQuest_HuntingQuestManager::RegisterKill(const FString& Species, const FString& WeaponUsed, float Distance)
{
    if (!bIsQuestActive)
    {
        return;
    }

    for (FQuest_HuntingTarget& Target : ActiveHuntingTargets)
    {
        if (Target.TargetSpecies == Species && Target.CurrentCount < Target.RequiredCount)
        {
            if (ValidateKill(Target, WeaponUsed, Distance))
            {
                Target.CurrentCount++;
                UE_LOG(LogTemp, Warning, TEXT("Kill registered: %s (%d/%d)"), 
                    *Species, Target.CurrentCount, Target.RequiredCount);

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                        FString::Printf(TEXT("%s killed! Progress: %d/%d"), 
                        *Species, Target.CurrentCount, Target.RequiredCount));
                }

                if (CheckQuestCompletion())
                {
                    CompleteQuest();
                }
                break;
            }
        }
    }
}

bool UQuest_HuntingQuestManager::CheckQuestCompletion()
{
    if (!bIsQuestActive)
    {
        return false;
    }

    for (const FQuest_HuntingTarget& Target : ActiveHuntingTargets)
    {
        if (Target.CurrentCount < Target.RequiredCount)
        {
            return false;
        }
    }

    return true;
}

void UQuest_HuntingQuestManager::CompleteQuest()
{
    if (!bIsQuestActive)
    {
        return;
    }

    bIsQuestActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Hunting quest completed! Rewards granted."));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Quest Complete! Rewards granted."));
    }

    // Grant rewards logic would go here
    // For now, just log the rewards
    for (const FString& Item : CurrentReward.ItemRewards)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reward item: %s"), *Item);
    }
    
    if (CurrentReward.ExperienceReward > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Experience reward: %d"), CurrentReward.ExperienceReward);
    }
}

void UQuest_HuntingQuestManager::FailQuest()
{
    if (!bIsQuestActive)
    {
        return;
    }

    bIsQuestActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Hunting quest failed!"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            TEXT("Quest Failed!"));
    }
}

TArray<FString> UQuest_HuntingQuestManager::GetQuestObjectives()
{
    TArray<FString> Objectives;
    
    for (const FQuest_HuntingTarget& Target : ActiveHuntingTargets)
    {
        FString Objective = FString::Printf(TEXT("Hunt %s: %d/%d"), 
            *Target.TargetSpecies, Target.CurrentCount, Target.RequiredCount);
        
        if (Target.bRequiresSpecificWeapon)
        {
            Objective += FString::Printf(TEXT(" (Use %s)"), *Target.RequiredWeaponType);
        }
        
        Objectives.Add(Objective);
    }
    
    return Objectives;
}

float UQuest_HuntingQuestManager::GetQuestProgress()
{
    if (ActiveHuntingTargets.Num() == 0)
    {
        return 0.0f;
    }

    int32 TotalRequired = 0;
    int32 TotalCompleted = 0;

    for (const FQuest_HuntingTarget& Target : ActiveHuntingTargets)
    {
        TotalRequired += Target.RequiredCount;
        TotalCompleted += Target.CurrentCount;
    }

    return TotalRequired > 0 ? (float)TotalCompleted / (float)TotalRequired : 0.0f;
}

FString UQuest_HuntingQuestManager::GetQuestStatusText()
{
    if (!bIsQuestActive)
    {
        return TEXT("No active quest");
    }

    float Progress = GetQuestProgress() * 100.0f;
    float TimeRemaining = QuestTimeLimit - ElapsedTime;
    int32 MinutesRemaining = FMath::FloorToInt(TimeRemaining / 60.0f);
    int32 SecondsRemaining = FMath::FloorToInt(TimeRemaining - (MinutesRemaining * 60));

    return FString::Printf(TEXT("Progress: %.1f%% | Time: %02d:%02d"), 
        Progress, MinutesRemaining, SecondsRemaining);
}

void UQuest_HuntingQuestManager::UpdateQuestTimer(float DeltaTime)
{
    ElapsedTime += DeltaTime;
}

void UQuest_HuntingQuestManager::CheckTimeLimit()
{
    if (ElapsedTime >= QuestTimeLimit)
    {
        FailQuest();
    }
}

bool UQuest_HuntingQuestManager::ValidateKill(const FQuest_HuntingTarget& Target, const FString& WeaponUsed, float Distance)
{
    // Check weapon requirement
    if (Target.bRequiresSpecificWeapon && Target.RequiredWeaponType != WeaponUsed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Kill invalid: Wrong weapon. Required: %s, Used: %s"), 
            *Target.RequiredWeaponType, *WeaponUsed);
        return false;
    }

    // Check distance requirement
    if (Distance < Target.MinimumDistance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Kill invalid: Too close. Required: %.1f, Actual: %.1f"), 
            Target.MinimumDistance, Distance);
        return false;
    }

    return true;
}