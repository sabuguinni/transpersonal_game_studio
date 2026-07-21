#include "Narr_EmergencyDialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ANarr_EmergencyDialogueSystem::ANarr_EmergencyDialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Set default values
    DetectionRadius = 2000.0f;
    CooldownTime = 10.0f;
    LastTriggerTime = 0.0f;
    bIsActive = true;

    // Initialize emergency dialogues
    EmergencyDialogues.Empty();
    
    // Predator sighting dialogue
    FNarr_EmergencyDialogue PredatorDialogue;
    PredatorDialogue.ThreatType = TEXT("Predator");
    PredatorDialogue.DialogueText = TEXT("Critical alert! Large predator detected in the vicinity. Seek immediate shelter and avoid sudden movements.");
    PredatorDialogue.UrgencyLevel = 5.0f;
    PredatorDialogue.bRequiresImmedateAction = true;
    EmergencyDialogues.Add(PredatorDialogue);

    // Weather warning dialogue
    FNarr_EmergencyDialogue WeatherDialogue;
    WeatherDialogue.ThreatType = TEXT("Weather");
    WeatherDialogue.DialogueText = TEXT("Storm system approaching rapidly. Lightning and heavy rainfall imminent. Find shelter immediately.");
    WeatherDialogue.UrgencyLevel = 4.0f;
    WeatherDialogue.bRequiresImmedateAction = true;
    EmergencyDialogues.Add(WeatherDialogue);

    // Resource critical dialogue
    FNarr_EmergencyDialogue ResourceDialogue;
    ResourceDialogue.ThreatType = TEXT("Resource");
    ResourceDialogue.DialogueText = TEXT("Critical resource shortage detected. Water or food supplies dangerously low. Prioritize resource gathering.");
    ResourceDialogue.UrgencyLevel = 3.0f;
    ResourceDialogue.bRequiresImmedateAction = false;
    EmergencyDialogues.Add(ResourceDialogue);

    // Injury alert dialogue
    FNarr_EmergencyDialogue InjuryDialogue;
    InjuryDialogue.ThreatType = TEXT("Injury");
    InjuryDialogue.DialogueText = TEXT("Injury detected. Health status compromised. Seek medical attention and avoid strenuous activity.");
    InjuryDialogue.UrgencyLevel = 4.0f;
    InjuryDialogue.bRequiresImmedateAction = true;
    EmergencyDialogues.Add(InjuryDialogue);

    // Environmental hazard dialogue
    FNarr_EmergencyDialogue HazardDialogue;
    HazardDialogue.ThreatType = TEXT("Environmental");
    HazardDialogue.DialogueText = TEXT("Environmental hazard detected. Unstable terrain or dangerous gases present. Exercise extreme caution.");
    HazardDialogue.UrgencyLevel = 3.5f;
    HazardDialogue.bRequiresImmedateAction = true;
    EmergencyDialogues.Add(HazardDialogue);
}

void ANarr_EmergencyDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Emergency Dialogue System initialized"));
    
    // Set up timer for threat checking
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &ANarr_EmergencyDialogueSystem::CheckForThreats,
            2.0f,
            true
        );
    }
}

void ANarr_EmergencyDialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_EmergencyDialogueSystem::TriggerEmergencyDialogue(ENarr_EmergencyType EmergencyType, const FVector& ThreatLocation)
{
    if (!CanTriggerAlert())
    {
        return;
    }

    FString DialogueText = GetEmergencyDialogueForType(EmergencyType);
    
    if (!DialogueText.IsEmpty())
    {
        PlayEmergencyAlert(DialogueText);
        LastTriggerTime = GetWorld()->GetTimeSeconds();
        
        // Trigger Blueprint event
        OnEmergencyTriggered(EmergencyType, DialogueText);
        
        UE_LOG(LogTemp, Warning, TEXT("Emergency Alert: %s"), *DialogueText);
    }
}

void ANarr_EmergencyDialogueSystem::PlayEmergencyAlert(const FString& AlertText)
{
    if (!AudioComponent)
    {
        return;
    }

    // Log the alert text for now (audio will be handled by audio system)
    UE_LOG(LogTemp, Warning, TEXT("EMERGENCY ALERT: %s"), *AlertText);
    
    // In a full implementation, this would trigger audio playback
    // AudioComponent->Play();
}

bool ANarr_EmergencyDialogueSystem::CanTriggerAlert() const
{
    if (!bIsActive)
    {
        return false;
    }

    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        return (CurrentTime - LastTriggerTime) >= CooldownTime;
    }

    return false;
}

void ANarr_EmergencyDialogueSystem::SetEmergencyDialogue(const FString& ThreatType, const FString& DialogueText, float UrgencyLevel)
{
    FNarr_EmergencyDialogue NewDialogue;
    NewDialogue.ThreatType = ThreatType;
    NewDialogue.DialogueText = DialogueText;
    NewDialogue.UrgencyLevel = UrgencyLevel;
    NewDialogue.bRequiresImmedateAction = UrgencyLevel >= 4.0f;
    
    EmergencyDialogues.Add(NewDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("Added emergency dialogue for threat type: %s"), *ThreatType);
}

void ANarr_EmergencyDialogueSystem::CheckForThreats()
{
    if (!GetWorld())
    {
        return;
    }

    // Get player location
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    
    float DistanceToPlayer = FVector::Dist(PlayerLocation, MyLocation);
    
    // Check if player is within detection radius
    if (DistanceToPlayer <= DetectionRadius)
    {
        // Simple threat detection logic
        // In a full implementation, this would check for actual threats
        
        // Example: Random chance of weather warning
        if (FMath::RandRange(0.0f, 1.0f) < 0.05f) // 5% chance per check
        {
            TriggerEmergencyDialogue(ENarr_EmergencyType::WeatherWarning, PlayerLocation);
        }
    }
}

FString ANarr_EmergencyDialogueSystem::GetEmergencyDialogueForType(ENarr_EmergencyType EmergencyType)
{
    FString TargetThreatType;
    
    switch (EmergencyType)
    {
        case ENarr_EmergencyType::PredatorSighted:
            TargetThreatType = TEXT("Predator");
            break;
        case ENarr_EmergencyType::WeatherWarning:
            TargetThreatType = TEXT("Weather");
            break;
        case ENarr_EmergencyType::ResourceCritical:
            TargetThreatType = TEXT("Resource");
            break;
        case ENarr_EmergencyType::InjuryAlert:
            TargetThreatType = TEXT("Injury");
            break;
        case ENarr_EmergencyType::EnvironmentalHazard:
            TargetThreatType = TEXT("Environmental");
            break;
        default:
            return FString();
    }

    // Find matching dialogue
    for (const FNarr_EmergencyDialogue& Dialogue : EmergencyDialogues)
    {
        if (Dialogue.ThreatType == TargetThreatType)
        {
            return Dialogue.DialogueText;
        }
    }

    return FString();
}