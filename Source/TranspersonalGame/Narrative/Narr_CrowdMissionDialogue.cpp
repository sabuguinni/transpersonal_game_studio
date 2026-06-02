#include "Narr_CrowdMissionDialogue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNarr_CrowdMissionDialogue::UNarr_CrowdMissionDialogue()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentDetectionLevel = ENarr_DetectionLevel::Hidden;
    ActiveMissionType = ENarr_CrowdMissionType::Stealth;
    bMissionActive = false;
    bDialoguePlaying = false;
    DialogueCooldownTime = 3.0f;
    LastDialogueTime = 0.0f;
    
    // Initialize mission context
    CurrentMissionContext.SettlementName = TEXT("Ancient Settlement");
    CurrentMissionContext.ThreatDescription = TEXT("Hostile guards patrol the area");
    CurrentMissionContext.ObjectiveReason = TEXT("Gather information about the old ways");
    CurrentMissionContext.ConsequencesOfDetection.Add(TEXT("Guards will hunt you through the settlement"));
    CurrentMissionContext.ConsequencesOfDetection.Add(TEXT("Mission will fail and you must escape"));
    CurrentMissionContext.ConsequencesOfDetection.Add(TEXT("Future stealth missions become harder"));
    CurrentMissionContext.CrowdDensityRequired = 5;
    CurrentMissionContext.bNightTimePreferred = true;
}

void UNarr_CrowdMissionDialogue::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogueTable();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_CrowdMissionDialogue: Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNarr_CrowdMissionDialogue::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update dialogue cooldown
    if (bDialoguePlaying && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastDialogueTime > DialogueCooldownTime)
        {
            bDialoguePlaying = false;
        }
    }
}

void UNarr_CrowdMissionDialogue::TriggerMissionDialogue(ENarr_CrowdMissionType MissionType, ENarr_DetectionLevel DetectionLevel)
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    ActiveMissionType = MissionType;
    CurrentDetectionLevel = DetectionLevel;
    
    FNarr_CrowdMissionDialogue DialogueData = GetDialogueForSituation(MissionType, DetectionLevel);
    
    // Log the dialogue trigger
    FString MissionTypeStr = UEnum::GetValueAsString(MissionType);
    FString DetectionStr = UEnum::GetValueAsString(DetectionLevel);
    LogMissionEvent(FString::Printf(TEXT("Triggered dialogue for %s mission at %s detection level"), 
                                   *MissionTypeStr, *DetectionStr));
    
    // Display dialogue text
    if (GEngine && DialogueData.bShowOnHUD)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), 
                                            *DialogueData.SpeakerName, 
                                            *DialogueData.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueData.DialogueDuration, 
                                       DialogueData.bIsUrgent ? FColor::Red : FColor::Yellow, 
                                       DisplayText);
    }
    
    // Play audio if available
    if (!DialogueData.AudioPath.IsEmpty())
    {
        PlayDialogueAudio(DialogueData.AudioPath);
    }
    
    // Update state
    bDialoguePlaying = true;
    LastDialogueTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UNarr_CrowdMissionDialogue::PlayDetectionWarning(ENarr_DetectionLevel NewDetectionLevel)
{
    if (NewDetectionLevel == CurrentDetectionLevel)
    {
        return; // No change in detection level
    }
    
    ENarr_DetectionLevel PreviousLevel = CurrentDetectionLevel;
    CurrentDetectionLevel = NewDetectionLevel;
    
    // Trigger appropriate dialogue based on detection escalation
    if (NewDetectionLevel > PreviousLevel)
    {
        TriggerMissionDialogue(ActiveMissionType, NewDetectionLevel);
        
        // Log detection change
        FString PrevStr = UEnum::GetValueAsString(PreviousLevel);
        FString NewStr = UEnum::GetValueAsString(NewDetectionLevel);
        LogMissionEvent(FString::Printf(TEXT("Detection escalated from %s to %s"), *PrevStr, *NewStr));
    }
}

void UNarr_CrowdMissionDialogue::SetMissionContext(const FNarr_MissionContext& NewContext)
{
    CurrentMissionContext = NewContext;
    
    LogMissionEvent(FString::Printf(TEXT("Mission context updated: %s - %s"), 
                                   *NewContext.SettlementName, 
                                   *NewContext.ObjectiveReason));
}

