#include "Quest_InteractiveTutorialSystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AQuest_InteractiveTutorialSystem::AQuest_InteractiveTutorialSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize tutorial state
    CurrentStep = EQuest_TutorialStep::None;
    bTutorialActive = false;
    bTutorialCompleted = false;

    // Initialize resource counters
    StonesCollected = 0;
    SticksCollected = 0;
    ToolsCrafted = 0;
    bWaterSourceFound = false;
    bShelterBuilt = false;
    bPreyHunted = false;

    // Set audio URLs from TTS generation
    ElderSurvivorAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777714273357_Elder_Survivor.mp3");
    ScoutRangerAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777714279595_Scout_Ranger.mp3");
}

void AQuest_InteractiveTutorialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTutorialObjectives();
    
    // Auto-start tutorial after 2 seconds
    FTimerHandle StartTimer;
    GetWorld()->GetTimerManager().SetTimer(StartTimer, this, &AQuest_InteractiveTutorialSystem::StartTutorial, 2.0f, false);
}

void AQuest_InteractiveTutorialSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bTutorialActive && !bTutorialCompleted)
    {
        UpdateObjectiveProgress();
    }
}

void AQuest_InteractiveTutorialSystem::StartTutorial()
{
    if (bTutorialCompleted)
    {
        return;
    }

    bTutorialActive = true;
    CurrentStep = EQuest_TutorialStep::GatherResources;
    
    UE_LOG(LogTemp, Warning, TEXT("Tutorial Started: Gather Resources Phase"));
    
    // Play Elder Survivor introduction
    PlayAudioCue(ElderSurvivorAudioURL);
    
    // Show first objective
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
            TEXT("TUTORIAL: Gather 3 stones and 2 sticks to craft your first tool"));
    }
}

void AQuest_InteractiveTutorialSystem::AdvanceTutorialStep()
{
    switch (CurrentStep)
    {
        case EQuest_TutorialStep::GatherResources:
            if (StonesCollected >= 3 && SticksCollected >= 2)
            {
                CurrentStep = EQuest_TutorialStep::CraftTool;
                CompleteObjective(EQuest_TutorialStep::GatherResources);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
                        TEXT("Resources gathered! Now craft a stone axe."));
                }
            }
            break;
            
        case EQuest_TutorialStep::CraftTool:
            if (ToolsCrafted >= 1)
            {
                CurrentStep = EQuest_TutorialStep::FindWater;
                CompleteObjective(EQuest_TutorialStep::CraftTool);
                PlayAudioCue(ScoutRangerAudioURL);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Blue, 
                        TEXT("Tool crafted! Now find a water source."));
                }
            }
            break;
            
        case EQuest_TutorialStep::FindWater:
            if (bWaterSourceFound)
            {
                CurrentStep = EQuest_TutorialStep::BuildShelter;
                CompleteObjective(EQuest_TutorialStep::FindWater);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, 
                        TEXT("Water found! Build a shelter for protection."));
                }
            }
            break;
            
        case EQuest_TutorialStep::BuildShelter:
            if (bShelterBuilt)
            {
                CurrentStep = EQuest_TutorialStep::HuntPrey;
                CompleteObjective(EQuest_TutorialStep::BuildShelter);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange, 
                        TEXT("Shelter built! Hunt small prey for food."));
                }
            }
            break;
            
        case EQuest_TutorialStep::HuntPrey:
            if (bPreyHunted)
            {
                CurrentStep = EQuest_TutorialStep::SurviveNight;
                CompleteObjective(EQuest_TutorialStep::HuntPrey);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, 
                        TEXT("Prey hunted! Prepare to survive the night."));
                }
            }
            break;
            
        case EQuest_TutorialStep::SurviveNight:
            CurrentStep = EQuest_TutorialStep::Completed;
            CompleteObjective(EQuest_TutorialStep::SurviveNight);
            CheckTutorialCompletion();
            break;
            
        default:
            break;
    }
}

void AQuest_InteractiveTutorialSystem::CompleteObjective(EQuest_TutorialStep Step, int32 Amount)
{
    for (FQuest_TutorialObjective& Objective : TutorialObjectives)
    {
        if (Objective.RequiredStep == Step)
        {
            Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Amount, Objective.RequiredCount);
            
            if (Objective.CurrentCount >= Objective.RequiredCount)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s"), *Objective.ObjectiveText);
            }
            break;
        }
    }
}

bool AQuest_InteractiveTutorialSystem::IsObjectiveCompleted(EQuest_TutorialStep Step) const
{
    for (const FQuest_TutorialObjective& Objective : TutorialObjectives)
    {
        if (Objective.RequiredStep == Step)
        {
            return Objective.bIsCompleted;
        }
    }
    return false;
}

FString AQuest_InteractiveTutorialSystem::GetCurrentObjectiveText() const
{
    for (const FQuest_TutorialObjective& Objective : TutorialObjectives)
    {
        if (Objective.RequiredStep == CurrentStep && !Objective.bIsCompleted)
        {
            return FString::Printf(TEXT("%s (%d/%d)"), 
                *Objective.ObjectiveText, 
                Objective.CurrentCount, 
                Objective.RequiredCount);
        }
    }
    return TEXT("No active objective");
}

void AQuest_InteractiveTutorialSystem::OnResourceCollected(const FString& ResourceType)
{
    if (ResourceType == TEXT("Stone"))
    {
        StonesCollected++;
        UE_LOG(LogTemp, Warning, TEXT("Stone collected! Total: %d"), StonesCollected);
    }
    else if (ResourceType == TEXT("Stick"))
    {
        SticksCollected++;
        UE_LOG(LogTemp, Warning, TEXT("Stick collected! Total: %d"), SticksCollected);
    }
    
    if (CurrentStep == EQuest_TutorialStep::GatherResources)
    {
        AdvanceTutorialStep();
    }
}

