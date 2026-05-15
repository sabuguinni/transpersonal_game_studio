#include "Quest_DinosaurHuntManager.h"
#include "TranspersonalCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UQuest_DinosaurHuntManager::UQuest_DinosaurHuntManager()
{
    BaseTimeLimit = 600.0f; // 10 minutes default
    MaxActiveHunts = 1;
}

void UQuest_DinosaurHuntManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDifficultyScores();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurHuntManager initialized"));
}

void UQuest_DinosaurHuntManager::Deinitialize()
{
    if (GetWorld() && QuestTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(QuestTimerHandle);
    }
    
    Super::Deinitialize();
}

void UQuest_DinosaurHuntManager::StartHuntQuest(EDinosaurSpecies Species, int32 KillCount, float TimeLimit, const FString& QuestName)
{
    if (CurrentHuntQuest.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start hunt quest - another quest is already active"));
        return;
    }

    CurrentHuntQuest.TargetSpecies = Species;
    CurrentHuntQuest.RequiredKills = KillCount;
    CurrentHuntQuest.CurrentKills = 0;
    CurrentHuntQuest.TimeLimit = TimeLimit;
    CurrentHuntQuest.ElapsedTime = 0.0f;
    CurrentHuntQuest.bIsActive = true;
    CurrentHuntQuest.QuestName = QuestName;
    
    // Set description based on species
    FString SpeciesName;
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            SpeciesName = TEXT("T-Rex");
            break;
        case EDinosaurSpecies::Raptor:
            SpeciesName = TEXT("Velociraptor");
            break;
        case EDinosaurSpecies::Triceratops:
            SpeciesName = TEXT("Triceratops");
            break;
        case EDinosaurSpecies::Brachiosaurus:
            SpeciesName = TEXT("Brachiosaurus");
            break;
        default:
            SpeciesName = TEXT("Unknown Dinosaur");
            break;
    }
    
    CurrentHuntQuest.Description = FString::Printf(TEXT("Hunt and kill %d %s within %d minutes"), 
        KillCount, *SpeciesName, FMath::RoundToInt(TimeLimit / 60.0f));

    // Start quest timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(QuestTimerHandle, this, &UQuest_DinosaurHuntManager::UpdateQuestTimer, 1.0f, true);
    }

    OnHuntQuestStarted.Broadcast(Species, QuestName);
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt quest started: %s - Target: %s, Kills: %d, Time: %.1f"), 
        *QuestName, *SpeciesName, KillCount, TimeLimit);
}

void UQuest_DinosaurHuntManager::CompleteHuntQuest()
{
    if (!CurrentHuntQuest.bIsActive)
    {
        return;
    }

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestTimerHandle);
    }

    OnHuntQuestCompleted.Broadcast(CurrentHuntQuest.TargetSpecies, CurrentHuntQuest.CurrentKills);
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt quest completed: %s"), *CurrentHuntQuest.QuestName);
    
    // Reset quest data
    CurrentHuntQuest.bIsActive = false;
    CurrentHuntQuest.CurrentKills = 0;
    CurrentHuntQuest.ElapsedTime = 0.0f;
}

void UQuest_DinosaurHuntManager::CancelHuntQuest()
{
    if (!CurrentHuntQuest.bIsActive)
    {
        return;
    }

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestTimerHandle);
    }

    OnHuntQuestFailed.Broadcast(TEXT("Quest cancelled by player"));
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt quest cancelled: %s"), *CurrentHuntQuest.QuestName);
    
    // Reset quest data
    CurrentHuntQuest.bIsActive = false;
    CurrentHuntQuest.CurrentKills = 0;
    CurrentHuntQuest.ElapsedTime = 0.0f;
}