FNarr_CrowdMissionDialogue UNarr_CrowdMissionDialogue::GetDialogueForSituation(ENarr_CrowdMissionType MissionType, ENarr_DetectionLevel DetectionLevel)
{
    FNarr_CrowdMissionDialogue DefaultDialogue;
    
    // Generate contextual dialogue based on mission type and detection level
    switch (MissionType)
    {
        case ENarr_CrowdMissionType::Stealth:
            switch (DetectionLevel)
            {
                case ENarr_DetectionLevel::Hidden:
                    DefaultDialogue.DialogueText = TEXT("Move like shadow through the crowds. The guards know your face, but they cannot see through the veil of common folk.");
                    DefaultDialogue.SpeakerName = TEXT("Stealth Guide");
                    break;
                case ENarr_DetectionLevel::Suspicious:
                    DefaultDialogue.DialogueText = TEXT("Careful now! The watchers grow suspicious. Lower your gaze and match their pace.");
                    DefaultDialogue.SpeakerName = TEXT("Warning Voice");
                    DefaultDialogue.bIsUrgent = true;
                    break;
                case ENarr_DetectionLevel::Detected:
                    DefaultDialogue.DialogueText = TEXT("You've been spotted! Find cover in the crowds quickly!");
                    DefaultDialogue.SpeakerName = TEXT("Urgent Warning");
                    DefaultDialogue.bIsUrgent = true;
                    break;
            }
            break;
            
        case ENarr_CrowdMissionType::Infiltration:
            switch (DetectionLevel)
            {
                case ENarr_DetectionLevel::Hidden:
                    DefaultDialogue.DialogueText = TEXT("Blend with the settlement folk. You are one of them now - invisible in plain sight.");
                    DefaultDialogue.SpeakerName = TEXT("Infiltration Guide");
                    break;
                case ENarr_DetectionLevel::Suspicious:
                    DefaultDialogue.DialogueText = TEXT("Their eyes scan the crowd like predators seeking prey. Breathe with the rhythm of the settlement.");
                    DefaultDialogue.SpeakerName = TEXT("Crowd Mentor");
                    break;
            }
            break;
            
        case ENarr_CrowdMissionType::Following:
            DefaultDialogue.DialogueText = TEXT("Follow the marked one through the settlement paths. Stay three paces behind, no closer.");
            DefaultDialogue.SpeakerName = TEXT("Tracking Elder");
            break;
            
        case ENarr_CrowdMissionType::Escape:
            DefaultDialogue.DialogueText = TEXT("Run with the panicked crowds toward the outer gates! Use their fear as your shield!");
            DefaultDialogue.SpeakerName = TEXT("Escape Guide");
            DefaultDialogue.bIsUrgent = true;
            break;
            
        case ENarr_CrowdMissionType::Observation:
            DefaultDialogue.DialogueText = TEXT("Watch and learn from the shadows. The settlement holds secrets of the old world.");
            DefaultDialogue.SpeakerName = TEXT("Observer");
            break;
    }
    
    DefaultDialogue.MissionType = MissionType;
    DefaultDialogue.TriggerDetectionLevel = DetectionLevel;
    DefaultDialogue.DialogueDuration = 8.0f;
    
    return DefaultDialogue;
}

void UNarr_CrowdMissionDialogue::StartMissionBriefing(ENarr_CrowdMissionType MissionType)
{
    bMissionActive = true;
    ActiveMissionType = MissionType;
    CurrentDetectionLevel = ENarr_DetectionLevel::Hidden;
    
    // Generate mission briefing based on context and type
    FString BriefingText;
    switch (MissionType)
    {
        case ENarr_CrowdMissionType::Stealth:
            BriefingText = FString::Printf(TEXT("Mission: Infiltrate %s undetected. %s. Objective: %s"), 
                                         *CurrentMissionContext.SettlementName,
                                         *CurrentMissionContext.ThreatDescription,
                                         *CurrentMissionContext.ObjectiveReason);
            break;
        case ENarr_CrowdMissionType::Infiltration:
            BriefingText = FString::Printf(TEXT("Mission: Blend into %s population. Gather intelligence without raising suspicion."), 
                                         *CurrentMissionContext.SettlementName);
            break;
        case ENarr_CrowdMissionType::Following:
            BriefingText = TEXT("Mission: Follow the target through crowded areas. Maintain distance and avoid detection.");
            break;
        case ENarr_CrowdMissionType::Escape:
            BriefingText = TEXT("Mission: Escape the settlement using crowd cover. Reach the extraction point safely.");
            break;
        case ENarr_CrowdMissionType::Observation:
            BriefingText = TEXT("Mission: Observe settlement patterns and guard routines. Gather intelligence for future operations.");
            break;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, 
                                       FString::Printf(TEXT("[MISSION BRIEFING]: %s"), *BriefingText));
    }
    
    LogMissionEvent(FString::Printf(TEXT("Mission briefing started: %s"), *BriefingText));
}

void UNarr_CrowdMissionDialogue::UpdateMissionStatus(const FString& StatusMessage, bool bIsSuccess)
{
    FColor MessageColor = bIsSuccess ? FColor::Green : FColor::Orange;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, MessageColor, 
                                       FString::Printf(TEXT("[MISSION UPDATE]: %s"), *StatusMessage));
    }
    
    LogMissionEvent(FString::Printf(TEXT("Mission status: %s (Success: %s)"), 
                                   *StatusMessage, bIsSuccess ? TEXT("Yes") : TEXT("No")));
}

void UNarr_CrowdMissionDialogue::PlayDialogueAudio(const FString& AudioPath)
{
    // Audio playback would be handled by the Audio Agent's system
    // For now, log the audio request
    LogMissionEvent(FString::Printf(TEXT("Audio requested: %s"), *AudioPath));
}

void UNarr_CrowdMissionDialogue::StopCurrentDialogue()
{
    bDialoguePlaying = false;
    LogMissionEvent(TEXT("Dialogue stopped"));
}

void UNarr_CrowdMissionDialogue::InitializeDialogueTable()
{
    // Initialize dialogue table - would load from data table in full implementation
    LogMissionEvent(TEXT("Dialogue table initialized"));
}

bool UNarr_CrowdMissionDialogue::CanPlayDialogue() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return !bDialoguePlaying || (CurrentTime - LastDialogueTime > DialogueCooldownTime);
}

void UNarr_CrowdMissionDialogue::LogMissionEvent(const FString& EventDescription)
{
    UE_LOG(LogTemp, Log, TEXT("Narr_CrowdMissionDialogue [%s]: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
           *EventDescription);
}