void AQuest_InteractiveTutorialSystem::OnToolCrafted(const FString& ToolType)
{
    ToolsCrafted++;
    UE_LOG(LogTemp, Warning, TEXT("Tool crafted: %s"), *ToolType);
    
    if (CurrentStep == EQuest_TutorialStep::CraftTool)
    {
        AdvanceTutorialStep();
    }
}

void AQuest_InteractiveTutorialSystem::OnWaterSourceFound()
{
    bWaterSourceFound = true;
    UE_LOG(LogTemp, Warning, TEXT("Water source found!"));
    
    if (CurrentStep == EQuest_TutorialStep::FindWater)
    {
        AdvanceTutorialStep();
    }
}

void AQuest_InteractiveTutorialSystem::OnShelterBuilt()
{
    bShelterBuilt = true;
    UE_LOG(LogTemp, Warning, TEXT("Shelter built!"));
    
    if (CurrentStep == EQuest_TutorialStep::BuildShelter)
    {
        AdvanceTutorialStep();
    }
}

void AQuest_InteractiveTutorialSystem::OnPreyHunted()
{
    bPreyHunted = true;
    UE_LOG(LogTemp, Warning, TEXT("Prey hunted!"));
    
    if (CurrentStep == EQuest_TutorialStep::HuntPrey)
    {
        AdvanceTutorialStep();
    }
}

void AQuest_InteractiveTutorialSystem::InitializeTutorialObjectives()
{
    TutorialObjectives.Empty();
    
    // Gather Resources objective
    FQuest_TutorialObjective GatherObj;
    GatherObj.ObjectiveText = TEXT("Gather stones and sticks");
    GatherObj.RequiredStep = EQuest_TutorialStep::GatherResources;
    GatherObj.RequiredCount = 5; // 3 stones + 2 sticks
    TutorialObjectives.Add(GatherObj);
    
    // Craft Tool objective
    FQuest_TutorialObjective CraftObj;
    CraftObj.ObjectiveText = TEXT("Craft a stone tool");
    CraftObj.RequiredStep = EQuest_TutorialStep::CraftTool;
    CraftObj.RequiredCount = 1;
    TutorialObjectives.Add(CraftObj);
    
    // Find Water objective
    FQuest_TutorialObjective WaterObj;
    WaterObj.ObjectiveText = TEXT("Find a water source");
    WaterObj.RequiredStep = EQuest_TutorialStep::FindWater;
    WaterObj.RequiredCount = 1;
    TutorialObjectives.Add(WaterObj);
    
    // Build Shelter objective
    FQuest_TutorialObjective ShelterObj;
    ShelterObj.ObjectiveText = TEXT("Build a basic shelter");
    ShelterObj.RequiredStep = EQuest_TutorialStep::BuildShelter;
    ShelterObj.RequiredCount = 1;
    TutorialObjectives.Add(ShelterObj);
    
    // Hunt Prey objective
    FQuest_TutorialObjective HuntObj;
    HuntObj.ObjectiveText = TEXT("Hunt small prey");
    HuntObj.RequiredStep = EQuest_TutorialStep::HuntPrey;
    HuntObj.RequiredCount = 1;
    TutorialObjectives.Add(HuntObj);
    
    // Survive Night objective
    FQuest_TutorialObjective SurviveObj;
    SurviveObj.ObjectiveText = TEXT("Survive the night");
    SurviveObj.RequiredStep = EQuest_TutorialStep::SurviveNight;
    SurviveObj.RequiredCount = 1;
    TutorialObjectives.Add(SurviveObj);
}

void AQuest_InteractiveTutorialSystem::UpdateObjectiveProgress()
{
    // Update progress based on current step
    switch (CurrentStep)
    {
        case EQuest_TutorialStep::GatherResources:
            if (StonesCollected >= 3 && SticksCollected >= 2)
            {
                AdvanceTutorialStep();
            }
            break;
            
        case EQuest_TutorialStep::CraftTool:
            if (ToolsCrafted >= 1)
            {
                AdvanceTutorialStep();
            }
            break;
            
        case EQuest_TutorialStep::FindWater:
            if (bWaterSourceFound)
            {
                AdvanceTutorialStep();
            }
            break;
            
        case EQuest_TutorialStep::BuildShelter:
            if (bShelterBuilt)
            {
                AdvanceTutorialStep();
            }
            break;
            
        case EQuest_TutorialStep::HuntPrey:
            if (bPreyHunted)
            {
                AdvanceTutorialStep();
            }
            break;
            
        default:
            break;
    }
}

void AQuest_InteractiveTutorialSystem::CheckTutorialCompletion()
{
    bool bAllCompleted = true;
    for (const FQuest_TutorialObjective& Objective : TutorialObjectives)
    {
        if (!Objective.bIsCompleted)
        {
            bAllCompleted = false;
            break;
        }
    }
    
    if (bAllCompleted)
    {
        bTutorialCompleted = true;
        bTutorialActive = false;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, 
                TEXT("TUTORIAL COMPLETED! You have learned the basics of survival."));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Tutorial system completed successfully!"));
    }
}

void AQuest_InteractiveTutorialSystem::PlayAudioCue(const FString& AudioURL)
{
    // Log the audio cue for now - in a full implementation this would trigger audio playback
    UE_LOG(LogTemp, Warning, TEXT("Playing audio cue: %s"), *AudioURL);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
            FString::Printf(TEXT("Audio: %s"), *AudioURL));
    }
}