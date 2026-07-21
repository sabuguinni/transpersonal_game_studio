#include "Narr_ContextualVoiceSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"

ANarr_ContextualVoiceSystem::ANarr_ContextualVoiceSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    VoiceVolume = 0.8f;
    MinTimeBetweenVoiceLines = 5.0f;
    LastVoiceLineTime = 0.0f;
    bSystemEnabled = true;
}

void ANarr_ContextualVoiceSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultVoiceLines();
    CreateBiomeDiscoveryTriggers();
    CreateSurvivalEventTriggers();
}

void ANarr_ContextualVoiceSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSystemEnabled)
    {
        CheckPlayerProximity();
    }
}

void ANarr_ContextualVoiceSystem::PlayVoiceLine(const FNarr_VoiceLine& VoiceLine)
{
    if (!IsVoiceLineAvailable())
    {
        return;
    }

    // Log the voice line for debugging
    UE_LOG(LogTemp, Log, TEXT("Playing voice line: %s"), *VoiceLine.Text);

    // Update last voice line time
    LastVoiceLineTime = GetWorld()->GetTimeSeconds();

    // In a real implementation, this would load and play the audio file from VoiceLine.AudioURL
    // For now, we'll use the audio component with default settings
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(VoiceVolume);
        // AudioComponent->SetSound(LoadedSoundFromURL); // Would load from VoiceLine.AudioURL
    }
}

void ANarr_ContextualVoiceSystem::TriggerContextualVoice(ENarr_VoiceContextType ContextType, const FVector& PlayerLocation)
{
    if (!bSystemEnabled || !IsVoiceLineAvailable())
    {
        return;
    }

    // Find appropriate contextual trigger
    for (FNarr_ContextualTrigger& Trigger : ContextualTriggers)
    {
        if (!Trigger.bIsActive || Trigger.CurrentTriggerCount >= Trigger.MaxTriggers)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Trigger.Location);
        if (Distance <= Trigger.TriggerRadius)
        {
            // Find voice line matching context type
            TArray<FNarr_VoiceLine> MatchingVoiceLines;
            for (const FNarr_VoiceLine& VoiceLine : Trigger.VoiceLines)
            {
                if (VoiceLine.ContextType == ContextType)
                {
                    MatchingVoiceLines.Add(VoiceLine);
                }
            }

            if (MatchingVoiceLines.Num() > 0)
            {
                FNarr_VoiceLine SelectedVoiceLine = SelectBestVoiceLine(MatchingVoiceLines, ContextType);
                PlayVoiceLine(SelectedVoiceLine);
                
                Trigger.CurrentTriggerCount++;
                if (!SelectedVoiceLine.bIsRepeatable && Trigger.CurrentTriggerCount >= Trigger.MaxTriggers)
                {
                    Trigger.bIsActive = false;
                }
                break;
            }
        }
    }
}

void ANarr_ContextualVoiceSystem::AddContextualTrigger(const FNarr_ContextualTrigger& NewTrigger)
{
    ContextualTriggers.Add(NewTrigger);
}

void ANarr_ContextualVoiceSystem::SetSystemEnabled(bool bEnabled)
{
    bSystemEnabled = bEnabled;
}

bool ANarr_ContextualVoiceSystem::IsVoiceLineAvailable() const
{
    if (!bSystemEnabled)
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastVoiceLineTime) >= MinTimeBetweenVoiceLines;
}