void UQuest_DinosaurHuntManager::RegisterDinosaurKill(EDinosaurSpecies Species, ATranspersonalCharacter* Hunter)
{
    if (!CurrentHuntQuest.bIsActive)
    {
        return;
    }

    if (Species != CurrentHuntQuest.TargetSpecies)
    {
        UE_LOG(LogTemp, Log, TEXT("Killed wrong species - Target: %d, Killed: %d"), 
            (int32)CurrentHuntQuest.TargetSpecies, (int32)Species);
        return;
    }

    CurrentHuntQuest.CurrentKills++;
    
    OnDinosaurKilled.Broadcast(Species, CurrentHuntQuest.CurrentKills, CurrentHuntQuest.RequiredKills);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur killed! Progress: %d/%d"), 
        CurrentHuntQuest.CurrentKills, CurrentHuntQuest.RequiredKills);

    CheckQuestCompletion();
}

bool UQuest_DinosaurHuntManager::IsHuntQuestActive() const
{
    return CurrentHuntQuest.bIsActive;
}

FQuest_DinosaurHuntData UQuest_DinosaurHuntManager::GetCurrentHuntQuest() const
{
    return CurrentHuntQuest;
}

float UQuest_DinosaurHuntManager::GetHuntQuestProgress() const
{
    if (!CurrentHuntQuest.bIsActive || CurrentHuntQuest.RequiredKills == 0)
    {
        return 0.0f;
    }

    return (float)CurrentHuntQuest.CurrentKills / (float)CurrentHuntQuest.RequiredKills;
}

void UQuest_DinosaurHuntManager::CreateRaptorHuntQuest()
{
    StartHuntQuest(EDinosaurSpecies::Raptor, 3, 300.0f, TEXT("Pack Hunter"));
}

void UQuest_DinosaurHuntManager::CreateTRexHuntQuest()
{
    StartHuntQuest(EDinosaurSpecies::TRex, 1, 900.0f, TEXT("Apex Predator"));
}

void UQuest_DinosaurHuntManager::CreateTriceratopsHuntQuest()
{
    StartHuntQuest(EDinosaurSpecies::Triceratops, 2, 600.0f, TEXT("Horned Beast"));
}

void UQuest_DinosaurHuntManager::CreateBrachiosaurusHuntQuest()
{
    StartHuntQuest(EDinosaurSpecies::Brachiosaurus, 1, 1200.0f, TEXT("Giant Hunter"));
}

void UQuest_DinosaurHuntManager::UpdateQuestTimer()
{
    if (!CurrentHuntQuest.bIsActive)
    {
        return;
    }

    CurrentHuntQuest.ElapsedTime += 1.0f;

    // Check if time limit exceeded
    if (CurrentHuntQuest.ElapsedTime >= CurrentHuntQuest.TimeLimit)
    {
        OnHuntQuestFailed.Broadcast(TEXT("Time limit exceeded"));
        
        UE_LOG(LogTemp, Warning, TEXT("Hunt quest failed - time limit exceeded"));
        
        // Clear timer and reset quest
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(QuestTimerHandle);
        }
        
        CurrentHuntQuest.bIsActive = false;
        CurrentHuntQuest.CurrentKills = 0;
        CurrentHuntQuest.ElapsedTime = 0.0f;
    }
}

void UQuest_DinosaurHuntManager::CheckQuestCompletion()
{
    if (!CurrentHuntQuest.bIsActive)
    {
        return;
    }

    if (CurrentHuntQuest.CurrentKills >= CurrentHuntQuest.RequiredKills)
    {
        CompleteHuntQuest();
    }
}

void UQuest_DinosaurHuntManager::InitializeDifficultyScores()
{
    DinosaurDifficultyScores.Empty();
    DinosaurDifficultyScores.Add(EDinosaurSpecies::Raptor, 3);
    DinosaurDifficultyScores.Add(EDinosaurSpecies::TRex, 10);
    DinosaurDifficultyScores.Add(EDinosaurSpecies::Triceratops, 6);
    DinosaurDifficultyScores.Add(EDinosaurSpecies::Brachiosaurus, 8);
}