void ANarr_ContextualVoiceSystem::InitializeDefaultVoiceLines()
{
    // Create default voice lines for different contexts
    
    // Discovery voice lines
    FNarr_VoiceLine DiscoveryLine1;
    DiscoveryLine1.Text = TEXT("Fascinating! This area shows signs of significant dinosaur activity.");
    DiscoveryLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/discovery_1.mp3");
    DiscoveryLine1.ContextType = ENarr_VoiceContextType::Discovery;
    DiscoveryLine1.Priority = 2.0f;
    DiscoveryLine1.bIsRepeatable = false;

    // Warning voice lines
    FNarr_VoiceLine WarningLine1;
    WarningLine1.Text = TEXT("Caution advised - predator tracks detected in this vicinity.");
    WarningLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/warning_1.mp3");
    WarningLine1.ContextType = ENarr_VoiceContextType::Warning;
    WarningLine1.Priority = 3.0f;
    WarningLine1.bIsRepeatable = true;

    // Emergency voice lines
    FNarr_VoiceLine EmergencyLine1;
    EmergencyLine1.Text = TEXT("Immediate evacuation recommended - large theropod approaching!");
    EmergencyLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/emergency_1.mp3");
    EmergencyLine1.ContextType = ENarr_VoiceContextType::Emergency;
    EmergencyLine1.Priority = 5.0f;
    EmergencyLine1.bIsRepeatable = true;

    // Store these as part of default triggers (will be added in CreateBiomeDiscoveryTriggers)
}

void ANarr_ContextualVoiceSystem::CheckPlayerProximity()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check all contextual triggers for proximity
    for (FNarr_ContextualTrigger& Trigger : ContextualTriggers)
    {
        if (!Trigger.bIsActive || Trigger.CurrentTriggerCount >= Trigger.MaxTriggers)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Trigger.Location);
        if (Distance <= Trigger.TriggerRadius && Trigger.VoiceLines.Num() > 0)
        {
            // Trigger the first available voice line
            FNarr_VoiceLine SelectedVoiceLine = Trigger.VoiceLines[0];
            TriggerContextualVoice(SelectedVoiceLine.ContextType, PlayerLocation);
        }
    }
}

FNarr_VoiceLine ANarr_ContextualVoiceSystem::SelectBestVoiceLine(const TArray<FNarr_VoiceLine>& VoiceLines, ENarr_VoiceContextType ContextType) const
{
    if (VoiceLines.Num() == 0)
    {
        return FNarr_VoiceLine();
    }

    // Select voice line with highest priority
    FNarr_VoiceLine BestVoiceLine = VoiceLines[0];
    for (const FNarr_VoiceLine& VoiceLine : VoiceLines)
    {
        if (VoiceLine.Priority > BestVoiceLine.Priority)
        {
            BestVoiceLine = VoiceLine;
        }
    }

    return BestVoiceLine;
}

void ANarr_ContextualVoiceSystem::CreateBiomeDiscoveryTriggers()
{
    // Pantano discovery trigger
    FNarr_ContextualTrigger PantanoTrigger;
    PantanoTrigger.Location = FVector(-50000, -45000, 100);
    PantanoTrigger.TriggerRadius = 2000.0f;
    PantanoTrigger.MaxTriggers = 1;
    
    FNarr_VoiceLine PantanoDiscovery;
    PantanoDiscovery.Text = TEXT("Entering wetland biome. High humidity detected. Expect amphibious species and dense vegetation.");
    PantanoDiscovery.ContextType = ENarr_VoiceContextType::Discovery;
    PantanoDiscovery.Priority = 2.0f;
    PantanoTrigger.VoiceLines.Add(PantanoDiscovery);
    
    AddContextualTrigger(PantanoTrigger);

    // Floresta discovery trigger
    FNarr_ContextualTrigger FlorestaTrigger;
    FlorestaTrigger.Location = FVector(-45000, 40000, 100);
    FlorestaTrigger.TriggerRadius = 2000.0f;
    FlorestaTrigger.MaxTriggers = 1;
    
    FNarr_VoiceLine FlorestaDiscovery;
    FlorestaDiscovery.Text = TEXT("Dense forest canopy detected. Limited visibility - proceed with caution. Herbivore feeding grounds likely.");
    FlorestaDiscovery.ContextType = ENarr_VoiceContextType::Discovery;
    FlorestaDiscovery.Priority = 2.0f;
    FlorestaTrigger.VoiceLines.Add(FlorestaDiscovery);
    
    AddContextualTrigger(FlorestaTrigger);

    // Deserto discovery trigger
    FNarr_ContextualTrigger DesertoTrigger;
    DesertoTrigger.Location = FVector(55000, 0, 100);
    DesertoTrigger.TriggerRadius = 2000.0f;
    DesertoTrigger.MaxTriggers = 1;
    
    FNarr_VoiceLine DesertoDiscovery;
    DesertoDiscovery.Text = TEXT("Arid environment confirmed. Water sources scarce. Monitor hydration levels closely.");
    DesertoDiscovery.ContextType = ENarr_VoiceContextType::Discovery;
    DesertoDiscovery.Priority = 2.0f;
    DesertoTrigger.VoiceLines.Add(DesertoDiscovery);
    
    AddContextualTrigger(DesertoTrigger);

    // Montanha discovery trigger
    FNarr_ContextualTrigger MontanhaTrigger;
    MontanhaTrigger.Location = FVector(40000, 50000, 600);
    MontanhaTrigger.TriggerRadius = 2000.0f;
    MontanhaTrigger.MaxTriggers = 1;
    
    FNarr_VoiceLine MontanhaDiscovery;
    MontanhaDiscovery.Text = TEXT("High altitude terrain reached. Temperature dropping. Potential shelter locations identified.");
    MontanhaDiscovery.ContextType = ENarr_VoiceContextType::Discovery;
    MontanhaDiscovery.Priority = 2.0f;
    MontanhaTrigger.VoiceLines.Add(MontanhaDiscovery);
    
    AddContextualTrigger(MontanhaTrigger);
}

void ANarr_ContextualVoiceSystem::CreateSurvivalEventTriggers()
{
    // First dinosaur sighting trigger (Savana center)
    FNarr_ContextualTrigger DinosaurSightingTrigger;
    DinosaurSightingTrigger.Location = FVector(2000, 3000, 100);
    DinosaurSightingTrigger.TriggerRadius = 1000.0f;
    DinosaurSightingTrigger.MaxTriggers = 1;
    
    FNarr_VoiceLine FirstSighting;
    FirstSighting.Text = TEXT("Remarkable! First direct dinosaur observation confirmed. Maintain safe distance and document behavior.");
    FirstSighting.ContextType = ENarr_VoiceContextType::Achievement;
    FirstSighting.Priority = 3.0f;
    DinosaurSightingTrigger.VoiceLines.Add(FirstSighting);
    
    AddContextualTrigger(DinosaurSightingTrigger);

    // Danger zone warning trigger
    FNarr_ContextualTrigger DangerTrigger;
    DangerTrigger.Location = FVector(5000, -2000, 100);
    DangerTrigger.TriggerRadius = 800.0f;
    DangerTrigger.MaxTriggers = 3;
    
    FNarr_VoiceLine DangerWarning;
    DangerWarning.Text = TEXT("Warning: Elevated predator activity in this area. Consider alternative route.");
    DangerWarning.ContextType = ENarr_VoiceContextType::Warning;
    DangerWarning.Priority = 4.0f;
    DangerWarning.bIsRepeatable = true;
    DangerTrigger.VoiceLines.Add(DangerWarning);
    
    AddContextualTrigger(DangerTrigger);

    // Safe zone trigger
    FNarr_ContextualTrigger SafeTrigger;
    SafeTrigger.Location = FVector(-3000, -1000, 100);
    SafeTrigger.TriggerRadius = 600.0f;
    SafeTrigger.MaxTriggers = 1;
    
    FNarr_VoiceLine SafeZone;
    SafeZone.Text = TEXT("Relatively secure area identified. Good location for temporary camp establishment.");
    SafeZone.ContextType = ENarr_VoiceContextType::Instruction;
    SafeZone.Priority = 2.0f;
    SafeTrigger.VoiceLines.Add(SafeZone);
    
    AddContextualTrigger(SafeTrigger);
}